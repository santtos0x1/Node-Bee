// Local libs
#include "fsm.h"
#include "bt_scan.h"
#include "wifi_scan.h"
#include "data_logger.h"
#include "watchdog.h"
#include "indicator.h"
#include "http_server.h"
#include "wardrive.h"
#include "config.h"

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
    #if ENABLE_WIFI
        setupWiFi();
    #endif
    #if ENABLE_BT
        setupBT();
    #endif    
    #if ENABLE_SD
        setupSD();
    #endif    
    setupIndicator(Pins::BUILT_IN_LED);
    serverStatus = startServer();
    setupWardrive();

    // Setting PINs
    pinMode(Pins::BTN_A, INPUT_PULLUP);
    pinMode(Pins::BTN_A, INPUT_PULLUP);
    pinMode(Pins::BTN_C, INPUT_PULLUP);

    // Start state
    currentState = IDLE;
}

void runFSM()
{
    bool btnAPressed = (digitalRead(Pins::BTN_A) == LOW && btnALastState == HIGH);
    bool btnBPressed = (digitalRead(Pins::BTN_B) == LOW && btnBLastState == HIGH);
    bool btnCPressed = (digitalRead(Pins::BTN_C) == LOW && btnCLastState == HIGH);

    switch(currentState)
    {
        case IDLE:
        {
            if(digitalRead(Pins::BTN_A) == LOW)
            {
                delay(Time::LOW_DELAY);
                unsigned long gap = millis() + 400;
                bool doubleClicked = false;

                while(digitalRead(Pins::BTN_A) == LOW);

                while(millis() < gap) {
                    if (digitalRead(Pins::BTN_A) == LOW) {
                        doubleClicked = true;
                        break;
                    }
                }

                if(doubleClicked)
                {
                    scanMode = "WD";
                    currentState = WARDRIVE_MODE;
                    showOn(Pins::BUILT_IN_LED);
                } else {
                    scanMode = "WF";
                    currentState = SCAN;
                }
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
        }

        case SCAN:
        {
            DEBUG_PRINTLN("Current FSM state: SCAN");
            #if ENABLE_SD
                bool SDReport = SDDoctor();            
                if(!SDReport)
                {
                    showError(Pins::BUILT_IN_LED);
                    //Try to restart the SD
                    setupSD();
                    currentState = IDLE;
                    break;
                }
            #endif
            
            if(scanMode == "WF")
            {
                #if ENABLE_WIFI
                    WiFiSniffer();
                    currentState = PROCESS;
                    showSuccess(Pins::BUILT_IN_LED);
                    break;
                #else
                    currentState = IDLE;
                    break;
                #endif

            } else if(scanMode == "BT") {
                #if ENABLE_BT
                    BTSniffer();
                    currentState = PROCESS;
                    showSuccess(Pins::BUILT_IN_LED);
                    break;
                #else
                    currentState = IDLE;
                    break;
                #endif
            }
        }
        
        case PROCESS:
        {
            DEBUG_PRINTLN("Current FSM state: PROCESS");

            if(scanMode == "WF")
            {
                #if ENABLE_SD
                    logWiFiData();
                    currentState = IDLE;
                    break;
                #else
                    currentState = IDLE;
                    break;
                #endif
            } else if (scanMode == "BT") {
                #if ENABLE_SD
                    logBTData();
                    currentState = IDLE;
                    break;
                #else
                    currentState = IDLE;
                    break;
                #endif
            }

            showProcessing(Pins::BUILT_IN_LED);
        }

        case WEB_SERVER:
        {
            DEBUG_PRINTLN("Current FSM state: WEB_SERVER");
            
            if(serverStatus && scanMode == "WS")
            {
                showSuccess(Pins::BUILT_IN_LED);
                serverCFG();
            } else {
                showError(Pins::BUILT_IN_LED);
            }

            currentState = IDLE;
            break;
        }

        case WARDRIVE_MODE:
        {
            startWardrive();
            logWDData();
        }
    }

    btnALastState = digitalRead(Pins::BTN_A);
    btnBLastState = digitalRead(BTN_B_PINOUT);
    btnCLastState = digitalRead(BTN_C_PINOUT);
}