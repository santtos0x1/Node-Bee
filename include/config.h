#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/* * =================================================================
 * GLOBAL SETTINGS (MACROS)
 * =================================================================
 */
#define ENABLE_SD        0     
#define ENABLE_WIFI      1
#define ENABLE_BT        1
#define DEBUG            1
#define ASYNC_SD_HANDLER 1

/* * =================================================================
 * DEBUGGING MACROS
 * =================================================================
 */
#if DEBUG
  #define DEBUG_PRINTLN(x)     Serial.println(x)
  #define DEBUG_PRINT(x)       Serial.print(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, ##__VA_ARGS__)
#else
  #define DEBUG_PRINTLN(x)             
  #define DEBUG_PRINT(x)               
  #define DEBUG_PRINTF(x, ...) 
#endif

/* * =================================================================
 * MODULE PARAMETERS (.cpp)
 * =================================================================
 */
// bt_scan.cpp
#define SCAN_TIME              5

// data_logger.cpp
#define RAM_FLUSH_LIM          5

// wifi_scan.cpp
#define CONN_TIMEOUT_MS       (10 * 1000)

// http_server.cpp
#define WEB_SERVER_PORT       80
#define HND_BUFFER_SIZE       512
#define SERVER_ATTEMPTS_LIMIT 20

/* * =================================================================
 * HARDWARE PINOUTS
 * =================================================================
 */
namespace Pins 
{
  static constexpr uint8_t BTN_A        = 14;
  static constexpr uint8_t BTN_B        = 16;
  static constexpr uint8_t BTN_C        = 18;
  static constexpr uint8_t BUILT_IN_LED = 2;
  static constexpr uint8_t SD_CS        = 5;
}

/* * =================================================================
 * TIME CONSTANTS (DELAYS)
 * =================================================================
 */
namespace Time {
  static constexpr uint16_t LOW_DELAY  = 100;
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
};

// Queue handles 
extern QueueHandle_t WiFiQueue;
extern QueueHandle_t BTQueue;
extern QueueHandle_t WDQueue;

#endif // !CONFIG_H