#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void initLEDs();
void setColorFromBytes(const uint8_t *colorData);
void updateColorSets(const uint8_t *colorData, size_t length);
void switchToNextColor();
void loadStoredColors();

#endif
