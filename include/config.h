#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct WiFiData
{
  //Without connection
  char ssid[33];
  int8_t rssi;
  char bssid[20];
  uint8_t encryptationType;
  int channel;
  // Connected
  char hostname[33];
  char dnsIP[16];
  char subNetMask[16];
  char localIP[16];
  char dhcp[15];
}

struct BTData
{
    char name[33];
    char address[18];
    int rssi;        
    char addressType[20];
    int channel;
}

extern QueueHandle_t WiFiQueue;
extern QueueHandle_t BTQueue;

#endif // !CONFIG_H
