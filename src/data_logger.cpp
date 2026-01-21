// Local libs
#include "data_logger.h"
#include "config.h"
#include "utils.h"

// Libs
#include <Arduino.h>
#include <SD.h>

// Persistence variable to track the current logging session
int session_id = 0;

// Base directory paths for data organization on the SD card
String WiFiFolderSDPath = "/wifi_log_data";
String BTFolderSDPath = "/bluetooth_log_data";
String WDFolderSDPath = "/wardrive_log_data";

/**
 * Initializes the SD card module, ensures directory structure, 
 * and manages session ID persistence via a counter file.
 */
void setupSD()
{
    DEBUG_PRINTLN(F(CLR_YELLOW "Starting the SD Card..." CLR_RESET));
    bool startSD = SD.begin(Pins::SD_CS);
    
    // Hardware handshake: Blocks execution until the SD card is detected
    while (!startSD)
    {
        delay(Time::MID_DELAY);
        DEBUG_PRINT(F(CLR_RED "." CLR_RESET));
        startSD = SD.begin(Pins::SD_CS); // Retry connection
    }

    // Ensure dedicated folders exist to maintain file system organization
    SD.mkdir(WiFiFolderSDPath);
    SD.mkdir(BTFolderSDPath);
    SD.mkdir(WDFolderSDPath);

    DEBUG_PRINTLN(F(CLR_GREEN "\nSuccessfully started the SD Card!" CLR_RESET));

    /* Session Management: Reads the last session ID from 'session.txt' 
       to prevent file overwriting after a system reboot.
    */
    DEBUG_PRINTLN(F(CLR_YELLOW "Reading the session file..." CLR_RESET));
    File rCounterData = SD.open("/session.txt", FILE_READ);
    if(rCounterData)
    {
        String content = rCounterData.readString();
        session_id = content.toInt();
        rCounterData.close();
        DEBUG_PRINTLN(F(CLR_GREEN "Done!" CLR_RESET));
    }

    // Increment session ID for the current power cycle
    session_id++;

    /* Updates the persistent counter file with the new session ID.
       Using "w" (FILE_WRITE) to overwrite the previous value.
    */
    DEBUG_PRINTLN(F(CLR_YELLOW "Defining a new session in session file..." CLR_RESET));
    File wCounterData = SD.open("/session.txt", FILE_WRITE);
    if(wCounterData)
    {
        wCounterData.print(session_id);
        wCounterData.close();
        DEBUG_PRINTLN(F(CLR_GREEN "Done!" CLR_RESET));
    }

    #if ASYNC_SD_HANDLER
        xTaskCreatePinnedToCore(
            logSDTask,
            "SD_Task",
            8192,
            NULL,
            1,   
            NULL,
            0
        );
        DEBUG_PRINTLN(F(CLR_GREEN "SD Async Task launched on Core 0." CLR_RESET));
    #endif
}

#if ASYNC_SD_HANDLER
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
                 DEBUG_PRINTLN(F(CLR_YELLOW "Receiving data from the WiFi queue..." CLR_RESET));
                 processWiFiLog(wifiData);
                 DEBUG_PRINTLN(F(CLR_GREEN "Done!" CLR_RESET));
            }

            // Non-blocking check for Bluetooth Queue
            if (xQueueReceive(BTQueue, &btData, 0) == pdPASS) {
                 DEBUG_PRINTLN(F(CLR_YELLOW "Receiving data from the Bluetooth queue..." CLR_RESET));
                 processBluetoothLog(btData);
                 DEBUG_PRINTLN(F(CLR_GREEN "Done!" CLR_RESET));
            }

            // Non-blocking check for Wardrive Queue
            if (xQueueReceive(WDQueue, &wdData, 0) == pdPASS) {
                 DEBUG_PRINTLN(F(CLR_YELLOW "Receiving data from the Wardrive queue..." CLR_RESET));
                 processWardriveLog(wdData);
                 DEBUG_PRINTLN(F(CLR_GREEN "Done!" CLR_RESET));
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
#else
    /**
     * Single Core Helper: Processes all pending logs in the queues.
     * This should be called periodically in the FSM or main loop.
     */
    void processAllLogsSequential()
    {
        WiFiData wifiData;
        BTData btData;
        WardriveData wdData;

        // Drain WiFi Queue
        while (xQueueReceive(WiFiQueue, &wifiData, 0) == pdPASS) {
            processWiFiLog(wifiData);
        }

        // Drain BT Queue
        while (xQueueReceive(BTQueue, &btData, 0) == pdPASS) {
            processBluetoothLog(btData);
        }

        // Drain Wardrive Queue
        while (xQueueReceive(WDQueue, &wdData, 0) == pdPASS) {
            processWardriveLog(wdData);
        }
    }
#endif

/**
 * Appends WiFi data to a CSV file.
 * Automatically generates a session-specific filename.
 */
void processWiFiLog(WiFiData data)
{
    // Dynamic filename generation using the current session ID
    String WiFiFileName = WiFiFolderSDPath + "/wf_" + String(session_id) + ".csv";
    
    DEBUG_PRINTF(CLR_YELLOW "Target file: '%s'\n" CLR_RESET, WiFiFileName.c_str());

    // Checks if file exists to determine if headers are needed
    bool fileExists = SD.exists(WiFiFileName);
    
    File dataFile = SD.open(WiFiFileName, FILE_APPEND);

    if (dataFile)
    {
        DEBUG_PRINTLN(F("Writing data to SD..."));
        
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
        DEBUG_PRINTF(CLR_GREEN "Successfully saved to %s\n" CLR_RESET, WiFiFileName.c_str());
    } 
    else {
        DEBUG_PRINTF(CLR_RED "Error opening %s\n" CLR_RESET, WiFiFileName.c_str());
    }
}

/**
 * Appends Bluetooth data to the SD card.
 */
void processBluetoothLog(BTData data)
{
    String BTFileName = BTFolderSDPath + "/bt_" + String(session_id) + ".csv";
    
    DEBUG_PRINTF(CLR_YELLOW "Target file: '%s'.\n" CLR_RESET, BTFileName.c_str());
    
    bool fileExists = SD.exists(BTFileName);
    File dataFile = SD.open(BTFileName, FILE_APPEND);
    
    if (dataFile)
    {
        DEBUG_PRINTLN(F("Appending Bluetooth data..."));
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
        DEBUG_PRINTF(CLR_GREEN "Successfully saved to %s\n" CLR_RESET, BTFileName.c_str());
    } else {
        DEBUG_PRINTF(CLR_RED "Error opening %s\n" CLR_RESET, BTFileName.c_str());
    }
}

/**
 * Appends Wardrive data to the SD card.
 */
void processWardriveLog(WardriveData data)
{
    String WDFileName = WDFolderSDPath + "/wd_" + String(session_id) + ".csv";
    
    DEBUG_PRINTF(CLR_YELLOW "Target file: '%s'.\n" CLR_RESET, WDFileName.c_str());
    
    bool fileExists = SD.exists(WDFileName);
    File dataFile = SD.open(WDFileName, FILE_APPEND);
    
    if(dataFile)
    {
        DEBUG_PRINTLN(F("Writing Wardrive log..."));
        if (!fileExists) {
            dataFile.println("SSID, RSSI");
        }
        dataFile.printf("%s,%d\n", data.ssid, data.rssi);
        dataFile.close();
        DEBUG_PRINTF(CLR_GREEN "Successfully saved to %s\n" CLR_RESET, WDFileName.c_str());
    } else {
        DEBUG_PRINTF(CLR_RED "Error opening %s\n" CLR_RESET, WDFileName.c_str());
    }
}