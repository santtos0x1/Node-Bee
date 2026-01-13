// Local libs
#include "config.h"

// Libs
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <Arduino.h>
#include <BLEScan.h>

QueueHandle_t BTQueue;
BLEScan *pBLEscan;
BTData data;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        DEBUG_PRINTLN("Cleaning data from the struct.");
        // Clean the data from "BTData" struct
        memset(&data, 0, sizeof(BTData));
        
        // Gets the data w/o connecting to the device
        bool haveName =  advertisedDevice.haveName();
        DEBUG_PRINTLN("Getting device name...");
        if (haveName)
        {
            DEBUG_PRINTF("Device have name: %s\n", advertisedDevice.getName().c_str());
            strncpy(data.name, advertisedDevice.getName().c_str(), sizeof(data.name));
        } else {
            DEBUG_PRINTLN("Device does not have a name!");
            strcpy(data.name, "Unknown");
        }
        DEBUG_PRINTLN("Getting device address...");
        strncpy(data.address, advertisedDevice.getAddress().toString().c_str(), sizeof(data.address));

        DEBUG_PRINTLN("Getting device RSSI...");
        data.rssi = advertisedDevice.getRSSI();
        
        DEBUG_PRINTLN("Getting device adress type...");
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
        DEBUG_PRINTLN("Defining channel...");
        data.channel = 0;

        DEBUG_PRINTLN("Sending to the queue...");
        // Send to the queue
        xQueueSend(BTQueue, &data, pdMS_TO_TICKS(10));
    }
};

void setupBT()
{
    DEBUG_PRINTLN("Creating the queue...");
    // Create the queue
    BTQueue = xQueueCreate(20, sizeof(BTData));

    DEBUG_PRINTLN("Starting bluetooth modules...");
    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();

    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void BTSniffer()
{
    DEBUG_PRINTLN("Starting bluetooth scan...");
    BLEDevice::getScan()->start(SCAN_TIME, false);
    BLEDevice::getScan()->clearResults();
    DEBUG_PRINTLN("BLE Scan done!");
}
