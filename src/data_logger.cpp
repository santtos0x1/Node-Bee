// Local libs
#include "data_logger.h"
#include "config.h"

// Libs
#include <Arduino.h>
#include <Arduino_UUID.h>
#include <SD.h>

#define RAM_FLUSH_LIM 5

String WiFiFileName;
String BTFileName;

WiFiData receivedWiFiData;
BTData receivedBTData;

int session_id = 0;

void setupSD()
{
    Serial.println("Starting the SD Card");
    while (!SD.begin(5))
    {
        delay(500);
        Serial.print(".");
    }
    
    SD.mkdir("/wifi_log_data");
    SD.mkdir("/bluetooth_log_data");

    Serial.println("\nSucessfully started the SD Card!");

    File rCounterData = SD.open("c_tr.txt",FILE_READ);
    if(rCounterData)
    {
        String content = rCounterData.readString();
        session_id = content.toInt();
        rCounterData.close();
    }

    session_id++;

    File wCounterData = SD.open("c_tr.txt", FILE_WRITE | O_TRUNC);
    if(wCounterData)
    {
        wCounterData.print(session_id);
        wCounterData.close()
    }
}

WiFiFileName = "/wifi_log_data/wf_" + (String)session_id + ".csv";
BTFileName = "/bluetooth_log_data/bt_" + (String)session_id + ".csv";

void logWiFiData()
{
    if (xQueueReceive(WiFiQueue, &receivedWiFiData, pdMS_TO_TICKS(100)))
    {
        Serial.printf("Creating file: '%s'", WiFiFileName)

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
            Serial.printf("Sucessfully saved on %s\n", logWiFiData);
        }
        else
        {
            Serial.printf("Error opening %s\n", WiFiFileName);
        }
        if(session_id > RAM_FLUSH_LIM)
        {
            Serial.println("RAM flush sucessfully done!");
            dataFile.flush();
        }
    }
}

void logBTData()
{
    if (xQueueReceive(BTQueue, &receivedBTData, pdMS_TO_TICKS(100)))
    {
        File dataFile = SD.open(BTFileName);

        Serial.printf("Creating file: '%s'.", BTFileName);
        
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
            Serial.println("Data sucessfully saved");
        }
        else
        {
            Serial.printf("Error opening %s\n", BTFileName);
        }
        if(session_id > RAM_FLUSH_LIM)
        {
            Serial.println("RAM flush sucessfully done!");
            dataFile.flush();
        }
    }
}