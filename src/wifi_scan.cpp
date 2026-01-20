// Local libs
#include "wifi_scan.h"
#include "config.h"
#include "utils.h"

// Libs
#include <esp_wifi.h>
#include <Arduino.h>
#include <WiFi.h>

QueueHandle_t WiFiQueue;

void setupWiFi() {
    #if ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
        DEBUG_PRINTLN(F(CLR_YELLOW "Creating WiFi queue..." CLR_RESET));    
        WiFiQueue = xQueueCreate(DUALCORE_MAX_XQUEUE, sizeof(WiFiData));    
    #elif !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
        DEBUG_PRINTLN(F(CLR_YELLOW "Creating WiFi queue..." CLR_RESET));    
        WiFiQueue = xQueueCreate(SINGLECORE_MAX_XQUEUE, sizeof(WiFiData));
    #endif
}

void wifiSniffer() {
    // Resets WiFi radio state to Station Mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    DEBUG_PRINTLN(F("\n" CLR_YELLOW "--- Starting network scan... ---" CLR_RESET));
    int numNetworksFound = WiFi.scanNetworks();

    if (numNetworksFound <= 0) {
        DEBUG_PRINTLN(F(CLR_RED "No networks found." CLR_RESET));
    } else {
        DEBUG_PRINTF(F("[ WIFI ] %d networks detected.\n"), numNetworksFound);
    }

    for (int i = 0; i < numNetworksFound; i++) {
        WiFiData data;
        memset(&data, 0, sizeof(WiFiData)); // Memory integrity check

        // Passive Data Collection
        strncpy(data.ssid, WiFi.SSID(i).c_str(), sizeof(data.ssid) - 1);
        data.rssi = WiFi.RSSI(i);
        strncpy(data.dbmQuality, GET_RSSI_QUALITY(data.rssi), sizeof(data.dbmQuality) - 1);
        strncpy(data.bssid, WiFi.BSSIDstr(i).c_str(), sizeof(data.bssid));
        data.encryptionType = WiFi.encryptionType(i);
        data.channel = WiFi.channel(i);

        // --- EXIBIÇÃO DOS RESULTADOS NO SERIAL ---
        DEBUG_PRINTF(F(CLR_CYN "\n[ NETWORK %02d ]" CLR_RESET), i + 1);
        DEBUG_PRINTF(F("\n  SSID            : %s"), data.ssid);
        DEBUG_PRINTF(F("\n  BSSID           : %s"), data.bssid);
        DEBUG_PRINTF(F("\n  RSSI            : %d dBm (%s)"), data.rssi, data.dbmQuality);
        DEBUG_PRINTF(F("\n  Channel         : %d"), data.channel);
        DEBUG_PRINTF(F("\n  Encryption type : %s\n"), (data.encryptionType == WIFI_AUTH_OPEN) ? "OPEN" : "SECURED");

        /* Active Probing: Only attempts connection on Unsecured (Open) networks */
        if (data.encryptionType == WIFI_AUTH_OPEN && data.rssi >= -75) {
            DEBUG_PRINTLN(F(CLR_GREEN "          [!] Attempting active probe..." CLR_RESET));
            unsigned long initTimer = millis();
            WiFi.begin(data.ssid);
            
            while (WiFi.status() != WL_CONNECTED) {
                if (millis() - initTimer > CONN_TIMEOUT_MS) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            if (WiFi.status() == WL_CONNECTED) {
                // Internal Network Data Extraction
                strncpy(data.hostname, WiFi.getHostname(), sizeof(data.hostname) - 1);
                strncpy(data.localIP, WiFi.localIP().toString().c_str(), sizeof(data.localIP) - 1);
                strncpy(data.dnsIP, WiFi.dnsIP(0).toString().c_str(), sizeof(data.dnsIP) - 1);
                strncpy(data.subNetMask, WiFi.subnetMask().toString().c_str(), sizeof(data.subNetMask) - 1);

                tcpip_adapter_dhcp_status_t DHCPStatus;
                tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &DHCPStatus);
                strncpy(data.dhcp, (DHCPStatus == TCPIP_ADAPTER_DHCP_STARTED) ? "Active" : "Static", sizeof(data.dhcp));

                DEBUG_PRINTF(F(CLR_GREEN "          [+] Connected!\n" CLR_RESET));
                DEBUG_PRINTF(F("          IP Address : %s\n"), data.localIP);
                DEBUG_PRINTF(F("          Hostname   : %s\n"), data.hostname);
                DEBUG_PRINTF(F("          DHCP       : %s\n"), data.dhcp);
                DEBUG_PRINTF(F("          Mask       : %s\n"), data.subNetMask);
                DEBUG_PRINTF(F("          DNS        : %s\n"), data.dnsIP);

                WiFi.disconnect();
            } else {
                DEBUG_PRINTLN(F(CLR_RED "          [-] Connection failed (Timeout)." CLR_RESET));
            }
        }

        #if (ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE) || (!ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE)
            // Offload captured data to the processing queue
            if (xQueueSend(WiFiQueue, &data, pdMS_TO_TICKS(100)) != pdPASS) {
                DEBUG_PRINTLN(F(CLR_RED "WiFi Queue Full! Data lost." CLR_RESET));
            }
        #endif
    }
    
    DEBUG_PRINTLN(F(CLR_YELLOW "\nCleaning WiFi scan results..." CLR_RESET));
    WiFi.scanDelete();
}