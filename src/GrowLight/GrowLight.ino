#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <string.h>
#include <WiFiUdp.h>
#include "WebSocketsServer.h"
#include "NTPClient.h"
#include "GrowLight.h"
#include "RelayDriver.h"

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

const char *ssid = "GrowBox";
const char *password = "GrowBox2020";

const char *OTAName = "GrowBox";
const char *OTAPassword = "GrowBox2020";

const char* mdnsName = "growbox";

WiFiUDP UDP;
NTPClient timeClient(UDP);

RelayDriver rd;
GrowBox gb(rd, timeClient);

void setup() {
	rd.init();
	gb.init();

	WiFi.softAP(ssid, password);

	wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
	wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
	wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

	while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) { delay(250); }
	ArduinoOTA.setHostname(OTAName);
	ArduinoOTA.setPassword(OTAPassword);
	ArduinoOTA.begin();
	SPIFFS.begin();
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	MDNS.begin(mdnsName);
	server.onNotFound(handleNotFound);
	server.begin();
}

void loop() {
	webSocket.loop();
	server.handleClient();
	ArduinoOTA.handle();
	timeClient.update();
	uint8_t s = gb.update();
	if (s == 1 || s == 2) sendTime();
	if (s == 2)	sendRelayState();
}

void handleNotFound(){
	if(!handleFileRead(server.uri())){
		server.send(404, "text/plain", "404: File Not Found");
	}
}

bool handleFileRead(String path) {
	if (path.endsWith("/")) path += "index.html";
	String contentType = getContentType(path);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz)) path += ".gz";
		File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
		file.close();
		return true;
	}
	return false;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
	switch (type) {
	case WStype_DISCONNECTED:
		break;
	case WStype_CONNECTED:
		break;
	case WStype_TEXT:
		if (payload[0] == 'M') {
			uint8_t m = payload[1] - '0';
			gb.mode = m;
			sendMode();
		}
		else if (payload[0] == 'R') {
			uint8_t r = payload[1] - '0';
			uint8_t state = payload[2] - '0';
			if (state)	rd.turn_on(r);
			else rd.turn_off(r);
			sendRelayState();
		}
		else if (payload[0] == 'T') {
			if (payload[1] == 'O') {
				int8_t t = (int8_t) strtol((const char *) &payload[2], NULL, 10);
				gb.setTimeOffset(t);
				sendTimeOffset();
				sendTime();
			}
			else if (payload[1] == 'T') {
				String s = (char*)&payload[2];
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
			else if (payload[1] == 'R') {
				if (payload[2] == '1') gb.t.relay[0] = payload[3] == '1' ? 1 : 0;
				if (payload[2] == '2') gb.t.relay[1] = payload[3] == '1' ? 1 : 0;
				if (payload[2] == '3') gb.t.relay[2] = payload[3] == '1' ? 1 : 0;
				if (payload[2] == '4') gb.t.relay[3] = payload[3] == '1' ? 1 : 0;
				sendTimerRelay();
			}
		}
		else if (payload[0] == '~') {
			sendMode();
			sendRelayState();
			sendTime();
			sendTimeOffset();
			sendTimerData();
			sendTimerRelay();
		}
		break;
	}
}

String getContentType(String filename) {
	if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif"))	return "image/gif";
	else if (filename.endsWith(".jpg"))	return "image/jpeg";
	return "text/plain";
}

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
	char str[3];
	sprintf(str, "%c%d", 'M', gb.mode);
	webSocket.broadcastTXT(str);
}

void sendRelayState() {
	char str[6];
	sprintf(str, "R%d%d%d%d", rd.get_state(1), rd.get_state(2), rd.get_state(3), rd.get_state(4));
	webSocket.broadcastTXT(str);
}

void sendTime() {
	String t = timeClient.getFormattedTime();
	webSocket.broadcastTXT("TC" + t);
}

void sendTimeOffset() {
	String t(gb.getTimeOffset());
	webSocket.broadcastTXT("TO" + t);
}

void sendTimerData() {
	String startH = gb.t.startTime[0] < 10 ? "0" + String(gb.t.startTime[0]) : String(gb.t.startTime[0]);
	String startM = gb.t.startTime[1] < 10 ? "0" + String(gb.t.startTime[1]) : String(gb.t.startTime[1]);
	String endH = gb.t.endTime[0] < 10 ? "0" + String(gb.t.endTime[0]) : String(gb.t.endTime[0]);
	String endM = gb.t.endTime[1] < 10 ? "0" + String(gb.t.endTime[1]) : String(gb.t.endTime[1]);
	String str = "TT" + startH + ":" + startM + ":" + endH + ":" + endM;
	webSocket.broadcastTXT(str);
}

void sendTimerRelay() {
	char str[7];
	sprintf(str, "TR%d%d%d%d", gb.t.relay[0], gb.t.relay[1], gb.t.relay[2], gb.t.relay[3]);
	webSocket.broadcastTXT(str);
}
