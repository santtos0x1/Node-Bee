// Local libs
#include "http_server.h"

// Libs
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>   

WiFiServer server(80);

const char* ssid = "";
const char* pass = "";

bool startServer() 
{
    WiFi.disconnect();
    delay(100);

    // Starts the scan
    int networks = WiFi.scanNetworks();

    for (int i = 0; i < networks; i++)
    {
        if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN)
        {
            Serial.printf("Trying to connect to %s\n", WiFi.SSID(i));
            
            if(ssid == "")
            {
                WiFi.begin(WiFi.SSID(i).c_str());
            } else {
                WiFi.begin(ssid, pass);
            }

            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 20) 
            {
                delay(500);
                Serial.print(".");
                attempts++;
            }

            if(WiFi.status() == WL_CONNECTED)
            {
                Serial.println("Connection estabilished!");
                break;
            } else {
                Serial.println("Cannot connect to this network!");
            }
        }
    }
    if(WiFi.status() == WL_CONNECTED)
    {
        server.begin();
        Serial.println("HTTP Server started!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("Cannot open server!");
        return false;
    }
}

void handleDownload(WiFiClient& client, String path)
{
    if (SD.exists(path)) {
        File dataFile = SD.open(path);
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/octet-stream");
        client.print("Content-Length: ");
        client.println(dataFile.size());
        client.println("Connection: close");
        client.println();

        uint8_t buffer[512];
        while (dataFile.available()) {
            int bytesRead = dataFile.read(buffer, sizeof(buffer));
            client.write(buffer, bytesRead);
        }
        dataFile.close();
    } else {
        client.println("HTTP/1.1 404 Not Found\r\n\r\nFile not found.");
    }
}

void sendIndexSD(WiFiClient& client)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println();
    client.println("<h1>WiFi logs folder - SD files</h1>");
    client.println("<ul>");

    File WFRoot = SD.open("/wifi_log_data/");
    File WFFile = WFRoot.openNextFile();
    
    while (WFFile) {
        String WDLogName = WFFile.name();
        client.print("<li><a href=\"/wifi_log_data/");
        client.print(WDLogName);
        client.print("\">");
        client.print(WDLogName);
        client.println("</a></li>");
        
        WFFile = WFRoot.openNextFile();
    }
    client.println("</ul>");

    WFRoot.close();

    client.println("<h1>Bluetooth logs folder - SD files</h1>");
    client.println("<ul>");

    File BTRoot = SD.open("/bluetooth_log_data/");
    File BTFile = BTRoot.openNextFile();

    while (BTFile) {
        String BTLogName = BTFile.name();
        client.print("<li><a href=\"/bluetooth_log_data/");
        client.print(BTLogName);
        client.print("\">");
        client.print(BTLogName);
        client.println("</a></li>");
        
        BTFile = BTRoot.openNextFile();
    }
    client.println("</ul>");

    BTRoot.close();
}    


void serverCFG()
{
    WiFiClient client = server.available();
    
    if(client)
    {
        String request = client.readStringUntil('\r');
        client.readStringUntil('\n');

        int addrStart = request.indexOf('/');
        int addrEnd = request.indexOf(' ', addrStart);
        String fileName = request.substring(addrStart, addrEnd);

        if(fileName == "/")
        {
            sendIndexSD(client);
        } else {
            handleDownload(client, fileName);
        }

        delay(1);
        client.stop();
    }
}
