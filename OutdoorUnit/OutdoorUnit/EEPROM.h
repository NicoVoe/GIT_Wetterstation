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

#define EEPROM_DDR		DDRB
#define EEPROM_PORT		PORTB
#define EEPROM_CS		PB0

#define EEPROM_BUFFER_SIZE		257
#define SET_EEPROM_CS			EEPROM_PORT &= ~(1 << EEPROM_CS);
#define CLEAR_EEPROM_CS			EEPROM_PORT |= (1<<EEPROM_CS);
#define CLEAR_EEPROM_REQUEST	eeprom_request = 0;


//--------------------------------------------------------------------------------
// Prototypes:
//--------------------------------------------------------------------------------
void eeprom_init (void);
void eeprom_state_machine (void);
uint8_t eeprom_get_data(uint8_t *eeprom_data, uint16_t eeprom_data_length);
uint8_t eeprom_put_data(uint8_t *eeprom_data, uint16_t eeprom_data_length);
void eeprom_start_reading(void);


static void eeprom_idle (void);
static void eeprom_read_page (void);
static void eeprom_wait_for_collecting_data(void);
static void eeprom_write_page (void);
static void eeprom_erase_page (void);
static uint8_t eeprom_status (void);
static void eeprom_write_adress(uint32_t adresss);
static uint8_t eeprom_write_pointer(uint32_t adress, uint8_t *data, uint16_t length);
static uint8_t eeprom_read_pointer(uint32_t adress, uint8_t *data, uint16_t length);


//--------------------------------------------------------------------------------
// Variables:
//--------------------------------------------------------------------------------
typedef struct
{
	uint8_t adress[4], data_lengh;
	uint8_t *Buffer;
}EEPROM_Dataframe;


enum {eeprom_state_idle = 1, eeprom_state_read, eeprom_state_wait_for_collecting_data, eeprom_state_write, eeprom_state_erase};
enum {read_rq=1, write_rq};


//--------------------------------------------------------------------------------
// Commands:
//--------------------------------------------------------------------------------

#define EEPROM_RDSR			0b00000101			// Read STATUS register
#define EEPROM_READ			0b00000011			// Read data from memory array beginning at selected address
#define EEPROM_WRITE		0b00000010			// Write data to memory array beginning at selected address
#define EEPROM_WREN			0b00000110			// Set the write enable latch (enable write operations)
#define EEPROM_WRDI			0b00000100			// Reset the write enable latch (disable write operations)
#define EEPROM_PAGE_ERASE	0b01000010			// Page Erase – erase one page in memory array
#define NOP					0xFF






#endif