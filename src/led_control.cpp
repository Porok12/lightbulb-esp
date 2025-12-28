#include "led_control.h"
#include "config.h"
#include <Preferences.h>
#include <math.h>

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);
Preferences preferences;

uint8_t storedColors[MAX_COLOR_SETS][4];
int storedColorCount = 0;
int colorSetIndex = 0;

// Animation state
uint8_t animationType = 0;
uint8_t animationSpeed = 50;
unsigned long animationLastUpdate = 0;
uint32_t animationStep = 0;
uint8_t animationColors[2][4] = {{255, 0, 0, 0}, {0, 0, 255, 0}};
uint8_t animationParams[8] = {0};

// Sleep timer state
unsigned long sleepTimerStart = 0;
uint16_t sleepTimerMinutes = 0;
bool sleepTimerActive = false;

static void initDefaultColors()
{
    storedColorCount = 4;
    storedColors[0][0] = 255; storedColors[0][1] = 0;   storedColors[0][2] = 0;   storedColors[0][3] = 0;
    storedColors[1][0] = 0;   storedColors[1][1] = 255; storedColors[1][2] = 0;   storedColors[1][3] = 0;
    storedColors[2][0] = 0;   storedColors[2][1] = 0;   storedColors[2][2] = 255; storedColors[2][3] = 0;
    storedColors[3][0] = 0;   storedColors[3][1] = 0;   storedColors[3][2] = 0;   storedColors[3][3] = 255;
}

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

    if (!preferences.begin(STORAGE_NAMESPACE, true)) {
        Serial.println("⚠️ Failed to open preferences for reading, using defaults");
        initDefaultColors();
        return;
    }

    int length = preferences.getInt("color_size", 0);

    if (length > 0 && length <= MAX_COLOR_SETS * 4 && (length % 4 == 0))
    {
        size_t bytesRead = preferences.getBytes("colors", storedColors, length);
        if (bytesRead == length) {
            storedColorCount = length / 4;
            Serial.print("✅ Loaded ");
            Serial.print(storedColorCount);
            Serial.println(" color sets from storage");
        } else {
            Serial.println("⚠️ Failed to read color data, using defaults");
            initDefaultColors();
        }
    } else {
        Serial.println("⚠️ Invalid color data length, using defaults");
        initDefaultColors();
    }
    preferences.end();
}

void saveColorSets(const uint8_t *colorData, size_t length)
{
    if (colorData == nullptr || length == 0 || length > MAX_COLOR_SETS * 4) {
        Serial.println("❌ Invalid color data for saving");
        return;
    }

    Serial.println("Saving color sets to storage...");
    if (!preferences.begin(STORAGE_NAMESPACE, false)) {
        Serial.println("❌ Failed to open preferences for writing");
        return;
    }
    
    if (preferences.putBytes("colors", colorData, length) == length) {
        preferences.putInt("color_size", length);
        Serial.println("✅ Color sets saved successfully");
    } else {
        Serial.println("❌ Failed to save color sets");
    }
    preferences.end();
}

void updateColorSets(const uint8_t *colorData, size_t length)
{
    Serial.println("Updating stored color sets...");

    if (colorData == nullptr || length == 0 || length > MAX_COLOR_SETS * 4 || (length % 4 != 0)) {
        Serial.println("❌ Invalid color data length");
        return;
    }

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
    if (colorData == nullptr) {
        Serial.println("❌ Invalid color data pointer");
        return;
    }

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

void flashAllColorsAnimation(unsigned int delayMs)
{
    Serial.println("✨ Flashing all colors animation...");
    
    if (storedColorCount == 0) {
        Serial.println("⚠️ No colors to flash");
        return;
    }
    
    for (int cycle = 0; cycle < 2; cycle++) {
        for (int i = 0; i < storedColorCount; i++) {
            setColorFromBytes(storedColors[i]);
            delay(delayMs);
            turnOffLEDs();
            delay(delayMs / 2);
        }
    }
    
    if (storedColorCount > 0) {
        setColorFromBytes(storedColors[0]);
        colorSetIndex = 1;
    }
}

void setIndividualLEDColors(const uint8_t *colorData, size_t numLEDs)
{
    if (colorData == nullptr || numLEDs == 0) {
        Serial.println("❌ Invalid color data for individual LEDs");
        return;
    }

    Serial.println("Setting individual LED colors...");
    
    size_t maxLEDs = (numLEDs < NUM_LEDS) ? numLEDs : NUM_LEDS;
    
    for (size_t i = 0; i < maxLEDs; i++) {
        uint8_t r = colorData[i * 4 + 0];
        uint8_t g = colorData[i * 4 + 1];
        uint8_t b = colorData[i * 4 + 2];
        uint8_t w = colorData[i * 4 + 3];
        strip.setPixelColor(i, strip.Color(r, g, b, w));
    }
    
    strip.show();
    animationType = 0;
}

void setSleepTimer(uint16_t minutes)
{
    if (minutes == 0) {
        sleepTimerActive = false;
        sleepTimerMinutes = 0;
        Serial.println("⏰ Sleep timer cancelled");
        return;
    }
    
    sleepTimerStart = millis();
    sleepTimerMinutes = minutes;
    sleepTimerActive = true;
    Serial.print("⏰ Sleep timer set for ");
    Serial.print(minutes);
    Serial.println(" minutes");
}

bool checkSleepTimer()
{
    if (!sleepTimerActive) {
        return false;
    }
    
    unsigned long elapsedMinutes = (millis() - sleepTimerStart) / 60000;
    
    if (elapsedMinutes >= sleepTimerMinutes) {
        sleepTimerActive = false;
        Serial.println("⏰ Sleep timer expired - shutting down");
        return true;
    }
    
    return false;
}

void setAnimation(uint8_t animType, uint8_t speed, uint8_t *params, size_t paramsLength)
{
    animationType = animType;
    animationSpeed = speed;
    animationStep = 0;
    animationLastUpdate = millis();
    
    if (params != nullptr && paramsLength > 0) {
        size_t copyLen = (paramsLength < 8) ? paramsLength : 8;
        memcpy(animationParams, params, copyLen);
        
        if (paramsLength >= 8) {
            animationColors[0][0] = params[0];
            animationColors[0][1] = params[1];
            animationColors[0][2] = params[2];
            animationColors[0][3] = params[3];
            animationColors[1][0] = params[4];
            animationColors[1][1] = params[5];
            animationColors[1][2] = params[6];
            animationColors[1][3] = params[7];
        }
    }
    
    Serial.print("🎬 Animation set: type=");
    Serial.print(animType);
    Serial.print(", speed=");
    Serial.println(speed);
}

void updateAnimation()
{
    if (animationType == 0) {
        return;
    }
    
    unsigned long currentTime = millis();
    if (currentTime - animationLastUpdate < animationSpeed) {
        return;
    }
    animationLastUpdate = currentTime;
    
    switch (animationType) {
        case 1: {
            float brightness = (sin(animationStep * 0.05) + 1.0) / 2.0;
            uint8_t r = (uint8_t)(animationColors[0][0] * brightness);
            uint8_t g = (uint8_t)(animationColors[0][1] * brightness);
            uint8_t b = (uint8_t)(animationColors[0][2] * brightness);
            uint8_t w = (uint8_t)(animationColors[0][3] * brightness);
            
            for (int i = 0; i < NUM_LEDS; i++) {
                strip.setPixelColor(i, strip.Color(r, g, b, w));
            }
            strip.show();
            animationStep++;
            break;
        }
        
        case 2: {
            float progress = (sin(animationStep * 0.05) + 1.0) / 2.0;
            uint8_t r = (uint8_t)(animationColors[0][0] * (1.0 - progress) + animationColors[1][0] * progress);
            uint8_t g = (uint8_t)(animationColors[0][1] * (1.0 - progress) + animationColors[1][1] * progress);
            uint8_t b = (uint8_t)(animationColors[0][2] * (1.0 - progress) + animationColors[1][2] * progress);
            uint8_t w = (uint8_t)(animationColors[0][3] * (1.0 - progress) + animationColors[1][3] * progress);
            
            for (int i = 0; i < NUM_LEDS; i++) {
                strip.setPixelColor(i, strip.Color(r, g, b, w));
            }
            strip.show();
            animationStep++;
            break;
        }
        
        case 3: {
            float brightness = (sin(animationStep * 0.05) + 1.0) / 2.0;
            float minBrightness = animationParams[0] / 255.0;
            brightness = minBrightness + (brightness * (1.0 - minBrightness));
            
            for (int i = 0; i < NUM_LEDS; i++) {
                uint8_t r = (uint8_t)(storedColors[i % storedColorCount][0] * brightness);
                uint8_t g = (uint8_t)(storedColors[i % storedColorCount][1] * brightness);
                uint8_t b = (uint8_t)(storedColors[i % storedColorCount][2] * brightness);
                uint8_t w = (uint8_t)(storedColors[i % storedColorCount][3] * brightness);
                strip.setPixelColor(i, strip.Color(r, g, b, w));
            }
            strip.show();
            animationStep++;
            break;
        }
        
        default:
            animationType = 0;
            break;
    }
}
