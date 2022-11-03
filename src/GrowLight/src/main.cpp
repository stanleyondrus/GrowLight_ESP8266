#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <string.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AsyncElegantOTA.h>
#include "GrowLight.h"  
#include "RelayDriver.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

char incoming_packet[50];
uint8_t process_flag = 0;

ESP8266WiFiMulti wifi_multi;

const char *ssid = "<ssid2>";
const char *password = "<password>";

const char *ssid1 = "<ssid1>";
const char *password1 = "<password1>";

const char *ssid2 = "<ssid2>";
const char *password2 = "<password2>";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

RelayDriver rd;
GrowBox gb(rd, timeClient);

String getValue(String data, char separator, int index) {
	int found = 0;
	int strIndex[] = {0, -1};
	int maxIndex = data.length()-1;

	for(int i=0; i<=maxIndex && found<=index; i++){
		if(data.charAt(i)==separator || i==maxIndex){
			found++;
			strIndex[0] = strIndex[1]+1;
			strIndex[1] = (i == maxIndex) ? i+1 : i;
		}
	}
	return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void sendMode() {
	char str[5];
	sprintf(str, "%c%d", 'M', gb.mode);
	ws.textAll(str);
}

void sendRelayState() {
	char str[10];
	sprintf(str, "R%d%d%d%d", rd.get_state(1), rd.get_state(2), rd.get_state(3), rd.get_state(4));
	ws.textAll(str);
}

void sendTime() {
	ws.textAll("TC" + timeClient.getFormattedTime());
}

void sendTimeOffset() {
  char str[10];
	sprintf(str, "TO%d", gb.getTimeOffset());
	ws.textAll(str);
}

void sendTimerData() {
	String startH = gb.t.startTime[0] < 10 ? "0" + String(gb.t.startTime[0]) : String(gb.t.startTime[0]);
	String startM = gb.t.startTime[1] < 10 ? "0" + String(gb.t.startTime[1]) : String(gb.t.startTime[1]);
	String endH = gb.t.endTime[0] < 10 ? "0" + String(gb.t.endTime[0]) : String(gb.t.endTime[0]);
	String endM = gb.t.endTime[1] < 10 ? "0" + String(gb.t.endTime[1]) : String(gb.t.endTime[1]);
	String str = "TT" + startH + ":" + startM + ":" + endH + ":" + endM;
	ws.textAll(str);
}

void sendTimerRelay() {
	char str[10];
	sprintf(str, "TR%d%d%d%d", gb.t.relay[0], gb.t.relay[1], gb.t.relay[2], gb.t.relay[3]);
	ws.textAll(str);
}

void parseMessage(char* msg) {
  if (msg[0] == 'M') {
    uint8_t m = msg[1] - '0';
    gb.mode = m;
    sendMode();
  }
  else if (msg[0] == 'R') {
    uint8_t r = msg[1] - '0';
    uint8_t state = msg[2] - '0';
    if (state)	rd.turn_on(r);
    else rd.turn_off(r);
    sendRelayState();
  }
  else if (msg[0] == 'T') {
    if (msg[1] == 'O') {
      int8_t t = (int8_t)strtol((const char*)&msg[2], NULL, 10);
      gb.setTimeOffset(t);
      sendTimeOffset();
      sendTime();
    }
    else if (msg[1] == 'T') {
      String s = (char*)&msg[2];
      uint8_t startTime_h = getValue(s, ':', 0).toInt();
      uint8_t startTime_m = getValue(s, ':', 1).toInt();
      uint8_t endTime_h = getValue(s, ':', 2).toInt();
      uint8_t endTime_m = getValue(s, ':', 3).toInt();
      gb.t.startTime[0] = startTime_h;
      gb.t.startTime[1] = startTime_m;
      gb.t.endTime[0] = endTime_h;
      gb.t.endTime[1] = endTime_m;
      sendTimerData();
    }
    else if (msg[1] == 'R') {
      if (msg[2] == '1') gb.t.relay[0] = msg[3] == '1' ? 1 : 0;
      if (msg[2] == '2') gb.t.relay[1] = msg[3] == '1' ? 1 : 0;
      if (msg[2] == '3') gb.t.relay[2] = msg[3] == '1' ? 1 : 0;
      if (msg[2] == '4') gb.t.relay[3] = msg[3] == '1' ? 1 : 0;
      sendTimerRelay();
    }
  }
  else if (msg[0] == '~') {
    sendMode();
    sendRelayState();
    sendTime();
    sendTimeOffset();
    sendTimerData();
    sendTimerRelay();
  }
}

void processIncomingPacket() {
    if (process_flag == 0) return;
    char* packet = strtok((char*)incoming_packet, ";"); // split on ;
    while (packet != 0) {
        parseMessage(packet);
        packet = strtok(0, ";"); // search for another
    }
    process_flag = 0;
}

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
    case WS_EVT_CONNECT:
        // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        // Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
    {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        // Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            data[len] = 0;
            strcpy(incoming_packet, (char*)data);
            process_flag = 1;
        }
        break;
    }
    case WS_EVT_ERROR:
        // Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
        break;
    case WS_EVT_PONG:
        // Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
        break;
    }
}

void setup() {
	// Serial.begin(115200);
  rd.init();
	gb.init();

  // if (!LittleFS.begin()) { Serial.println("An error has occurred while mounting LittleFS"); }
  // else { Serial.println("LittleFS mounted successfully"); }

  LittleFS.begin();

  wifi_multi.addAP(ssid,password);
  wifi_multi.addAP(ssid1,password1);
  wifi_multi.addAP(ssid2,password2);  
 
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi ..");
  // while (WiFi.status() != WL_CONNECTED) {
  while (wifi_multi.run(5000) != WL_CONNECTED) {
    // Serial.print('.');
    delay(1000);
  }
  // Serial.println(WiFi.localIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  AsyncElegantOTA.begin(&server);
  
  server.begin();
}

void loop() {
	ws.cleanupClients();
  timeClient.update();
  processIncomingPacket();
	uint8_t s = gb.update();
	if (s == 1 || s == 2) sendTime();
	if (s == 2)	sendRelayState();
}