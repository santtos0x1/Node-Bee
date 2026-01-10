// Local libs
#include "fsm.h"
#include "bt_scan.h"
#include "wifi_scan.h"
#include "data_logger.h"
#include "watchdog.h"

// Libs
#include <Arduino.h>

#define BTN_A_PINOUT 14
#define BTN_B_PINOUT 16
#define BUILT_IN_LED 2

bool btnALastState = HIGH;
bool btnBLastState = HIGH;

String scanMode;

void setupFSM()
{
    // Initing modules
    setupWiFi();
    setupBT();
    setupSD();

    // Setting PINs
    pinMode(BTN_A_PINOUT, INPUT_PULLUP);
    pinMode(BTN_B_PINOUT, INPUT_PULLUP);
    pinMode(BUILT_IN_LED, OUTPUT);

    // Start state
    currentState = IDLE;
}

void runFSM()
{
    bool btnAPressed = (digitalRead(BTN_A_PINOUT) == LOW && btnALastState == HIGH);
    bool btnBPressed = (digitalRead(BTN_B_PINOUT) == LOW && btnBLastState == HIGH);

    switch(currentState)
    {
        case IDLE:
            Serial.println("Current FSM state: IDLE")

            if(btnAPressed) scanMode = "WF";
            else if(btnBPressed) scanMode = "BT";

            currentState = SCAN;
            break;

        case SCAN:
            Serial.println("Current FSM state: SCAN");

            if(!runWD)
            {
                digitalWrite(BUILT_IN_LED, HIGH); 
                delay(2000);
                digitalWrite(BUILT_IN_LED, LOW); 
                setupSD();
                currentState = IDLE;
                break;
            }
            
            digitalWrite(BUILT_IN_LED, HIGH);
            
            if(scanMode == "WF") WiFiSniffer();
            else if(scanMode == "BT") BTSniffer();

            for(int i = 0; i < 5; i++)
            {
                digitalWrite(BUILT_IN_LED, HIGH);
                delay(200);
                digitalWrite(BUILT_IN_LED, LOW);            
            }

            currentState = PROCESS;
            break;
        
        case PROCESS:
            Serial.println("Current FSM state: PROCESS");

            if(scanMode == "WF") logWiFiData();
            else if (scanMode == "BT") logBTData();

            for(int i = 0; i <= 2; i++)
            {
                digitalWrite(BUILT_IN_LED, HIGH);
                delay(400);
                digitalWrite(BUILT_IN_LED, LOW);
            }
            currentState = IDLE;
            break;
    }

    btnALastState = digitalRead(BTN_A_PINOUT);
    btnBLastState = digitalRead(BTN_B_PINOUT);
}