#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "SPI.h"
#include "NRF24L01.h" 
#include "system_time.h"
#include "DCF77.h"
#include "OneWire.h"
#include "EEPROM.h"


#define DEBUG_LED_DDR DDRB
#define DEBUG_LED_PORT PORTB
#define DEBUG_LED PB2
uint16_t Debug_Delay = 0xFFFF;

void send_date(void);
void send_nrf_string(char *s);

uint16_t debug_delay=0xFFFF;
extern uint8_t dcf_request, dcf_ready;
extern Time_Data time;
extern int16_t ow_temp, ow_temp2; 

int main(void) 
{
    DDRB |= (1<<PB2);
	
	spi_init();
	nrf_init();
	system_time_init();
	ow_init();
	sei();
	_delay_ms(2000);

	dcf_request = 1;
	char string[32] = {0};
	char tmp[10] = {0};
	double temp = 0.0;
	double temp2 = 0.0;
	//uint64_t ID = 0xD204165168A5FF28;	//{0xD2}{0x04}{0x16}{0x51}{0x68}{0xA5}{0xFF}{0x28} Flach
	uint64_t ID2= 0x7F0416A04081FF28;	//{0x7F}{0x04}{0x16}{0xA0}{0x40}{0x81}{0xFF}{0x28} rund
	uint64_t ID =	0xCF031661B4DDFF28;	//{0xCF}{0x03}{0x16}{0x61}{0xB4}{0xDD}{0xFF}{0x28} meins
	char temp_string[10] = {0};
	
	nrf_send("Outdoor Unit>> Begin:\n", sizeof("Outdoor Unit>> Begin:\n"));
	
	ow_start();
	
	while(1) 
	{
		
		nrf_state_machine();
		dcf_state_machine();
		ow_state_machine();
		
		if(ow_state_machine())
		{
			ow_get_temp(ID);		
			OW_CLEAR_CONV_COMPLETE
		}
		
		
		if (dcf_ready) 
		{
			send_date();
			dcf_ready = 0;
		}
		
		if(achieved_time(1000 ,&debug_delay)) 
		{
			DEBUG_LED_PORT ^= 1 << DEBUG_LED;
			itoa(ow_temp, temp_string, 10);
			nrf_send(temp_string,5);
			nrf_send("--",3);		
			
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
			ow_start();
		}
    }
}

void send_nrf_string(char *s) 
{
	uint8_t i=0;
	/*while(s[i]!=0) 
	{
		Tx_Data.Buffer[i] = s[i];
		i++;
	}
	SET_TX_REQUEST*/
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