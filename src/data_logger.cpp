#include "data_logger.h"
#include "config.h"
#include <Arduino.h>
#include <Arduino_UUID.h>
#include <SD.h>

String WiFiFileName;
String BTFileName;

UUID sessionUUID;

WiFiData receivedWiFiData;
BTData receivedBTData;

bool setupSD()
{
    sessionUUID = UUID::generate();
    Serial.println("Starting the SD Card");
    while (!SD.begin())
    {
        delay(500);
        Serial.print(".");
    }
    SD.mkdir("/wifi_log_data");
    SD.mkdir("/bluetooth_log_data");
    Serial.println("\nSucessfully started the SD Card!");
}

WiFiFileName = "/wifi_log_data/wf_" + sessionUUID.toString() + ".csv";
BTFileName = "/bluetooth_log_data/bt_" + sessionUUID.toString() + ".csv";

void logWiFiData()
{
    if (xQueueReceive(WiFiQueue, &receivedWiFiData, pdMS_TO_TICKS(100)))
    {
        File dataFile = SD.open(WiFiFileName);
        if (dataFile)
        {
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
    }
}

void logBTData()
{
    if (xQueueReceive(BTQueue, &receivedBTData, pdMS_TO_TICKS(100)))
    {
        File dataFile = SD.open(BTFileName);
        if (dataFile)
        {
            dataFile.printf("%s,%s,%d,%s,%d\n",
                            receivedBTData.name,
                            receivedBTData.address,
                            receivedBTData.rssi,
                            receivedBTData.addressType,
                            receivedBTData.channel);
            dataFile.close();
            Serial.printf("Sucessfully saved on %s\n", logBTData);
        }
        else
        {
            Serial.printf("Error opening %s\n", BTFileName);
        }
    }
}