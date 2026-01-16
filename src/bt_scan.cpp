// Local libs
#include "config.h"

// Libs
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <Arduino.h>
#include <BLEScan.h>

QueueHandle_t BTQueue;
BLEScan *pBLEscan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        /*
            Local struct to ensure thread-safety during rapid discovery.
        */
        BTData data;

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
            strncpy(data.name, advertisedDevice.getName().c_str(), sizeof(data.name) - 1);
        } else {
            DEBUG_PRINTLN("Device does not have a name!");
            strncpy(data.name, "Unknown", sizeof(data.name) - 1);
        }

        DEBUG_PRINTLN("Getting device address...");
        strncpy(data.address, advertisedDevice.getAddress().toString().c_str(), sizeof(data.address) - 1);

        DEBUG_PRINTLN("Getting device RSSI...");
        data.rssi = advertisedDevice.getRSSI();

        DEBUG_PRINTLN("Getting device address type...");
        esp_ble_addr_type_t type = advertisedDevice.getAddressType();
        switch (type)
        {
        case BLE_ADDR_TYPE_PUBLIC:
            strncpy(data.addressType, "Public", sizeof(data.addressType) - 1);
            break;
        case BLE_ADDR_TYPE_RANDOM:
            strncpy(data.addressType, "Random", sizeof(data.addressType) - 1);
            break;
        case BLE_ADDR_TYPE_RPA_PUBLIC:
            strncpy(data.addressType, "RPA_Public", sizeof(data.addressType) - 1);
            break;
        case BLE_ADDR_TYPE_RPA_RANDOM:
            strncpy(data.addressType, "RPA_Random", sizeof(data.addressType) - 1);
            break;
        default:
            strncpy(data.addressType, "Unknown", sizeof(data.addressType) - 1);
            break;
        }

        DEBUG_PRINTLN("Defining channel...");
        data.channel = 0; // BLE scanning on multiple channels (defaulting to 0 for log)


        /* Sends the populated struct to the queue receiver.
            Functions like an operational treadmill with a 100ms timeout per sent item.
        */
        DEBUG_PRINTLN("Sending to the queue...");
        if (xQueueSend(BTQueue, &data, pdMS_TO_TICKS(100)) != pdPASS) {
            DEBUG_PRINTLN("Bluetooth Queue Full! Data lost.");
        }
    }
};

void setupBT()
{
    /*
        Initializes the queue, defining the maximum number of items (50) 
        and the size of each BTData struct.    
    */
    DEBUG_PRINTLN("Creating the queue...");
    BTQueue = xQueueCreate(50, sizeof(BTData));

    DEBUG_PRINTLN("Starting bluetooth modules...");
    BLEDevice::init("");

    pBLEscan = BLEDevice::getScan(); 
    pBLEscan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEscan->setActiveScan(true);
    pBLEscan->setInterval(100); 
    pBLEscan->setWindow(99);    // Set the scanning window (actual time spent scanning).
}

void BTSniffer()
{
    DEBUG_PRINTLN("Starting bluetooth scan...");
    pBLEscan->start(SCAN_TIME, false);
    pBLEscan->clearResults(); // Clear results from memory to prevent overflow
    DEBUG_PRINTLN("BLE Scan done!");
}