#include "ble_server.h"
#include "led_control.h"
#include "config.h"
#include <NimBLEDevice.h>
//#include <NimBLEOta.h>

NimBLEServer *pServer = nullptr;
NimBLECharacteristic *lightCharacteristic;
NimBLECharacteristic *batteryCharacteristic;
NimBLECharacteristic *firmwareCharacteristic;

//static NimBLEOta bleOta;

bool deviceConnected = false;
uint8_t batteryLevel = 85; // Simulated battery percentage (0-100%)

class MyServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
    {
        deviceConnected = true;
    }

    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override
    {
        deviceConnected = false;
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

        // const uint8_t* data = reinterpret_cast<const uint8_t*>(receivedData.data());
        // size_t length = receivedData.length();

        if (receivedData.empty())
            return;

        uint8_t command = receivedData[0]; // First byte is the command ID
        // uint8_t command = data[0]; // First byte is the command ID

        switch (command)
        {
        case CMD_SET_COLOR:
            if (length == 5)
            {                                                   // Expecting 4 data bytes
                setColorFromBytes((uint8_t *)&receivedData[1]); // Pass RGBW bytes
            }
            break;

        case CMD_SET_COLOR_SETS:
            if (length >= 5 && (length - 1) % 4 == 0)
            {
                updateColorSets((uint8_t *)&receivedData[1], length - 1);
            }
            break;

        case CMD_DISABLE_BLE:
            disableBLE();
            break;
        }
    }
};

void initBLE()
{
    // NimBLEDevice::deinit();
    // sleep(2000);

    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_N0);

    // NimBLEDevice::init("NimBLE OTA");
    // bleOta.start();

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // ✅ **Custom Light Control Service**
    NimBLEService *lightService = pServer->createService(LIGHT_SERVICE_UUID);
    lightCharacteristic = lightService->createCharacteristic(
        LIGHT_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::READ);
    lightCharacteristic->setCallbacks(new LightCharacteristicCallbacks());
    lightService->start();

    // ✅ **Battery Service (0x180F)**
    NimBLEService *batteryService = pServer->createService(BATTERY_SERVICE_UUID);
    batteryCharacteristic = batteryService->createCharacteristic(
        BATTERY_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    batteryCharacteristic->setValue(&batteryLevel, 1);
    batteryService->start();

    // ✅ **Device Information Service (0x180A)**
    NimBLEService *deviceInfoService = pServer->createService(DEVICE_INFO_SERVICE_UUID);
    firmwareCharacteristic = deviceInfoService->createCharacteristic(
        FIRMWARE_VERSION_UUID,
        NIMBLE_PROPERTY::READ);
    firmwareCharacteristic->setValue(FIRMWARE_VERSION);
    deviceInfoService->start();

    // ✅ **BLE Advertising**
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
