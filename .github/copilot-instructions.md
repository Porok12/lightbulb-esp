<!-- Copilot / AI agent instructions for lightbulb-esp -->
# Quick instructions for AI coding agents

This project is an Arduino/PlatformIO firmware for an ESP32-C3 based RGBW NeoPixel "lightbulb" with a custom BLE control protocol. Keep changes minimal and preserve embedded-device constraints (memory, blocking calls, and deep-sleep flow).

- **Big picture:** firmware runs in `src/main.cpp`. It initializes LEDs, button handling, and a NimBLE-based BLE server. LED control, animations, and persistent color storage live in `src/led_control.cpp`. BLE command parsing and battery reporting are in `src/ble_server.cpp`. Button press handling and deep-sleep sequencing are in `src/button_handler.cpp`. Protocol details are documented in `docs/protocol.md`.

- **Build / flash / monitor:** uses PlatformIO. Default environment: `esp32-c3-devkitm-1` (see `platformio.ini`). Typical commands:

  - Build: `pio run`
  - Build + upload: `pio run -e esp32-c3-devkitm-1 -t upload`
  - Serial monitor: `pio device monitor -e esp32-c3-devkitm-1 --baud 115200`

- **Runtime notes / debugging:** Serial output is used extensively at `115200` baud. Look at `Serial.print` messages in `src/*.cpp` to trace flows (BLE connect/disconnect, command parsing errors, storage reads/writes, sleep transitions).

- **Key patterns to follow / preserve**
  - Non-blocking animation and timing: `updateAnimation()` is called from `loop()` rather than long blocking delays — preserve this when changing animation logic.
  - BLE command handling: single-byte command ID followed by payload handled in `LightCharacteristicCallbacks::onWrite` (`src/ble_server.cpp`). Validate lengths exactly as current code does (e.g., `CMD_SET_COLOR` == 5 bytes).
  - Persistent color sets: stored via `Preferences` in `src/led_control.cpp`. Data layout: consecutive 4-byte color entries (R,G,B,W); max sets defined by `MAX_COLOR_SETS` in headers.
  - Deep-sleep / button sequence: `goToDeepSleep()` detaches the button interrupt and turns off LEDs before entering deep sleep — do not remove `detachInterrupt()` or `turnOffLEDs()` without understanding wake-up noise implications (see `src/button_handler.cpp`).

- **APIs / data formats to reference**
  - BLE protocol and example byte arrays: `docs/protocol.md` (e.g., `CMD_SET_COLOR: [0x01 R G B W]`, `CMD_SET_ANIMATION` formats).
  - Battery reading: ADC conversion in `src/ble_server.cpp` (`readBatteryVoltage()` and `batteryPercent()`); battery notifications use `batteryCharacteristic->notify()` when connected.

- **Where to make changes** (minimal, focused edits)
  - Add new BLE command: update `include/ble_server.h` (command IDs), implement handling in `LightCharacteristicCallbacks::onWrite`, and add helper in `src/led_control.cpp` if it affects LEDs or storage.
  - Modify animations: update `src/led_control.cpp` functions `setAnimation()` and `updateAnimation()`. Keep `animationSpeed` and `animationLastUpdate` semantics to remain non-blocking.
  - Storage schema changes: update `loadStoredColors()` / `saveColorSets()` and bump a small version marker (if needed) — handle migration gracefully if previous data is present.

- **Conventions and constraints**
  - Colors are 4 bytes in order R,G,B,W. Many functions expect lengths to be multiples of 4 (see `updateColorSets`, `setIndividualLEDColors`).
  - Avoid heavy libc usage or dynamic allocation; aim for stack/static buffers similar to existing code.
  - Use `Serial.println` for human-readable debug logs consistent with existing emoji-prefixed messages.
  - **Do not create summary files or documentation comments in the codebase.** Implement changes directly without adding extra `.md` files, summary comments, or explanatory headers. Keep code focused on functionality only.

- **Files to inspect for context/examples**
  - `src/main.cpp` — initialization and main loop
  - `src/ble_server.cpp` — BLE initialization, characteristic callbacks, battery reporting
  - `src/led_control.cpp` — NeoPixel control, animations, storage
  - `src/button_handler.cpp` — button debounce, long-press deep-sleep logic
  - `include/config.h` and other headers — pin definitions, `MAX_COLOR_SETS`, `NUM_LEDS`, UUIDs, and command ID constants
  - `platformio.ini` — build environment and library deps (NimBLE, Adafruit NeoPixel)

If anything here is unclear or you want additional examples (unit test harness, simulated host utilities, or CI-based flashing), tell me which area to expand.
