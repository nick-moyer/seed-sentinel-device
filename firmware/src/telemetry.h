#pragma once

#include <Arduino.h>

void sendTelemetry(const String &deviceMac, int dryValue, int wetValue, const char *serverUrl, int SOIL_SENSOR_PIN);
