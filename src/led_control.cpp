#include "led_control.h"
#include "config.h"

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);
int colorIndex = 0;

void initLEDs() {
    strip.begin();
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 255)); // Initialize to White
    }
    strip.show();
}

void changeColor() {
    colorIndex = (colorIndex + 1) % 5;

    uint32_t color;
    switch (colorIndex) {
        case 0: color = strip.Color(255, 0, 0, 0); break;  // Red
        case 1: color = strip.Color(0, 255, 0, 0); break;  // Green
        case 2: color = strip.Color(0, 0, 255, 0); break;  // Blue
        case 3: color = strip.Color(0, 0, 0, 255); break;  // White
        case 4: color = strip.Color(0, 0, 0, 0); break;    // Off
    }

    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

void setColorFromHex(const std::string &hex) {
    if (hex.length() != 4) {
        for (int i = 0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
        }
        strip.show();
        return;
    }

    int r = static_cast<uint8_t>(hex[0]);
    int g = static_cast<uint8_t>(hex[1]);
    int b = static_cast<uint8_t>(hex[2]);
    int w = static_cast<uint8_t>(hex[3]);

    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b, w));
    }
    strip.show();
}
