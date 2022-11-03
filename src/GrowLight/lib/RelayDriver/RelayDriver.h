#ifndef RELAY_DRIVER_LIB_H
#define RELAY_DRIVER_LIB_H
#include <Arduino.h>

#define UART_BAUD_RATE 115200
#define UART_NUM_BYTES 4

struct rel {
	uint8_t open_seq[4];
	uint8_t close_seq[4];
	uint8_t state = 0;
};

class RelayDriver {
public:
	void init();
	void turn_on(uint8_t r);
	void turn_off(uint8_t r);
	uint8_t get_state(uint8_t r);
private:
	rel r1, r2, r3, r4;
	rel relays[4];
};
#endif // RELAY_DRIVER_LIB_H
