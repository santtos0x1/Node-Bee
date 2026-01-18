/**
 * Project: Noctua - ESP32 WiFi & Bluetooth Scanner
 * Entry Point: Main Application Loop
 * * Description:
 * This file acts as the bootloader for the firmware logic. 
 * It initializes the Serial interface for debugging and hands over 
 * control to the Finite State Machine (FSM) module.
 */

// Local libs
#include "data_logger.h"
#include "wifi_scan.h"
#include "bluetooth_scan.h"
#include "config.h"
#include "fsm.h"
#include "boot_manager.h"

// Libs
#include <Arduino.h>

/**
 * System Setup
 * Runs once at power-up or reset. Responsible for:
 * 1. Initializing the debug interface.
 * 2. Bootstrapping the FSM which handles all hardware (SD, WiFi, LEDs).
 */
void setup()
{
    // Initialize UART for Serial Monitor debugging
    // Note: Baud rate must match the configuration in platformio.ini or monitor
    Serial.begin(BAUD_RATE);
    
    // Print firmware banner to confirm boot success
    logoInit();
    configCheck();
    
    /* * Delegates system initialization to the FSM module.
     * This function will setup SD Card, WiFi/BT stacks, and GPIOs 
     * based on the macros defined in config.h.
     */
    setupFSM();
}

/**
 * Main Loop
 * Runs continuously. Keeps the Finite State Machine "ticking".
 */
void loop()
{
    // Updates the state machine logic
    runFSM();
}