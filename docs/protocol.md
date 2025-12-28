# BLE Protocol Documentation

## Overview

The device implements a custom BLE (Bluetooth Low Energy) protocol for controlling RGBW LED lights. The protocol uses standard BLE services and characteristics with custom UUIDs for light control.

## Device Information

- **Device Name**: `KulaPrzema`
- **Firmware Version**: Available via Device Information Service

## BLE Services

### 1. Light Control Service
- **UUID**: `12345678-1234-5678-1234-56789abcdef0`
- **Characteristic UUID**: `abcdef01-1234-5678-1234-56789abcdef0`
- **Properties**: READ, WRITE, WRITE_NR (write without response)

### 2. Battery Service (Standard)
- **Service UUID**: `180F` (Standard Battery Service)
- **Characteristic UUID**: `2A19` (Battery Level)
- **Properties**: READ, NOTIFY
- **Value**: Single byte (0-100) representing battery percentage

### 3. Device Information Service (Standard)
- **Service UUID**: `180A` (Standard Device Information Service)
- **Characteristic UUID**: `2A26` (Firmware Version)
- **Properties**: READ
- **Value**: Firmware version string (e.g., "v0.1.0")

## Protocol Commands

All commands are sent to the Light Control Service characteristic. The protocol uses a command-based format where the first byte is the command ID, followed by command-specific data.

### Command Format

```
[Command ID: 1 byte][Data: variable length]
```

### Available Commands

#### CMD_SET_COLOR (0x01)
Set a single RGBW color for all LEDs.

**Format:**
```
[0x01][R: 1 byte][G: 1 byte][B: 1 byte][W: 1 byte]
```

**Total Length**: 5 bytes

**Example:**
- Red: `01 FF 00 00 00`
- Green: `01 00 FF 00 00`
- Blue: `01 00 00 FF 00`
- White: `01 00 00 00 FF`

#### CMD_SET_COLOR_SETS (0x02)
Set multiple color sets that can be cycled through using the button.

**Format:**
```
[0x02][Color Set 1: 4 bytes][Color Set 2: 4 bytes][...]
```

**Total Length**: 1 + (N × 4) bytes, where N is the number of color sets (max 5)

**Color Set Format**: Each color set is 4 bytes: `[R][G][B][W]`

**Example:**
Set 3 colors (Red, Green, Blue):
```
02 FF 00 00 00  00 FF 00 00  00 00 FF 00
```

**Limitations:**
- Maximum 5 color sets
- Total data length must be a multiple of 4 (excluding command byte)
- Minimum length: 5 bytes (1 command + 1 color set)

#### CMD_DISABLE_BLE (0x03)
Disable the BLE server and stop advertising.

**Format:**
```
[0x03]
```

**Total Length**: 1 byte

**Note**: This command stops BLE advertising. The device will need to be restarted to re-enable BLE.

#### CMD_SET_INDIVIDUAL_COLORS (0x04)
Set individual RGBW color for each LED in the strip.

**Format:**
```
[0x04][LED0_R][LED0_G][LED0_B][LED0_W][LED1_R][LED1_G][LED1_B][LED1_W][...]
```

**Total Length**: 1 + (N × 4) bytes, where N is the number of LEDs (max 5)

**Example:**
Set different colors for 3 LEDs (Red, Green, Blue):
```
04 FF 00 00 00  00 FF 00 00  00 00 FF 00
```

**Limitations:**
- Maximum 5 LEDs (device has 5 LEDs)
- Total data length must be a multiple of 4 (excluding command byte)
- Minimum length: 5 bytes (1 command + 1 LED color)
- If fewer LEDs are specified than available, remaining LEDs are not changed

#### CMD_SET_SLEEP_TIMER (0x05)
Set a timer to automatically turn off the LEDs after specified minutes.

**Format:**
```
[0x05][Minutes High Byte][Minutes Low Byte]
```

**Total Length**: 3 bytes

**Parameters:**
- Minutes: 16-bit value (0-65535 minutes)
  - High byte: `(minutes >> 8) & 0xFF`
  - Low byte: `minutes & 0xFF`
  - Value 0 cancels the timer

**Example:**
- Set timer for 30 minutes: `05 00 1E` (0x001E = 30)
- Set timer for 60 minutes: `05 00 3C` (0x003C = 60)
- Cancel timer: `05 00 00`

**Behavior:**
- When timer expires, all LEDs are turned off
- Timer can be cancelled by sending 0 minutes
- Timer is checked every loop iteration

#### CMD_SET_ANIMATION (0x06)
Set animation mode with configurable parameters.

**Format:**
```
[0x06][Animation Type][Speed][Optional Parameters...]
```

**Total Length**: 3+ bytes (minimum 3, up to 11 bytes)

**Parameters:**
- **Animation Type** (1 byte):
  - `0x00`: Disable animation
  - `0x01`: Pulse brightness (single color)
  - `0x02`: Color transition (between two colors)
  - `0x03`: Pulse brightness (using stored color sets)
- **Speed** (1 byte): Update delay in milliseconds (1-255)
  - Lower values = faster animation
  - Recommended: 20-100ms
- **Optional Parameters** (8 bytes, only for types 1 and 2):
  - For type 1: `[R1][G1][B1][W1][0][0][0][0]` - Color to pulse
  - For type 2: `[R1][G1][B1][W1][R2][G2][B2][W2]` - Colors to transition between
  - For type 3: `[MinBrightness][0][0][0][0][0][0][0]` - Minimum brightness (0-255)

**Examples:**

1. **Pulse single color (Red):**
   ```
   06 01 50  FF 00 00 00  00 00 00 00
   ```
   - Type: 1 (pulse)
   - Speed: 80ms
   - Color: Red

2. **Color transition (Red to Blue):**
   ```
   06 02 30  FF 00 00 00  00 00 FF 00
   ```
   - Type: 2 (transition)
   - Speed: 48ms
   - From: Red, To: Blue

3. **Pulse stored colors:**
   ```
   06 03 40  80 00 00 00  00 00 00 00
   ```
   - Type: 3 (pulse stored)
   - Speed: 64ms
   - Min brightness: 128 (50%)

4. **Disable animation:**
   ```
   06 00 00
   ```

**Animation Details:**
- **Type 1 (Pulse)**: Smoothly pulses brightness of a single color using sine wave
- **Type 2 (Transition)**: Smoothly transitions between two colors using sine wave
- **Type 3 (Pulse Stored)**: Pulses brightness of stored color sets, each LED uses different color from stored sets
- Animations use sine wave for smooth transitions
- Animation updates are non-blocking and run in main loop

## Response Handling

- **No Response**: Commands do not return explicit responses. The device executes the command immediately.
- **Error Handling**: Invalid commands or data lengths are logged to Serial but do not cause errors.
- **Validation**: 
  - `CMD_SET_COLOR`: Must be exactly 5 bytes
  - `CMD_SET_COLOR_SETS`: Must be at least 5 bytes and (length - 1) must be divisible by 4
  - `CMD_SET_INDIVIDUAL_COLORS`: Must be at least 5 bytes and (length - 1) must be divisible by 4
  - `CMD_SET_SLEEP_TIMER`: Must be exactly 3 bytes
  - `CMD_SET_ANIMATION`: Must be at least 3 bytes

## Battery Level

The battery level is available via the standard Battery Service:
- Read the characteristic to get current battery level (0-100)
- Subscribe to notifications to receive updates (updated every 5 seconds)

## Connection Flow

1. Scan for device with name "KulaPrzema"
2. Connect to the device
3. Discover services and characteristics
4. Write commands to Light Control Service characteristic
5. Read/Subscribe to Battery Service for battery level updates
6. Read Device Information Service for firmware version

## Example Usage

### Set Single Color (Red)
```
Write to Light Control Characteristic:
[0x01, 0xFF, 0x00, 0x00, 0x00]
```

### Set Multiple Colors
```
Write to Light Control Characteristic:
[0x02, 0xFF, 0x00, 0x00, 0x00,    // Red
       0x00, 0xFF, 0x00, 0x00,    // Green
       0x00, 0x00, 0xFF, 0x00]     // Blue
```

### Disable BLE
```
Write to Light Control Characteristic:
[0x03]
```

### Set Individual LED Colors
```
Write to Light Control Characteristic:
[0x04, 0xFF, 0x00, 0x00, 0x00,    // LED 0: Red
       0x00, 0xFF, 0x00, 0x00,    // LED 1: Green
       0x00, 0x00, 0xFF, 0x00]     // LED 2: Blue
```

### Set Sleep Timer (30 minutes)
```
Write to Light Control Characteristic:
[0x05, 0x00, 0x1E]  // 30 minutes = 0x001E
```

### Set Animation (Pulse Red)
```
Write to Light Control Characteristic:
[0x06, 0x01, 0x50,           // Type: Pulse, Speed: 80ms
       0xFF, 0x00, 0x00, 0x00, // Color: Red
       0x00, 0x00, 0x00, 0x00]  // Padding
```

## Notes

- Color values are 8-bit (0-255) for each channel (R, G, B, W)
- The device stores color sets in non-volatile memory
- Button on device cycles through stored color sets
- Long button press (2+ seconds) triggers wake-up animation
- Animations run continuously in the background until disabled or new command is sent
- Sleep timer can be set while animations are active
- Setting a new color or individual colors disables active animations

