#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct wifiData
{
  //Without connection
  char ssid[33];
  int8_t rssi;
  char bssid[20];
  uint8_t encryptationType;
  int channel;
  //WiFi connected
  char hostname[33];
  char dnsIP[16];
  char subNetMask[16];
  char localIP[16];
  char dhcp[15];
  unsigned long timestamp;
}

extern QueueHandle_t wifiQueue;

struct bleData
{

}

#endif // !CONFIG_H
