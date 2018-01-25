//*************************************************************************************************
//		One Wire Library
//
//		date: 24.01.2018
//*************************************************************************************************
// version ##

#include "OneWire.h"

/////////////////////////////////////////////////////////////////////////////////
// variables
////////////////////////////////////////////////////////////////////////////////

uint16_t conv_delay = 0xFFFF;
int16_t ow_temp = 0, ow_temp2 = 0;
enum {ow_state_idle = 1, ow_state_start_conversation, ow_state_wait_for_conv, temp_data_available_ow, rw_register_ow, ow_state_start_read, ow_state_fail};
uint8_t ow_state = 1, ow_flags = 0;
uint64_t ow_current_id=0;
int (*ptr) (char *s);

// UART initialisieren

void ow_init(void)
{
	// Baudrate einstellen (Normaler Modus)
	UBRRH = 0;
	UBRRL = 103;							// 25 -> 2400Baud, 103->9600Baud 8-> 115200Baud

	// Aktivieren des Empfängers, des Senders und des "Daten empfangen"-Interrupts
	UCSRB |= (0<<RXCIE)|(0<<TXCIE)|(1<<RXEN)|(1<<TXEN);

	// Einstellen des Datenformats: 8 Datenbits, 1 Stoppbit
	UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRA |= (1<<U2X);
}

void ow_start(void)
{
	OW_SET_CONV_REQUEST
}

void ow_get_temp(uint64_t id) 
{
	ow_current_id = id;
	
	OW_SET_READ_REQUEST	
}

uint8_t ow_state_machine(void)
{
	switch(ow_state)
	{
		case ow_state_idle:					ow_idle(); break;
		case ow_state_start_conversation:	ow_start_conversation(); break;
		case ow_state_wait_for_conv:		ow_conversion(); break;
		case ow_state_start_read:			ow_read_temp(); break;
		case ow_state_fail:					ow_fail();break;
		default: break;
	}

// current status return
	if(OW_CONV_REQUEST_IS_SET || OW_READ_REQUEST_IS_SET)		// OneWire state machine is busy
	{															// do not start a request
		return 0;
	}
	else														// OneWire is ready
	{
		return 1;
	}	
	
}

void ow_idle(void)
{
	if (OW_CONV_REQUEST_IS_SET)
	{
		ow_state = ow_state_start_conversation;
	}
	if (OW_READ_REQUEST_IS_SET)
	{
		ow_state = ow_state_start_read;
	}
}


void ow_start_conversation(void)
{
	if (ow_reset()) 
	{
		ow_write_byte(OW_SKIP_ROM);
		ow_write_byte(CONVERT_T);
		ow_state = ow_state_wait_for_conv;
	}
	else
	{
		ow_state = ow_state_fail;
	}
}

void ow_conversion(void) 
{
	if (achieved_time(750, &conv_delay)) 
	{
		OW_SET_CONV_COMPLETE
		OW_CLEAR_CONV_REQUEST
		ow_state = ow_state_idle;
	}
}

void ow_read_temp(void)
{
	uint8_t ow_temp_low = 0, ow_temp_high = 0;
	
	ow_reset();
	ow_write_byte(MATCH_ROM);				// Skip ROM, only one device on the bus
	ow_write_rom(ow_current_id);
	ow_write_byte(OW_READ_SCRATCHPAD);		// read scratchpad
	
	ow_temp_low = ow_read_byte();
	ow_temp_high = ow_read_byte();
	ow_temp = ow_temp_low | (ow_temp_high << 8);
	
	OW_CLEAR_READ_REQUEST
	
	ow_state = ow_state_idle;
}

void ow_fail(void)
{
	OW_SET_FAIL
	ow_state = ow_state_idle;
}

uint8_t ow_reset(void)
{
	ow_set_baud(OW_RESET_BAUD);
	uint8_t tmp = ow_write_bit(PRESENCE_PULSE);
	if((tmp >= 0x10) && (tmp <= 0xE0))	
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint64_t ow_read_rom(void)
{
	uint64_t rom=0;
	uint8_t raw_rom[8]={0};
	ow_reset();
	ow_write_byte(OW_READ_ROM);				// read ROM command
	for (uint8_t i = 0; i < 8; i++ )		// reads the 64bit content and saves in 8 x 8 Bit array
	{
		raw_rom[i] = ow_read_byte();
	}
	rom = *(uint64_t*)&raw_rom[0];
return rom;
}

uint8_t ow_read_byte (void)
{
	uint8_t ow_read_return_bit = 0;
	uint8_t received_byte = 0;
	
	for (uint8_t i = 0; i<8; i++)
	{
		ow_read_return_bit=ow_write_bit(0xFF);
		if(ow_read_return_bit <= 0xfe)
		{
			received_byte &= ~(1<<i);
		}
		else
		{
			received_byte |= (1<<i);
		}
	}
	return (received_byte);
}


void ow_write_byte (uint8_t Data_Byte)
{
	ow_set_baud(OW_RW_BAUD);
	for (uint8_t i = 0; i < 8; i++)
	{
		if (Data_Byte & (1<<i))
		{
			ow_write_bit(0xFF);
		}
		else
		{
			ow_write_bit(0x00);
		}
	}
}

void ow_write_rom(uint64_t ROM) 
{
	for(int i=0; i<8; i++) 
	{
		ow_write_byte((ROM>>(i*8)&0xFF));
	}
	
}

uint8_t ow_write_bit (uint8_t Data_Bit)
{
	UDR = Data_Bit;
	while(!(UCSRA & (1<<RXC)));
	return (UDR);
}

void ow_set_baud (uint8_t Baud_Rate) 
{
	while(!(UCSRA & (1<<UDRE)));
	UCSRB &= ~((1<<RXEN)|(1<<TXEN));
	UBRRL = Baud_Rate;
	UCSRB |= (0<<RXCIE)|(0<<TXCIE)|(1<<RXEN)|(1<<TXEN);
}



