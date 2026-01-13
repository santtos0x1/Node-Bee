// Local libs
#include "wifi_scan.h"
#include "config.h"

// Libs
#include <WiFi.h>
#include <esp_wifi.h>
#include <Arduino.h>

QueueHandle_t WiFiQueue;

void setupWiFi()
{
    // Create the queue
    WiFiQueue = xQueueCreate(10, sizeof(WiFiData));
}

void WiFiSniffer()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Starts the scan
    int numNetworksFound = WiFi.scanNetworks();

    for (int i = 0; i < numNetworksFound; i++)
    {
        WiFiData data;

        // Clean the data from "BTData" struct
        memset(&data, 0, sizeof(WiFiData));

        DEBUG_PRINTF("Collecting data from network %d...\n", i);

        // Gets the data w/o connecting to the network
        strncpy(data.ssid, WiFi.SSID(i).c_str(), sizeof(data.ssid));
        data.rssi = WiFi.RSSI(i);
        strncpy(data.bssid, WiFi.BSSIDstr(i).c_str(), sizeof(data.bssid));
        data.encryptionType = WiFi.encryptionType(i);
        data.channel = WiFi.channel(i);

        // Try to connect
        if (data.encryptionType == WIFI_AUTH_OPEN)
        {
            unsigned long initTimer = millis();

            // Starts the connection
            WiFi.begin(data.ssid);
            DEBUG_PRINT("Trying to connect");
            wl_status_t WFStatus = WiFi.status();

            while (WFStatus != WL_CONNECTED)
            {
                if (millis() - initTimer > CONN_TIMEOUT_MS)
                {
                    DEBUG_PRINT("\nConnection timeout...");
                    break;
                }
                delay(Time::MID_DELAY);
                DEBUG_PRINT(".");
            }

            if (WFStatus == WL_CONNECTED)
            {
                DEBUG_PRINTF("\nSuccessfully connected on %s\n", data.ssid);

                // Gets the data of the network
                strncpy(data.hostname, WiFi.getHostname(), sizeof(data.hostname));
                strncpy(data.localIP, WiFi.localIP().toString().c_str(), sizeof(data.localIP));
                strncpy(data.dnsIP, WiFi.dnsIP(0).toString().c_str(), sizeof(data.dnsIP));
                strncpy(data.subNetMask, WiFi.subnetMask().toString().c_str(), sizeof(data.subNetMask));

                // Verifies if the DHCP is enabled
                tcpip_adapter_dhcp_status_t DHCPStatus;
                tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &DHCPStatus);
                if (DHCPStatus == TCPIP_ADAPTER_DHCP_STARTED)
                {
                    strcpy(data.dhcp, "DHCP status: Active");
                } else {
                    strcpy(data.dhcp, "DHCP status: Static");
                }

                WiFi.disconnect();
                DEBUG_PRINTF("Data successfully collected. Disconnected from %s\n", data.ssid);
            }
        }

        // Send to the queue
        xQueueSend(WiFiQueue, &data, pdMS_TO_TICKS(10));
    }
    
    // Cleans the scan data in memory
    WiFi.scanDelete();
}