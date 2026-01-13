// Local libs
#include "wifi_scan.h"
#include "config.h"

// Libs
#include <esp_wifi.h>
#include <Arduino.h>
#include <WiFi.h>

QueueHandle_t WiFiQueue;

void setupWiFi()
{
    // Create the queue
    DEBUG_PRINTLN("Creating WiFi queue...");
    WiFiQueue = xQueueCreate(10, sizeof(WiFiData));
}

void WiFiSniffer()
{
    DEBUG_PRINTLN("Setting mode to STA...");
    WiFi.mode(WIFI_STA);
    DEBUG_PRINTLN("Disconecting last WiFi connection...");
    WiFi.disconnect();

    // Starts the scan
    DEBUG_PRINTLN("Starting network scan...");
    int numNetworksFound = WiFi.scanNetworks();

    for (int i = 0; i < numNetworksFound; i++)
    {
        WiFiData data;

        // Clean the data from "BTData" struct
        DEBUG_PRINTLN("Cleaning data from the struct.");
        memset(&data, 0, sizeof(WiFiData));

        DEBUG_PRINTF("Collecting data from network %d...\n", i);

        // Gets the data w/o connecting to the network
        DEBUG_PRINTF("Getting SSID: %s\n", WiFi.SSID(i).c_str());
        strncpy(data.ssid, WiFi.SSID(i).c_str(), sizeof(data.ssid));
        DEBUG_PRINTF("Getting RSSI: %d\n", WiFi.RSSI(i));
        data.rssi = WiFi.RSSI(i);
        DEBUG_PRINTF("Getting BSSID: %s\n", WiFi.BSSIDstr(i).c_str());
        strncpy(data.bssid, WiFi.BSSIDstr(i).c_str(), sizeof(data.bssid));
        DEBUG_PRINTF("Getting encryption type: %d\n", WiFi.encryptionType(i));
        data.encryptionType = WiFi.encryptionType(i);
        DEBUG_PRINTF("Getting channel: %d\n", WiFi.channel(i));
        data.channel = WiFi.channel(i);

        // Try to connect
        DEBUG_PRINTLN("Trying to connect to the network...");
        if (data.encryptionType == WIFI_AUTH_OPEN)
        {
            unsigned long initTimer = millis();

            // Starts the connection
            WiFi.begin(data.ssid);
            DEBUG_PRINT("Starting connection");
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
                DEBUG_PRINTF("Getting hostname: %s\n", WiFi.getHostname());
                strncpy(data.hostname, WiFi.getHostname(), sizeof(data.hostname));

                DEBUG_PRINTF("Getting local IP: %s\n", WiFi.localIP().toString().c_str());
                strncpy(data.localIP, WiFi.localIP().toString().c_str(), sizeof(data.localIP));
                
                DEBUG_PRINTF("Getting local DNS IP: %s\n", WiFi.dnsIP().toString().c_str());
                strncpy(data.dnsIP, WiFi.dnsIP(0).toString().c_str(), sizeof(data.dnsIP));
                
                DEBUG_PRINTF("Getting local IP: %s\n", WiFi.subnetMask().toString().c_str());
                strncpy(data.subNetMask, WiFi.subnetMask().toString().c_str(), sizeof(data.subNetMask));

                // Verifies if the DHCP is enabled
                DEBUG_PRINTLN("Getting DHCP status...")
                tcpip_adapter_dhcp_status_t DHCPStatus;
                tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &DHCPStatus);
                if (DHCPStatus == TCPIP_ADAPTER_DHCP_STARTED)
                {
                    DEBUG_PRINTLN("DHCP status: Active");
                    strcpy(data.dhcp, "DHCP status: Active");
                } else {
                    DEBUG_PRINTLN("DHCP status: Static");
                    strcpy(data.dhcp, "DHCP status: Static");
                }

                WiFi.disconnect();
                DEBUG_PRINTF("Data successfully collected. Disconnected from %s\n", data.ssid);
            }
        }

        // Send to the queue
        DEBUG_PRINTLN("Sending data to queue...");
        xQueueSend(WiFiQueue, &data, pdMS_TO_TICKS(10));
        DEBUG_PRINTLN("Done!");
    }
    
    // Cleans the scan data in memory
    DEBUG_PRINTLN("Deleting scan from the memory...");
    WiFi.scanDelete();
    DEBUG_PRINTLN("Done!");
}