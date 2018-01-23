#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "SPI.h"
#include "NRF24L01.h" 
#include "system_time.h"
#include "DCF77.h"
#include "OneWire.h"

void send_date(void);
void send_nrf_string(char *s);

uint16_t debug_delay=0xFFFF;
extern dataframe Tx_Data;
extern uint8_t dcf_request, dcf_ready;
extern Time_Data time;
extern uint16_t ow_temp, ow_temp2;


int main(void) 
{
    DDRB |= (1<<PB2);
	
	SPI_Init();
	nrf_init();
	system_time_init();
	ow_init();
	sei();
	_delay_ms(2000);
	send_nrf_string("Au�enmodul> Begin:\n");
	
	dcf_request = 1;
	char string[32] = {0};
	char tmp[10] = {0};
	double temp = 0.0;
	double temp2 = 0.0;
	uint64_t ID = 0xD204165168A5FF28;	//{0xD2}{0x04}{0x16}{0x51}{0x68}{0xA5}{0xFF}{0x28} Flach
	uint64_t ID2= 0x7F0416A04081FF28;	//{0x7F}{0x04}{0x16}{0xA0}{0x40}{0x81}{0xFF}{0x28} rund

	SET_TX_REQUEST
	
	while(1) 
	{
		
		nrf_state_machine();
		dcf_state_machine();
		ow_state_machine();
		
		if(OW_CONV_COMPLETE_IS_SET)
		{
			ow_get_temp(ID, ID2);		
			OW_CLEAR_CONV_COMPLETE
		}
		
		
		if (dcf_ready) 
		{
			send_date();
			dcf_ready = 0;
		}
		
		if(achieved_time(3000 ,&debug_delay)) 
		{
			PORTB ^= (1<<PB2);	
			/*
			ID = ow_read_rom();
			for(int i=0; i<8; i++) {
				Tx_Data.Buffer[i] = (ID>>(i*8)&0xFF);
			}
			Tx_Data.Buffer[8]=0;
			SET_TX_REQUEST
			*/
			temp=(double)ow_temp/16.0;
			dtostrf(temp, 5, 2, string);
			string[4]='�';
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
			string[13]='�';
			string[14]='C';
			string[15] = '\n';
			string[16] = 0;
			
			nrf_send_data(string);
			ow_start();
		}
    }
}

void send_nrf_string(char *s) 
{
	uint8_t i=0;
	while(s[i]!=0) 
	{
		Tx_Data.Buffer[i] = s[i];
		i++;
	}
	SET_TX_REQUEST
}


void send_date(void) {
	
				uint8_t tmp=0;
				char string[10]={0};
					
				itoa(time.hour, string, 10);
				for(int i=0; string[i]!=0; i++) {
					Tx_Data.Buffer[i] = string[i];
					tmp++;
				}
				Tx_Data.Buffer[tmp] = ':';
				tmp++;
				itoa(time.minute, string, 10);
				for(int i=0; string[i]!=0; i++) {
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				
				Tx_Data.Buffer[tmp] = ' ';
				tmp++;
				for(int i=0; time.weekday[i]!=0; i++) {
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
				for(int i=0; string[i]!=0; i++) {
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				
				Tx_Data.Buffer[tmp] = '.';
				tmp++;
				itoa(time.month, string, 10);
				for(int i=0; string[i]!=0; i++) {
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				
				Tx_Data.Buffer[tmp] = '.';
				tmp++;
				itoa(time.year, string, 10);
				for(int i=0; string[i]!=0; i++) {
					Tx_Data.Buffer[tmp] = string[i];
					tmp++;
				}
				Tx_Data.Buffer[tmp] = 0;
				SET_TX_REQUEST
}