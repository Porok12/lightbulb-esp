#include "button_handler.h"
#include "config.h"
#include <Arduino.h>

volatile bool buttonPressed = false;
volatile unsigned long lastPressTime = 0;

void IRAM_ATTR handleButtonPress()
{
    unsigned long currentTime = millis();
    if (currentTime - lastPressTime > DEBOUNCE_DELAY)
    {
        buttonPressed = true;
        lastPressTime = currentTime;
    }
}

void setupButton()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, handleButtonPress, FALLING);
}
