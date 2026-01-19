#include "config.h"
#include "bluetooth_scan.h"
#include "utils.h"
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <Arduino.h>
#include <BLEScan.h>
#include <WiFi.h>

QueueHandle_t BTQueue;
BLEScan *pBLEscan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        BTData data;
        memset(&data, 0, sizeof(BTData));

        // Coleta de metadados
        bool haveName = advertisedDevice.haveName();
        if (haveName) {
            strncpy(data.name, advertisedDevice.getName().c_str(), sizeof(data.name) - 1);
        } else {
            strncpy(data.name, "Unknown", sizeof(data.name) - 1);
        }

        strncpy(data.address, advertisedDevice.getAddress().toString().c_str(), sizeof(data.address) - 1);
        data.rssi = advertisedDevice.getRSSI();
        esp_ble_addr_type_t type = advertisedDevice.getAddressType();
        strncpy(data.addressType, GET_ADDR_TYPE(type), sizeof(data.addressType) - 1);
        data.channel = 0; 

        // --- EXIBIÇÃO DOS RESULTADOS NO SERIAL (Igual ao WiFi) ---
        DEBUG_PRINTF(F(CLR_GREEN "------[ BT DEV ]------\nMAC: %s\n" CLR_RESET), data.address);
        DEBUG_PRINTF(F("Name: %-20s\n"), data.name);
        DEBUG_PRINTF(F("RSSI: %d dBm | Address Type: %s\n"), data.rssi, data.addressType);
        DEBUG_PRINTLN(F("-------------------------------------------------"));

        #if (ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE) || (!ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE)
            /* Sends the populated struct to the queue receiver. */
            if (xQueueSend(BTQueue, &data, pdMS_TO_TICKS(100)) != pdPASS) {
                DEBUG_PRINTLN(F(CLR_RED "BT Queue Full! Data lost." CLR_RESET));
            }
        #endif
    }
};

void setupBluetooth()
{   
    #if ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
        DEBUG_PRINTLN(F(CLR_YELLOW "Creating Bluetooth queue..." CLR_RESET));
        BTQueue = xQueueCreate(DUALCORE_MAX_XQUEUE, sizeof(BTData));
    #elif !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
        DEBUG_PRINTLN(F(CLR_YELLOW "Creating Bluetooth queue..." CLR_RESET));    
        BTQueue = xQueueCreate(SINGLECORE_MAX_XQUEUE, sizeof(BTData));
    #endif

    DEBUG_PRINTLN(F(CLR_YELLOW "Initializing BLE Device..." CLR_RESET));
    BLEDevice::init("");
    pBLEscan = BLEDevice::getScan(); 

    pBLEscan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEscan->setActiveScan(true);
    pBLEscan->setInterval(100); 
    pBLEscan->setWindow(99);    
}

void BluetoothSniffer()
{
    WiFi.mode(WIFI_OFF);
    
    DEBUG_PRINTLN(F("\n" CLR_GREEN "--- Starting Bluetooth Scan... ---" CLR_RESET));
    pBLEscan->start(SCAN_TIME, false);
    pBLEscan->clearResults(); 
    DEBUG_PRINTLN(F(CLR_GREEN "--- BLE Scan Done! ---" CLR_RESET));
}