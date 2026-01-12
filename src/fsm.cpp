// Local libs
#include "fsm.h"
#include "bt_scan.h"
#include "wifi_scan.h"
#include "data_logger.h"
#include "watchdog.h"
#include "indicator.h"
#include "http_server.h"

// Libs
#include <Arduino.h>

bool btnALastState = HIGH;
bool btnBLastState = HIGH;
bool btnCLastState = HIGH;

String scanMode;

bool serverStatus = false;

void setupFSM()
{
    // Initing modules
    setupWiFi();
    setupBT();
    setupSD();
    setupIndicator(BUILT_IN_LED);
    serverStatus = startServer();

    // Setting PINs
    pinMode(BTN_A_PINOUT, INPUT_PULLUP);
    pinMode(BTN_B_PINOUT, INPUT_PULLUP);
    pinMode(BTN_C_PINOUT, INPUT_PULLUP);

    // Start state
    currentState = IDLE;
}

void runFSM()
{
    bool btnAPressed = (digitalRead(BTN_A_PINOUT) == LOW && btnALastState == HIGH);
    bool btnBPressed = (digitalRead(BTN_B_PINOUT) == LOW && btnBLastState == HIGH);
    bool btnCPressed = (digitalRead(BTN_C_PINOUT) == LOW && btnCLastState == HIGH);

    switch(currentState)
    {
        case IDLE:
            Serial.println("Current FSM state: IDLE");

            if(btnAPressed)
            {
                scanMode = "WF";
                currentState = SCAN;
                break;
            } else if(btnBPressed) {
                scanMode = "BT";
                currentState = SCAN;
                break;
            } else if(btnCPressed) {
                scanMode = "WS";
                currentState = WEB_SERVER;
                break;
            }

        case SCAN:
            Serial.println("Current FSM state: SCAN");

            bool SDReport = SDDoctor()

            if(!SDReport)
            {
                showError(BUILT_IN_LED);
                //Try to restart the SD
                setupSD();
                currentState = IDLE;
                break;
            }
            
            if(scanMode == "WF")
            {
                WiFiSniffer();
            } else if(scanMode == "BT") {
                BTSniffer();
            }

            showSuccess(BUILT_IN_LED);

            currentState = PROCESS;
            break;
        
        case PROCESS:
            Serial.println("Current FSM state: PROCESS");

            if(scanMode == "WF")
            {
                logWiFiData();
            } else if (scanMode == "BT") {
                logBTData();
            }

            showProcessing(BUILT_IN_LED);

            currentState = IDLE;
            break;

        case WEB_SERVER:
            Serial.println("Current FSM state: WEB_SERVER");
            
            if(serverStatus && scanMode == "WS")
            {
                showSuccess(BUILT_IN_LED);
                serverCFG();
            } else {
                showError(BUILT_IN_LED);
            }

            currentState = IDLE;
            break;
    }

    btnALastState = digitalRead(BTN_A_PINOUT);
    btnBLastState = digitalRead(BTN_B_PINOUT);
    btnCLastState = digitalRead(BTN_C_PINOUT);
}