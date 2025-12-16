#include "calibration.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

void sendCalibration(const String &deviceMac, int dryValue, int wetValue, const char *serverUrl)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String url = String(SERVER_URL) + "/calibrate";
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        String json = "{\"id\": \"" + deviceMac + "\", \"dry\": " + String(dryValue) + ", \"wet\": " + String(wetValue) + "}";
        Serial.println("Sending Calibration...");
        int response = http.POST(json);
        if (response > 0)
        {
            Serial.printf("Calibration sent! Server says: %d\n", response);
        }
        else
        {
            Serial.printf("Error: %s\n", http.errorToString(response).c_str());
        }
        http.end();
    }
}