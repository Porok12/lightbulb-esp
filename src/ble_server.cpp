#include "ble_server.h"
#include "led_control.h"
#include "config.h"
#include <NimBLEDevice.h>

NimBLEServer *pServer = nullptr;
NimBLECharacteristic *pCharacteristic;
bool deviceConnected = false;

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

class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
    {
        std::string receivedData = pCharacteristic->getValue();
        if (receivedData.empty())
            return;

        uint8_t command = receivedData[0]; // First byte is the command ID

        switch (command)
        {
        case CMD_SET_COLOR:
            if (receivedData.length() == 5)
            {                                                   // Expecting 4 data bytes
                setColorFromBytes((uint8_t *)&receivedData[1]); // Pass RGBW bytes
            }
            break;

        case CMD_SET_COLOR_SETS:
            if (receivedData.length() >= 5 && (receivedData.length() - 1) % 4 == 0)
            {
                updateColorSets((uint8_t *)&receivedData[1], receivedData.length() - 1);
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
    NimBLEDevice::init(DEVICE_NAME);
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        BLE_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);

    pService->start();
    pCharacteristic->setValue("Hello from ESP32-C3!");
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->start(0, 0);
}

void disableBLE()
{
    pServer->getAdvertising()->stop();
    NimBLEDevice::deinit(true);
}
