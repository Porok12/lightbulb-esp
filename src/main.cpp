#include "config.h"
#include "led_control.h"
#include "ble_server.h"
#include "button_handler.h"
#include <Arduino.h>
#include <esp_sleep.h>

void setup()
{
  delay(100);
  Serial.begin(115200);
  Serial.println("Initializing...");

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("🌙 Woke up from deep sleep via button");

    pinMode(BAT_PIN, INPUT);
    setupButton();
    initLEDs();
    flashAllColorsAnimation(200);
    initBLE();
    Serial.println("✅ ESP awakened and ready");
    return;
  }

  pinMode(BAT_PIN, INPUT);
  setupButton();
  initLEDs();
  initBLE();

  Serial.println("Starting up...");
}

void loop()
{
  handleButtonPress();
  updateAnimation();

  if (checkSleepTimer()) {
    goToDeepSleep();
  }

  ensureBLEAdvertising();

  static unsigned long lastBat = 0;
  if (millis() - lastBat > 5000) {
    updateBatteryLevelBLE();
    lastBat = millis();
  }
}
