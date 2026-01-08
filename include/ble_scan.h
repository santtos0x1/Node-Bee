#ifndef BLE_SCAN_H
#define BLE_SCAN_H

void initBLE();
void bleSniffer();


struct BTData {
    char name[33];          
    char address[18];      
    int rssi;               
    char addressType[20];  
    unsigned long timestamp;
    int channel;             
};

extern QueueHandle_t btQueue;

#endif // !BLE_SCAN_H