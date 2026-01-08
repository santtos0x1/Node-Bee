#include "wifi_scan.h"
#include "config.h"
#include <WiFi.h>
#include <esp_wifi.h>

#define CONN_TIMEOUT_MS (10 * 1000) 

QueueHandle_t wifiQueue;

void wifiInit() {
    wifiQueue = xQueueCreate(10, sizeof(wifiData));
}

void wifiSniffer() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    int numNetworksFound = WiFi.scanNetworks();
    
    for(int i = 0; i < numNetworksFound; i++) {
      wifiData data;
      memset(&data, 0, sizeof(wifiData));

      strncpy(data.ssid, WiFi.SSID(i).c_str(), 32);
      data.rssi = WiFi.RSSI(i);
      strncpy(data.bssid, WiFi.BSSIDstr(i).c_str(), 19);
      data.encryptationType = WiFi.encryptionType(i);
      data.channel = WiFi.channel(i);
      data.timestamp = millis();
      
      if(data.encryptationType == WIFI_AUTH_OPEN)
      {
        unsigned long initTimer = millis();
      
        WiFi.begin(data.ssid);
        Serial.print("Connecting");
      
        while(WiFi.status() != WL_CONNECTED) {
          if (millis() - initTimer >= CONN_TIMEOUT_MS) {
              Serial.println("\nConnection timeout...");
              break;
          }     
          delay(500);
          Serial.print("."); 
        }
      
        if(WiFi.status() == WL_CONNECTED) {
          Serial.printf("\nSuccessfully connected on %s\n", data.ssid);
          
          strncpy(data.hostname, WiFi.getHostname(), 32);      
          strncpy(data.localIP, WiFi.localIP().toString().c_str(), 15);
          strncpy(data.dnsIP, WiFi.dnsIP(0).toString().c_str(), 15);
          strncpy(data.subNetMask, WiFi.subnetMask().toString().c_str(), 15);
      
          tcpip_adapter_dhcp_status_t status;
          tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &status);
      
          if (status == TCPIP_ADAPTER_DHCP_STARTED) {
            strcpy(data.dhcp, "Active");
          } else {
            strcpy(data.dhcp, "Static");
          }
          
          WiFi.disconnect();
          Serial.println("Data collected and disconnected.");
        }
      }
      xQueueSend(wifiQueue, &data, pdMS_TO_TICKS(10));
    }
    WiFi.scanDelete();
}