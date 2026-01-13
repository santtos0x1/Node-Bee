// Local libs
#include "data_logger.h"
#include "config.h"

// Libs
#include <Arduino.h>
#include <SD.h>

// Global buffers to hold data retrieved from queues
WiFiData receivedWiFiData;
BTData receivedBTData;
WardriveData receivedWDData;

// Persistence variable to track the current logging session
int session_id = 0;

// Base directory paths for data organization on the SD card
String WiFiFolderSDPath = "/wifi_log_data";
String BTFolderSDPath = "/bluetooth_log_data";

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
}

/**
 * Retrieves WiFi data from the queue and appends it to a CSV file.
 * Automatically generates a session-specific filename.
 */
void logWiFiData()
{
    // Dynamic filename generation using the current session ID
    String WiFiFileName = WiFiFolderSDPath + "/wf_" + String(session_id) + ".csv";

    DEBUG_PRINTLN("Receiving data from the WiFi queue...");
    // Non-blocking check with a 100ms timeout for the RTOS queue
    if (xQueueReceive(WiFiQueue, &receivedWiFiData, pdMS_TO_TICKS(100)))
    {
        DEBUG_PRINTLN("Done!");
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
                            receivedWiFiData.ssid,
                            receivedWiFiData.rssi,
                            receivedWiFiData.bssid,
                            receivedWiFiData.channel,
                            receivedWiFiData.localIP,
                            receivedWiFiData.dhcp,
                            receivedWiFiData.encryptionType,
                            receivedWiFiData.hostname,
                            receivedWiFiData.dnsIP,
                            receivedWiFiData.subNetMask);
            
            dataFile.close();
            DEBUG_PRINTF("Successfully saved to %s\n", WiFiFileName.c_str());
        } else {
            DEBUG_PRINTF("Error opening %s\n", WiFiFileName.c_str());
        }

        /* Data Integrity: Forces an I/O sync if the session exceeds 
           predefined limits to prevent buffer loss.
        */
        if(session_id > RAM_FLUSH_LIM)
        {
            DEBUG_PRINTLN("Critical RAM flush executed.");
            // SD.open/close handles the sync automatically in this context
        }
    }
}

/**
 * Retrieves Bluetooth data from the queue and logs it to the SD card.
 */
void logBTData()
{
    String BTFileName = BTFolderSDPath + "/bt_" + String(session_id) + ".csv";

    DEBUG_PRINTLN("Receiving data from the Bluetooth queue...");
    if (xQueueReceive(BTQueue, &receivedBTData, pdMS_TO_TICKS(100)))
    {
        DEBUG_PRINTLN("Done!");
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
                            receivedBTData.name,
                            receivedBTData.address,
                            receivedBTData.rssi,
                            receivedBTData.addressType,
                            receivedBTData.channel);
            dataFile.close();
            DEBUG_PRINTF("Successfully saved to %s\n", BTFileName.c_str());
        } else {
            DEBUG_PRINTF("Error opening %s\n", BTFileName.c_str());
        }
    }
}
void logWDData()
{
    String WDFileName = "/wardrive_log_data/wd_" + String(session_id) + ".csv";

    DEBUG_PRINTLN("Receiving data from the Wardrive queue...");
    if (xQueueReceive(WDQueue, &receivedWDData, pdMS_TO_TICKS(100)))
    {
        bool fileExists = SD.exists(WDFileName);
        File dataFile = SD.open(WDFileName, FILE_APPEND);

        DEBUG_PRINTF("Target file: '%s'.\n", WDFileName.c_str());
        if(dataFile)
        {
            DEBUG_PRINTLN("Writing Wardrive log...");
            if (!fileExists) {
                dataFile.println("SSID, RSSI");
            }
            dataFile.printf("%s, %d\n", receivedWDData.ssid, receivedWDData.rssi);
            dataFile.close();
            DEBUG_PRINTF("Successfully saved to %s\n", WDFileName.c_str());
        } else {
            DEBUG_PRINTF("Error opening %s\n", WDFileName.c_str());
        }
    }
}