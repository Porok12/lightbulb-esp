#include "led_control.h"
#include "config.h"
#include <Preferences.h>

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);
Preferences preferences;

uint8_t storedColors[MAX_COLOR_SETS][4];
int storedColorCount = 0;
int colorSetIndex = 0;

void initLEDs()
{
    strip.setPin(LED_PIN);
    strip.begin();
    loadStoredColors();
    strip.show();
}

void loadStoredColors()
{
    Serial.println("Loading stored color sets from storage...");

    preferences.begin(STORAGE_NAMESPACE, true);
    int length = preferences.getInt("color_size", 0);

    if (length > 0 && length <= MAX_COLOR_SETS * 4)
    {
        preferences.getBytes("colors", storedColors, length);
        storedColorCount = length / 4;
    } else {
        storedColorCount = 4;

        storedColors[0][0] = 255; storedColors[0][1] = 0;   storedColors[0][2] = 0;   storedColors[0][3] = 0;     // Red
        storedColors[1][0] = 0;   storedColors[1][1] = 255; storedColors[1][2] = 0;   storedColors[1][3] = 0;     // Green
        storedColors[2][0] = 0;   storedColors[2][1] = 0;   storedColors[2][2] = 255; storedColors[2][3] = 0;     // Blue
        storedColors[3][0] = 0;   storedColors[3][1] = 0;   storedColors[3][2] = 0;   storedColors[3][3] = 255;   // White
    }
    preferences.end();
}

void saveColorSets(const uint8_t *colorData, size_t length)
{
    Serial.println("Saving color sets to storage...");
    preferences.begin(STORAGE_NAMESPACE, false);
    preferences.putBytes("colors", colorData, length);
    preferences.putInt("color_size", length);
    preferences.end();
}

void updateColorSets(const uint8_t *colorData, size_t length)
{
    Serial.println("Updating stored color sets...");

    if (length > MAX_COLOR_SETS * 4)
        return;

    memcpy(storedColors, colorData, length); 
    storedColorCount = length / 4;

    saveColorSets(colorData, length);
}

void turnOffLEDs() {
    Serial.println("Turning off LEDs.");

    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
    }
    strip.show();
}

void switchToNextColor() {
    Serial.println("Switching to next color set...");

    if (storedColorCount == 0) return;

    if (colorSetIndex < storedColorCount) {
        setColorFromBytes(storedColors[colorSetIndex]);
        colorSetIndex++;
    } else {
        turnOffLEDs();
        colorSetIndex = 0;
    }
}


void setColorFromBytes(const uint8_t *colorData)
{
    Serial.println("Setting LED color...");

    uint8_t r = colorData[0];
    uint8_t g = colorData[1];
    uint8_t b = colorData[2];
    uint8_t w = colorData[3];

    for (int i = 0; i < NUM_LEDS; i++)
    {
        strip.setPixelColor(i, strip.Color(r, g, b, w));
    }
    strip.show();
}
