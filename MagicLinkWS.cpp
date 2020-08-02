#include "MagicLinkWS.h"

void MagicLinkWS::init(Stream &d,bool en){
	DEBUG_SERIAL=&d;
	debug=en;
}
bool MagicLinkWS::connect(char* d,char* k){
	deviceId=d;
	key=k;
	char buffer[256];
	byte digest[2 + strlen(key)];
	BYTE hash[SHA256_BLOCK_SIZE];
	unsigned long rnd; 
	
	Sha256* sha = new Sha256();

	if(WiFi.status() != WL_CONNECTED){
		if(debug)
			DEBUG_SERIAL->println("WiFi not connected");
		return false;
	}
	if(!discover(deviceId,host,&port)){
		if(debug)
			DEBUG_SERIAL->println("Discovery failed");
		return false;
	}
	
	if(!client.connect(host,port)){
		if(debug)
			DEBUG_SERIAL->println("Socket connection failed");
		return false;
	}
	webSocketClient.host=host;
	webSocketClient.path=WS_PATH;
	if(!webSocketClient.handshake(client)){
		if(debug)
			DEBUG_SERIAL->println("WS failed");
		return false;
	}

	buffer[0] = 0x00;
	buffer[1] = strlen(deviceId);
	memcpy(buffer+2,deviceId,buffer[1]);

	rnd = random(65535);
	challenge = random(65535);
	buffer[buffer[1] + 2] = (rnd >> 8) & 0xFF;
	buffer[buffer[1] + 3] = rnd & 0xFF;

	digest[0] = buffer[buffer[1] + 2];
	digest[1] = buffer[buffer[1] + 3];
	memcpy(digest+2,key,strlen(key));

	sha->update(digest, 2 + strlen(key));
	sha->final(hash);
	memcpy(buffer+buffer[1]+4,hash,32);
	delete sha;
	buffer[36 + buffer[1]] = (challenge >> 8) & 0xFF;
	buffer[37 + buffer[1]] = challenge & 0xFF;

	webSocketClient.sendData((unsigned char*)buffer,38+buffer[1],WS_OPCODE_TEXT);
	lastUplink=millis();
	state= STATE_AUTH_PENDING;
	if(debug)
		DEBUG_SERIAL->println("Tx Auth");
	return true;
}
void MagicLinkWS::loop(){
	if(state==STATE_AUTH_COMPLETE){
		if(millis()-lastUplink>KA_INTERVAL || millis()<lastUplink){
			if(!client.connected()){
				state=STATE_IDLE;
			}
			else{
				char buffer[4];
				buffer[0]=0x02;
				webSocketClient.sendData((unsigned char*)buffer,1,WS_OPCODE_TEXT);
				lastUplink=millis();
				if(debug)
					DEBUG_SERIAL->println("Sending keepalive");
			}
		}
		if(!client.connected()){
			state=STATE_IDLE;

		}
	}
	else if(state==STATE_AUTH_PENDING){
		char buffer[256];
		if(!client.connected()){
			return;
		}

		int readLength=webSocketClient.getData(buffer);

		if(readLength<=0)
			return;
		if(buffer[0]!=0x02)
			return;
		if(debug)
			DEBUG_SERIAL->println("Rx Auth Resp");
		byte digest[2 + strlen(key)];
		BYTE hash[SHA256_BLOCK_SIZE];
		digest[0] = (challenge >> 8) & 0xFF;
		digest[1] = challenge & 0xFF;
		memcpy(digest+2,key,strlen(key));
		
		Sha256* sha = new Sha256();
		sha->update(digest, 2 + strlen(key));
		sha->final(hash);
		if(memcmp(hash,buffer+1,32)!=0){
			client.stop();
			state=STATE_IDLE;
			return;
		}

		digest[0] = buffer[33];
		digest[1] = buffer[34];

		Sha256* sha2 = new Sha256();
		sha2->update(digest, 2 + strlen(key));
		sha2->final(hash);
		buffer[0] = 0x00;
		buffer[1] = strlen(deviceId);
		memcpy(buffer+2,deviceId,strlen(deviceId));
		memcpy(buffer+buffer[1]+2,hash,32);
		state=STATE_AUTH_COMPLETE;
		webSocketClient.sendData((unsigned char*)buffer,buffer[1] + 34,WS_OPCODE_TEXT);
		lastUplink=millis();
		if(debug)
			DEBUG_SERIAL->println("Tx Auth complete");
	}
	

}
bool MagicLinkWS::write(char* buffer, int length){
	rc4(buffer,length);
	webSocketClient.sendData((unsigned char*)buffer,length,WS_OPCODE_TEXT);
	lastUplink=millis();
	return true;
}
int MagicLinkWS::read(char* buffer){
	if(state!=STATE_AUTH_COMPLETE)
		return -1;
	int readLength=webSocketClient.getData(buffer);
	rc4(buffer,readLength);
	if(readLength>0 && debug){
		for(int i=0;i<readLength;i++){
			DEBUG_SERIAL->print(buffer[i],HEX);
			DEBUG_SERIAL->print(" ");
		}
		DEBUG_SERIAL->println();
	}

	if(buffer[0]!=0x01)
		return -1;
	return readLength;
}

bool MagicLinkWS::isConnected(){
	if(state==STATE_AUTH_COMPLETE)
		return true;
	return false;
}
bool MagicLinkWS::discover(char* deviceId,char* host,int* port){
	HTTPClient http;
	char url[256];
	sprintf(url,DISCOVERY_URL,deviceId);
	http.begin(url); 
	if(debug)
		DEBUG_SERIAL->println("Sending discovery");
	if (http.GET() > 0) { 

		const char* response=http.getString().c_str();
		
		char* idx=strchr(response,':');
		if(idx==NULL){
			http.end();
			return false;
		}
		strncpy(host,response,idx-response);
		if(debug)
			DEBUG_SERIAL->println(host);
		
		*port=atoi(idx+1);
		if(debug)
			DEBUG_SERIAL->println(*port);
		http.end();
		return true;
	}

	else {
		Serial.println("Error on HTTP request");
		http.end();
	}

	return false;
}
void MagicLinkWS::rc4(char* str, int length) {	
	char S[256];
	char enc[256];
	int i, j, x;

	for (i = 0; i < 256; i++) {
		S[i] = i;
	}
	j = 0;
	for (i = 0; i < 256; i++) {
		j = (j + S[i] + key[i % strlen(key)]) % 256;
		x = S[j];
		S[j] = S[i];
		S[i] = x;

	}
	i = j = 0;
	for (int k = 0; k < length; k++) {
		i = (i + 1) % 256;
		j = (j + S[i]) % 256;
		x = S[j];
		S[j] = S[i];
		S[i] = x;

		enc[k] = (str[k] ^ S[(S[i] + S[j]) % 256]);

	}
	memcpy(str,enc,length);
}