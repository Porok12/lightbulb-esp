#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

void setupButton();
void handleButtonPress();

extern volatile bool buttonPressed;

#endif
