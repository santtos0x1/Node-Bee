// Local Libs
#include "watchdog.h"
#include "config.h"

// Libs
#include <Arduino.h>
#include <SD.h>

bool SDDoctor()
{
    DEBUG_PRINTLN("Starting diagnostic...");
    DEBUG_PRINTLN("Getting card type");
    sdcard_type_t cardType = SD.cardType();
    if(cardType == CARD_NONE)
    {
        DEBUG_PRINTLN("Error: Card not found!");
        return false;
    }
    DEBUG_PRINTLN("Status: Healthy");
    return true;
}