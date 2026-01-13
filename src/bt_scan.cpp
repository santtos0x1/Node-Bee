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
        /*
            Clears the data struct before populating it to ensure memory integrity, 
            saving approximately 80 bytes of memory.
        */
        DEBUG_PRINTLN("Cleaning data from the struct.");
        memset(&data, 0, sizeof(BTData));

        /*
            Retrieves device metadata (Name, MAC Address, etc.) without 
            establishing a connection (Passive/Active Scanning).
        */
        bool haveName =  advertisedDevice.haveName();
        DEBUG_PRINTLN("Getting device name...");
        if (haveName)
        {
            DEBUG_PRINTF("Device has a name: %s\n", advertisedDevice.getName().c_str());
            strncpy(data.name, advertisedDevice.getName().c_str(), sizeof(data.name));
        } else {
            DEBUG_PRINTLN("Device does not have a name!");
            strcpy(data.name, "Unknown");
        }

        DEBUG_PRINTLN("Getting device address...");
        strncpy(data.address, advertisedDevice.getAddress().toString().c_str(), sizeof(data.address));

        DEBUG_PRINTLN("Getting device RSSI...");
        data.rssi = advertisedDevice.getRSSI();

        DEBUG_PRINTLN("Getting device address type...");
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
        data.channel = 0; // BLE scanning on multiple channels (defaulting to 0 for log)


        /* Sends the populated struct to the queue receiver.
            Functions like an operational treadmill with a 10ms timeout per sent item.
        */
        DEBUG_PRINTLN("Sending to the queue...");
        xQueueSend(BTQueue, &data, pdMS_TO_TICKS(10));
    }
};

void setupBT()
{
    /*
        Initializes the queue, defining the maximum number of items (20) 
        and the size of each BTData struct.    
    */
    DEBUG_PRINTLN("Creating the queue...");
    BTQueue = xQueueCreate(20, sizeof(BTData));

    DEBUG_PRINTLN("Starting bluetooth modules...");
    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();

    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100); // Set the scanning interval (time between scans).
    pBLEScan->setWindow(99);    // Set the scanning window (actual time spent scanning).
}

void BTSniffer()
{
    DEBUG_PRINTLN("Starting bluetooth scan...");
    BLEDevice::getScan()->start(SCAN_TIME, false);
    BLEDevice::getScan()->clearResults(); // Clear results from memory to prevent overflow
    DEBUG_PRINTLN("BLE Scan done!");
}