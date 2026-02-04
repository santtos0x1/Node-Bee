#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/* * =================================================================
 * GLOBAL SETTINGS (MACROS)
 * =================================================================
 */
#define SYS_FEATURE_SD_STORAGE    1     
#define SYS_FEATURE_WIFI_SCAN     1
#define SYS_FEATURE_WARDRIVE_SCAN 1
#define SYS_FEATURE_BLE_STACK     1
#define SYS_CFG_DEBUG_MODE        1
#define ASYNC_SD_HANDLER          1
#define SYS_CFG_USE_ANSI_COLORS   0
#define SYS_FEATURE_SERVER        1

/* * =================================================================
 * DEBUGGING MACROS
 * =================================================================
 */

#if SYS_CFG_DEBUG_MODE
  #define DEBUG_PRINTLN(x)     Serial.println(x)
  #define DEBUG_PRINT(x)       Serial.print(x)
  #define DEBUG_PRINTF(f, ...) Serial.printf((const char*)(f), ##__VA_ARGS__)
#else
  #define DEBUG_PRINTLN(x)             
  #define DEBUG_PRINT(x)               
  #define DEBUG_PRINTF(f, ...) 
#endif

/* * =================================================================
 * MODULE PARAMETERS (.cpp)
 * =================================================================
 */
// Global
#define DUALCORE_MAX_XQUEUE    50
#define SINGLECORE_MAX_XQUEUE  20

// bt_scan.cpp
#define SCAN_TIME              5

//main.cpp
#define BAUD_RATE              115200

// data_logger.cpp
#define RAM_FLUSH_LIM          5

// http_server.cpp
#define WEB_SERVER_PORT        80
#define HANDLER_BUFFER_SIZE    512

// wardriving.cpp
#define MAX_KNOWN_NETWORKS 50

/* * =================================================================
 * HARDWARE PINOUTS
 * =================================================================
 */
namespace Pins 
{
  // Buttons
  static constexpr uint8_t BTN_A         = 14;
  static constexpr uint8_t BTN_B         = 16;
  // Leds
  static constexpr uint8_t BUILT_IN_LED  = 2;
  static constexpr uint8_t LED_1         = 18; 
  static constexpr uint8_t LED_2         = 17;
  static constexpr uint8_t LED_3         = 22;
  // SD module
  static constexpr uint8_t SD_CS         = 5;
  static constexpr uint8_t MISO          = 19;
  static constexpr uint8_t MOSI          = 23;
  static constexpr uint8_t SCK           = 27;
}

/* * =================================================================
 * TIME CONSTANTS (DELAYS)
 * =================================================================
 */
namespace Time {
  static constexpr uint8_t  LOW_DELAY  = 100;
  static constexpr uint16_t LMID_DELAY = 300;
  static constexpr uint16_t MID_DELAY  = 500;
  static constexpr uint16_t HMID_DELAY = 1000;
  static constexpr uint16_t HIGH_DELAY = 2000;
  static constexpr uint16_t IDLE_DELAY = 3000;
}

/* * =================================================================
 * DATA STRUCTURES & QUEUES (FREERTOS)
 * =================================================================
 */
struct WiFiData
{
  // Passive Scanning
  char    ssid[33];
  int8_t  rssi;
  char    bssid[20];
  uint8_t encryptionType;
  int     channel;
  
  // Active Connection
  char    hostname[33];
  char    dnsIP[16];
  char    subNetMask[16];
  char    localIP[16];
  char    dhcp[20];
  char    dbmQuality[8];
};

struct BTData
{
  char    name[33];
  char    address[18];
  int     rssi;        
  char    addressType[20];
  int     channel;
};

struct WardriveData
{
  char    ssid[33];
  int8_t  rssi;
  char    bssid[20];
};

// Queue handles 
extern QueueHandle_t WiFiQueue;
extern QueueHandle_t BTQueue;
extern QueueHandle_t WDQueue;

#endif // !CONFIG_H