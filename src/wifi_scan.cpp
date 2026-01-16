// Local libs
#include "wifi_scan.h"
#include "config.h"

// Libs
#include <esp_wifi.h>
#include <Arduino.h>
#include <WiFi.h>

QueueHandle_t WiFiQueue;

/**
 * Initializes the RTOS queue for WiFi metadata storage.
 */
void setupWiFi()
{
    DEBUG_PRINTLN("Creating WiFi queue...");
    WiFiQueue = xQueueCreate(50, sizeof(WiFiData));
}

/**
 * Executes a full WiFi scan and attempts to probe open networks 
 * for deeper network configuration details (DHCP, IP, DNS).
 */
void WiFiSniffer()
{
    // Resets WiFi radio state to Station Mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    DEBUG_PRINTLN("Starting network scan...");
    int numNetworksFound = WiFi.scanNetworks();

    for (int i = 0; i < numNetworksFound; i++)
    {
        WiFiData data;
        memset(&data, 0, sizeof(WiFiData)); // Memory integrity check

        // Passive Data Collection (SSID, RSSI, BSSID, Channel)
        strncpy(data.ssid, WiFi.SSID(i).c_str(), sizeof(data.ssid) - 1);
        data.rssi = WiFi.RSSI(i);
        if(data.rssi >= -50)
        {
            strncpy(data.dbmQuality, "Strong", sizeof(data.dbmQuality) - 1);
        } else if (data.rssi >= -70)
        {
            strncpy(data.dbmQuality, "Medium", sizeof(data.dbmQuality) - 1);
        } else if (data.rssi >= -85)
        {
            strncpy(data.dbmQuality, "Weak", sizeof(data.dbmQuality) - 1);
        }

        strncpy(data.bssid, WiFi.BSSIDstr(i).c_str(), sizeof(data.bssid));
        data.encryptionType = WiFi.encryptionType(i);
        data.channel = WiFi.channel(i);

        /* Active Probing: Only attempts connection on Unsecured (Open) networks and with,
            an Ok quallity to stay connected.
        */
        if (data.encryptionType == WIFI_AUTH_OPEN && data.rssi >= -75)
        {
            unsigned long initTimer = millis();
            WiFi.begin(data.ssid);
            
            // Non-blocking connection attempt with timeout
            while (WiFi.status() != WL_CONNECTED)
            {
                if (millis() - initTimer > CONN_TIMEOUT_MS) break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                // Internal Network Data Extraction
                strncpy(data.hostname, WiFi.getHostname(), sizeof(data.hostname) - 1);
                strncpy(data.localIP, WiFi.localIP().toString().c_str(), sizeof(data.localIP) - 1);
                strncpy(data.dnsIP, WiFi.dnsIP(0).toString().c_str(), sizeof(data.dnsIP) - 1);
                strncpy(data.subNetMask, WiFi.subnetMask().toString().c_str(), sizeof(data.subNetMask) - 1);

                // DHCP Status check via ESP-IDF adapter
                tcpip_adapter_dhcp_status_t DHCPStatus;
                tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &DHCPStatus);
                strncpy(data.dhcp, (DHCPStatus == TCPIP_ADAPTER_DHCP_STARTED) ? "Active" : "Static", sizeof(data.dhcp));

                WiFi.disconnect();
            }
        }

        // Offload captured data to the processing queue
        if (xQueueSend(WiFiQueue, &data, pdMS_TO_TICKS(100)) != pdPASS) {
            DEBUG_PRINTLN("WiFi Queue Full! Data lost.");
        }
    }
    
    // Manual memory management for scan results
    WiFi.scanDelete();
}