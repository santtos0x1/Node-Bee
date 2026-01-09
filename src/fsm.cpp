#include "fsm.h"
#include "bt_scan.h"
#include "wifi_scan.h"
#include <Arduino.h>

#define BTN_A_PINOUT 14
#define BTN_B_PINOUT 16

bool btnALastState = HIGH;
bool btnBLastState = HIGH;

void setupFSM()
{
    setupWiFi();
    setupBT();

    pinMode(BTN_A_PINOUT, INPUT_PULLUP);
    pinMode(BTN_B_PINOUT, INPUT_PULLUP);

    currentState = IDLE;
}

void runFSM()
{
    bool btnACurrentState = (digitalRead(BTN_A_PINOUT) == LOW);
    bool btnBCurrentState = (digitalRead(BTN_B_PINOUT) == LOW);

    switch(currentState)
    {
        case IDLE:
            if(btnACurrentState && !btnALastState)
            {
                currentState = SCAN;
            }
            break;

        case SCAN:
            if(btnACurrentState && !btnALastState)
            {
                WiFiSniffer();
            }
            else if(btnBCurrentState && !btnBLastState)
            {
                BTSniffer();
            }
            currentState = PROCESS;
            break;
        
        case PROCESS:
            
            currentState = IDLE;
            break;
    }
    btnALastState = btnACurrentState;
}