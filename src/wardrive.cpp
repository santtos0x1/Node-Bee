// Local libs
#include "indicator.h"
#include "wardrive.h"
#include "config.h"

// Libs
#include <Arduino.h>
#include <WiFi.h>

QueueHandle_t WDQueue;

/**
 * Initializes the FreeRTOS queue to handle Wardriving data packets.
 */
void setupWardrive()
{
    DEBUG_PRINTLN("Creating wardrive queue...");
    WDQueue = xQueueCreate(50, sizeof(WardriveData));
    DEBUG_PRINTLN("Done!");
}

/**
 * Scans for networks, populates the data struct, and pushes to the Queue.
 * Returns true if an Open Network (No Auth) is detected during the scan.
 */
bool startWardrive()
{
    bool openNetworkfound = false;
    DEBUG_PRINTLN("Starting network scan...");
    
    // Synchronous scan (blocks execution until finished)
    int networks = WiFi.scanNetworks();

    for(int n = 0; n < networks; n++) {
        WardriveData data;

        DEBUG_PRINTLN("Getting encryption type...");
        wifi_auth_mode_t encryptionType = WiFi.encryptionType(n);

        DEBUG_PRINTLN("Getting SSID & RSSI...");
        strncpy(data.ssid, WiFi.SSID(n).c_str(), sizeof(data.ssid) - 1);
        data.rssi = WiFi.RSSI(n);

        // Debug output
        DEBUG_PRINTF("SSID: %s | RSSI: %d\n", data.ssid, data.rssi);

        // Push data to the queue with a 10ms timeout

        if (encryptionType == WIFI_AUTH_OPEN)
        {
            DEBUG_PRINTLN("Open network found!");
            openNetworkfound = true;
        }

        if (xQueueSend(WDQueue, &data, pdMS_TO_TICKS(100)) != pdPASS) {
            DEBUG_PRINTLN("Wardrive Queue Full! Data lost.");
        }
    }

    /* Crucial: Clears the scan results from RAM. 
       Without this, the ESP32 heap will crash after a few scans. */
    DEBUG_PRINTLN("Cleaning scan memory...");
    WiFi.scanDelete();
    
    return openNetworkfound;
}