#include "button_handler.h"
#include "config.h"
#include "led_control.h"
#include <Arduino.h>
#include <esp_sleep.h>

volatile bool buttonPressed = false;
volatile unsigned long lastPressTime = 0;
volatile bool isShuttingDown = false;

void IRAM_ATTR onButtonPressISR()
{
    buttonPressed = true;
}

void setupButton()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, onButtonPressISR, FALLING);
    
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
}

void goToDeepSleep()
{
    Serial.println("😴 Preparing for deep sleep...");
    delay(100);

    // Disable button interrupt before sleep to prevent false wake-ups from LED animations
    // This is the KEY FIX - without this, LED animations during shutdown can trigger wake-ups
    detachInterrupt(BUTTON_PIN);

    // Turn off all LEDs before sleep to prevent electrical noise from triggering wake-ups
    turnOffLEDs();
    delay(100);

    // Configure GPIO wakeup - wake when BUTTON_PIN goes LOW (button pressed)
    esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);

    // Enter deep sleep mode - the button interrupt will be registered again after wake-up in setup()
    Serial.println("💤 Entering deep sleep...");
    delay(100);
    esp_deep_sleep_start();
}

void handleButtonPress()
{
    // Don't process button input during shutdown sequence
    if (isShuttingDown) {
        return;
    }

    static unsigned long buttonPressStart = 0;
    static bool buttonWasPressed = false;
    static unsigned long lastDebounceTime = 0;
    static unsigned long pendingActionTime = 0;
    static bool hasPendingAction = false;
    
    static unsigned long resetPressStart = 0;
    bool resetPressed = digitalRead(RESET_BUTTON_PIN) == LOW;
    
    if (resetPressed && resetPressStart == 0) {
        resetPressStart = millis();
    } else if (!resetPressed) {
        resetPressStart = 0;
    } else if (resetPressed && (millis() - resetPressStart > 3000)) {
        Serial.println("🔄 Reset button held for 3s - Resetting device...");
        ESP.restart();
    }
    
    bool buttonState = digitalRead(BUTTON_PIN) == LOW;
    unsigned long currentTime = millis();
    
    if (buttonState != buttonWasPressed) {
        lastDebounceTime = currentTime;
    }
    
    if ((currentTime - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (buttonState && buttonPressStart == 0) {
            buttonPressStart = currentTime;
        } else if (!buttonState && buttonPressStart > 0) {
            unsigned long pressDuration = currentTime - buttonPressStart;
            
            if (pressDuration >= MIN_PRESS_DURATION) {

                if (hasPendingAction && (currentTime - pendingActionTime) < CLICK_MERGE_DELAY) {
                    pendingActionTime = currentTime;
                } else {
                    hasPendingAction = true;
                    pendingActionTime = currentTime;
                }
            }
            
            buttonPressStart = 0;
        } else if (buttonState && buttonPressStart > 0) {
            unsigned long pressDuration = currentTime - buttonPressStart;
            
            if (pressDuration >= LONG_PRESS_DELAY) {
                hasPendingAction = false;
                pendingActionTime = 0;
                isShuttingDown = true;

                Serial.println("🔘 Long press detected - Shutting down...");
                buttonPressStart = 0;
                flashAllColorsAnimation(200);
                Serial.println("💤 Starting deep sleep sequence...");
                delay(500);
                goToDeepSleep();
            }
        }
    }
    
    if (hasPendingAction && (currentTime - pendingActionTime) >= CLICK_MERGE_DELAY) {
        Serial.println("🔘 Button clicked - Switching Colors...");
        switchToNextColor();
        hasPendingAction = false;
        pendingActionTime = 0;
    }
    
    buttonWasPressed = buttonState;
    buttonPressed = false;
}
