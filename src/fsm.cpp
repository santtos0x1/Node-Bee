// Local libs
#include "http_server.h"
#include "data_logger.h"
#include "wifi_scan.h"
#include "indicator.h"
#include "watchdog.h"
#include "wardrive.h"
#include "bt_scan.h"
#include "config.h"
#include "fsm.h"
#include "utils.h"

// Libs
#include <Arduino.h>

// Edge detection variables for physical buttons (Debouncing)
bool btnALastState = HIGH;
bool btnBLastState = HIGH;
bool btnCLastState = HIGH;

// Operational flag to define the current scanning technology (WiFi, BT, or Wardrive)
String scanMode;

// Status flag for the asynchronous HTTP service
bool serverStatus = false;
uint8_t systemState = 0;

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
    
    // Core services and UI feedback initialization
    setupIndicator(Pins::BUILT_IN_LED);
    serverStatus = startServer(); // Initialize the data access portal
    setupWardrive();

    // Hardware Input configuration with internal pull-up resistors
    pinMode(Pins::BTN_A, INPUT_PULLUP);
    pinMode(Pins::BTN_B, INPUT_PULLUP); 
    pinMode(Pins::BTN_C, INPUT_PULLUP);

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
    bool btnCPressed = (digitalRead(Pins::BTN_C) == LOW && btnCLastState == HIGH);

    switch(currentState)
    {
        /* IDLE STATE:
           Monitoring user input to select operational mode.
        */
        case IDLE:
        {
            if(digitalRead(Pins::BTN_A) == LOW)
            {
                // Double-click detection logic for mode switching
                delay(Time::LOW_DELAY);
                unsigned long gap = millis() + 400; // 400ms detection window
                bool doubleClicked = false;

                while(digitalRead(Pins::BTN_A) == LOW); // Wait for release of first click

                while(millis() < gap) {
                    if (digitalRead(Pins::BTN_A) == LOW) {
                        doubleClicked = true;
                        break;
                    }
                }

                if(doubleClicked)
                {
                    scanMode = "WD"; // Enter Wardriving mode (Signal)
                    currentState = WARDRIVE_MODE;
                    showOn(Pins::BUILT_IN_LED);
                } else {
                    scanMode = "WF"; // Enter standard WiFi Sniffer mode
                    currentState = SCAN;
                }
                break;
            } else if(btnBPressed) {
                scanMode = "BT"; // Enter Bluetooth Sniffer mode
                currentState = SCAN;
                break;
            } else if(btnCPressed) {
                scanMode = "WS"; // Enter Web server mode
                currentState = WEB_SERVER;
                break;
            } 
            break;
        }

        /* SCAN STATE:
           Executes the sniffing routine based on the active scanMode.
        */
        case SCAN:
        {
            DEBUG_PRINTLN(F(CLR_YELLOW "Current FSM state: SCAN" CLR_RESET));
            
            #if SYS_FEATURE_SD_STORAGE
                // Verify storage health before attempting to write
                bool sdReport = SdHealthyChecker();            
                if(!sdReport)
                {
                    DEBUG_PRINTLN(F(CLR_RED "SD Health Check FAILED!" CLR_RESET));
                    showError(Pins::BUILT_IN_LED);
                    setupSD(); // Attempt hardware re-initialization
                    currentState = IDLE;
                    break;
                }
            #endif
            
            if(scanMode == "WF")
            {   
                #if SYS_FEATURE_WIFI_SCAN
                    wifiSniffer(); // Start 802.11 packet capture
                    #if !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
                        processAllLogsSequential();
                    #endif    
                    showSuccess(Pins::BUILT_IN_LED);
                #endif

                currentState = IDLE;
                break;

            } else if(scanMode == "BT") {
                #if SYS_FEATURE_BLE_STACK
                    BluetoothSniffer(); // Start BLE advertising discovery    
                    #if !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
                        processAllLogsSequential();
                    #endif
                    showSuccess(Pins::BUILT_IN_LED);
                #endif

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
            DEBUG_PRINTLN(F(CLR_YELLOW "Current FSM state: WEB_SERVER" CLR_RESET));
            
            if(serverStatus && scanMode == "WS")
            {
                showSuccess(Pins::BUILT_IN_LED);
                serverRun(); // Maintain server runtime configurations and connections
                
                // Exit condition for Server Mode
                if(btnBPressed)
                {
                    showError(Pins::BUILT_IN_LED);

                    currentState = IDLE;    
                    break;
                }
            } else {
                DEBUG_PRINTLN(F(CLR_RED "Server Error or Invalid Mode!" CLR_RESET));
                showError(Pins::BUILT_IN_LED);

                currentState = IDLE;
                break;
            }
            
            break;
        }

        /* WARDRIVE_MODE:
           Continuous capture mode combining WiFi signal strength.
        */
        case WARDRIVE_MODE:
        {
            // Exit condition
            if(btnBPressed) 
            {
                DEBUG_PRINTLN(F(CLR_YELLOW "Exiting WARDRIVE_MODE..." CLR_RESET));

                currentState = IDLE;
                break;
            }

            bool openFound = startWardrive();
            #if !ASYNC_SD_HANDLER && SYS_FEATURE_SD_STORAGE
                processAllLogsSequential();
            #endif
            
            if(openFound) {
                // Visual feedback for open networks found during wardrive
                DEBUG_PRINTLN(F(CLR_GREEN "Open Network Found!" CLR_RESET));
                showSuccess(Pins::BUILT_IN_LED); 
            }
            
            break;
        }
    }

    // Update edge detection buffers for the next cycle
    btnALastState = digitalRead(Pins::BTN_A);
    btnBLastState = digitalRead(Pins::BTN_B);
    btnCLastState = digitalRead(Pins::BTN_C);
}