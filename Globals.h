#pragma once

extern bool isWSConnected();
extern bool isWiFiConnected();
struct MConfig{
	char deviceId[64];
	char key[64];
	char ssid[32];
	char wifiPassword[32];
	char adminPassword[32];
	char name[16];
};
extern MConfig mConfig;
extern char txBuffer[64];
extern int txLength;