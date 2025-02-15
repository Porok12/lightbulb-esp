#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Adafruit_NeoPixel.h>
#include <string>

void initLEDs();
void changeColor();
void setColorFromHex(const std::string &hex);

#endif
