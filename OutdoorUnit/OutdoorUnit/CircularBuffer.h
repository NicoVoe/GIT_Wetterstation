//*************************************************************************************************
//		One Wire Library
//
//		date: 24.01.2018
//*************************************************************************************************
// Version 1.0

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_

#include <avr/io.h>
#include <inttypes.h>

typedef struct
{
	uint8_t *buffer;
	uint16_t write_index;
	uint16_t read_index;
	uint16_t size;
}circular_buffer;

uint8_t cb_push(circular_buffer *cb, uint8_t *data, uint16_t length);
uint16_t cb_pop(circular_buffer *cb, uint8_t *data, uint16_t length);
uint8_t cb_is_empty(circular_buffer *cb);
uint8_t cb_is_full(circular_buffer *cb);
void cb_delete(circular_buffer *cb);

#endif /* CIRCULARBUFFER_H_ */