#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "config.h"

void setupSD();
void processWiFiLog(WiFiData data);
void processBTLog(BTData data);
void processWDLog(WardriveData data);

void logSDTask(void * pvParameters);
void processAllLogsSequential();

#endif // !DATA_LOGGER_H
