#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include <inttypes.h>

#define PORT_SPI    PORTB
#define DDR_SPI     DDRB
#define DD_MISO     PB4
#define DD_MOSI     PB3
#define DD_SS       PB2
#define DD_SCK      PB5

#define buffer_size 20

void SPI_Init(void);
uint8_t SPI_SendByte (uint8_t data);
void spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len);
void spi_transmit_sync (uint8_t * dataout, uint8_t len);

#endif /* SPI_H_ */