#include "WebSocketClient.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Base64.h>
#include "sha256.h"
#include <Stream.h>

#define DISCOVERY_URL "http://magicblocks.io/index.php/welcome/epDiscovery/%s"
#define WS_PATH "/epConnect"
#define KA_INTERVAL 60000
#define STATE_IDLE 0
#define STATE_AUTH_PENDING 1
#define STATE_AUTH_COMPLETE 2

class MagicLinkWS{
public:
	void init(Stream &d,bool en);
	bool connect(char* d,char* k);
	void loop();
	bool write(char* buffer, int length);
	int read(char* buffer);
	bool isAvailable();
	bool isConnected();
private:
	bool discover(char* deviceId,char* host,int* port);
	void rc4(char* str, int length);

	Stream* DEBUG_SERIAL;
	bool debug=false;
	WebSocketClient webSocketClient;
	WiFiClient client;
	char* deviceId;
	char* key;
	char host[64];
	int port;
	bool isAuthenticated=false;
	unsigned long challenge;
	int state=0;
	unsigned long lastUplink=0;
	
};