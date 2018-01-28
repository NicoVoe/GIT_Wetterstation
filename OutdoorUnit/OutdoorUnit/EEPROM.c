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
uint8_t eeprom_state = 1;
uint8_t eeprom_flag = 0;
uint8_t eeprom_buffer[EEPROM_BUFFER_SIZE] = {0};
circular_buffer eeprom_cb = {eeprom_buffer,0,0,EEPROM_BUFFER_SIZE};
	
//--------------------------------------------------------------------------------
// EEPROM initialisation:
//-------------------------------------------------------------------------------- 
void eeprom_init ()
{
	EEPROM_DDR |= 1 << EEPROM_CS;
	EEPROM_PORT |= 1 << EEPROM_CS;
}

void eeprom_start_reading(void)
{
	eeprom_flag = 1;
}

uint8_t eeprom_get_data(uint8_t *eeprom_data, uint16_t eeprom_data_length)
{
	if (eeprom_state == eeprom_state_wait_for_collecting_data)
	{
		return (cb_pop(&eeprom_cb, eeprom_data, eeprom_data_length));
	}
	else
	{
		return 0;
	}
}

uint8_t eeprom_put_data(uint8_t *eeprom_data, uint16_t eeprom_data_length)
{
	if (eeprom_state == eeprom_state_idle)
	{
		return (cb_push(&eeprom_cb, eeprom_data, eeprom_data_length));
	}
	else
	{
		return 0;
	}
}

static uint8_t eeprom_write_pointer(uint32_t adress, uint32_t pointer, uint16_t length)
{
	eeprom_write_adress(0x20000);
	for(uint8_t i = 0; i<4; i++)
	{
			spi_send_byte((adresss>>(i*8)&0xFF));								// send Data via SPI
	}		
}

static uint8_t eeprom_read_pointer(uint32_t adress, uint32_t pointer, uint16_t length)
{
		eeprom_write_adress(0x20000);
		for(uint8_t i = 0; i<4; i++)
		{
			spi_send_byte((adresss>>(i*8)&0xFF));								// send Data via SPI
		}
}
//--------------------------------------------------------------------------------
// EEPROM state machine:
//--------------------------------------------------------------------------------
void eeprom_state_machine ()
{
	switch(eeprom_state)
	{
		case eeprom_state_idle:		eeprom_idle();	break;
		case eeprom_state_read:		eeprom_read_page();	break;
		case eeprom_state_wait_for_collecting_data: eeprom_wait_for_collecting_data();break;
		case eeprom_state_write:	eeprom_write_page(); break;
		case eeprom_state_erase:		eeprom_erase_page();break;
		default: break;
	}
}

//--------------------------------------------------------------------------------
// Idle
//--------------------------------------------------------------------------------
static void eeprom_idle ()
{

	if(cb_is_full(&eeprom_cb))
	{
		eeprom_state = eeprom_state_write;
	}
	if(eeprom_flag && cb_is_empty(&eeprom_cb)) 
	{
		eeprom_state= eeprom_state_read;
	}
	
}

//--------------------------------------------------------------------------------
// Read instruction:
//--------------------------------------------------------------------------------
static void eeprom_read_page ()
{
	uint8_t data_byte = 0;
	uint16_t i=0;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_READ);									// read instruction
	eeprom_write_adress(256);
	while(!cb_is_full(&eeprom_cb) && i<(EEPROM_BUFFER_SIZE-1))
	{
		data_byte = (spi_send_byte(NOP));							// dummy byte
		cb_push(&eeprom_cb, &data_byte,1);
		i++;
	}
	
	CLEAR_EEPROM_CS
	eeprom_state = eeprom_state_wait_for_collecting_data;
	eeprom_flag=0;

}

static void eeprom_wait_for_collecting_data(void) 
{
	if(cb_is_empty(&eeprom_cb))
	{
		eeprom_state = eeprom_state_idle;
	}
}

//--------------------------------------------------------------------------------
// Write instruction:
//--------------------------------------------------------------------------------
static void eeprom_write_page ()
{
	uint16_t i = 0;
	uint8_t data_byte = 0;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WREN);									// write enable
	CLEAR_EEPROM_CS
	_delay_us(100);
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WRITE);								// write instruction
	eeprom_write_adress(0);
	while((!cb_is_empty(&eeprom_cb)) && i<(EEPROM_BUFFER_SIZE-1))					// writes all data in EEPROM until the Buffer is empty OR the page size ist exeeded
	{
		cb_pop(&eeprom_cb,&data_byte,1);
		spi_send_byte(data_byte);								// send Data via SPI
		i++;													
	}
	CLEAR_EEPROM_CS
	eeprom_state = eeprom_state_erase;
}

//--------------------------------------------------------------------------------
// Page erase instruction:
//--------------------------------------------------------------------------------
static void eeprom_erase_page ()
{
	if(eeprom_flag == 1)
	{
		eeprom_state = eeprom_state_read;
	}
}

//--------------------------------------------------------------------------------
// Read status register:
//--------------------------------------------------------------------------------
static uint8_t eeprom_status ()
{
	uint8_t status = 0;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_RDSR);
	status = spi_send_byte(EEPROM_RDSR);
	CLEAR_EEPROM_CS
	
	return (status);
}

static void eeprom_write_adress(uint32_t adresss)
{
	
	for (uint8_t i = 3; i>0; i--)
	{
		spi_send_byte((adresss>>(i*8)&0xFF));									// 24bit adress with 7 don't care MSB's
	}
}