#include "config.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

QueueHandle_t btQueue;

int scanTime = 5;
BLEScan* pBLEscan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        BTData data;
        memset(&data, 0, sizeof(BTData));

        if(advertisedDevice.haveName())
        {
            strncpy(data.name, advertisedDevice.getName().c_str(), 32);
        } else {
            strcpy(data.name, "Unknown");
        }
        strncpy(data.address, advertisedDevice.getAddress().toString().c_str(), 17);
        data.rssi = advertisedDevice.getRSSI();
        
        esp_ble_addr_type_t type = advertisedDevice.getAddressType();      
        switch (type) {
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

        data.timestamp = millis();
        data.channel = 0;

        xQueueSend(btQueue, &data, pdMS_TO_TICKS(10));
    }    
};

void initBLE()
{
    btQueue = xQueueCreate(20, sizeof(BTData));
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void bleSniffer()
{
    Serial.println("Starting BLE Scan...");
    BLEDevice::getScan()->start(scanTime, false);
    BLEDevice::getScan()->clearResults();
    Serial.println("Scan BLE done.");
}