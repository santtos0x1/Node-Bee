#include "config.h"
#include "boot_manager.h"
#include "utils.h"

#include <Arduino.h>

void logoInit()
{
    DEBUG_PRINTLN(F("\n==============================================="));
    DEBUG_PRINTF(F("%s[ SYSTEM ] NOCTUA WIRELESS SNIFFER%s\n"), CLR_GREEN, CLR_RESET);
    DEBUG_PRINTF(F("%s[ STATUS ] ONLINE - VERSION 1.0%s\n"), CLR_YELLOW, CLR_RESET);
    DEBUG_PRINTLN(F("[ SOURCE ] OPEN SOURCE PROJECT"));
    DEBUG_PRINTLN(F("==============================================="));
}

void configCheck()
{
    DEBUG_PRINTF(F("%s[BOOT]%s Checking hardware & modules...\n"), CLR_GREEN, CLR_RESET);

    DEBUG_PRINT(F("> WiFi Stack:      "));
    #if SYS_FEATURE_WIFI_SCAN
        DEBUG_PRINTF(F("%s[ ENABLED  ]%s\n"), CLR_GREEN, CLR_RESET);
    #else
        DEBUG_PRINTF(F("%s[ DISABLED  ]%s\n"), CLR_RED, CLR_RESET);
    #endif

    DEBUG_PRINT(F("> Bluetooth Stack: "));
    #if SYS_FEATURE_BLE_STACK
        DEBUG_PRINTF(F("%s[ ENABLED  ]%s\n"), CLR_GREEN, CLR_RESET);
    #else
        DEBUG_PRINTF(F("%s[ DISABLED  ]%s\n"), CLR_RED, CLR_RESET);
    #endif

    DEBUG_PRINT(F("> SD Storage:      "));
    #if SYS_FEATURE_SD_STORAGE
        DEBUG_PRINTF(F("%s[ ENABLED  ]%s\n"), CLR_GREEN, CLR_RESET);
    #else
        DEBUG_PRINTF(F("%s[ DISABLED  ]%s\n"), CLR_RED, CLR_RESET);
    #endif

    DEBUG_PRINTF(F(" > Internal RAM:    %d KB Free\n"), ESP.getFreeHeap() / 1024);
    DEBUG_PRINTF(F(" > CPU Frequency:   %d MHz\n"), ESP.getCpuFreqMHz());
    DEBUG_PRINTF(F(" > SDK Version:     %s\n"), ESP.getSdkVersion());
    DEBUG_PRINTF(F(" > Chip model:      %s\n"), ESP.getChipModel());

    // System Health
    DEBUG_PRINTLN(F("-----------------------------------------------"));

    DEBUG_PRINTF(F(" > HEAP Status:     %d KB Free\n"), ESP.getFreeHeap() / 1024);
    DEBUG_PRINTF(F(" > Min Free Heap:   %d KB (Ever)\n"), ESP.getMinFreeHeap() / 1024);
    
    // CORREÇÃO: S minúsculo no %s final e fechamento do F()
    DEBUG_PRINTF(F(" > PSRAM Available: %s%s%s\n"), 
        ESP.getPsramSize() > 0 ? CLR_GREEN : CLR_RED, 
        ESP.getPsramSize() > 0 ? "YES" : "NO",
        CLR_RESET
    );

    DEBUG_PRINTLN(F("-----------------------------------------------"));
}