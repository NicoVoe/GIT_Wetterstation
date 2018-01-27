#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "SPI.h"
#include "NRF24L01.h"
#include "system_time.h"
#include "UART.h"
#include "CircularBuffer.h"

#define DEBUG_LED_DDR DDRD
#define DEBUG_LED_PORT PORTD
#define DEBUG_LED PD5

uint16_t debug_delay=0xFFFF;
extern uint8_t NRF_Receive_Buffer[32];

#define BUFFER_SIZE 20

uint8_t storage[BUFFER_SIZE]={0};
circular_buffer nrf_buffer={storage,0,0,BUFFER_SIZE};

int main(void) 
{
	DEBUG_LED_DDR |= (1<<DEBUG_LED);
	
	SPI_Init();
	nrf_init();
	system_time_init();
	uart_init();
	sei();
	SET_RX_REQUEST
	
	uart_send("Indoor Unit>> Begin: \n");
	char string[33]={0};
	
	while(1) 
	{
		
		nrf_state_machine();
		
		if(RX_COMPLETE_IS_SET) 
		{
			RESET_RX_COMPLETE
			for(int i=0; i<32; i++) 
			{
				string[i]= NRF_Receive_Buffer[i];
			}
			string[32] = 0;

			SET_RX_REQUEST
			
			uart_send(string);
		}

		if(achieved_time(1000 ,&debug_delay)) 
		{
			DEBUG_LED_PORT ^= 1 << DEBUG_LED;
		}
	}
}

