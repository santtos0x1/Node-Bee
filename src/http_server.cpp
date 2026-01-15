// Local libs
#include "http_server.h"
#include "data_logger.h"
#include "config.h"

// Libs
#include <Arduino.h>
#include <WiFi.h>   
#include <SPI.h>
#include <SD.h>

// Instantiates the WiFi server on the port defined in config.h
WiFiServer server(WEB_SERVER_PORT);

// Network credentials for manual connection mode
const char* ssid = "";
const char* pass = "";
const bool manualWiFiConnection = false;

/**
   Scans for available networks and attempts to establish an HTTP server.
   Priority is given to open networks (No Auth) for field accessibility.
 */
bool startServer() 
{
    // Ensure the radio is in a clean state before starting the scan
    DEBUG_PRINTLN("Resetting WiFi connection...");
    WiFi.disconnect();
    delay(Time::LOW_DELAY);

    // Perform a synchronous scan of nearby Access Points
    DEBUG_PRINTLN("Scanning for available networks...");
    int networks = WiFi.scanNetworks();

    for (int i = 0; i < networks; i++)
    {
        // Check the security protocol of the detected network
        wifi_auth_mode_t encryptionType = WiFi.encryptionType(i);
        int32_t dbm = WiFi.RSSI(i);
        if(encryptionType == WIFI_AUTH_OPEN && dbm >= -65)
        {
            DEBUG_PRINTLN("Open network identified!");
            DEBUG_PRINTF("Attempting connection to SSID: %s\n", WiFi.SSID(i).c_str());
            
            // Logic switch between hardcoded credentials and open connection
            if(!manualWiFiConnection)
            {
                DEBUG_PRINTLN("Mode: Automatic Association");
                WiFi.begin(WiFi.SSID(i).c_str());
            } else {
                DEBUG_PRINTLN("Mode: Manual Configuration");
                WiFi.begin(ssid, pass);
            }

            // Connection timeout loop to prevent infinite blocking
            int attempts = 0;
            wl_status_t connStatus = WiFi.status();

            while (connStatus != WL_CONNECTED && attempts < SERVER_ATTEMPTS_LIMIT) 
            {
                delay(Time::MID_DELAY);
                DEBUG_PRINT(".");
                attempts++;
                connStatus = WiFi.status(); // Update status in each iteration
            }

            if(connStatus == WL_CONNECTED)
            {
                DEBUG_PRINTLN("Connection established successfully!");
                break; // Exit scan loop once connected
            } else {
                DEBUG_PRINTLN("Failed to associate with this network.");
            }
        }
    }

    // Final check to initiate the HTTP service
    if(WiFi.status() == WL_CONNECTED)
    {
        DEBUG_PRINTLN("Initializing HTTP Listener...");
        server.begin();
        DEBUG_PRINTLN("Server active!");
        DEBUG_PRINT("Local IP Address: ");
        DEBUG_PRINTLN(WiFi.localIP());
        return true;
    } else {
        DEBUG_PRINTLN("Server startup failed: No valid connection.");
        return false;
    }
}

/**
   Streams a file from the SD card to the connected HTTP client.
   Uses a buffered approach to handle large CSV files efficiently.
 */
void handleDownload(WiFiClient& client, String path)
{
    DEBUG_PRINTF("Verifying path: %s...", path.c_str());
    if (SD.exists(path))
    {
        DEBUG_PRINTLN("File found.");
        File dataFile = SD.open(path);

        // Constructing standard HTTP headers for binary file transfer
        DEBUG_PRINTLN("Sending HTTP 200 Headers...");
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/octet-stream"); // Triggers browser download
        client.print("Content-Length: ");
        client.println(dataFile.size());
        client.println("Connection: close");
        client.println();

        // Data streaming loop using a memory-efficient buffer
        uint8_t buffer[HND_BUFFER_SIZE];
        DEBUG_PRINTLN("Streaming data payload...");
        while (dataFile.available())
        {
            int bytesRead = dataFile.read(buffer, sizeof(buffer));
            client.write(buffer, bytesRead);
        }
        
        dataFile.close();
        DEBUG_PRINTLN("Transfer complete. File closed.");
    } else {
        // Standard 404 response for non-existent log files
        client.println("HTTP/1.1 404 Not Found\r\n\r\nError: File not found on SD.");
    }
}

/**
   Generates a dynamic HTML index page listing all captured logs.
   Iterates through SD directories to build interactive download links.
 */
void sendIndexSD(WiFiClient& client)
{
    // Send basic HTML document headers
    DEBUG_PRINTLN("Generating Index Page...");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println();

    client.println("<html><body>");
    
    // --- WiFi Logs Section ---
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

    // --- Bluetooth Logs Section ---
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
}    

/**
   Main server runtime loop. Listens for incoming TCP connections 
   and parses HTTP GET requests to serve files or the index.
 */
void serverRun()
{   
    // Check for an active incoming client connection
    WiFiClient client = server.available();
    
    if(client)
    {
        DEBUG_PRINTLN("New client connection established.");
        
        // Simple HTTP GET request parsing
        String request = client.readStringUntil('\r');
        client.readStringUntil('\n');

        // Extract the requested path/filename from the HTTP header
        int addrStart = request.indexOf('/');
        int addrEnd = request.indexOf(' ', addrStart);
        String fileName = request.substring(addrStart, addrEnd);

        if(fileName == "/")
        {
            // Serve the root directory listing
            sendIndexSD(client);
        } else {
            // Serve specific file download
            handleDownload(client, fileName);
        }

        // Standard delay for connection stabilization before closing
        delay(Time::LOW_DELAY);
        client.stop();
        DEBUG_PRINTLN("Client session terminated.");
    }
}