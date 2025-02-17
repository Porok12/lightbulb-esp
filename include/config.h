#ifndef CONFIG_H
#define CONFIG_H

// https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf

#define BATTERY_SERVICE_UUID "180F"        // Standard Battery Service
#define BATTERY_CHARACTERISTIC_UUID "2A19" // Battery Level
#define DEVICE_INFO_SERVICE_UUID "180A"    // Standard Device Information Service
#define FIRMWARE_VERSION_UUID "2A26"       // Firmware Version
// Define a unique 128-bit UUID for your BLE service
#define LIGHT_SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"        // Custom Service
#define LIGHT_CHARACTERISTIC_UUID "abcdef01-1234-5678-1234-56789abcdef0" // Light Color

// https://files.seeedstudio.com/wiki/XIAO_WiFi/pin_map-2.png

#define LED_PIN 10                // GPIO pin connected to the NeoPixel
#define NUM_LEDS 5                // Number of LEDs in the strip
#define BUTTON_PIN 2              // GPIO2 - Button connected to GND
#define DEVICE_NAME "KulaPrzema"  // Bluetooth Device Name
#define FIRMWARE_VERSION "v0.1.0" // Firmware Version
#define DEBOUNCE_DELAY 200        // Debounce time in milliseconds
#define RESET_BUTTON_PIN 3        // GPIO3 - Button resetting the device

// BLE Commands
#define CMD_SET_COLOR 0x01      // Set a single RGBW color
#define CMD_SET_COLOR_SETS 0x02 // Set multiple colors
#define CMD_DISABLE_BLE 0x03    // Disable BLE

// Storage namespace for Preferences API
#define STORAGE_NAMESPACE "color_storage"
#define MAX_COLOR_SETS 5 // Max stored color sets

#endif
