// Local libs
#include "http_server.h"
#include "config.h"
#include "utils.h"

// Libs
#include <Arduino.h>
#include <WiFi.h>   
#include <SPI.h>
#include <SD.h>

// Instantiates the WiFi server on the port defined in config.h
WiFiServer server(WEB_SERVER_PORT);

// Network credentials
const char* ssid= "Main C";
const char* pass = "";

bool startServer() 
{
    WiFi.persistent(false);
    // Resets WiFi radio state to Station Mode
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    delay(Time::LOW_DELAY);

    DEBUG_PRINT(F(CLR_YELLOW "Connecting to WiFi" CLR_RESET));
    WiFi.begin(ssid, pass);

    int8_t counter = 0;
    while(WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINT(F("."));
        delay(Time::LOW_DELAY);
        counter++;
        if(counter == 50) break;
    };
    
    if(WiFi.status() == WL_CONNECTED)
    {
        DEBUG_PRINTLN(F(CLR_GREEN "\nSupa!" CLR_RESET));
        DEBUG_PRINTLN(F(CLR_YELLOW "Initializing listener..." CLR_RESET));
        server.begin();
        DEBUG_PRINTLN(F(CLR_GREEN "Server active!" CLR_RESET));
        DEBUG_PRINT(F(CLR_YELLOW "Local IP Address: " CLR_RESET));
        DEBUG_PRINTLN(WiFi.localIP());
        return true;
    } else {
        DEBUG_PRINTLN(F(CLR_RED "\nFailed to connect to the network" CLR_RESET));
        DEBUG_PRINTLN(F(CLR_RED "Server startup failed: No valid connection." CLR_RESET));
        return false;
    }
}

/**
   Streams a file from the SD card to the connected HTTP client.
 */
void handleDownload(WiFiClient& client, String path)
{
    // String de formato no PRINTF corrigida
    DEBUG_PRINTF(F(CLR_YELLOW "Verifying path: %s..." CLR_RESET), path.c_str());
    
    if (SD.exists(path))
    {
        DEBUG_PRINTLN(F(CLR_GREEN "File found." CLR_RESET));
        File dataFile = SD.open(path);

        DEBUG_PRINTLN(F(CLR_YELLOW "Sending HTTP 200 Headers..." CLR_RESET));
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/octet-stream");
        client.print("Content-Length: ");
        client.println(dataFile.size());
        client.println("Connection: close");
        client.println();

        uint8_t buffer[HANDLER_BUFFER_SIZE];
        DEBUG_PRINTLN(F(CLR_YELLOW "Streaming data payload..." CLR_RESET));
        while (dataFile.available())
        {
            int bytesRead = dataFile.read(buffer, sizeof(buffer));
            client.write(buffer, bytesRead);
        }
        
        dataFile.close();
        DEBUG_PRINTLN(F(CLR_GREEN "Transfer complete. File closed." CLR_RESET));
    } else {
        DEBUG_PRINTLN(F(CLR_RED "File NOT found on SD card." CLR_RESET));
        client.println("HTTP/1.1 404 Not Found\r\n\r\nError: File not found on SD.");
    }
}

/**
   Generates a dynamic HTML index page listing all captured logs.
 */
void sendIndexSD(WiFiClient& client)
{
    DEBUG_PRINTLN(F(CLR_YELLOW "Generating Index Page..." CLR_RESET));
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println();

    client.println("<html><body>");
    
    client.println("<h1>WiFi Logs Inventory</h1><ul>");
    File WFRoot = SD.open("/wifi_log_data/");
    File WFFile = WFRoot.openNextFile();
    
    while (WFFile)
    {
        String fileName = WFFile.name();
        client.print("<li><a href=\"/wifi_log_data/");
        client.print(fileName);
        client.print("\">");
        client.print(fileName);
        client.println("</a></li>");
        WFFile = WFRoot.openNextFile();
    }
    WFRoot.close();
    client.println("</ul><hr>");

    client.println("<h1>Bluetooth Logs Inventory</h1><ul>");
    File BTRoot = SD.open("/bluetooth_log_data/");
    File BTFile = BTRoot.openNextFile();

    while (BTFile)
    {
        String fileName = BTFile.name();
        client.print("<li><a href=\"/bluetooth_log_data/");
        client.print(fileName);
        client.print("\">");
        client.print(fileName);
        client.println("</a></li>");
        BTFile = BTRoot.openNextFile();
    }
    BTRoot.close();
    
    client.println("</ul></body></html>");
    DEBUG_PRINTLN(F(CLR_GREEN "Index page sent successfully." CLR_RESET));
}    

/**
   Main server runtime loop.
 */
bool serverRun()
{   
    WiFiClient client = server.available();
    
    if (digitalRead(Pins::BTN_B) == LOW) {
        WiFi.mode(WIFI_OFF);
        client.stop();
        WiFi.disconnect(true);
        return false;
    }

    if(client)
    {
        long timeout = millis();
        while(!client.available() && millis() - timeout < 500)
        {
            delay(Time::HIGH_DELAY);void allLeds(int state);
        }

        if(!client.available()) {
            client.stop();
            return false;
        }

        DEBUG_PRINTLN(F(CLR_GREEN "New client connection established." CLR_RESET));
        
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

        delay(Time::LOW_DELAY);
        client.stop();
        DEBUG_PRINTLN(F(CLR_YELLOW "Client session terminated." CLR_RESET));
        WiFi.disconnect();
    }
    return true;
}