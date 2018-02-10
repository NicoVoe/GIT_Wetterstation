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
uint8_t eeprom_current_cs = 0;
uint32_t eeprom_erase_adress = 0;
uint16_t eeprom_write_cycle_delay = 0xFFFF;
uint8_t eeprom_buffer[EEPROM_BUFFER_SIZE] = {0};
circular_buffer eeprom_cb = {eeprom_buffer,0,0,EEPROM_BUFFER_SIZE};
	
//--------------------------------------------------------------------------------
// static function prototypes:
//--------------------------------------------------------------------------------	
static void eeprom_idle (void);
static void eeprom_read_page (void);
static void eeprom_wait_for_collecting_data(void);
static void eeprom_write_page (void);
static void eeprom_write_cycle(void);
static void eeprom_erase_page (void);
static void eeprom_wait_for_erase(void);
static uint8_t eeprom_status (void);
static void eeprom_write_adress(uint32_t adresss);
static void eeprom_write_wpointer(uint32_t pointer_adress);
static uint32_t eeprom_read_wpointer(void);
static uint32_t eeprom_read_rpointer(void);
static void eeprom_write_rpointer(uint32_t pointer_adress);
static void eeprom_refresh_rpointer(void);
static void eeprom_wpointer_cycle(void);
static void eeprom_refresh_wpointer(void);
	
//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------
void eeprom_init ()
{
	EEPROM_DDR |= EEPROM_DDR_VALUE;
	EEPROM_PORT |= EEPROM_DDR_VALUE;
	uint64_t read_out_header = eeprom_read_header(0);
	
	eeprom_write_rpointer(0x00);
	_delay_ms(10);
	
	if(read_out_header != EEPROM_HEADER_ID) 
	{
		write_header(0, EEPROM_HEADER_ID);
		_delay_ms(10);
		eeprom_write_wpointer(0x00);
		_delay_ms(10);
		eeprom_write_rpointer(0x00);
		_delay_ms(10);
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
uint8_t eeprom_start_reading(uint8_t chip_select)
{
	if(eeprom_state == eeprom_state_idle) 
	{
		uint32_t wpointer = eeprom_read_wpointer();
		uint32_t rpointer = eeprom_read_rpointer();
		if(wpointer == rpointer)
		{
			return 2;
		}
		else
		{
			eeprom_flag = 1;
			eeprom_current_cs = chip_select;
			return 1;
		}
	}
	else
	{
		return 0;
	}
}

void read_irgendwas(uint32_t adress, uint8_t *data, uint16_t length)
{
	eeprom_current_cs = 0;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_READ);								// read instruction
	eeprom_write_adress(adress);
	for(int i=0; i<length; i++)
	{
		data[i] = spi_send_byte(NOP);		// dummy byte
	}
	CLEAR_EEPROM_CS
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
uint64_t eeprom_read_header(uint8_t chip_select) 
{
	uint8_t header[8] = {0};
	eeprom_current_cs = chip_select;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_READ);								// read instruction
	eeprom_write_adress(EEPROM_END_ADRESS-7);
	for(int i=0; i<8; i++)
	{
		header[i] |= spi_send_byte(NOP);		// dummy byte
	}
	CLEAR_EEPROM_CS
return *(uint64_t*)&header[0];
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void write_header(uint8_t chip_select, uint64_t header)
{
	eeprom_current_cs = chip_select;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WREN);									// write enable
	CLEAR_EEPROM_CS
	_delay_us(100);
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WRITE);						// write instruction
	eeprom_write_adress(EEPROM_END_ADRESS-7);
	for(int i=0; i<8; i++)
	{
		spi_send_byte(((header>>(i*8))&0xFF));				// dummy byte
	}
	CLEAR_EEPROM_CS
	_delay_ms(10);
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
uint8_t eeprom_get_data(uint8_t *eeprom_data, uint16_t eeprom_data_length)
{
	if (eeprom_state == eeprom_state_wait_for_collecting_data)
	{
		return(cb_pop(&eeprom_cb, eeprom_data, eeprom_data_length));
	}
	else
	{
		return 0;
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
uint8_t eeprom_put_data(uint8_t *eeprom_data, uint16_t eeprom_data_length, uint8_t chip_select)
{
	if (eeprom_state == eeprom_state_idle)
	{
		uint32_t wpointer = eeprom_read_wpointer();
		uint32_t rpointer = eeprom_read_rpointer();
		
		if(((wpointer+1) == rpointer)||((rpointer==0)&&((wpointer+1)==EEPROM_LAST_ADRESS)))
		{
			return 2;
		}
		else
		{
			eeprom_current_cs = chip_select;
			return (cb_push(&eeprom_cb, eeprom_data, eeprom_data_length));
		}
	}
	else
	{
		return 0;
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
uint8_t eeprom_start_erase_page(uint32_t adress)
{
	if(eeprom_state == eeprom_state_idle) 
	{
		eeprom_erase_adress = adress;
		eeprom_flag = 2;
		return 1;
	}
	else
	{
		return 0;
	}
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
static void eeprom_write_wpointer(uint32_t pointer_adress)
{
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WREN);									// write enable
	CLEAR_EEPROM_CS
	_delay_us(100);
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WRITE);
	eeprom_write_adress(EEPROM_WPOINTER_ADRESS);
	for(uint8_t i = 0; i<4; i++)
	{
		spi_send_byte((pointer_adress>>(i*8)&0xFF));			// send Data via SPI
	}	
	CLEAR_EEPROM_CS	
}

static uint32_t eeprom_read_wpointer(void)
{
	uint32_t adress = 0;
	read_irgendwas(EEPROM_WPOINTER_ADRESS, &adress, 4);
return adress;
}

static void eeprom_write_rpointer(uint32_t pointer_adress)
{
	eeprom_current_cs = 0;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WREN);									// write enable
	CLEAR_EEPROM_CS
	_delay_us(100);
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WRITE);
	eeprom_write_adress(EEPROM_RPOINTER_ADRESS);
	for(uint8_t i = 0; i<4; i++)
	{
		spi_send_byte((pointer_adress>>(i*8)&0xFF));								// send Data via SPI
	}
	CLEAR_EEPROM_CS
}

static uint32_t eeprom_read_rpointer(void)
{
	uint32_t adress = 0;
	read_irgendwas(EEPROM_RPOINTER_ADRESS, &adress, 4);
return adress;
}
//--------------------------------------------------------------------------------
// EEPROM state machine:
//--------------------------------------------------------------------------------
void eeprom_state_machine ()
{
	switch(eeprom_state)
	{
		case eeprom_state_idle:						eeprom_idle();	break;
		case eeprom_state_read:						eeprom_read_page();	break;
		case eeprom_state_refresh_rpointer:			eeprom_refresh_rpointer();break;
		case eeprom_state_wait_for_collecting_data: eeprom_wait_for_collecting_data();break;
		case eeprom_state_write:					eeprom_write_page(); break;
		case eeprom_state_write_cycle:				eeprom_write_cycle();break;
		case eeprom_state_refresh_wpointer:			eeprom_refresh_wpointer();break;
		case eeprom_state_wpointer_cycle:			eeprom_wpointer_cycle();break;
		case eeprom_state_erase:					eeprom_erase_page();break;
		case eeprom_state_wait_for_erase:			eeprom_wait_for_erase();break;
		default: break;
	}
}

//--------------------------------------------------------------------------------
// Idle
//--------------------------------------------------------------------------------
static void eeprom_idle (void)
{
	if(cb_is_full(&eeprom_cb))
	{
		eeprom_state = eeprom_state_write;
	}
	if(eeprom_flag == 1 && cb_is_empty(&eeprom_cb)) 
	{
		eeprom_state = eeprom_state_read;
	}
	if(eeprom_flag == 2)
	{
		eeprom_state = eeprom_state_erase;
	}
}

//--------------------------------------------------------------------------------
// Read instruction:
//--------------------------------------------------------------------------------
static void eeprom_read_page (void)
{
	uint8_t data_byte = 0;
	uint16_t i=0;
	uint32_t rpointer = eeprom_read_rpointer();
	SET_EEPROM_CS
	spi_send_byte(EEPROM_READ);									// read instruction
	eeprom_write_adress(rpointer);
	while(!cb_is_full(&eeprom_cb) && i<(EEPROM_BUFFER_SIZE-1))
	{
		data_byte = (spi_send_byte(NOP));							// dummy byte
		cb_push(&eeprom_cb, &data_byte,1);
		i++;
	}
	CLEAR_EEPROM_CS
	eeprom_state = eeprom_state_refresh_rpointer;
	eeprom_flag=0;
}
//--------------------------------------------------------------------------------
// Read instruction:
//--------------------------------------------------------------------------------
static void eeprom_refresh_rpointer(void)
{
	uint32_t rpointer = eeprom_read_rpointer();
	eeprom_write_rpointer(rpointer+0x100);
	eeprom_state = eeprom_state_wait_for_collecting_data;	
}
//--------------------------------------------------------------------------------
// Write instruction:
//--------------------------------------------------------------------------------


static void eeprom_wait_for_collecting_data(void) 
{
	if(cb_is_empty(&eeprom_cb))
	{
		if(achieved_time(EEPROM_WRITE_CYCLE_MS, &eeprom_write_cycle_delay))
		{	
			eeprom_state = eeprom_state_idle;
		}
	}
}

//--------------------------------------------------------------------------------
// Write instruction:
//--------------------------------------------------------------------------------
static void eeprom_write_page ()
{
	uint16_t i = 0;
	uint8_t data_byte = 0;
	uint32_t wpointer = eeprom_read_wpointer();
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WREN);									// write enable
	CLEAR_EEPROM_CS
	_delay_us(100);
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WRITE);								// write instruction
	eeprom_write_adress(wpointer);
	while((!cb_is_empty(&eeprom_cb)) && i<(EEPROM_BUFFER_SIZE-1))					// writes all data in EEPROM until the Buffer is empty OR the page size ist exeeded
	{
		cb_pop(&eeprom_cb,&data_byte,1);
		spi_send_byte(data_byte);								// send Data via SPI
		i++;													
	}
	CLEAR_EEPROM_CS
	eeprom_state = eeprom_state_write_cycle;
}

//--------------------------------------------------------------------------------
// Wait for eeprom to write data
//--------------------------------------------------------------------------------
static void eeprom_write_cycle(void)
{
	if(achieved_time(EEPROM_WRITE_CYCLE_MS ,&eeprom_write_cycle_delay)) 
	{
		eeprom_state = eeprom_state_refresh_wpointer;
	}
}
//--------------------------------------------------------------------------------
// Wait for eeprom to write data
//--------------------------------------------------------------------------------
static void eeprom_refresh_wpointer(void)
{
	uint32_t wpointer = eeprom_read_wpointer();
	nrf_send(&wpointer, 4);
	eeprom_write_wpointer(wpointer+0x100);
	eeprom_state = eeprom_state_wpointer_cycle;
}
//--------------------------------------------------------------------------------
// Wait for eeprom to write data
//--------------------------------------------------------------------------------
static void eeprom_wpointer_cycle(void)
{
	if(achieved_time(EEPROM_WRITE_CYCLE_MS ,&eeprom_write_cycle_delay))
	{
		eeprom_state = eeprom_state_idle;
	}
}
//--------------------------------------------------------------------------------
// Page erase instruction:
//--------------------------------------------------------------------------------
static void eeprom_erase_page (void)
{
	SET_EEPROM_CS
	spi_send_byte(EEPROM_WREN);
	CLEAR_EEPROM_CS
	_delay_us(100);
	SET_EEPROM_CS
	spi_send_byte(EEPROM_PAGE_ERASE);
	eeprom_write_adress(eeprom_erase_adress);
	CLEAR_EEPROM_CS
	
	eeprom_state = eeprom_state_wait_for_erase;
}
//--------------------------------------------------------------------------------
// Wait for erase 
//--------------------------------------------------------------------------------
static void eeprom_wait_for_erase(void)
{
	if(achieved_time(EEPROM_WRITE_CYCLE_MS,&eeprom_write_cycle_delay))
	{
		eeprom_state = eeprom_state_idle;
	}
}

//--------------------------------------------------------------------------------
// Read status register:
//--------------------------------------------------------------------------------
static uint8_t eeprom_status(void)
{
	uint8_t status = 0;
	SET_EEPROM_CS
	spi_send_byte(EEPROM_RDSR);
	status = spi_send_byte(EEPROM_RDSR);
	CLEAR_EEPROM_CS
	
	return (status);
}

static void eeprom_write_adress(uint32_t adress)
{
	//uint8_t test[3]={0x00,0x01,0x00};
	for (uint8_t i = 3; i>0; i--)
	{
		spi_send_byte(adress>>((i-1)*8)&0xFF);									// 24bit adress with 7 don't care MSB's
	}
}