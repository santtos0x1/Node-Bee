// Local Libs
#include "watchdog.h"

// Libs
#include <Arduino.h>
#include <SD.h>

bool SDDoctor()
{
    if(SD.cardType() == CARD_NONE)
    {
        Serial.println("Error: Card not found!");
        return false;
    }
    return true;
}