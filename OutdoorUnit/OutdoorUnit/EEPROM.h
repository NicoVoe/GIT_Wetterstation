//
//		EEPROM Library
//
//		date: 06.01.2018
//////////////////////////////////////////////////////////////////////////////////

#ifndef EEPROM_H_
#define EEPROM_H_

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "SPI.h"
#include "CircularBuffer.h"
#include "system_time.h"
#include "NRF24L01.h"

#define EEPROM_DDR				DDRB
#define EEPROM_PORT				PORTB
#define EEPROM_BUFFER_SIZE		257
#define EEPROM_DDR_VALUE		((1<<PB0)|(1<<PB1))
#define EEPROM_HEADER_ID		0x576574746572CAFE		//muss exakt 6 bytes lang sein  
#define EEPROM_PAGE_SIZE		256					// in bytes
#define EEPROM_END_ADRESS		0x1FFFF
#define EEPROM_WRITE_CYCLE_MS	10 



#define EEPROM_LAST_ADRESS		EEPROM_END_ADRESS-EEPROM_PAGE_SIZE
#define EEPROM_WPOINTER_ADRESS	0x1FF00
#define EEPROM_RPOINTER_ADRESS	0x1FF10
#define SET_EEPROM_CS			EEPROM_PORT &= ~(1 << eeprom_current_cs);
#define CLEAR_EEPROM_CS			EEPROM_PORT |= (1 << eeprom_current_cs);
#define SET_EEPROM_REQUEST		eeprom_flag |= (1 << 0);
#define CLEAR_EEPROM_REQUEST	eeprom_flag &= ~(1 << 0);
#define SET_EEPROM_OVERFLOW		eeprom_flag |= (1 << 1);
#define CLEAR_EEPROM_OVERFLOW	eeprom_flag &= ~(1 << 1);


//--------------------------------------------------------------------------------
// Prototypes:
//--------------------------------------------------------------------------------
void eeprom_init (void);
void eeprom_state_machine (void);
uint8_t eeprom_get_data(uint8_t *eeprom_data, uint16_t eeprom_data_length);
uint8_t eeprom_put_data(uint8_t *eeprom_data, uint16_t eeprom_data_length, uint8_t chip_select);
uint8_t eeprom_start_reading(uint8_t chip_select);
uint64_t eeprom_read_header(uint8_t chip_select);
void write_header(uint8_t chip_select, uint64_t header);
uint64_t eeprom_read_header(uint8_t chip_select);
void read_irgendwas(uint32_t adress, uint8_t *data, uint16_t length);
uint8_t eeprom_start_erase_page(uint32_t adress);

//--------------------------------------------------------------------------------
// Variables:
//--------------------------------------------------------------------------------
typedef struct
{
	uint8_t adress[4], data_lengh;
	uint8_t *Buffer;
}EEPROM_Dataframe;

enum {eeprom_state_idle = 1, eeprom_state_read, eeprom_state_refresh_rpointer, eeprom_state_wait_for_collecting_data, eeprom_state_write, eeprom_state_write_cycle, eeprom_state_refresh_wpointer, eeprom_state_wpointer_cycle, eeprom_state_erase, eeprom_state_wait_for_erase};

extern uint8_t eeprom_current_cs;
extern uint8_t eeprom_flag;

//--------------------------------------------------------------------------------
// Commands:
//--------------------------------------------------------------------------------

#define EEPROM_RDSR			0b00000101			// Read STATUS register
#define EEPROM_READ			0b00000011			// Read data from memory array beginning at selected address
#define EEPROM_WRITE		0b00000010			// Write data to memory array beginning at selected address
#define EEPROM_WREN			0b00000110			// Set the write enable latch (enable write operations)
#define EEPROM_WRDI			0b00000100			// Reset the write enable latch (disable write operations)
#define EEPROM_PAGE_ERASE	0b01000010			// Page Erase – erase one page in memory array
#define EEPROM_CHIP_ERASE   0b11000111			// Chip Erase – erase all sectors in memory array
#define NOP					0xFF


#endif