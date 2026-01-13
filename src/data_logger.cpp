// Local libs
#include "data_logger.h"
#include "config.h"

// Libs
#include <Arduino.h>
#include <SD.h>

WiFiData receivedWiFiData;
BTData receivedBTData;
WardriveData receivedWDData;

int session_id = 0;

String WiFiFolderSDPath = "/wifi_log_data";
String BTFolderSDPath = "/bluetooth_log_data";

void setupSD()
{
    DEBUG_PRINTLN("Starting the SD Card");
    bool startSD = SD.begin(Pins::SD_CS);
    while (!startSD)
    {
        delay(Time::MID_DELAY);
        DEBUG_PRINT(".");
    }

    SD.mkdir(WiFiFolderSDPath);
    SD.mkdir(BTFolderSDPath);

    DEBUG_PRINTLN("\nSucessfully started the SD Card!");

    DEBUG_PRINTLN("Reading the session file...");
    File rCounterData = SD.open("session.txt",FILE_READ);
    if(rCounterData)
    {
        String content = rCounterData.readString();
        session_id = content.toInt();
        rCounterData.close();
        DEBUG_PRINTLN("Done!");
    }

    session_id++;

    DEBUG_PRINTLN("Defining a new session in session file...");
    File wCounterData = SD.open("session.txt", "w");
    if(wCounterData)
    {
        wCounterData.print(session_id);
        wCounterData.close();
        DEBUG_PRINTLN("Done!");
    }
}

String WiFiFileName = "/wifi_log_data/wf_" + (String)session_id + ".csv";
String BTFileName = "/bluetooth_log_data/bt_" + (String)session_id + ".csv";
String WDFileName = "/wardrive_log_data/wd_" + (String)session_id + ".csv";

void logWiFiData()
{
    DEBUG_PRINTLN("Receiving data from the WiFi queue...");
    if (xQueueReceive(WiFiQueue, &receivedWiFiData, pdMS_TO_TICKS(100)))
    {
        DEBUG_PRINTLN("Done!");
        DEBUG_PRINTF("Creating file: '%s'", WiFiFileName);

        File dataFile = SD.open(WiFiFileName);

        if (dataFile)
        {
            DEBUG_PRINTLN("Writing data in file on SD...");
            dataFile.println("SSID, RSSI, BSSID, CHANNEL, IP, DHCP, ENCRYPTATION TYPE, HOSTNAME, DNS IP, SUBNET MASK");
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
            DEBUG_PRINTF("Sucessfully saved on %s\n", WiFiFileName);
        } else {
            DEBUG_PRINTF("Error opening %s\n", WiFiFileName);
        }

        if(session_id > RAM_FLUSH_LIM)
        {
            DEBUG_PRINTLN("RAM flush sucessfully done!");
            dataFile.flush();
        }
    }
}

void logBTData()
{
    DEBUG_PRINTLN("Receiving data from the Bluetooth queue...");
    if (xQueueReceive(BTQueue, &receivedBTData, pdMS_TO_TICKS(100)))
    {
        DEBUG_PRINTLN("Done!");
        DEBUG_PRINTF("Creating file: '%s'.", BTFileName);
        File dataFile = SD.open(BTFileName);
        
        if (dataFile)
        {
            DEBUG_PRINTLN("Writing data in file on SD...");
            dataFile.println("NAME, ADRESS, RSSI, ADDRESS TYPE, CHANELL");
            dataFile.printf("%s,%s,%d,%s,%d\n",
                            receivedBTData.name,
                            receivedBTData.address,
                            receivedBTData.rssi,
                            receivedBTData.addressType,
                            receivedBTData.channel);
            dataFile.close();
            DEBUG_PRINTF("Sucessfully saved on %s\n", BTFileName);
        } else {
            DEBUG_PRINTF("Error opening %s\n", BTFileName);
        }
        
        if(session_id > RAM_FLUSH_LIM)
        {
            DEBUG_PRINTLN("RAM flush sucessfully done!");
            dataFile.flush();
        }
    }
}

void logWDData()
{
    DEBUG_PRINTLN("Receiving data from the Bluetooth queue...");
    if (xQueueReceive(WDQueue, &receivedWDData, pdMS_TO_TICKS(100)))
    {
        File dataFile = SD.open(WDFileName);
        DEBUG_PRINTF("Creating file: '%s'.", WDFileName);
        if(dataFile)
        {
            DEBUG_PRINTLN("Writing data in file on SD...");
            dataFile.println("SSID, RSSI");
            dataFile.printf("%s, %d\n", receivedWDData.ssid, receivedWDData.rssi);
            dataFile.close();
            DEBUG_PRINTF("Sucessfully saved on %s\n", WDFileName);
        } else {
            DEBUG_PRINTF("Error opening %s\n", WDFileName);
        }
        
        if(session_id > RAM_FLUSH_LIM)
        {
            DEBUG_PRINTLN("RAM flush sucessfully done!");
            dataFile.flush();
        }
    }
}