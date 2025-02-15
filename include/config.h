#ifndef CONFIG_H
#define CONFIG_H

#define LED_PIN 10               // GPIO pin connected to the NeoPixel
#define NUM_LEDS 5               // Number of LEDs in the strip
#define BUTTON_PIN 2             // GPIO2 - Button connected to GND
#define DEVICE_NAME "KulaPrzema" // Bluetooth Device Name
#define DEBOUNCE_DELAY 200       // Debounce time in milliseconds

// Define a unique 128-bit UUID for your BLE service
#define BLE_SERVICE_UUID "f3b5e3a6-7fd5-4a0a-832f-63bb1e98e7f5"
#define BLE_CHARACTERISTIC_UUID "a7e82db5-bbef-4f24-b451-37d60c6a30e2"

// BLE Commands
#define CMD_SET_COLOR 0x01      // Set a single RGBW color
#define CMD_SET_COLOR_SETS 0x02 // Set multiple colors
#define CMD_DISABLE_BLE 0x03    // Disable BLE

#endif
