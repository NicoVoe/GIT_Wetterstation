#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "SPI.h"
#include "NRF24L01.h" 
#include "CircularBuffer.h"
#include "system_time.h"
#include "DCF77.h"
#include "OneWire.h"
#include "EEPROM.h"


#define DEBUG_LED_DDR DDRB
#define DEBUG_LED_PORT PORTB
#define DEBUG_LED PB2

void send_date(void);
void send_nrf_string(char *s);

uint16_t debug_delay=0xFFFF;
uint16_t debug_delay2=0xFFFF;
extern uint8_t dcf_request, dcf_ready;
extern Time_Data time;
extern int16_t ow_temp, ow_temp2; 
extern circular_buffer eeprom_cb;

uint8_t eeprom_test = 0;

int main(void) 
{
    DDRB |= (1<<PB2);
	PORTB &=~(1<<PB2);
	DDRC |= (1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0);
	spi_init();
	nrf_init();
	system_time_init();
	ow_init();
	eeprom_init();
	sei();
	_delay_ms(1000);

	dcf_request = 1;
	char string[32] = {0};
	unsigned char tmp[10] = {0};
	//uint64_t ID = 0xD204165168A5FF28;	//{0xD2}{0x04}{0x16}{0x51}{0x68}{0xA5}{0xFF}{0x28} Flach
	//uint64_t ID2= 0x7F0416A04081FF28;	//{0x7F}{0x04}{0x16}{0xA0}{0x40}{0x81}{0xFF}{0x28} rund
	uint64_t ID =	0xCF031661B4DDFF28;	//{0xCF}{0x03}{0x16}{0x61}{0xB4}{0xDD}{0xFF}{0x28} meins
	char temp_string[10] = {0};
	uint8_t buffer123[30] = {0};
	uint8_t k = 100, length=0, taster=0, l=0;
	uint16_t j = 300;
	nrf_send("Outdoor Unit>> Begin:\n", sizeof("Outdoor Unit>> Begin:\n"));
	 //ow_start();
	
	/*for(uint16_t i=256; i>0; i--) 
	{
		eeprom_put_data(&i,1,0);
		//cb_push(&eeprom_cb, &i, 1);	
	}*/
	//uint8_t test2 = eeprom_put_data("12345", sizeof("12345"), 0);
	//test2 = cb_push(&eeprom_cb,"12345", sizeof("12345"));
	
	/*
	if(test2 == 2) 
	{
		PORTC |= (1<<PC0);
	}
	if(test2 == 1) 
	{
		PORTC |= (1<<PC1);
	}
	if(test2 == 0) 
	{
		PORTC |= (1<<PC2);
	}*/
	//eeprom_start_reading(1);
	eeprom_start_erase_page(0x00);
	while(1) 
	{
		
		nrf_state_machine();
		//dcf_state_machine();
		//ow_state_machine();
		eeprom_state_machine();
		/*
		if(ow_state_machine())
		{
			ow_get_temp(ID);		
			OW_CLEAR_CONV_COMPLETE
		}
		*/
		
		/*if (dcf_ready) 
		{
			send_date();
			dcf_ready = 0;
		}*/
		if(bit_is_clear(PIND, PD5)&&taster==0)
		{
			for(int i=0; i<30; i++)
			{
				buffer123[i]=0;
			}
			_delay_ms(20);
			taster=1;
			read_irgendwas(0x1FF00, buffer123, 30);
			k=0;
		}
		if(taster==1 && bit_is_set(PIND,PD5)) 
		{
			taster=0;
		}
		
		if(achieved_time(10 ,&debug_delay)) 
		{
			//nrf_send("ich lebe noch :)...", sizeof("ich lebe noch :)..."));
			if(k<30) 
			{
				itoa(buffer123[k], tmp, 10);
				length = 0;
				while(tmp[length]!=0)
				{
					if(length >= 10 )
					{
						break;
					}
					length++;
				}
				tmp[length]= '-';
				nrf_send(tmp, sizeof(tmp));
				k++;
				for(int i=0; i<10; i++)
				{
					tmp[i]=0;
				}
			}
			if (eeprom_get_data(&l, 1))
			{
				itoa(l, tmp, 10);
				length = 0;
				while(tmp[length]!=0)
				{
					if(length >= 10 )
					{
						break;
					}
					length++;
				}
				tmp[length]= '-';
				//length++;
				nrf_send(tmp, sizeof(tmp));
				for(int i=0; i<10; i++)
				{
				tmp[i]=0;
				}
			}
			/*
			if(j < 256)
			{
				cb_pop(&eeprom_cb, string, 1); //cb_pop(&eeprom_cb, eeprom_data, eeprom_data_length)
				itoa(string[0], tmp, 10);
				length = 0;
				while(tmp[length]!=0)
				{
					if(length >= 10 )
					{
						break;
					}
					length++;
				}
				tmp[length]= '-';
				nrf_send(tmp, sizeof(tmp));
				for(int i=0; i<10; i++)
				{
					tmp[i]=0;
				}
				j++;
			}*/
			
		}
		
		if(achieved_time(4000 ,&debug_delay2))
		{	
			DEBUG_LED_PORT ^= 1 << DEBUG_LED;
			
			if(cb_is_full(&eeprom_cb)) 
			{
				//DEBUG_LED_PORT ^= 1 << DEBUG_LED;
			}
			//read_irgendwas(0x00, eeprom_cb.buffer, 256);
			j = 0;
			eeprom_start_reading(0);
		}
		
		
    }
}

void send_date(void) 
{
/*
				uint8_t tmp=0;
				char string[10]={0};
					
				itoa(time.hour, string, 10);
				for(int i=0; string[i]!=0; i++) 
				{
					Tx_Data.Buffer[i] = string[i];
					tmp++;
				}
				Tx_Data.Buffer[tmp] = ':';
				tmp++;
				itoa(time.minute, string, 10);
				for(int i=0; string[i]!=0; i++) 
				{
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				
				Tx_Data.Buffer[tmp] = ' ';
				tmp++;
				for(int i=0; time.weekday[i]!=0; i++) 
				{
					Tx_Data.Buffer[tmp] = time.weekday[i];
					tmp++;
				}
				
				Tx_Data.Buffer[tmp] = ',';
				tmp++;
				Tx_Data.Buffer[tmp] = ' ';
				tmp++;
				Tx_Data.Buffer[tmp] = 'd';
				tmp++;
				Tx_Data.Buffer[tmp] = 'e';
				tmp++;
				Tx_Data.Buffer[tmp] = 'n';
				tmp++;
				Tx_Data.Buffer[tmp] = ' ';
				tmp++;
				itoa(time.day, string, 10);
				for(int i=0; string[i]!=0; i++) 
				{
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				
				Tx_Data.Buffer[tmp] = '.';
				tmp++;
				itoa(time.month, string, 10);
				for(int i=0; string[i]!=0; i++) 
				{
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				
				Tx_Data.Buffer[tmp] = '.';
				tmp++;
				itoa(time.year, string, 10);
				for(int i=0; string[i]!=0; i++) 
				{
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				Tx_Data.Buffer[tmp] = 0;
				SET_TX_REQUEST
*/
}

//itoa(ow_temp, temp_string, 10);
			//nrf_send(temp_string,5);
			//nrf_send("--",3);		
			
			/*uint64_t ID = ow_read_rom();
			for(int i=0; i<8; i++) 
			{
				Tx_Data.Buffer[i] = (ID>>(i*8)&0xFF);
			}
			Tx_Data.Buffer[8]=0;
			
			for (int i = 9; i<32; i++)
			{
				Tx_Data.Buffer[i]= 0;
			}
			SET_TX_REQUEST*/
			/*
			temp=(double)ow_temp/16.0;
			dtostrf(temp, 5, 2, string);
			string[4]='°';
			string[5]='C';
			string[6] = ' ';
			string[7] = '|';
			string[8]= ' ';
			
			temp2=(double)ow_temp2/16.0;
			dtostrf(temp2, 5, 2, tmp);
			string[9]=tmp[0];
			string[10]=tmp[1];
			string[11]=tmp[2];
			string[12]=tmp[3];
			string[13]='°';
			string[14]='C';
			string[15] = '\n';
			string[16] = 0;
			*/
			//ow_start();