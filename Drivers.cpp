#include "Drivers.h"

char txBuffer[256];
int txLength;
int lastBaud = 4;
unsigned long baud[5] = {9600, 19200, 38400, 57600, 115200};
extern void onCustomPayload(char* payload,int length);

void driverInit() {

  ioInit();
 
}

bool checkErase() {
  pinMode(CFG_ERASE_PIN,INPUT_PULLUP);
  int result=digitalRead(CFG_ERASE_PIN) == 1;
  pinMode(CFG_ERASE_PIN,OUTPUT);
  return result;
}


void ioInit() {
 pinMode(CFG_ERASE_PIN,OUTPUT);
}


void driverLoop() {
  int t = 0;

  t = MB_SERIAL.available();
  if (t > 0) {
    txBuffer[txLength] = 0x01;
    txBuffer[txLength + 1] = 0xF0;
    txBuffer[txLength + 2] = 0x63;
    for (int i = 0; i < t; i++) {
      txBuffer[txLength + 3 + i] = MB_SERIAL.read();
    }
    txBuffer[txLength + 3 + t] = 0xF7;
    txLength += (4 + t);
  }

}


void onReceive(char* buffer, int length) {
  if (buffer[0] != 0x01)
    return;
  int t = 1;
  int i = 0;
  int j = 0;
  int k = 0;
  float f1 = 0.0;
  float f2 = 0.0;
  int lastT = t;
  char* c;
  while (t < length) {

    lastT = t;
    switch (buffer[t]) {
      case 0xF4:
        switch (buffer[t + 2]) {
          case 0x00:
            pinMode(buffer[t + 1], INPUT);
            break;
          case 0x01:
            pinMode(buffer[t + 1], OUTPUT);
            break;
          case 0x03:
            pinMode(buffer[t + 1], OUTPUT);
            break;
        };
        t += 3;
        break;
      case 0x90:
        digitalWrite(buffer[t + 1], buffer[t + 2]);
        t += 3;
        break;
      case 0xD0:
        i = digitalRead(buffer[t + 1]);
        txBuffer[txLength] = 0x01;
        txBuffer[txLength + 1] = 0x90;
        txBuffer[txLength + 2] = buffer[t + 1];
        txBuffer[txLength + 3] = i;
        txLength += 4;
        t += 2;
        break;

      case 0xE0:
        i = buffer[t + 2];
        i = (i << 8);
        i += buffer[t + 3];
        i = i / 4;
        analogWrite(buffer[t + 1], i);
        t += 4;
        break;
      case 0xC0:
        i = analogRead(buffer[t + 1]);
        txBuffer[txLength] = 0x01;
        txBuffer[txLength + 1] = 0xE0;
        txBuffer[txLength + 2] = buffer[t + 1];
        txBuffer[txLength + 3] = (i >> 8);
        txBuffer[txLength + 4] = (i & 0xFF);
        txLength += 5;
        t += 2;
        break;

      case 0xF0:
        switch (buffer[t + 1]) {
          case 0x62:
            if (lastBaud != buffer[t + 2]) {
              MB_SERIAL.end();
             MB_SERIAL.begin(baud[buffer[t + 2]]);
              delay(100);
              lastBaud = buffer[t + 2];
            }
            for (i = 0; i < buffer[t + 3]; i++) {
              MB_SERIAL.write(buffer[t + 4 + i]);
            }
            t += (5 + buffer[t + 3]);
            break;
          case 0xA0:
            onCustomPayload(buffer+t+3,buffer[t+2]);
            t += (4 + buffer[t + 2]);
            break;
        };
        break;
    };
    if (t == lastT)
      break;
  }

}
