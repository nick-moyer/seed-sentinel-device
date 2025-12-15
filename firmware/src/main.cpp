#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

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
const int BUTTON_PIN = 0;          // The "BOOT" button on most ESP32 boards
const char *PLANT_NAME = "Tomato"; // Hardcoded for now

// Variables for button debouncing
int lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
bool isPressing = false;

String deviceMac; // <-- Add this global variable

void setup()
{
  Serial.begin(115200);

  // 1. Setup Button (Internal Pullup is needed for GPIO 0)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 2. Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  deviceMac = WiFi.macAddress(); // <-- Cache MAC address after connection
  Serial.print("My MAC Address: ");
  Serial.println(deviceMac);
}

// Helper: Send Config Payload
void sendConfig()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = String(SERVER_URL) + "/config";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // JSON Payload: { "mac_address": "AA:BB:CC...", "plant_name": "Tomato" }
    String json = "{\"mac_address\": \"" + deviceMac + "\", \"plant_name\": \"" + String(PLANT_NAME) + "\"}";

    Serial.println("Sending Config...");
    int response = http.POST(json);

    if (response > 0)
    {
      Serial.printf("Registered! Server says: %d\n", response);
    }
    else
    {
      Serial.printf("Error: %s\n", http.errorToString(response).c_str());
    }
    http.end();
  }
}

void loop()
{
  // --- 1. BUTTON LOGIC (Long Press Detection) ---
  int currentState = digitalRead(BUTTON_PIN);

  if (currentState == LOW && lastButtonState == HIGH)
  {
    // Button just pressed
    buttonPressTime = millis();
    isPressing = true;
    Serial.println("🔘 Button Pressed...");
  }
  else if (currentState == HIGH && lastButtonState == LOW)
  {
    // Button released
    isPressing = false;
  }

  // If holding for > 2 seconds, trigger action
  if (isPressing && (millis() - buttonPressTime > 2000))
  {
    Serial.println("Triggering Configuration!");
    sendConfig();
    isPressing = false; // Prevent double trigger
    delay(1000);        // Debounce delay
  }

  lastButtonState = currentState;
}