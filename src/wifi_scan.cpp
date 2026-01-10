// Local libs
#include "wifi_scan.h"
#include "config.h"

// Libs
#include <WiFi.h>
#include <esp_wifi.h>
#include <Arduino.h>

#define CONN_TIMEOUT_MS (10 * 1000)

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

        Serial.printf("Collecting data from network %d...\n", i);

        // Gets the data w/o connecting to the network
        strncpy(data.ssid, WiFi.SSID(i).c_str(), 32);
        data.rssi = WiFi.RSSI(i);
        strncpy(data.bssid, WiFi.BSSIDstr(i).c_str(), 19);
        data.encryptationType = WiFi.encryptionType(i);
        data.channel = WiFi.channel(i);

        // Try to connect
        if (data.encryptationType == WIFI_AUTH_OPEN)
        {
            unsigned long initTimer = millis();

            // Starts the connection
            WiFi.begin(data.ssid);
            Serial.print("Trying to connect");

            while (WiFi.status() != WL_CONNECTED)
            {
                if (millis() - initTimer > CONN_TIMEOUT_MS)
                {
                    Serial.println("\nConnection timeout...");
                    break;
                }
                delay(500);
                Serial.print(".");
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.printf("\nSuccessfully connected on %s\n", data.ssid);

                // Gets the data of the network
                strncpy(data.hostname, WiFi.getHostname(), 32);
                strncpy(data.localIP, WiFi.localIP().toString().c_str(), 15);
                strncpy(data.dnsIP, WiFi.dnsIP(0).toString().c_str(), 15);
                strncpy(data.subNetMask, WiFi.subnetMask().toString().c_str(), 15);

                // Verifies if the DHCP is enabled
                tcpip_adapter_dhcp_status_t status;
                tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &status);
                if (status == TCPIP_ADAPTER_DHCP_STARTED)
                {
                    strcpy(data.dhcp, "DHCP status: Active");
                }
                else
                {
                    strcpy(data.dhcp, "DHCP status: Inactive, static.");
                }

                WiFi.disconnect();
                Serial.printf("Data successfully collected. Disconnected from %s\n", data.ssid);
            }
        }

        // Send to the queue
        xQueueSend(WiFiQueue, &data, pdMS_TO_TICKS(10));
    }
    
    // Cleans the scan data in memory
    WiFi.scanDelete();
}