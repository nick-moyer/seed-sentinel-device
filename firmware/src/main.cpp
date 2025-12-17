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
  int currentState = digitalRead(BUTTON_PIN);

  // --- Calibration Mode Logic ---
  if (calibState == IDLE)
  {
    setFlashingForState(IDLE);

    // Detect long press to start calibration
    if (currentState == LOW && lastButtonState == HIGH)
    {
      buttonPressTime = millis();
      isPressing = true;
    }
    else if (currentState == HIGH && lastButtonState == LOW)
    {
      isPressing = false;
    }
    if (isPressing && millis() - buttonPressTime > 2000)
    {
      Serial.println("Calibration started! Place sensor in AIR and press button.");
      calibState = WAIT_DRY;
      setFlashingForState(WAIT_DRY);
      isPressing = false;
      delay(500);
    }
  }
  else
  {
    setFlashingForState(calibState);

    // Detect long press to cancel calibration
    if (currentState == LOW && lastButtonState == HIGH)
    {
      buttonPressTime = millis();
      isPressing = true;
    }
    else if (currentState == HIGH && lastButtonState == LOW)
    {
      isPressing = false;
    }
    if (isPressing && millis() - buttonPressTime > 2000)
    {
      Serial.println("Calibration cancelled.");
      calibState = IDLE;
      setFlashingForState(IDLE);
      flashLed(10, 50); // Rapid flash for cancellation
      isPressing = false;
      delay(500);
    }
    // Short press to advance calibration steps
    else if (currentState == LOW && lastButtonState == HIGH && !isPressing)
    {
      switch (calibState)
      {
      case WAIT_DRY:
        dryValue = analogRead(SOIL_SENSOR_PIN);
        Serial.print("Dry value set: ");
        Serial.println(dryValue);
        Serial.println("Now place sensor in WATER and press button.");
        calibState = WAIT_WET;
        setFlashingForState(WAIT_WET);
        break;
      case WAIT_WET:
        wetValue = analogRead(SOIL_SENSOR_PIN);
        Serial.print("Wet value set: ");
        Serial.println(wetValue);
        Serial.println("Press button again to send calibration.");
        calibState = WAIT_SEND;
        setFlashingForState(WAIT_SEND);
        break;
      case WAIT_SEND:
        sendCalibration(deviceMac, dryValue, wetValue, SERVER_URL);
        calibState = IDLE;
        setFlashingForState(IDLE);
        Serial.println("Calibration complete!");
        break;
      default:
        break;
      }
    }
    isPressing = false;
  }

  // --- Send soil reading every 30 seconds ---
  if (calibState == IDLE && millis() - lastReadingSent > readingInterval)
  {
    sendTelemetry(deviceMac, dryValue, wetValue, SERVER_URL, SOIL_SENSOR_PIN);
    lastReadingSent = millis();
  }

  lastButtonState = currentState;
  updateLedFlashing();
}
