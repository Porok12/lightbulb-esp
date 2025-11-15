#include "config.h"
#include "led_control.h"
#include "ble_server.h"
#include "button_handler.h"
#include <Arduino.h>

void setup()
{
  delay(1000);
  Serial.begin(115200);
  //while (!Serial) { delay(10); }
  Serial.println("Initializing...");

  setupButton();
  initLEDs();
  // initBLE();/

  pinMode(8, OUTPUT); // LED_BUILTIN
  pinMode(BAT_PIN, INPUT);

  Serial.println("Starting up...");
}

void loop()
{
  if (buttonPressed)
  {
    Serial.println("🔘 Button Pressed! Switching Colors...");
    buttonPressed = false;
    switchToNextColor();
  }

  static unsigned long lastBat = 0;
  if (millis() - lastBat > 5000) {
    updateBatteryLevelBLE();
    lastBat = millis();
  }

  delay(10);

  // digitalWrite(8, HIGH);
  // delay(1000);
  // digitalWrite(8, LOW);
  // delay(1000);
}
