// Local libs
#include "indicator.h"
#include "wardrive.h"
#include "config.h"
#include "utils.h"

// Libs
#include <Arduino.h>
#include <WiFi.h>

QueueHandle_t WDQueue;

int nextIndex = 0;
char knownBSSIDs[MAX_KNOWN_NETWORKS][18];

bool isNewNetwork(const char* bssid) {
    for (int i = 0; i < MAX_KNOWN_NETWORKS; i++) {
        if (strcmp(knownBSSIDs[i], bssid) == 0) {
            return false;
        }
    }
    return true;
}

/**
 * Initializes the FreeRTOS queue to handle Wardriving data packets.
 */
void setupWardrive()
{
    #if ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
        DEBUG_PRINTLN(F(CLR_YELLOW "Creating wardrive queue..." CLR_RESET));
       WDQueue = xQueueCreate(DUALCORE_MAX_XQUEUE, sizeof(WardriveData));
    #elif !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
        DEBUG_PRINTLN(F(CLR_YELLOW "Creating wardrive queue..." CLR_RESET));    
        WDQueue = xQueueCreate(SINGLECORE_MAX_XQUEUE, sizeof(WardriveData));
    #endif
}

/**
 * Scans for networks, populates the data struct, and pushes to the Queue.
 * Returns true if an Open Network (No Auth) is detected during the scan.
 */
bool startWardrive()
{   
    // Check the current status of the scan
    int16_t scanStatus = WiFi.scanComplete();
    
    // 1. If scan is still running, we exit to keep the main loop (and buttons) responsive
    if (scanStatus == WIFI_SCAN_RUNNING) 
    {
        return false;
    } 
    // 2. If no scan is active, start a new asynchronous scan
    else if (scanStatus == WIFI_SCAN_FAILED)
    {
        // 'true' for async mode
        WiFi.scanNetworks(true);
        return false;
    } 
    // 3. If scan is finished (scanStatus > 0), process the results
    else if (scanStatus > 0)
    {
        for(int n = 0; n < scanStatus; n++)
        {   
            bool openNetwork = false;
            if (digitalRead(Pins::BTN_B) == LOW) {
                WiFi.scanDelete(); // Free memory before leaving
                return false; 
            }

            WardriveData data;

            // Copy BSSID
            strncpy(data.bssid, WiFi.BSSIDstr(n).c_str(), sizeof(data.bssid) - 1);
            strncpy(data.ssid, WiFi.SSID(n).c_str(), sizeof(data.ssid) - 1);

            // Filter out already known networks
            if (!isNewNetwork(data.bssid)) {
                continue; 
            }

            if((WiFi.encryptionType(n) == WIFI_AUTH_OPEN) && isNewNetwork(data.bssid))
            {
                openNetwork = true;
            }
            
            // Add new BSSID to the circular buffer
            strncpy(knownBSSIDs[nextIndex], data.bssid, sizeof(data.bssid));
            DEBUG_PRINTF("SSID: %s\n", data.ssid);
            DEBUG_PRINTF("BSSID: %s\n", data.bssid);
            nextIndex = (nextIndex + 1) % MAX_KNOWN_NETWORKS;

            return openNetwork;

            #if (ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE) || (!ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE)
                // Push data to the queue with a 10ms timeout
                if (xQueueSend(WDQueue, &data, pdMS_TO_TICKS(100)) != pdPASS) {
                    DEBUG_PRINTLN(F(CLR_RED "Wardrive Queue Full! Data lost." CLR_RESET));
                }
            #endif
        }

        /* Crucial: Clears the scan results from RAM. */
        WiFi.scanDelete();
    }

    return false;
}
