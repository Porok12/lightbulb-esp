#include "ble_server.h"
#include "led_control.h"
#include "button_handler.h"
#include "config.h"
#include <NimBLEDevice.h>

NimBLEServer *pServer = nullptr;
NimBLECharacteristic *lightCharacteristic;
NimBLECharacteristic *batteryCharacteristic;
NimBLECharacteristic *firmwareCharacteristic;

#define ADC_MAX 4095
#define VREF 1100

float readBatteryVoltage() {
    int raw = analogRead(BAT_PIN);
    float voltage_mv = ((float)raw / ADC_MAX) * VREF;
    float real_battery_mv = voltage_mv * 2.0;
    return real_battery_mv / 1000.0;
}

int batteryPercent(float v) {
    if (v >= 4.20) return 100;
    if (v >= 4.00) return 90;
    if (v >= 3.85) return 75;
    if (v >= 3.75) return 50;
    if (v >= 3.65) return 25;
    if (v >= 3.45) return 10;
    return 0;
}

bool deviceConnected = false;
uint8_t batteryLevel = 0; // Simulated battery percentage (0-100%)

class MyServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
    {
        deviceConnected = true;
        Serial.println("📱 Device connected");
    }

    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override
    {
        deviceConnected = false;
        Serial.print("🔌 Device disconnected (reason: ");
        Serial.print(reason);
        Serial.println(")");
        
        delay(500);
        
        NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
        if (pAdvertising != nullptr && !pAdvertising->isAdvertising()) {
            pAdvertising->start(0, 0);
            Serial.println("✅ Advertising restarted - device available for connection");
        }
    }
};

class LightCharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
    {
        Serial.println("📩 BLE Write Received!");

        std::string receivedData = pCharacteristic->getValue();
        size_t length = receivedData.length();

        Serial.print("📥 Received Data: ");
        Serial.println(receivedData.c_str());

        if (receivedData.empty())
            return;

        uint8_t command = receivedData[0];

        switch (command)
        {
        case CMD_SET_COLOR:
            if (length == 5) {
                setColorFromBytes((uint8_t *)&receivedData[1]);
            } else {
                Serial.print("❌ Invalid CMD_SET_COLOR length: ");
                Serial.println(length);
            }
            break;

        case CMD_SET_COLOR_SETS:
            if (length >= 5 && (length - 1) % 4 == 0)
            {
                updateColorSets((uint8_t *)&receivedData[1], length - 1);
            } else {
                Serial.print("❌ Invalid CMD_SET_COLOR_SETS length: ");
                Serial.println(length);
            }
            break;

        case CMD_DISABLE_BLE:
            Serial.println("🔌 CMD_DISABLE_BLE received - Going to deep sleep...");
            goToDeepSleep();
            break;

        case CMD_SET_INDIVIDUAL_COLORS:
            if (length >= 5 && (length - 1) % 4 == 0) {
                size_t numLEDs = (length - 1) / 4;
                setIndividualLEDColors((uint8_t *)&receivedData[1], numLEDs);
            } else {
                Serial.print("❌ Invalid CMD_SET_INDIVIDUAL_COLORS length: ");
                Serial.println(length);
            }
            break;

        case CMD_SET_SLEEP_TIMER:
            if (length == 3) {
                uint16_t minutes = (receivedData[1] << 8) | receivedData[2];
                setSleepTimer(minutes);
            } else {
                Serial.print("❌ Invalid CMD_SET_SLEEP_TIMER length: ");
                Serial.println(length);
            }
            break;

        case CMD_SET_ANIMATION:
            if (length >= 3) {
                uint8_t animType = receivedData[1];
                uint8_t speed = receivedData[2];
                uint8_t *params = (length > 3) ? (uint8_t *)&receivedData[3] : nullptr;
                size_t paramsLength = (length > 3) ? (length - 3) : 0;
                setAnimation(animType, speed, params, paramsLength);
            } else {
                Serial.print("❌ Invalid CMD_SET_ANIMATION length: ");
                Serial.println(length);
            }
            break;

        default:
            Serial.print("❌ Unknown command: 0x");
            Serial.println(command, HEX);
            break;
        }
    }
};

void initBLE()
{
    if (!NimBLEDevice::init(DEVICE_NAME)) {
        Serial.println("❌ Failed to initialize BLE");
        return;
    }
    
    NimBLEDevice::setPower(ESP_PWR_LVL_N0);

    pServer = NimBLEDevice::createServer();
    if (pServer == nullptr) {
        Serial.println("❌ Failed to create BLE server");
        return;
    }
    
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService *lightService = pServer->createService(LIGHT_SERVICE_UUID);
    lightCharacteristic = lightService->createCharacteristic(
        LIGHT_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::READ);
    lightCharacteristic->setCallbacks(new LightCharacteristicCallbacks());
    lightService->start();

    NimBLEService *batteryService = pServer->createService(BATTERY_SERVICE_UUID);
    batteryCharacteristic = batteryService->createCharacteristic(
        BATTERY_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    batteryCharacteristic->setValue(&batteryLevel, 1);
    batteryService->start();

    NimBLEService *deviceInfoService = pServer->createService(DEVICE_INFO_SERVICE_UUID);
    firmwareCharacteristic = deviceInfoService->createCharacteristic(
        FIRMWARE_VERSION_UUID,
        NIMBLE_PROPERTY::READ);
    firmwareCharacteristic->setValue(FIRMWARE_VERSION);
    deviceInfoService->start();
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName(DEVICE_NAME);
    pAdvertising->addServiceUUID(lightService->getUUID());
    pAdvertising->addServiceUUID(batteryService->getUUID());
    pAdvertising->addServiceUUID(deviceInfoService->getUUID());
    pAdvertising->enableScanResponse(true);
    pAdvertising->start(0, 0);
}

void debugScan()
{
    Serial.println("🔍 Scanning for nearby BLE devices...");

    NimBLEScan *pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    pBLEScan->setMaxResults(10);

    bool scanStarted = pBLEScan->start(5, false);

    if (!scanStarted)
    {
        Serial.println("❌ BLE Scan Failed to Start!");
        return;
    }

    NimBLEScanResults foundDevices = pBLEScan->getResults();

    Serial.print("📡 Found ");
    Serial.print(foundDevices.getCount());
    Serial.println(" devices!");

    for (int i = 0; i < foundDevices.getCount(); i++)
    {
        const NimBLEAdvertisedDevice *device = foundDevices.getDevice(i);
        Serial.print("🔹 Device ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(device->getAddress().toString().c_str());

        if (device->haveName())
        {
            Serial.print(" | Name: ");
            Serial.print(device->getName().c_str());
        }

        Serial.println();
    }
}

void debugBLE()
{
    if (pServer->getAdvertising()->isAdvertising())
    {
        Serial.println("📡 BLE Advertising is Active");
    }
    else
    {
        Serial.println("❌ BLE Advertising is Inactive");
    }

    if (pServer->getConnectedCount() > 0)
    {
        Serial.print("📡 Device Connected! Total Connections: ");
        Serial.println(pServer->getConnectedCount());
    }
    else
    {
        Serial.println("🛑 No BLE Connections");
    }

    debugScan();
}

void disableBLE()
{
    Serial.println("🔌 Disabling BLE Server...");
    pServer->getAdvertising()->stop();
    NimBLEDevice::deinit(true);
}

void updateBatteryLevelBLE()
{
    float voltage = readBatteryVoltage();
    batteryLevel = batteryPercent(voltage);

    Serial.print("🔋 Battery: ");
    Serial.print(voltage, 3);
    Serial.print("V (");
    Serial.print(batteryLevel);
    Serial.println("%)");

    if (batteryCharacteristic != nullptr)
    {
        batteryCharacteristic->setValue(&batteryLevel, 1);
        if (deviceConnected) {
            batteryCharacteristic->notify();
        }
    } else {
        Serial.println("⚠️ Battery characteristic not initialized");
    }
}

void ensureBLEAdvertising()
{
    if (pServer == nullptr) {
        return;
    }
    
    if (pServer->getConnectedCount() == 0) {
        NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
        if (pAdvertising != nullptr && !pAdvertising->isAdvertising()) {
            pAdvertising->start(0, 0);
            Serial.println("✅ Advertising restarted - device available");
        }
    }
}
