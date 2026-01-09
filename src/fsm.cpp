#include "fsm.h"
#include "bt_scan.h"
#include "wifi_scan.h"
#include "data_logger.h"
#include <Arduino.h>

#define BTN_A_PINOUT 14
#define BTN_B_PINOUT 16

bool btnALastState = HIGH;
bool btnBLastState = HIGH;

String scanMode = "";

void setupFSM()
{
    setupWiFi();
    setupBT();
    setupSD();

    pinMode(BTN_A_PINOUT, INPUT_PULLUP);
    pinMode(BTN_B_PINOUT, INPUT_PULLUP);

    currentState = IDLE;
}

void runFSM()
{
    bool btnAPressed = (digitalRead(BTN_A_PINOUT) == LOW && btnALastState == HIGH);
    bool btnBPressed = (digitalRead(BTN_B_PINOUT) == LOW && btnBLastState == HIGH);

    switch(currentState)
    {
        case IDLE:
            if(btnAPressed)
            {
                scanMode = "WF";
                currentState = SCAN;
            }
            else if(btnBPressed)
            {
                scanMode = "BT";
                currentState = SCAN;
            }
            break;

        case SCAN:
            if(scanMode == "WF")
            {
                WiFiSniffer();
            }
            else if(scanMode == "BT")
            {
                BTSniffer();
            }
            currentState = PROCESS;
            break;
        
        case PROCESS:
            if(scanMode == "WF") {
                logWiFiData();
            }
            else
            {
                logBTData();
            }
            currentState = IDLE;
            break;
    }
    btnALastState = digitalRead(BTN_A_PINOUT);
    btnBLastState = digitalRead(BTN_B_PINOUT);
}