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

void setupSD()
{
    bool startSD = SD.begin(Pins::SD_CS);
    DEBUG_PRINTLN("Starting the SD Card");
    while (!startSD)
    {
        delay(Time::MID_DELAY);
        Serial.print(".");
    }

    SD.mkdir("/wifi_log_data");
    SD.mkdir("/bluetooth_log_data");

    DEBUG_PRINTLN("\nSucessfully started the SD Card!");

    File rCounterData = SD.open("c_tr.txt",FILE_READ);
    if(rCounterData)
    {
        String content = rCounterData.readString();
        session_id = content.toInt();
        rCounterData.close();
    }

    session_id++;

    File wCounterData = SD.open("c_tr.txt", "w");
    if(wCounterData)
    {
        wCounterData.print(session_id);
        wCounterData.close();
    }
}

String WiFiFileName = "/wifi_log_data/wf_" + (String)session_id + ".csv";
String BTFileName = "/bluetooth_log_data/bt_" + (String)session_id + ".csv";
String WDFileName = "/wardrive_log_data/wd_" + (String)session_id + ".csv";

void logWiFiData()
{
    if (xQueueReceive(WiFiQueue, &receivedWiFiData, pdMS_TO_TICKS(100)))
    {
        DEBUG_PRINTF("Creating file: '%s'", WiFiFileName);

        File dataFile = SD.open(WiFiFileName);

        if (dataFile)
        {
            dataFile.println("SSID, RSSI, BSSID, CHANNEL, IP, DHCP, ENCRYPTATION TYPE, HOSTNAME, DNS IP, SUBNET MASK");
            dataFile.printf("%s,%d,%s,%d,%s,%s,%d,%s,%s,%s\n",
                            receivedWiFiData.ssid,
                            receivedWiFiData.rssi,
                            receivedWiFiData.bssid,
                            receivedWiFiData.channel,
                            receivedWiFiData.localIP,
                            receivedWiFiData.dhcp,
                            receivedWiFiData.encryptationType,
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
    if (xQueueReceive(BTQueue, &receivedBTData, pdMS_TO_TICKS(100)))
    {
        File dataFile = SD.open(BTFileName);

        DEBUG_PRINTF("Creating file: '%s'.", BTFileName);
        
        dataFile.println("NAME, ADRESS, RSSI, ADDRESS TYPE, CHANELL");
        if (dataFile)
        {
            dataFile.printf("%s,%s,%d,%s,%d\n",
                            receivedBTData.name,
                            receivedBTData.address,
                            receivedBTData.rssi,
                            receivedBTData.addressType,
                            receivedBTData.channel);
            dataFile.close();
            DEBUG_PRINTLN("Data sucessfully saved");
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
    if (xQueueReceive(WDQueue, &receivedWDData, pdMS_TO_TICKS(100)))
    {
        File dataFile = SD.open(WDFileName);
        DEBUG_PRINTF("Creating file: '%s'.", WDFileName);
        if(dataFile)
        {
            dataFile.println("SSID, RSSI");
            dataFile.printf("%s, %d\n", receivedWDData.ssid, receivedWDData.rssi);
            dataFile.close();
            DEBUG_PRINTLN("Data sucessfully saved");
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