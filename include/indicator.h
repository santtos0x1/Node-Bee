#ifndef INDICATOR_H
#define INDICATOR_H

#include <Arduino.h>

void setupIndicator(int ledPinout);
void showProcessing(int ledPinout);
void showSuccess(int ledPinout);
void showError(int ledPinout);


#endif // !INDICATOR_H