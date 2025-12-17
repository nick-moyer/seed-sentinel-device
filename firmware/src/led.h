#pragma once

#include <Arduino.h>

#include "calibration.h"

void setLed(int state);
void flashLed(int times, int delayMs);
void updateLedFlashing();
void setFlashingForState(CalibState state);