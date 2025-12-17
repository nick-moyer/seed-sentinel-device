#pragma once

#include <Arduino.h>

enum CalibState
{
    IDLE,
    WAIT_DRY,
    WAIT_WET,
    WAIT_SEND
};

void sendCalibration(const String &deviceMac, int dryValue, int wetValue, const char *serverUrl);