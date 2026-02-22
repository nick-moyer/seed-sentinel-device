#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "led.h"
#include "calibration.h"
#include "telemetry.h"

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

// --- WiFi & Config State ---
Preferences preferences;
WebServer server(80);
DNSServer dnsServer;
bool isProvisioning = false;
String configUrl; // Holds the dynamic server URL

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

// --- Provisioning HTML ---
const char *htmlForm = R"rawliteral(
<!DOCTYPE HTML><html>
<head><title>Seed Sentinel Setup</title><meta name="viewport" content="width=device-width, initial-scale=1"></head>
<body><h2>Seed Sentinel Setup</h2>
<form action="/save" method="POST">
  <label>WiFi SSID:</label><br><input type="text" name="ssid"><br>
  <label>WiFi Password:</label><br><input type="password" name="pass"><br>
  <label>Server URL (e.g. http://192.168.0.100:8080):</label><br><input type="text" name="url" value="http://192.168.0.100:8080"><br><br>
  <input type="submit" value="Save & Connect">
</form></body></html>)rawliteral";

void startProvisioning()
{
  isProvisioning = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Seed_Sentinel_Setup");
  dnsServer.start(53, "*", WiFi.softAPIP());
  flashInterval = 100; // Rapid flash for AP mode

  server.on("/", []() { server.send(200, "text/html", htmlForm); });
  server.on("/save", []() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    String url = server.arg("url");
    if (ssid.length() > 0)
    {
      preferences.begin("sentinel", false);
      preferences.putString("ssid", ssid);
      preferences.putString("pass", pass);
      preferences.putString("url", url);
      preferences.end();
      server.send(200, "text/html", "Saved! Restarting...");
      delay(1000);
      ESP.restart();
    }
    else
    {
      server.send(200, "text/html", "Error: SSID missing. <a href='/'>Back</a>");
    }
  });
  server.begin();
  Serial.println("Provisioning Mode Started. Connect to 'Seed_Sentinel_Setup'");
}

// --- Setup ---
void setup()
{
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  setLed(LOW);

  // Load Settings
  preferences.begin("sentinel", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  configUrl = preferences.getString("url", "");
  preferences.end();

  if (ssid == "")
  {
    startProvisioning();
  }
  else
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.print("Connecting to WiFi");
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20)
    {
      delay(500);
      Serial.print(".");
      retries++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      deviceMac = WiFi.macAddress();
    }
    else
    {
      Serial.println("\nFailed to connect. Starting Provisioning Mode.");
      startProvisioning();
    }
  }
}

// --- Main Loop ---
void loop()
{
  if (isProvisioning)
  {
    dnsServer.processNextRequest();
    server.handleClient();
    updateLedFlashing();
    return;
  }

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
      sendCalibration(deviceMac, dryValue, wetValue, configUrl.c_str());

      // Save calibration to persistent storage
      preferences.begin("sentinel", false);
      preferences.putInt("dry", dryValue);
      preferences.putInt("wet", wetValue);
      preferences.end();

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
    sendTelemetry(deviceMac, dryValue, wetValue, configUrl.c_str(), SOIL_SENSOR_PIN);
    lastReadingSent = currentTime;
  }
}
