#include "RelayDriver.h"

void RelayDriver::init() {
	Serial.begin(UART_BAUD_RATE);

	rel r1;
	r1.open_seq[0] = 0xA0;	r1.open_seq[1] = 0x01;	r1.open_seq[2] = 0x00;	r1.open_seq[3] = 0xA1;
	r1.close_seq[0] = 0xA0;	r1.close_seq[1] = 0x01;	r1.close_seq[2] = 0x01;	r1.close_seq[3] = 0xA2;

	rel r2;
	r2.open_seq[0] = 0xA0;	r2.open_seq[1] = 0x02;	r2.open_seq[2] = 0x00;	r2.open_seq[3] = 0xA2;
	r2.close_seq[0] = 0xA0;	r2.close_seq[1] = 0x02;	r2.close_seq[2] = 0x01;	r2.close_seq[3] = 0xA3;

	rel r3;
	r3.open_seq[0] = 0xA0;	r3.open_seq[1] = 0x03;	r3.open_seq[2] = 0x00;	r3.open_seq[3] = 0xA3;
	r3.close_seq[0] = 0xA0;	r3.close_seq[1] = 0x03;	r3.close_seq[2] = 0x01;	r3.close_seq[3] = 0xA4;

	rel r4;
	r4.open_seq[0] = 0xA0;	r4.open_seq[1] = 0x04;	r4.open_seq[2] = 0x00;	r4.open_seq[3] = 0xA4;
	r4.close_seq[0] = 0xA0;	r4.close_seq[1] = 0x04;	r4.close_seq[2] = 0x01;	r4.close_seq[3] = 0xA5;

	relays[0] = r1;
	relays[1] = r2;
	relays[2] = r3;
	relays[3] = r4;
}

void RelayDriver::turn_on(uint8_t r) {
	Serial.write(relays[r-1].close_seq, UART_NUM_BYTES);
	relays[r-1].state = 1;
	delay(50);
}

void RelayDriver::turn_off(uint8_t r) {
	Serial.write(relays[r-1].open_seq, UART_NUM_BYTES);
	relays[r-1].state = 0;
	delay(50);
}

uint8_t RelayDriver::get_state(uint8_t r) {
	return relays[r-1].state;
}
