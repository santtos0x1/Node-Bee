// Local libs
#include "indicator.h"

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
        delay(200);
        digitalWrite(ledPinout, LOW);
    }
}

void showSucess(int ledPinout)
{
    for(int i = 0; i <= 3; i++)
    {
        digitalWrite(ledPinout, HIGH);
        delay(500);
        digitalWrite(ledPinout, LOW);
    }
}

void showError(int ledPinout)
{
    digitalWrite(ledPinout, HIGH); 
    delay(2000);
    digitalWrite(ledPinout, LOW); 
}