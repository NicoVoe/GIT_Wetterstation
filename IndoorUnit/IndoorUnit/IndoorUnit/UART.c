#include "UART.h"

volatile uint8_t tx_state = transmit_complete;
volatile uint8_t RX_State = 0;

uint8_t uart_send_buffer[maxstrlen]={0};
circular_buffer uart_tx_buffer={uart_send_buffer,0,0,maxstrlen};

uint8_t uart_receive_buffer[maxstrlen]={0};
circular_buffer uart_rx_buffer={uart_receive_buffer,0,0,maxstrlen};
//---------------------------------------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------------------------------------
// UART initialisieren

void uart_init(void)
{
	// Baudrate einstellen (Normaler Modus)
	UBRRH = 0;
	UBRRL = UBRR_BAUD;							// 25 -> 2400Baud, 51->9600Baud

	// Aktivieren des Empfängers, des Senders und des "Daten empfangen"-Interrupts
	UCSRB |= (1<<RXCIE)|(1<<TXCIE)|(1<<RXEN)|(1<<TXEN);

	// Einstellen des Datenformats: 8 Datenbits, 1 Stoppbit
	UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRA |= (1<<U2X);
	
}

//---------------------------------------------------------------------------------------------------------
// Init Transmit
//---------------------------------------------------------------------------------------------------------
void Send_Init(unsigned char c)
{
	while(!(UCSRA & (1<<UDRE))) 
	{
								// Warten solange noch eine Übertragung statt findet
	}							// Solange noch ein String gesendet werden muss, wird hier Zeit vertrödelt
		UDR=c;					// Zeichen ins UDR Register schreiben.
}

//---------------------------------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------------------------------
uint8_t uart_send(char *s)
{
	uint8_t length=0;
	while(s[length]!=0) {length++;}		//find length of given string
	length++;
	uint8_t ret = cb_push(&uart_tx_buffer, s, length);
	if(tx_state == transmit_complete) 
	{
		Send_Init(0);					//trigger a transmission
	}
return ret;
}

//---------------------------------------------------------------------------------------------------------
// Interrupt Service Routine Receive
//---------------------------------------------------------------------------------------------------------
ISR(USART_RXC_vect)
{
	uint8_t c=UDR;
	cb_push(&uart_rx_buffer, &c, 1);
}

//---------------------------------------------------------------------------------------------------------
// Interrupt Service Routine Transmit
//---------------------------------------------------------------------------------------------------------

ISR(USART_TXC_vect)
{
	if(!cb_is_empty(&uart_tx_buffer)) 
	{			// send as long as an element is in the tx_buffer
		uint8_t c=0;
		cb_pop(&uart_tx_buffer, &c, 1);
		UDR = c;
		tx_state = transmit_busy;
	}
	else
	{
		tx_state = transmit_complete;
	}
}
