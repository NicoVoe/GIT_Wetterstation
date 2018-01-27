#ifndef USART_H_
#define USART_H_

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "CircularBuffer.h"

#define BAUD        76800UL
#define UBRR_BAUD  ((F_CPU/(8UL*BAUD))-1)

#define maxstrlen 50

// Status Flags, RX_State
#define transmit_complete 10
#define transmit_busy 0

///////////////////////////////////////////////////////////////////////////////////
//Prototypen
ISR(USART_RXC_vect);
ISR(USART_TXC_vect);
void uart_init(void);
uint8_t uart_send (char *s);
void Send_Init (unsigned char c);

 #endif /* USART_H_ */
