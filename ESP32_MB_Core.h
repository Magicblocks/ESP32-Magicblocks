/*

Author: Akalanka De Silva
Date: 2020-8-2

This is a library for ESP32 based devices to connect with the magicblocks.io platform. It uses Websockets over WiFi to maintain a realtime full duplex connection with magicblocks. 
This library handles the complexities related to WiFi credential management, web based configuration and connection management. 
THe folloiwing function should be implemented in the code:  void onCustomPayload(char* payload,int length) to process any messages sent via the 'Payload Out' block
The sendPayload(char* payload, int length) function can be called to send a custom payload which will can be received by the 'Payload In' block
loop() function should be called by the main loop() in the code
*/
#pragma once
#include "Arduino.h"
#include "MagicLinkWS.h"
#include "Drivers.h"
#include "Config.h"
#include "Globals.h"

#define WIFI_TIMEOUT 30000
#define MB_TIMEOUT 30000
#define MIN_TX_INTERVAL 500

bool isWSConnected();
bool isWiFiConnected();


class ESP32_MB_Core{

public:
	void init();
	void loop();
	void sendPayload(char* payload, int length);
private:
	
	bool wifiInit();
	bool wsInit();


};