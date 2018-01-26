//
//		EEPROM Library
//		
//		date: 06.01.2018
//////////////////////////////////////////////////////////////////////////////////

#include "EEPROM.h"

//--------------------------------------------------------------------------------
// variables:
//-------------------------------------------------------------------------------- 

uint8_t eeprom_request = 0;
uint8_t eeprom_state = 0;
EEPROM_Dataframe eeprom_data = {{0},0,{0}}; 
	
//--------------------------------------------------------------------------------
// EEPROM initialisation:
//-------------------------------------------------------------------------------- 
void eeprom_init ()
{
	EEPROM_DDR = 1 << EEPROM_CS;
	EEPROM_PORT = 1 << EEPROM_CS;
}

//--------------------------------------------------------------------------------
// EEPROM state machine:
//--------------------------------------------------------------------------------
void eeprom_state_machine ()
{
	switch(eeprom_state)
	{
		case idle_eeprom: eeprom_idle(); break;
		case read_status_eeprom: eeprom_status; break;
		case read_eeprom: eeprom_read(); break;
		case write_eeprom: eeprom_write(); break;
		case erase_eeprom: eeprom_erase(); break;

		default: break;
	}
}

//--------------------------------------------------------------------------------
// Instruction request
//--------------------------------------------------------------------------------
void eeprom_set_request(uint32_t adress, uint8_t *buffer, uint8_t data_lengh, uint8_t instruction)
{
	for (uint8_t i = 0; i<=2; i++)
	{
		eeprom_data.adress[i] =  (uint8_t)(adress>>(i*8));
	}
	eeprom_data.Buffer = buffer;
	eeprom_data.data_lengh = data_lengh;
	switch (instruction)											// available instructions
	{
		case 0: eeprom_state = read_status_eeprom; break;
		case 1: eeprom_state = read_eeprom; break;
		case 2: eeprom_state = write_eeprom; break;
		case 3: eeprom_state = erase_eeprom; break;
		default: break;
		
	}
}

//--------------------------------------------------------------------------------
// Idle
//--------------------------------------------------------------------------------
void eeprom_idle ()
{
	if ( eeprom_status() & (1<<0))
	{
		eeprom_state = idle_eeprom;							// busy
	}
}

//--------------------------------------------------------------------------------
// Read instruction:
//--------------------------------------------------------------------------------
void eeprom_read ()
{
	SET_EEPROM_CS
	SPI_SendByte(EEPROM_READ);									// read instruction
	for (uint8_t i = 3; i>0; i--)
	{
		SPI_SendByte(eeprom_data.adress[i-1]);					// 24bit adress with 7 don't care MSB's
	}
	for (uint8_t i = 0; i < eeprom_data.data_lengh; i++)
	{
		eeprom_data.Buffer[i] = SPI_SendByte(NOP);				// dummy byte
	}
	
	CLEAR_EEPROM_CS
	eeprom_state = idle_eeprom;
	
}

//--------------------------------------------------------------------------------
// Write instruction:
//--------------------------------------------------------------------------------
void eeprom_write ()
{
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WREN);									// write enable
	CLEAR_EEPROM_CS
	_delay_us(100);
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WRITE);									// write instruction
	for (uint8_t i = 3; i>0; i--)
	{
		spi_send_byte(eeprom_data.adress[i-1]);					// 24bit adress with 7 don't care MSB's
	}
	for (uint8_t i = 0; i < eeprom_data.data_lengh; i++)
	{
		spi_send_byte(eeprom_data.Buffer[i]);					// Data
	}
	CLEAR_EEPROM_CS
	eeprom_state = idle_eeprom;
}

//--------------------------------------------------------------------------------
// Page erase instruction:
//--------------------------------------------------------------------------------
void eeprom_erase ()
{
	
}

//--------------------------------------------------------------------------------
// Read status register:
//--------------------------------------------------------------------------------
uint8_t eeprom_status ()
{
	uint8_t status = 0;
	SET_EEPROM_CS
	spi_send_byte(RDSR);
	status = SPI_SendByte(RDSR);
	CLEAR_EEPROM_CS
	
	eeprom_state = idle_eeprom;
	return (status);
}