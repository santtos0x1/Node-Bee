// Local libs
#include "config.h"

// Libs
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>

QueueHandle_t BTQueue;
BLEScan *pBLEscan;

BTData data;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // Clean the data from "BTData" struct
        memset(&data, 0, sizeof(BTData));
        
        // Gets the data w/o connecting to the device
        bool haveName =  advertisedDevice.haveName();
        if (haveName)
        {
            strncpy(data.name, advertisedDevice.getName().c_str(), sizeof(data.name));
        } else {
            strcpy(data.name, "Unknown");
        }
        strncpy(data.address, advertisedDevice.getAddress().toString().c_str(), sizeof(data.address));
        data.rssi = advertisedDevice.getRSSI();

        esp_ble_addr_type_t type = advertisedDevice.getAddressType();
        switch (type)
        {
        case BLE_ADDR_TYPE_PUBLIC:
            strcpy(data.addressType, "Public");
            break;
        case BLE_ADDR_TYPE_RANDOM:
            strcpy(data.addressType, "Random");
            break;
        case BLE_ADDR_TYPE_RPA_PUBLIC:
            strcpy(data.addressType, "RPA_Public");
            break;
        case BLE_ADDR_TYPE_RPA_RANDOM:
            strcpy(data.addressType, "RPA_Random");
            break;
        default:
            strcpy(data.addressType, "Unknown");
            break;
        }
        
        data.channel = 0;

        // Send to the queue
        xQueueSend(BTQueue, &data, pdMS_TO_TICKS(10));
    }
};

void setupBT()
{
    // Create the queue
    BTQueue = xQueueCreate(20, sizeof(BTData));

    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();

    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void BTSniffer()
{
    DEBUG_PRINTLN("Starting bluetooth scan.");
    BLEDevice::getScan()->start(SCAN_TIME, false);
    BLEDevice::getScan()->clearResults();
    DEBUG_PRINTLN("BLE Scan done.");
}
