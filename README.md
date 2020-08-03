# ESP32_MB_Core
Arduino library for ESP32 devices to connect to magicblocks.io

## Description

This is a Arduino library to manage connections and configurations for ESP32 based boards to connect to the [Magicblocks.io](http://magicblocks.io) platform. Magicblocks has the following standard blocks in the cloud platform which can be used with all ESP32 based boards
* **Digital Out** - Drive pins HIGH or LOW from the cloud**
* **Digital In** - Read pin status from the cloud
* **Analog In** - Read ADC values from the cloud
* **Analog Out** - Control PWM/DAC outputs from the cloud
* **Serial In** - Asynchronously receive any upstream data received on the Serial port of the ESP32 device from the cloud. By default the 'Serial' is set as the default port. This can be changed to Serial2 by setting #define MB_SERIAL Serial2 in Drivers.h. By default baud rate is set to 115200. This will be changed automatically if any data is sent downstream using the 'Serial Out' block
* **Serial Out** - Send downstream data to the Serial port defined as MB_SERIAL in Drivers.h on a selected baud rate from 4800 to 115200
* **Payload In** - Custom data sent upstream by calling the 'sendPayload' function in the device library can be received on the cloud end via this block
```C
ESP32_MB_Core device;
device.sendPayload("HELLO",5);
```
* **Payload Out** - Custom data can be sent downstream from the cloud using this block. The .ino file should implement the following function as the callback for such custom downstream payloads.
```C
void onCustomPayload(char* payload,int length){
 
}
```

## Getting started

A sample Arduino .ino file will look like the following:
```C
#include "ESP32_MB_Core.h"
unsigned long lastPing=0;
ESP32_MB_Core device;

void setup(){
  device.init();
}

void onCustomPayload(char* payload,int length){
  Serial.println("Payload received!");
  Serial.println(payload);
}

void loop(){
  device.loop();
  if(millis()-lastPing>10000){
    lastPing=millis();
    device.sendPayload("HELLO",5);
  }
}
```

All the complexity of credential management, connection management is handled by the platform. To get started, follow the few steps below:
* Download or clone the library to Arduino libraries folder
* Create a new project in Arduino
* Copy past the above sample code to the Arduino project
* Upload the code to a ESP32 based board. Subsequently when the device is started up, it would show a WiFi access point "ESP-MB" 
* Connect to "ESP-MB" access point and open the config page [http://192.168.4.1](http://192.168.4.1) in the browser
* Select the WiFi access point and set the password
* Set the id and key for this device. (You should already have a [magicblocks.io](http://magicblocks.io) account and have created a device in Device Manager to complete this step)
* Save. The default admin password is 12345 which can be changed clicking the 'Admin' link in the config page. The device will restart and it should connect to the platform.

