#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "led.h"
#include "calibration.h"
#include "telemetry.h"

// --- MACROS (Injected from secrets.ini) ---
#ifndef WIFI_SSID
#define WIFI_SSID "DEFAULT"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "DEFAULT"
#endif
#ifndef SERVER_URL
#define SERVER_URL "http://localhost:8080"
#endif

// --- CONFIG ---
const int BUTTON_PIN = 0;
const int SOIL_SENSOR_PIN = 32;

int LED_PIN = 2;
unsigned long lastFlashTime = 0;
bool ledState = false;
int flashInterval = 0;

// --- Initial State ---
CalibState calibState = IDLE;
int dryValue = 0;
int wetValue = 0;

String deviceMac;

// --- Button State ---
int lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
bool isPressing = false;

// --- Debounce Variables ---
const unsigned long debounceDelay = 50; // Debounce delay in milliseconds
unsigned long lastDebounceTime = 0;

// --- Soil Reading Interval ---
unsigned long lastReadingSent = 0;
const unsigned long readingInterval = 30000; // 30 seconds

// --- Setup ---
void setup()
{
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  setLed(LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  deviceMac = WiFi.macAddress();
  Serial.print("My MAC Address: ");
  Serial.println(deviceMac);
}

// --- Main Loop ---
void loop()
{
  unsigned long currentTime = millis();
  int reading = digitalRead(BUTTON_PIN);

  // --- 1. Debounce & Event Detection ---
  bool buttonLongPress = false;
  bool buttonShortPress = false;

  if (reading != lastButtonState)
  {
    lastDebounceTime = currentTime;
  }

  if ((currentTime - lastDebounceTime) > debounceDelay)
  {
    // If state has changed (and is stable)
    if (reading != (isPressing ? LOW : HIGH))
    {

      // Update internal state
      if (reading == LOW)
      {
        // PRESS START
        isPressing = true;
        buttonPressTime = currentTime;
      }
      else
      {
        // PRESS END (Release)
        isPressing = false;
        unsigned long duration = currentTime - buttonPressTime;

        if (duration > 2000)
        {
          buttonLongPress = true;
        }
        else if (duration > 50)
        { // Simple noise filter
          buttonShortPress = true;
        }
      }
    }
  }

  // Update for next loop
  lastButtonState = reading;

  // --- 2. State Machine Logic ---

  // GLOBAL: Cancel or Start Calibration (Long Press)
  if (buttonLongPress)
  {
    if (calibState == IDLE)
    {
      Serial.println("Calibration started! Place sensor in AIR and press button.");
      calibState = WAIT_DRY;
      setFlashingForState(WAIT_DRY);
    }
    else
    {
      Serial.println("Calibration cancelled.");
      calibState = IDLE;
      setFlashingForState(IDLE);
      flashLed(10, 50);
    }
    return; // Stop processing this loop to prevent double actions
  }

  // CALIBRATION: Advance Steps (Short Press)
  if (buttonShortPress && calibState != IDLE)
  {
    switch (calibState)
    {
    case WAIT_DRY:
      dryValue = analogRead(SOIL_SENSOR_PIN);
      Serial.printf("Dry value set: %d\n", dryValue);
      Serial.println("Place in WATER and press button.");
      calibState = WAIT_WET;
      setFlashingForState(WAIT_WET);
      break;

    case WAIT_WET:
      wetValue = analogRead(SOIL_SENSOR_PIN);
      Serial.printf("Wet value set: %d\n", wetValue);
      Serial.println("Press button to send.");
      calibState = WAIT_SEND;
      setFlashingForState(WAIT_SEND);
      break;

    case WAIT_SEND:
      sendCalibration(deviceMac, dryValue, wetValue, SERVER_URL);
      calibState = IDLE;
      setFlashingForState(IDLE);
      Serial.println("Calibration complete!");
      lastReadingSent = currentTime; // Reset reading timer
      break;

    default:
      break;
    }
  }

  // --- 3. Background Tasks ---
  updateLedFlashing();

  // Send telemetry only if IDLE
  if (calibState == IDLE && (currentTime - lastReadingSent > readingInterval))
  {
    sendTelemetry(deviceMac, dryValue, wetValue, SERVER_URL, SOIL_SENSOR_PIN);
    lastReadingSent = currentTime;
  }
}
