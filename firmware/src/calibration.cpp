#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "calibration.h"
#include "led.h"

void sendCalibration(const String &deviceMac, int dryValue, int wetValue, const char *serverUrl)
{
    setLed(HIGH); // LED on while sending
    bool success = false;
    if (WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP)
    {
        HTTPClient http;
        String url = String(serverUrl) + "/calibrate";
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        String json = "{\"sensor_id\": \"" + deviceMac + "\", \"dry_reference\": " + String(dryValue) + ", \"wet_reference\": " + String(wetValue) + "}";
        Serial.println("Sending Calibration...");
        int response = http.POST(json);
        if (response > 0)
        {
            Serial.printf("Calibration sent! Server says: %d\n", response);
            success = true;
        }
        else
        {
            Serial.printf("Error: %s\n", http.errorToString(response).c_str());
        }
        http.end();
    }
    setLed(LOW); // LED off after sending

    // Indicate result
    if (success)
        flashLed(3, 100); // 3 quick flashes for success
    else
        flashLed(2, 400); // 2 slow flashes for error
}