#include "data_logger.h"
#include "config.h"
#include <Arduino.h>
#include <SD.h>

WiFiData receivedWiFiData;
BTData receivedBTData;

bool initSD()
{
    // init your SD here
}

void logWiFiData()
{
    if (xQueueReceive(wifiQueue, &receivedWiFiData, portMAX_DELAY))
    {
        // log information in SD here
    }
}

void logBTData()
{

}