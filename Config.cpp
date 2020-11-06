#include "Config.h"
AsyncWebServer configServer(80);
MConfig mConfig;
int nwCount=0;

bool configInit(){
  

  configServerInit();
 
  return deserializeConfig();
  
}

void configServerInit(){
  configServer.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
     nwCount = WiFi.scanNetworks();
    char* response = (char*)malloc(sizeof(indexHtml) + 134 + 50*nwCount);
    char s1[16];
    char s2[16];
    char s3[64];
    char s4[16];
    char s5[16];
    char s6[6];
    char s7[50*nwCount];
    char s8[50];
    strcpy(s5,mConfig.name);
    if(strlen(mConfig.ssid)==0 && strlen(mConfig.deviceId)==0){
      strcpy(s6,"true");
    }
    else{
      strcpy(s6,"false");
    }
    for(int i=0;i<nwCount;i++){
      sprintf(s8,"<option>%s</option>",WiFi.SSID(i).c_str());
      strcat(s7,s8);
    }
    if(isWiFiConnected()){
        sprintf(s3,"Connected to %s",mConfig.ssid);
        strcpy(s4,"");
    }
    else{
      strcpy(s3,"");
      strcpy(s4,"Not connected");
    }
    if(isWSConnected()){
      strcpy(s1,"Connected");
      strcpy(s2,"");
    }
    else{
      strcpy(s1,"");
      strcpy(s2,"Not Connected");
    }
    sprintf(response, indexHtml, s6,s1,s2,s5,mConfig.deviceId, s3,s4,s7);
    
    request->send_P(200, "text/html", response);
    free(response);
  });


  configServer.on("/submit", HTTP_POST, [](AsyncWebServerRequest * request) {
    String d, k, s, wp, np, npc, p, n;
    char* response = (char*)malloc(sizeof(submitHtml) + 64);
    bool dirty=false;
    if (request->hasParam("p", true)) {
      p = request->getParam("p", true)->value();
      
      delay(100);
      if (p==String(mConfig.adminPassword)) {
        if (request->hasParam("n", true));
        n = request->getParam("n", true)->value();
        if (request->hasParam("d", true));
        d = request->getParam("d", true)->value();
        if (request->hasParam("k", true));
        k = request->getParam("k", true)->value();
        if (request->hasParam("s", true));
        s = request->getParam("s", true)->value();
        if (request->hasParam("wp", true));
        wp = request->getParam("wp", true)->value();
        if (request->hasParam("np", true));
        np = request->getParam("np", true)->value();
        if (request->hasParam("npc", true));
        npc = request->getParam("npc", true)->value();

        if(strcmp(mConfig.name,n.c_str())!=0){
          dirty=true;
          strcpy(mConfig.name,n.c_str());
        }
        if (d.length() > 0 && k.length() > 0) {
          dirty=true;
          Serial.println("New device Id, key");
          Serial.println(d);
          strcpy(mConfig.deviceId,d.c_str());
          Serial.println(k);
          strcpy(mConfig.key,k.c_str());
          
        }
        if (s.length() > 0 && wp.length() > 0) {
          dirty=true;
          Serial.println("New ssid & pass");
          Serial.println(s);
          strcpy(mConfig.ssid,s.c_str());
          Serial.println(wp);
          strcpy(mConfig.wifiPassword,wp.c_str());
        }
        if (np.length() > 0 && npc.length() > 0) {
          if (np.equals(npc)) {
            dirty=true;
            Serial.println("New password");
            Serial.println(np);
            strcpy(mConfig.adminPassword,np.c_str());
          }
          else{
            sprintf(response, submitHtml, "Password mismatch!","Please re-enter the password");
          }

        }
      }
      else {
        sprintf(response, submitHtml, "Invalid password!","Please try again");
      }
    }
    else {
      sprintf(response, submitHtml, "Invalid password!","Please try again");
    }
    if(strlen(response)==0 && !dirty)
      sprintf(response, submitHtml, "No change to settings","");
    else if(dirty){
      sprintf(response,submitHtml,"Settings saved! Restarting....","You can close this window now");
      serializeConfig();
     
    }
    request->send_P(200, "text/html", response);
    free(response);
    if(dirty){
       delay(2000);
      ESP.restart();
    }
  });

  // Start server
  configServer.begin();
}

int serializeConfig(){
  char c;
  
  for(int i=0;i<sizeof(mConfig);i++){
   c= *((char*)&mConfig + i);
   EEPROM.write(CONFIG_OFFSET+i,c);
  }
  EEPROM.commit();
  return sizeof(mConfig);
}

void eraseConfig(){
 
  for(int i=0;i<sizeof(mConfig);i++){
   EEPROM.write(CONFIG_OFFSET+i,0);
  }
  EEPROM.commit();
  
}
bool deserializeConfig(){
  char c;
  bool isConfigValid=false;
  char last;
  for(int i=0;i<sizeof(mConfig);i++){
    c=EEPROM.read(CONFIG_OFFSET+i);
     *((char*)&mConfig + i) = c;
    
    if(i==0)
      last=c;
    else if(last!=c)
      isConfigValid=true;
  }
  
  if(!isConfigValid){
    Serial.println("Factory config");
    strcpy(mConfig.deviceId,"");
    strcpy(mConfig.key,"");
    strcpy(mConfig.ssid,"");
    strcpy(mConfig.wifiPassword,"");
    strcpy(mConfig.adminPassword,"12345");
    strcpy(mConfig.name,"ESP32-MB");
  }
  else{
    Serial.println(mConfig.deviceId);
    Serial.println(mConfig.key);
    Serial.println(mConfig.ssid);
    Serial.println(mConfig.wifiPassword);
    Serial.println(mConfig.adminPassword);
    Serial.println(mConfig.name);
  }
  return isConfigValid;
}
