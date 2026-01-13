// Local libs
#include "indicator.h"
#include "wardrive.h"
#include "config.h"

// Libs
#include <Arduino.h>
#include <WiFi.h>

QueueHandle_t WDQueue;

bool openNetworkfound = false;

void setupWardrive()
{
    DEBUG_PRINTLN("Creating wardrive queue...");
    WDQueue = xQueueCreate(10, sizeof(WardriveData));
    DEBUG_PRINTLN("Done!");
}

bool startWardrive()
{
    DEBUG_PRINTLN("Starting network scan...");
    int networks = WiFi.scanNetworks();

    for(int n = 0; n < networks; n++) {
        WardriveData data;

        DEBUG_PRINTLN("Getting encryption type from the network...");
        wifi_auth_mode_t encryptationType = WiFi.encryptionType(n);

        DEBUG_PRINTLN("Getting SSID...");
        strncpy(data.ssid, WiFi.SSID(n).c_str(), sizeof(data.ssid));
        DEBUG_PRINTF("SSID: %s\n", WiFi.SSID(n).c_str());

        DEBUG_PRINTLN("Getting RSSI...");
        data.rssi = WiFi.RSSI(n);
        DEBUG_PRINTF("RSSI: %d", WiFi.RSSI(n));

        // Send to the queue
        DEBUG_PRINTLN("Sending data to queue...");
        xQueueSend(WDQueue, &data, pdMS_TO_TICKS(10));
        DEBUG_PRINTLN("Done!");

        if (encryptationType == WIFI_AUTH_OPEN)
        {
            String NetworkName = WiFi.SSID(n);

            DEBUG_PRINTLN("Open network found: ");
            DEBUG_PRINT(NetworkName);

            openNetworkfound = true;
        }
    }
    DEBUG_PRINTLN("Deleting scan from the memory...");
    WiFi.scanDelete();
    DEBUG_PRINTLN("Done!");
    
    return openNetworkfound;
}
