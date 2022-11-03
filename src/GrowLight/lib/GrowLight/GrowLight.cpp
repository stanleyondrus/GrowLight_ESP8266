#include "GrowLight.h"

void GrowBox::init() {
	time.begin();
	time.setUpdateInterval(60000); // 1m
	setTimeOffset(timeOffset); // 2h

	t.startTime[0] = 10;
	t.startTime[1] = 00;
	t.endTime[0] = 17;
	t.endTime[1] = 30;

	t.relay[0] = 0;
	t.relay[1] = 0;
	t.relay[2] = 0;
	t.relay[3] = 0;
}

void GrowBox::setTimeOffset(int8_t t) {
	timeOffset = t;
	time.setTimeOffset(timeOffset * 3600);
}
	
uint8_t GrowBox::update() {
	if (millis() - relayLastCheck < relayCheckInterval || mode == 1) return 0;
	relayLastCheck = millis();

	uint16_t currentMinutes = time.getHours()*60 + time.getMinutes();
	uint16_t startMinutes = t.startTime[0]*60 + t.startTime[1];
	uint16_t endMinutes = t.endTime[0]*60 + t.endTime[1];
	uint8_t flag = 0;

	if ((endMinutes > startMinutes && currentMinutes >= startMinutes && currentMinutes < endMinutes) ||
		(endMinutes < startMinutes && (currentMinutes >= startMinutes || currentMinutes < endMinutes))) {
		for (uint8_t i = 0; i<4; i++) {
			if (t.relay[i] && !rd.get_state(i+1)) {
				rd.turn_on(i+1);
				flag = 1;
			}
		}
	}
	else {
		for (uint8_t i = 0; i<4; i++) {
			if (t.relay[i] && rd.get_state(i+1)) {
				rd.turn_off(i+1);
				flag = 1;
			}
		}
	}
	return flag ? 2 : 1;
}
