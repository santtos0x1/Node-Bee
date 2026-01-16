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

/**
 * Bootstraps the FSM and initializes all modular subsystems 
 * based on the global build configuration.
 */
void setupFSM()
{
    // Conditional initialization of hardware modules
    #if ENABLE_WIFI
        setupWiFi();
    #endif
    #if ENABLE_BT
        setupBT();
    #endif    
    #if ENABLE_SD
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
           - Button A: Single Click (WiFi Scan) / Double Click (Wardrive)
           - Button B: Bluetooth Scan
           - Button C: Web Server Mode
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
           Includes data integrity checks for the SD card.
        */
        case SCAN:
        {
            DEBUG_PRINTLN("Current FSM state: SCAN");
            
            #if ENABLE_SD
                // Verify storage health before attempting to write
                bool SDReport = SDDoctor();            
                if(!SDReport)
                {
                    showError(Pins::BUILT_IN_LED);
                    setupSD(); // Attempt hardware re-initialization
                    currentState = IDLE;
                    break;
                }
            #endif
            
            if(scanMode == "WF")
            {   
                #if ENABLE_WIFI
                    WiFiSniffer(); // Start 802.11 packet capture
                    #if !ASYNC_SD_HANDLER && ENABLE_SD
                        processAllLogsSequential();
                    #endif    
                    showSuccess(Pins::BUILT_IN_LED);
                #endif

                currentState = IDLE;
                break;
            } else if(scanMode == "BT") {
                #if ENABLE_BT
                    BTSniffer(); // Start BLE advertising discovery    
                    #if !ASYNC_SD_HANDLER && ENABLE_SD
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
            DEBUG_PRINTLN("Current FSM state: WEB_SERVER");
            
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
                showError(Pins::BUILT_IN_LED);
                currentState = IDLE;
                break;
            }
            break;
        }

        /* WARDRIVE_MODE:
           Continuous capture mode combining WiFi signal strength 
           with concurrent data logging.
        */
        case WARDRIVE_MODE:
        {
            // Exit condition
            if(btnBPressed) 
            {
                currentState = IDLE;
                break;
            }

            bool openFound = startWardrive();
            #if !ASYNC_SD_HANDLER && ENABLE_SD
                processAllLogsSequential();
            #endif
            
            if(openFound) {
                // Visual feedback for open networks found during wardrive
                showSuccess(Pins::BUILT_IN_LED); 
            }
            
            // Note: This mode stays in WARDRIVE_MODE until BT_B is pressed
            break;
        }
    }

    // Update edge detection buffers for the next cycle
    btnALastState = digitalRead(Pins::BTN_A);
    btnBLastState = digitalRead(Pins::BTN_B);
    btnCLastState = digitalRead(Pins::BTN_C);
}