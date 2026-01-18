// Local libs
#include "indicator.h"
#include "wardrive.h"
#include "config.h"
#include "utils.h"

// Libs
#include <Arduino.h>
#include <WiFi.h>

QueueHandle_t WDQueue;

/**
 * Initializes the FreeRTOS queue to handle Wardriving data packets.
 */
void setupWardrive()
{
    DEBUG_PRINTLN(F(CLR_YELLOW "Creating wardrive queue..." CLR_RESET));
    #if ASYNC_SD_HANDLER
        WDQueue = xQueueCreate(DUALCORE_MAX_XQUEUE, sizeof(WardriveData));
    #else
        WDQueue = xQueueCreate(SINGLECORE_MAX_XQUEUE, sizeof(WardriveData));
    #endif
    DEBUG_PRINTLN(F(CLR_GREEN "Done!" CLR_RESET));
}

/**
 * Scans for networks, populates the data struct, and pushes to the Queue.
 * Returns true if an Open Network (No Auth) is detected during the scan.
 */
bool startWardrive()
{
    bool openNetworkfound = false;
    DEBUG_PRINTLN(F(CLR_YELLOW "Starting network scan..." CLR_RESET));
    
    // Synchronous scan (blocks execution until finished)
    int networks = WiFi.scanNetworks();

    for(int n = 0; n < networks; n++) {
        WardriveData data;

        DEBUG_PRINTLN(F("Getting encryption type..."));
        wifi_auth_mode_t encryptionType = WiFi.encryptionType(n);

        DEBUG_PRINTLN(F("Getting SSID & RSSI..."));
        strncpy(data.ssid, WiFi.SSID(n).c_str(), sizeof(data.ssid) - 1);
        data.rssi = WiFi.RSSI(n);

        // Debug output - Formato corrigido para o printf
        DEBUG_PRINTF(F(CLR_YELLOW "SSID: %s | RSSI: %d\n" CLR_RESET), data.ssid, data.rssi);

        // Push data to the queue with a 10ms timeout

        if (encryptionType == WIFI_AUTH_OPEN)
        {
            DEBUG_PRINTLN(F(CLR_GREEN "Open network found!" CLR_RESET));
            openNetworkfound = true;
        }

        if (xQueueSend(WDQueue, &data, pdMS_TO_TICKS(100)) == pdPASS) {
            // Sucesso opcional no log
        } else {
            DEBUG_PRINTLN(F(CLR_RED "Wardrive Queue Full! Data lost." CLR_RESET));
        }
    }

    /* Crucial: Clears the scan results from RAM. 
       Without this, the ESP32 heap will crash after a few scans. */
    DEBUG_PRINTLN(F(CLR_YELLOW "Cleaning scan memory..." CLR_RESET));
    WiFi.scanDelete();
    
    return openNetworkfound;
}