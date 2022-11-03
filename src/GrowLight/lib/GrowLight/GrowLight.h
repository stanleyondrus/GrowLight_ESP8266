#ifndef GROWLIGHT_LIB_H
#define GROWLIGHT_LIB_H
#include <Arduino.h>
#include <NTPClient.h>
#include "../RelayDriver/RelayDriver.h"

struct timer {
	uint8_t startTime[2]; // [min,sec]
	uint8_t endTime[2]; // [min,sec]
	uint8_t relay[4]; // [r1,r2,r3,r4]
};

class GrowBox {
public:
	GrowBox(RelayDriver& rd_a, NTPClient& time_a) : rd(rd_a), time(time_a) {}
	void init();
	uint8_t update();
	void setTimeOffset(int8_t t);
	int8_t getTimeOffset() {return timeOffset;}
	timer t;
	uint8_t mode = 1;
private:
	NTPClient& time;
	RelayDriver& rd;
	int8_t timeOffset = 2;
	unsigned long relayCheckInterval = 1000; //[ms]
	unsigned long relayLastCheck = 0; //[ms]
};
#endif // GROWLIGHT_LIB_H
