// Local Libs
#include "watchdog.h"

// Libs
#include <Arduino.h>
#include <SD.h>

bool runWD()
{
    if(SD.cardType() == CARD_NONE)
    {
        Serial.prinln("Error: Card not found!")
        return false;
    }
    return true;
}