#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void initLEDs();
void setColorFromBytes(const uint8_t *colorData);
void updateColorSets(const uint8_t *colorData, size_t length);
void switchToNextColor();
void loadStoredColors();
void turnOffLEDs();
void flashAllColorsAnimation(unsigned int delayMs);
void setIndividualLEDColors(const uint8_t *colorData, size_t numLEDs);
void setSleepTimer(uint16_t minutes);
void setAnimation(uint8_t animationType, uint8_t speed, uint8_t *params, size_t paramsLength);
void updateAnimation();
bool checkSleepTimer();

#endif
