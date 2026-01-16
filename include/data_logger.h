#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "config.h"

void setupSD();
void logWiFiData(void * pvParameters);
void logBTData();
void logWDData();

#endif // !DATA_LOGGER_H
