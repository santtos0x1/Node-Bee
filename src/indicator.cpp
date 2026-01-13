// Local libs
#include "indicator.h"
#include "config.h"

// Libs
#include <Arduino.h>

void setupIndicator(int ledPinout)
{
    pinMode(ledPinout, OUTPUT);
}

void showProcessing(int ledPinout)
{
    for(int i = 0; i <= 5; i++)
    {
        digitalWrite(ledPinout, HIGH);
        delay(Time::LMID_DELAY);
        digitalWrite(ledPinout, LOW);
    }
}

void showSucess(int ledPinout)
{
    for(int i = 0; i <= 3; i++)
    {
        digitalWrite(ledPinout, HIGH);
        delay(Time::MID_DELAY);
        digitalWrite(ledPinout, LOW);
    }
}

void showError(int ledPinout)
{
    digitalWrite(ledPinout, HIGH); 
    delay(Time::HIGH_DELAY);
    digitalWrite(ledPinout, LOW); 
}

void showOn(int ledPinout)
{
    digitalWrite(ledPinout, HIGH); 
}

void showOff(int ledPinout)
{
    digitalWrite(ledPinout, LOW); 
}