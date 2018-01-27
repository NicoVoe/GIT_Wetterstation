#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_

#include <avr/io.h>

#define DEBUG_LED_DDR DDRB
#define DEBUG_LED_PORT PORTB
#define DEBUG_LED PB2

typedef struct
{
	uint8_t *buffer;
	uint8_t write_index;
	uint8_t read_index;
	uint8_t size;
}circular_buffer;

uint8_t cb_push(circular_buffer *cb, uint8_t *data, uint8_t length);
uint8_t cb_pop(circular_buffer *cb, uint8_t *data, uint8_t length);
uint8_t cb_is_empty(circular_buffer *cb);

#endif /* CIRCULARBUFFER_H_ */