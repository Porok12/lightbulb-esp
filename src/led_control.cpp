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
    strip.begin();
    loadStoredColors();
    strip.show();
}

void loadStoredColors()
{
    preferences.begin(STORAGE_NAMESPACE, true);
    int length = preferences.getInt("color_size", 0);

    if (length > 0 && length <= MAX_COLOR_SETS * 4)
    {
        preferences.getBytes("colors", storedColors, length);
        storedColorCount = length / 4;
    }
    preferences.end();
}

void saveColorSets(const uint8_t *colorData, size_t length)
{
    preferences.begin(STORAGE_NAMESPACE, false);
    preferences.putBytes("colors", colorData, length);
    preferences.putInt("color_size", length);
    preferences.end();
}

void updateColorSets(const uint8_t *colorData, size_t length)
{
    if (length > MAX_COLOR_SETS * 4)
        return;

    memcpy(storedColors, colorData, length); 
    storedColorCount = length / 4;

    saveColorSets(colorData, length);
}

void switchToNextColor()
{
    if (storedColorCount == 0)
        return;

    colorSetIndex = (colorSetIndex + 1) % storedColorCount;
    setColorFromBytes(storedColors[colorSetIndex]);
}

void turnOffLEDs() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
    }
    strip.show();
}

void switchToNextColorSet() {
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
