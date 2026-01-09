#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>

bool setupSD();
void logWiFiData();
void logBTData();

#endif // !DATA_LOGGER_H
