// Local libs
#include "indicator.h"
#include "config.h"

// Libs
#include <Arduino.h>

/**
   Configures the LED GPIO pin as a digital output.
   Essential step to allow current control for the visual indicator.
 */
void setupIndicator(int ledPinout)
{
    pinMode(ledPinout, OUTPUT);
}

/**
   Visual pattern: Rapid Blinking.
   Used to indicate active processing or busy states (e.g., writing to SD).
   Blocking function: The CPU waits here during the visual effect.
 */
void showProcessing(int ledPinout)
{
    for(int i = 0; i <= 5; i++)
    {
        digitalWrite(ledPinout, HIGH);
        delay(Time::LMID_DELAY);
        digitalWrite(ledPinout, LOW);
        delay(Time::LMID_DELAY);
    }
}

/**
   Visual pattern: Moderate Blinking.
   Signals the successful completion of a task (e.g., Scan finished, File Saved).
 */
void showSuccess(int ledPinout)
{
    for(int i = 0; i <= 3; i++)
    {
        digitalWrite(ledPinout, HIGH);
        delay(Time::MID_DELAY);
        digitalWrite(ledPinout, LOW);
        delay(Time::MID_DELAY);
    }
}

/**
   Visual pattern: Long Solid Pulse.
   Signals critical failures (e.g., SD Card missing, Wi-Fi Error).
 */
void showError(int ledPinout)
{
    digitalWrite(ledPinout, HIGH); 
    delay(Time::HIGH_DELAY);
    digitalWrite(ledPinout, LOW); 
}

/**
   Manual Control: Forces the indicator ON.
   Useful for debugging or indicating a specific steady state (e.g., Wardriving Mode active).
 */
void showOn(int ledPinout)
{
    digitalWrite(ledPinout, HIGH); 
}

/**
   Manual Control: Forces the indicator OFF.
   Used to clear status or save power.
 */
void showOff(int ledPinout)
{
    digitalWrite(ledPinout, LOW); 
}

unsigned long lastIdleUpdate = 0;
int idleStep = 0;
bool isIdleActive = true;

void idleState(int ledPinout1, int ledPinout2)
{
    unsigned long currentMillis = millis();
    if(currentMillis - lastIdleUpdate >= 200)
    {
        if (!isIdleActive) return;
        lastIdleUpdate = currentMillis;
        idleStep++;
        switch(idleStep) {
            case 1: 
                digitalWrite(ledPinout1, HIGH); 
                digitalWrite(ledPinout2, LOW); 
                break;
            case 2: 
                digitalWrite(ledPinout1, LOW); 
                digitalWrite(ledPinout2, HIGH); 
                break;
            case 3: 
                digitalWrite(ledPinout1, HIGH); 
                digitalWrite(ledPinout2, LOW); 
                break;
            default: 
                digitalWrite(ledPinout1, LOW); 
                digitalWrite(ledPinout2, LOW);
                idleStep = 0;
                isIdleActive = false;
                break;
        }
    }
}