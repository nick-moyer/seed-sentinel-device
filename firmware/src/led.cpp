#include "led.h"

extern int LED_PIN;
extern unsigned long lastFlashTime;
extern bool ledState;
extern int flashInterval;

void setLed(int state)
{
    digitalWrite(LED_PIN, state);
}

void flashLed(int times, int delayMs)
{
    for (int i = 0; i < times; i++)
    {
        setLed(HIGH);
        delay(delayMs);
        setLed(LOW);
        delay(delayMs);
    }
}

void updateLedFlashing()
{
    if (flashInterval > 0 && millis() - lastFlashTime >= (unsigned long)flashInterval)
    {
        ledState = !ledState;
        setLed(ledState ? HIGH : LOW);
        lastFlashTime = millis();
    }
}

void setFlashingForState(CalibState state)
{
    switch (state)
    {
    case WAIT_DRY:
        flashInterval = 100; // Very fast flash
        break;
    case WAIT_WET:
        flashInterval = 250; // Fast flash
        break;
    case WAIT_SEND:
        flashInterval = 500; // Slow flash
        break;
    default:
        flashInterval = 0;
        setLed(LOW);
        break;
    }
}