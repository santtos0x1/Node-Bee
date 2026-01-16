// Local libs
#include "data_logger.h"
#include "config.h"
#include <Arduino.h>
#include <SD.h>

// Persistence variable to track the current logging session
int session_id = 0;

// Base directory paths for data organization on the SD card
String WiFiFolderSDPath = "/wifi_log_data";
String BTFolderSDPath = "/bluetooth_log_data";
String WDFolderSDPath = "/wardrive_log_data";

// Function prototypes
void processWiFiLog(WiFiData data);
void processBTLog(BTData data);
void processWDLog(WardriveData data);
void logSDTask(void * pvParameters);

/**
 * Initializes the SD card module, ensures directory structure, 
 * and manages session ID persistence via a counter file.
 */
void setupSD()
{
    DEBUG_PRINTLN("Starting the SD Card");
    bool startSD = SD.begin(Pins::SD_CS);
    
    // Hardware handshake: Blocks execution until the SD card is detected
    while (!startSD)
    {
        delay(Time::MID_DELAY);
        DEBUG_PRINT(".");
        startSD = SD.begin(Pins::SD_CS); // Retry connection
    }

    // Ensure dedicated folders exist to maintain file system organization
    SD.mkdir(WiFiFolderSDPath);
    SD.mkdir(BTFolderSDPath);
    SD.mkdir(WDFolderSDPath);

    DEBUG_PRINTLN("\nSuccessfully started the SD Card!");

    /* Session Management: Reads the last session ID from 'session.txt' 
       to prevent file overwriting after a system reboot.
    */
    DEBUG_PRINTLN("Reading the session file...");
    File rCounterData = SD.open("/session.txt", FILE_READ);
    if(rCounterData)
    {
        String content = rCounterData.readString();
        session_id = content.toInt();
        rCounterData.close();
        DEBUG_PRINTLN("Done!");
    }

    // Increment session ID for the current power cycle
    session_id++;

    /* Updates the persistent counter file with the new session ID.
       Using "w" (FILE_WRITE) to overwrite the previous value.
    */
    DEBUG_PRINTLN("Defining a new session in session file...");
    File wCounterData = SD.open("/session.txt", FILE_WRITE);
    if(wCounterData)
    {
        wCounterData.print(session_id);
        wCounterData.close();
        DEBUG_PRINTLN("Done!");
    }

    xTaskCreatePinnedToCore(
        logSDTask,
        "SD_Task",
        8192,
        NULL,
        1,   
        NULL,
        0
    );
}

/**
 * Main SD Task running on Core 0.
 * Centralizes queue reception to ensure thread-safety for the SD card.
 */
void logSDTask(void * pvParameters)
{
    // Local buffers to hold data retrieved from queues
    WiFiData wifiData;
    BTData btData;
    WardriveData wdData;

    for(;;)
    {
        // Non-blocking check for WiFi Queue
        if (xQueueReceive(WiFiQueue, &wifiData, 0) == pdPASS) {
             DEBUG_PRINTLN("Receiving data from the WiFi queue...");
             processWiFiLog(wifiData);
             DEBUG_PRINTLN("Done!");
        }

        // Non-blocking check for Bluetooth Queue
        if (xQueueReceive(BTQueue, &btData, 0) == pdPASS) {
             DEBUG_PRINTLN("Receiving data from the Bluetooth queue...");
             processBTLog(btData);
             DEBUG_PRINTLN("Done!");
        }

        // Non-blocking check for Wardrive Queue
        if (xQueueReceive(WDQueue, &wdData, 0) == pdPASS) {
             DEBUG_PRINTLN("Receiving data from the Wardrive queue...");
             processWDLog(wdData);
             DEBUG_PRINTLN("Done!");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * Appends WiFi data to a CSV file.
 * Automatically generates a session-specific filename.
 */
void processWiFiLog(WiFiData data)
{
    // Dynamic filename generation using the current session ID
    String WiFiFileName = WiFiFolderSDPath + "/wf_" + String(session_id) + ".csv";
    
    DEBUG_PRINTF("Target file: '%s'\n", WiFiFileName.c_str());

    // Checks if file exists to determine if headers are needed
    bool fileExists = SD.exists(WiFiFileName);
    
    File dataFile = SD.open(WiFiFileName, FILE_APPEND);
    if (dataFile)
    {
        DEBUG_PRINTLN("Writing data to SD...");
        
        // Append CSV headers only to new files for data analysis compatibility
        if (!fileExists) {
            dataFile.println("SSID, RSSI, BSSID, CHANNEL, IP, DHCP, ENCRYPTION TYPE, HOSTNAME, DNS IP, SUBNET MASK");
        }
        
        dataFile.printf("%s,%d,%s,%d,%s,%s,%d,%s,%s,%s\n",
                        data.ssid,
                        data.rssi,
                        data.bssid,
                        data.channel,
                        data.localIP,
                        data.dhcp,
                        data.encryptionType,
                        data.hostname,
                        data.dnsIP,
                        data.subNetMask);
        
        dataFile.close();
        DEBUG_PRINTF("Successfully saved to %s\n", WiFiFileName.c_str());
    } 
    else {
        DEBUG_PRINTF("Error opening %s\n", WiFiFileName.c_str());
    }
}

/**
 * Appends Bluetooth data to the SD card.
 */
void processBTLog(BTData data)
{
    String BTFileName = BTFolderSDPath + "/bt_" + String(session_id) + ".csv";
    
    DEBUG_PRINTF("Target file: '%s'.\n", BTFileName.c_str());
    
    bool fileExists = SD.exists(BTFileName);
    File dataFile = SD.open(BTFileName, FILE_APPEND);
    
    if (dataFile)
    {
        DEBUG_PRINTLN("Appending Bluetooth data...");
        if (!fileExists) {
            dataFile.println("NAME, ADDRESS, RSSI, ADDRESS TYPE, CHANNEL");
        }

        dataFile.printf("%s,%s,%d,%s,%d\n",
                        data.name,
                        data.address,
                        data.rssi,
                        data.addressType,
                        data.channel);
        dataFile.close();
        DEBUG_PRINTF("Successfully saved to %s\n", BTFileName.c_str());
    } else {
        DEBUG_PRINTF("Error opening %s\n", BTFileName.c_str());
    }
}

/**
 * Appends Wardrive data to the SD card.
 */
void processWDLog(WardriveData data)
{
    String WDFileName = WDFolderSDPath + "/wd_" + String(session_id) + ".csv";
    
    DEBUG_PRINTF("Target file: '%s'.\n", WDFileName.c_str());
    
    bool fileExists = SD.exists(WDFileName);
    File dataFile = SD.open(WDFileName, FILE_APPEND);
    
    if(dataFile)
    {
        DEBUG_PRINTLN("Writing Wardrive log...");
        if (!fileExists) {
            dataFile.println("SSID, RSSI");
        }
        dataFile.printf("%s,%d\n", data.ssid, data.rssi);
        dataFile.close();
        DEBUG_PRINTF("Successfully saved to %s\n", WDFileName.c_str());
    } else {
        DEBUG_PRINTF("Error opening %s\n", WDFileName.c_str());
    }
}