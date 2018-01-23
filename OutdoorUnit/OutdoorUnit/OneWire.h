#ifndef OneWire_H_
#define OneWire_H_

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "system_time.h"

//DS18B20 Befehlssatz:
#define MATCH_ROM		0x55
#define OW_SKIP_ROM		0xCC
#define	SEARCH_ROM		0xF0
#define CONVERT_T		0x44	
#define OW_READ_ROM		0x33	
#define OW_READ_SCRATCHPAD	0xBE
#define WRITE			0x4E
#define EE_WRITE		0x48
#define EE_RECALL		0xB8
#define PRESENCE_PULSE	0xF0
//sonstiges
#define OW_RW_BAUD		8			// 8 -> 115200
#define OW_RESET_BAUD	103			// 103 -> 9600
//One Wire interne Flags
#define OW_CONV_REQUEST		0
#define OW_READ_REQUEST		1
#define OW_CONV_COMPLETE	5
#define OW_FAIL				7

//Macros:
#define OW_SET_CONV_REQUEST ow_flags |= (1 << OW_CONV_REQUEST);
#define OW_SET_READ_REQUEST ow_flags |= (1 << OW_READ_REQUEST);
#define OW_SET_CONV_COMPLETE ow_flags |= (1 << OW_CONV_COMPLETE);
#define OW_SET_FAIL ow_flags |= (1 << OW_FAIL);

#define OW_CLEAR_CONV_REQUEST ow_flags &= ~(1 << OW_CONV_REQUEST);
#define OW_CLEAR_READ_REQUEST ow_flags &= ~(1 << OW_READ_REQUEST);
#define OW_CLEAR_CONV_COMPLETE ow_flags &= ~(1 << OW_CONV_COMPLETE);
#define OW_CLEAR_FAIL ow_flags &= ~(1 << OW_FAIL);

#define OW_CONV_REQUEST_IS_SET ow_flags & (1 << OW_CONV_REQUEST)
#define OW_CONV_REQUEST_IS_CLEAR !(ow_flags & (1 << OW_CONV_REQUEST))
#define OW_READ_REQUEST_IS_SET ow_flags & (1 << OW_READ_REQUEST)
#define OW_READ_REQUEST_IS_CLEAR !(ow_flags & (1 << OW_READ_REQUEST))
#define OW_CONV_COMPLETE_IS_SET ow_flags & (1 << OW_CONV_COMPLETE)
#define OW_CONV_COMPLETE_IS_CLEAR !(ow_flags & (1 << OW_CONV_COMPLETE))
#define OW_FAIL_IS_SET ow_flags & (1 << OW_FAIL)
#define OW_FAIL_IS_CLEAR !(ow_flags & (1 << OW_FAIL))

extern uint8_t ow_flags;

//--------------------------------------------------------------------------------
// Prototypes:
//--------------------------------------------------------------------------------
void ow_init(void);
void ow_start(void);
void ow_get_temp(uint64_t id, uint64_t id2);
void ow_state_machine(void);
void ow_idle(void);
void ow_start_conversation(void);
void ow_conversion(void);
uint8_t ow_reset(void);
uint8_t ow_read_byte (void);
void ow_write_byte(uint8_t Data_Byte);
void ow_write_rom(uint64_t ROM);
uint8_t ow_write_bit(uint8_t Data_Bit);
void uart_send_byte(uint8_t data);
void ow_set_baud(uint8_t Baud_Rate);
void ow_read_temp(void);
uint64_t ow_read_rom(void);
void ow_fail(void);

#endif