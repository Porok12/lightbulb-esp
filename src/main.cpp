#include "config.h"
#include "led_control.h"
#include "ble_server.h"
#include "button_handler.h"
#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  setupButton();
  initLEDs();
  initBLE();
}

void loop()
{
  if (buttonPressed)
  {
    buttonPressed = false;
    switchToNextColor();
  }
}
