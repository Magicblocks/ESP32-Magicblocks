
#include "global.h"
#include "WebSocketClient.h"

#include "sha1.h"
#include "Base64.h"


bool WebSocketClient::handshake(Client &client) {

    socket_client = &client;

    // If there is a connected client->
    if (socket_client->connected()) {
        // Check request and look for websocket handshake
#ifdef DEBUGGING
            Serial.println(F("Client connected"));
#endif
        if (analyzeRequest()) {
#ifdef DEBUGGING
                Serial.println(F("Websocket established"));
#endif

                return true;

        } else {
            // Might just need to break until out of socket_client loop.
#ifdef DEBUGGING
            Serial.println(F("Invalid handshake"));
#endif
            disconnectStream();

            return false;
        }
    } else {
        return false;
    }
}

bool WebSocketClient::analyzeRequest() {
    char buffer[512];
    char c;
    int t;
    bool foundupgrade = false;
    unsigned long intkey[2];
    char serverKey[32];
    char seed[17];
    char b64Key[25];
    
    
    randomSeed(analogRead(0));

    for (int i=0; i<16; ++i) {
        seed[i] = (char)random(1, 256);
    }

    base64_encode(b64Key, seed, 16);
    b64Key[24]=0;
#ifdef DEBUGGING
    Serial.println(F("Sending websocket upgrade headers"));
#endif    
    sprintf(buffer,"GET %s HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nHost: %s\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Protocol: \r\nSec-WebSocket-Version: 13\r\n\r\n",
                    path,host,b64Key); 

#ifdef DEBUGGING
    Serial.println(buffer);
#endif
    delay(100);
    socket_client->print(buffer);
    socket_client->flush();
    memset(buffer,0,sizeof(buffer));

#ifdef DEBUGGING
    Serial.println(F("Analyzing response headers"));
#endif    

    while (socket_client->connected() && !socket_client->available()) {
        delay(100);
      
    }
    t=0;
    while ((c = socket_client->read()) != 0xFF) {

        buffer[t++]=c;
#ifdef DEBUGGING
            Serial.print(c);
#endif         
        if (c == '\n') {

#ifdef DEBUGGING
            Serial.print("Got Header: ");
#endif
            if (!foundupgrade && strncmp(buffer,"Upgrade: websocket",18)==0) {

                foundupgrade = true;
            } else if (strncmp(buffer,"Sec-WebSocket-Accept: ",22)==0) {
                memcpy(serverKey,buffer+22,strlen(buffer)-22);

            }
            memset(buffer,0,sizeof(buffer));
        }

        if (!socket_client->available()) {
          delay(20);
        }
    }

    uint8_t *hash;
    char result[21];
    char b64Result[30];
    SHA1Context sha;
    int err;
    uint8_t messageDigest[20];
    
    err = SHA1Reset(&sha);
    err = SHA1Input(&sha, reinterpret_cast<const uint8_t *>(b64Key), strlen(b64Key));
    err = SHA1Result(&sha, messageDigest);
    hash = messageDigest;

    memcpy(result,hash,20);
    result[20]=0;

    base64_encode(b64Result, result, 20);
  
    return true;
 
}
unsigned int WebSocketClient::handleStream(char* data, uint8_t *opcode) {
    uint8_t msgtype;
    uint8_t bite;
    unsigned int length;
    uint8_t mask[4];
    uint8_t index;
    unsigned int i;
    bool hasMask = false;

    if (!socket_client->connected() )
    {
        return 0;
    }      
    if(!socket_client->available()){
        return 0;
    }

    msgtype = timedRead();
    if (!socket_client->connected()) {
        return 0;
    }

    length = timedRead();

    if (length & WS_MASK) {
        hasMask = true;
        length = length & ~WS_MASK;
    }


    if (!socket_client->connected()) {
        return 0;
    }

    index = 6;

    if (length == WS_SIZE16) {
        length = timedRead() << 8;
        if (!socket_client->connected()) {
            return 0;
        }
            
        length |= timedRead();
        if (!socket_client->connected()) {
            return 0;
        }   

    } else if (length == WS_SIZE64) {
#ifdef DEBUGGING
        Serial.println(F("No support for over 16 bit sized messages"));
#endif
        return 0;
    }
   if(length>200)return 0;
    if (hasMask) {
        // get the mask
        mask[0] = timedRead();
        if (!socket_client->connected()) {
            return 0;
        }

        mask[1] = timedRead();
        if (!socket_client->connected()) {

            return 0;
        }

        mask[2] = timedRead();
        if (!socket_client->connected()) {
            return 0;
        }

        mask[3] = timedRead();
        if (!socket_client->connected()) {
            return 0;
        }
    }
        
    memset(data,0,sizeof(data));
        
    if (opcode != NULL)
    {
      *opcode = msgtype & ~WS_FIN;
    }
                
    if (hasMask) {
        for (i=0; i<length; ++i) {

            data[i]= (char) (timedRead() ^ mask[i % 4]);
           
            if (!socket_client->connected()) {
                return 0;
            }
        }
    } else {
        for (i=0; i<length; ++i) {
            data[i]= (char) timedRead();
            
            if (!socket_client->connected()) {
                return 0;
            }
        }            
    }
    
    return length;
}

unsigned int WebSocketClient::handleStream(String& data, uint8_t *opcode) {
    uint8_t msgtype;
    uint8_t bite;
    unsigned int length;
    uint8_t mask[4];
    uint8_t index;
    unsigned int i;
    bool hasMask = false;

    if (!socket_client->connected() )
    {
        return 0;
    }      
    if(!socket_client->available()){
        return 0;
    }

    msgtype = timedRead();
    if (!socket_client->connected()) {
        return 0;
    }

    length = timedRead();

    if (length & WS_MASK) {
        hasMask = true;
        length = length & ~WS_MASK;
    }


    if (!socket_client->connected()) {
        return 0;
    }

    index = 6;

    if (length == WS_SIZE16) {
        length = timedRead() << 8;
        if (!socket_client->connected()) {
            return 0;
        }
            
        length |= timedRead();
        if (!socket_client->connected()) {
            return 0;
        }   

    } else if (length == WS_SIZE64) {
#ifdef DEBUGGING
        Serial.println(F("No support for over 16 bit sized messages"));
#endif
        return 0;
    }
   if(length>200)return 0;
    if (hasMask) {
        // get the mask
        mask[0] = timedRead();
        if (!socket_client->connected()) {
            return 0;
        }

        mask[1] = timedRead();
        if (!socket_client->connected()) {

            return 0;
        }

        mask[2] = timedRead();
        if (!socket_client->connected()) {
            return 0;
        }

        mask[3] = timedRead();
        if (!socket_client->connected()) {
            return 0;
        }
    }
        
    data = "";
        
    if (opcode != NULL)
    {
      *opcode = msgtype & ~WS_FIN;
    }
                
    if (hasMask) {
        for (i=0; i<length; ++i) {

            data += (char) (timedRead() ^ mask[i % 4]);
           
            if (!socket_client->connected()) {
                return 0;
            }
        }
    } else {
        for (i=0; i<length; ++i) {
            data += (char) timedRead();
            
            if (!socket_client->connected()) {
                return 0;
            }
        }            
    }
    
    return length;
}

void WebSocketClient::disconnectStream() {
#ifdef DEBUGGING
    Serial.println(F("Terminating socket"));
#endif
    // Should send 0x8700 to server to tell it I'm quitting here.
    socket_client->write((uint8_t) 0x87);
    socket_client->write((uint8_t) 0x00);
    
    socket_client->flush();
    delay(10);
    socket_client->stop();
}

unsigned int WebSocketClient::getData(String& data, uint8_t *opcode) {
    return handleStream(data, opcode);
} 
unsigned int WebSocketClient::getData(char* data, uint8_t *opcode) {
    return handleStream(data, opcode);
}    

void WebSocketClient::sendData(const char *str, uint8_t opcode) {
#ifdef DEBUGGING
    Serial.print(F("Sending data: 2"));
    
#endif
    if (socket_client->connected()) {
        sendEncodedData(str, opcode);       
    }
}

void WebSocketClient::sendData(unsigned char *str,int size, uint8_t opcode) {
#ifdef DEBUGGING
    Serial.print(F("Sending data: "));

#endif
    if (socket_client->connected()) {
        sendEncodedData(str, size, opcode);       
    }
}

void WebSocketClient::sendData(String str, uint8_t opcode) {
#ifdef DEBUGGING
    Serial.print(F("Sending data: "));
    Serial.println(str);
#endif
    if (socket_client->connected()) {
        sendEncodedData(str, opcode);
    }
}

int WebSocketClient::timedRead() {
    int i=0;
  while (socket_client->available()<=0&&i<10) {
    delay(20);  
    i++;
  }

  return socket_client->read();

 
}

void WebSocketClient::sendEncodedData(char *str, uint8_t opcode) {
    uint8_t mask[4];
    int size = strlen(str);

    // Opcode; final fragment
    socket_client->write(opcode | WS_FIN);

    // NOTE: no support for > 16-bit sized messages
    if (size > 125) {
        socket_client->write(WS_SIZE16 | WS_MASK);
        socket_client->write((uint8_t) (size >> 8));
        socket_client->write((uint8_t) (size & 0xFF));
    } else {
        socket_client->write((uint8_t) size | WS_MASK);
    }

    mask[0] = random(0, 256);
    mask[1] = random(0, 256);
    mask[2] = random(0, 256);
    mask[3] = random(0, 256);
    
    socket_client->write(mask[0]);
    socket_client->write(mask[1]);
    socket_client->write(mask[2]);
    socket_client->write(mask[3]);
     
    for (int i=0; i<size; ++i) {
        socket_client->write(str[i] ^ mask[i % 4]);
    }
}

void WebSocketClient::sendEncodedData(String str, uint8_t opcode) {
    int size = str.length() + 1;
    char cstr[size];

    str.toCharArray(cstr, size);

    sendEncodedData(cstr, opcode);
}

void WebSocketClient::sendEncodedData(unsigned char *str, int size,uint8_t opcode) {
    uint8_t mask[4];
    uint8_t sendBuffer[size+8];
    int i=0;

    // Opcode; final fragment

    sendBuffer[i++]=opcode | WS_FIN;
    // NOTE: no support for > 16-bit sized messages
    if (size > 125) {
          sendBuffer[i++]=WS_SIZE16 | WS_MASK;
        sendBuffer[i++]=(uint8_t) (size >> 8);
        sendBuffer[i++]=(uint8_t) (size & 0xFF);
    } else {

         sendBuffer[i++]=(uint8_t) size | WS_MASK;
    }

    mask[0] = random(0, 256);
    mask[1] = random(0, 256);
    mask[2] = random(0, 256);
    mask[3] = random(0, 256);
    
    sendBuffer[i++]=mask[0];
    sendBuffer[i++]=mask[1];
    sendBuffer[i++]=mask[2];
    sendBuffer[i++]=mask[3];
     
    for (int j=0; j<size; ++j) {
        sendBuffer[i++]=str[j] ^ mask[j % 4];
    }
    #ifdef DEBUGGING
    Serial.print(F("Sending data: 3"));
    
#endif
    socket_client->write(sendBuffer,i);
}






