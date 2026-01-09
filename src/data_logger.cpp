#include "data_logger.h"
#include "config.h"
#include <Arduino.h>
#include <SD.h>

WiFiData receivedWiFiData;
BTData receivedBTData;

bool initSD()
{
    Serial.println("Starting the SD Card");
    while(!SD.begin()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nSucessfully started the SD Card!");
}

void logWiFiData()  
{
    if (xQueueReceive(WiFiQueue, &receivedWiFiData, pdMS_TO_TICKS(100)))
    {
        // log information in SD here
    }
}

void logBTData()
{
    if (xQueueReceive(BTQueue, &receivedBTData, pdMS_TO_TICKS(100)))
    {
        // log information in SD here
    }
}