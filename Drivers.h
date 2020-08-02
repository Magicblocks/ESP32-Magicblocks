#pragma once
#include <ESP32Servo.h>
#include "Arduino.h"

#define CFG_ERASE_PIN 2
#define MB_SERIAL Serial



void driverInit();
void driverLoop();
void onReceive(char* buffer,int length);
void ioInit();
bool checkErase();
