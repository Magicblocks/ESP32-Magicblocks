#include "ESP32_MB_Core.h"
MagicLinkWS ws;
char buffer[256];
int readLength = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastTx = 0;
bool isSetup = false;

bool isWSConnected() {
  return ws.isConnected();
}
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}
void ESP32_MB_Core::init() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(500);
  Serial.println("Starting..");
  WiFi.mode(WIFI_AP_STA);

  driverInit();
  delay(2000);
  if (checkErase()) {
    Serial.print("Factory Resetting....");
    eraseConfig();
    delay(3000);
    Serial.println("Complete\r\n Restarting..");
    ESP.restart();
  }


  if (configInit()) {
    if (strlen(mConfig.name) > 0)
      WiFi.softAP(mConfig.name);
    else
      WiFi.softAP("ESP32-MB");
    Serial.println("Starting..");
    sprintf(buffer,"Connecting to %s...",mConfig.ssid);
    Serial.println(buffer);
    if (wifiInit()) {
      Serial.println("Connecting to Magicblocks..");
      wsInit();
    }
    else {
      Serial.println("WiFi connection failed!");
    }
  }
  else {
    WiFi.softAP("ESP32-MB");
    Serial.println("Setup");
    sprintf(buffer,"Connect to \"ESP32-MB\" WiFi and go to http://192.168.4.1 in a browser to setup");
    Serial.println(buffer);
  }
}

bool ESP32_MB_Core::wifiInit() {
  if (strlen(mConfig.ssid) > 0 && strlen(mConfig.wifiPassword) > 0) {
      int nwCount = WiFi.scanNetworks();   
    bool ssidFound=false;
    for(int i=0;i<nwCount;i++){    
      if(strcmp(mConfig.ssid,WiFi.SSID(i).c_str())==0){
        ssidFound=true;
      }
    }
    if(!ssidFound)
      return false;
    unsigned long start = millis();
    Serial.print("Connecting to ");
    Serial.println(mConfig.ssid);
    WiFi.begin(mConfig.ssid, mConfig.wifiPassword);
    while (WiFi.status() != WL_CONNECTED && (millis() - start < WIFI_TIMEOUT)) {
      delay(1000);
      Serial.print(".");
    }
    return WiFi.status() == WL_CONNECTED;
  }
  return false;
}

bool ESP32_MB_Core::wsInit() {
  if (strlen(mConfig.deviceId) > 0 && strlen(mConfig.key) > 0) {
    isSetup = true;
    Serial.print("Connecting to Magicblocks with ID ");
    Serial.println(mConfig.deviceId);
    ws.init(Serial, true);
    ws.connect(mConfig.deviceId, mConfig.key);
    unsigned long start = millis();
    while (millis() - start < MB_TIMEOUT || millis() < start) {
      if (ws.isConnected())
        break;
      ws.loop();
    }
    if (!ws.isConnected()) {
      Serial.println("Error");
      Serial.println("Failed to connect to Magicblocks");
      Serial.println("Failed to connect to Magicblocks!");
    }
    else {
      Serial.println("Connected!");
     
    }
    return true;
  }
  else {
    Serial.println("Magicblocks id, key not found");
    sprintf(buffer,"Connect to \"%s\" WiFi and go to http://192.168.4.1 to setup deviceId & key",mConfig.name);
    Serial.println(buffer);
  }
  return false;
}

void ESP32_MB_Core::sendPayload(char* payload,int length){
  Serial.println("Sending payload");
  
   txBuffer[txLength] = 0x01;
    txBuffer[txLength + 1] = 0xF0;
    txBuffer[txLength + 2] = 0xA0;
    for (int i = 0; i < length; i++) {
      txBuffer[txLength + 3 + i] = payload[i];
    }
    txBuffer[txLength + 3 + length] = 0xF7;
    txLength += (4 + length);
}

void ESP32_MB_Core::loop() {
  ws.loop();
  driverLoop();
  readLength = ws.read(buffer);
  if (readLength > 0) {
    onReceive(buffer, readLength);
  }
  if (txLength > 0 && (millis() - lastTx > MIN_TX_INTERVAL || millis() < lastTx)) {
    ws.write(txBuffer, txLength);
    txLength = 0;
    lastTx = millis();
  }
  if (millis() - lastWiFiCheck > 10000 || millis() < lastWiFiCheck) {

    lastWiFiCheck = millis();
  
    if (WiFi.status() != WL_CONNECTED) {

      wifiInit();
    }
    if (!ws.isConnected()) {
      if (WiFi.status() == WL_CONNECTED)
        wsInit();
    }
    
  }
}
