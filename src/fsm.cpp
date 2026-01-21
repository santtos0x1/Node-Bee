#include "http_server.h"
#include "data_logger.h"
#include "wifi_scan.h"
#include "indicator.h"
#include "watchdog.h"
#include "wardrive.h"
#include "bluetooth_scan.h"
#include "config.h"
#include "fsm.h"
#include "utils.h"

// Libs
#include <Arduino.h>
#include <WiFi.h>

State currentState;

int menuIndex = 0;

// Edge detection variables for physical buttons
bool btnALastState = HIGH;
bool btnBLastState = HIGH;


// Operational flag to define the current scanning technology
String scanMode;

// Status flag for the asynchronous HTTP service
bool serverStatus = false;
/**
 * Bootstraps the FSM and initializes all modular subsystems 
 * based on the global build configuration.
 */
void setupFSM()
{
    // Conditional initialization of hardware modules
    #if SYS_FEATURE_WIFI_SCAN
        setupWiFi();
    #endif
    #if SYS_FEATURE_BLE_STACK
        setupBluetooth();
    #endif    
    #if SYS_FEATURE_SD_STORAGE
        setupSD();
    #endif    
    #if SYS_FEATURE_WARDRIVE_SCAN
        setupWardrive();
    #endif    
    #if SYS_FEATURE_SERVER
        //serverStatus = startServer(); // Initialize the data access portal
        serverStatus = false;
    #endif

    // Core services and UI feedback initialization
    setupIndicator(Pins::LED_1);
    setupIndicator(Pins::LED_2);
    setupIndicator(Pins::LED_3);

    // Hardware Input configuration with internal pull-up resistors
    pinMode(Pins::BTN_A, INPUT_PULLUP);
    pinMode(Pins::BTN_B, INPUT_PULLUP); 

    // Initial state assignment
    currentState = IDLE;
}

/**
 * Main FSM Execution Loop: Manages state transitions, 
 * asynchronous scanning, and user input handling.
 */
void runFSM()
{
    // Capture instantaneous button states for transition triggers
    bool btnAPressed = (digitalRead(Pins::BTN_A) == LOW && btnALastState == HIGH);
    bool btnBPressed = (digitalRead(Pins::BTN_B) == LOW && btnBLastState == HIGH);

    switch(currentState)
    {
        /* IDLE STATE:
           Monitoring user input to select operational mode.
        */
        case IDLE:
        {
            idleState(Pins::LED_1, Pins::LED_2, Pins::LED_3);

            if (btnAPressed)
            {
                menuIndex++;
                if (menuIndex > 3) menuIndex = 0;
                
                if(menuIndex == 0)
                {
                    showOff(Pins::LED_1);
                    showOff(Pins::LED_2);
                } else if (menuIndex == 1)
                {
                    showOn(Pins::LED_1);
                } else if (menuIndex == 2)
                {
                    showOff(Pins::LED_1);
                    showOn(Pins::LED_2);
                } else if (menuIndex == 3)
                {
                    showOn(Pins::LED_1); 
                    showOn(Pins::LED_2);
                }

                DEBUG_PRINTF("Menu Option Selected: %s\n", GET_STATE(menuIndex));
                // showOn(Pins::LED_1); 
                // delay(Time::LOW_DELAY);
                // showOff(Pins::LED_1);
            }

            if (btnBPressed)
            {
                DEBUG_PRINTF("Confirming Mode: %s\n", GET_STATE(menuIndex));
                    
                if (menuIndex == 0) scanMode = "WF";
                else if (menuIndex == 1) scanMode = "BT";
                else if (menuIndex == 2) scanMode = "WD";
                else if (menuIndex == 3) scanMode = "WS";

                if(menuIndex == 0 || menuIndex == 1) currentState = SCAN;
                else currentState = (State)menuIndex; 
                
                showOff(Pins::LED_1);
                showOff(Pins::LED_2);

                showOn(Pins::LED_3); 
                delay(Time::LMID_DELAY);
                showOff(Pins::LED_3);
            }
            
            break;
        }

        /* SCAN STATE:
           Executes the sniffing routine based on the active scanMode.
        */
        case SCAN:
        {   
            #if SYS_FEATURE_SD_STORAGE
                // Verify storage health before attempting to write
                bool sdReport = SdHealthyChecker();            
                if(!sdReport)
                {
                    DEBUG_PRINTLN(F(CLR_RED "SD Health Check FAILED!" CLR_RESET));
                    showError(Pins::LED_2);
                    setupSD(); // Attempt hardware re-initialization
                    currentState = IDLE;
                    break;
                }
            #endif
            
            if(scanMode == "WF")
            {   
                showOn(Pins::LED_1);
                delay(Time::LMID_DELAY);
                showOff(Pins::LED_1);

                #if SYS_FEATURE_WIFI_SCAN
                    wifiSniffer(); // Start 802.11 packet capture
                    #if !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
                        processAllLogsSequential();
                    #endif    
                    showSuccess(Pins::LED_3);
                #endif

                currentState = IDLE;
                break;

            } else if(scanMode == "BT") {
                showOn(Pins::LED_1);
                delay(Time::LMID_DELAY);
                showOff(Pins::LED_1);

                #if SYS_FEATURE_BLE_STACK
                    BluetoothSniffer(); // Start BLE advertising discovery    
                    #if !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
                        processAllLogsSequential();
                    #endif
                    showSuccess(Pins::LED_3);
                #endif

                menuIndex = 0;
                currentState = IDLE;
                break;
            }

            break;
        }

        /* WEB_SERVER STATE:
           Enables the Access Point and HTTP server for file exfiltration.
        */
        case WEB_SERVER:
        {            
            #if SYS_FEATURE_SERVER
                if (!serverStatus) {
                    DEBUG_PRINTLN(F("Initializing Server on demand..."));
                    serverStatus = startServer(); 
                    
                    if (!serverStatus) {
                        DEBUG_PRINTLN(F(CLR_RED "Failed to connect to Home WiFi!" CLR_RESET));
                        menuIndex = 0;
                        currentState = IDLE;

                        showError(Pins::LED_2);
                        break;
                    }
                }
                
                // Exit condition
                if (!serverRun()) { 
                    DEBUG_PRINTLN("Exiting and shutting down WiFi...");
                    serverStatus = false;
                    menuIndex = 0; 
                    currentState = IDLE;
                    
                    showError(Pins::LED_2);
                    break;
                }

                digitalWrite(Pins::LED_3, (millis() % 2000 < 100));
            #else
                DEBUG_PRINTLN(F("Server Feature Disabled in Config!"));
                currentState = IDLE;
            #endif
            break;
        }

        /* WARDRIVE_MODE:
           Continuous capture mode combining WiFi signal strength.
        */
        case WARDRIVE_MODE:
        {
            // Exit condition - Check physical state for immediate response
            if(digitalRead(Pins::BTN_B) == LOW) 
            {
                DEBUG_PRINTLN(F(CLR_YELLOW "Exiting WARDRIVE_MODE..." CLR_RESET));
                
                WiFi.scanDelete(); 
                
                currentState = IDLE;
                showOff(Pins::LED_1);
                break;
            }

            #if SYS_FEATURE_WARDRIVE_SCAN
                bool openFound = startWardrive();
                showOn(Pins::LED_1);
                #if !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
                    processAllLogsSequential();
                #endif

                if(openFound)
                {
                    // Visual feedback for open networks found during wardrive
                    DEBUG_PRINTLN(F(CLR_GREEN "Open Network Found!" CLR_RESET));
                    showSuccess(Pins::LED_3); 
                }
            #endif
            break;   
        }
    }

    // Update edge detection buffers for the next cycle
    btnALastState = digitalRead(Pins::BTN_A);
    btnBLastState = digitalRead(Pins::BTN_B);
}