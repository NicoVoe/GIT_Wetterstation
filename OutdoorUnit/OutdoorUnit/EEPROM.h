//
//		EEPROM Library
//
//		date: 06.01.2018
//////////////////////////////////////////////////////////////////////////////////

#ifndef EEPROM_H_
#define EEPROM_H_

#include <avr/io.h>
#include <inttypes.h>
#include "SPI.h"

#define EEPROM_DDR		DDRB
#define EEPROM_PORT		PORTB
#define EEPROM_CS		PB0

#define SET_EEPROM_CS			EEPROM_PORT &= ~(1 << EEPROM_CS);
#define CLEAR_EEPROM_CS			EEPROM_PORT |= (1<<EEPROM_CS);
#define CLEAR_EEPROM_REQUEST	eeprom_request = 0;


//--------------------------------------------------------------------------------
// Prototypes:
//--------------------------------------------------------------------------------

void eeprom_init ();
void eeprom_state_machine ();
void eeprom_idle ();
void eeprom_set_request (uint32_t adress, uint8_t *buffer, uint8_t data_lengh, uint8_t direction);
void eeprom_read ();
void eeprom_write ();
void eeprom_erase ();
uint8_t eeprom_status ();

//--------------------------------------------------------------------------------
// Variables:
//--------------------------------------------------------------------------------
typedef struct
{
	uint8_t adress[4], data_lengh;
	uint8_t *Buffer;
}EEPROM_Dataframe;


enum {idle_eeprom = 1, read_status_eeprom, wait_for_standby_eeprom, read_eeprom, write_setup_eeprom, write_eeprom, erase_eeprom};
enum {read_rq=1, write_rq};


//--------------------------------------------------------------------------------
// Commands:
//--------------------------------------------------------------------------------

#define RDSR			0b00000101
#define EEPROM_READ		0b00000011
#define EEPROM_WRITE	0b00000010
#define EEPROM_WREN		0b00000110
#define NOP				0xFF






#endif