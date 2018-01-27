#include "NRF24L01.h"
#include "SPI.h"

uint8_t nrf_state = idle;
uint8_t nrf_command = 0;
uint8_t NRF_Receive_Buffer[32] = {0};
	
const uint8_t NRF_TX_ADRESS[5] = {0xD7, 0xD7, 0xD7, 0xD7, 0xD7};
const uint8_t NRF_RX_ADRESS[5] = {0xD7, 0xD7, 0xD7, 0xD7, 0xD7};
	
dataframe Tx_Data={0,{0}};

//--------------------------------------------------------------------------------
//Funktionen fuer die Ablaufsteuerung des Funkmoduls
//--------------------------------------------------------------------------------
void nrf_init()										//Interrupt und Chipselect init
{
	CS_DDR|= ((1<<CSN)|(1<<CE));
	NRF24_UNSELECT
	NRF24_DEACTIVATE_CE
	
	MCUCR |= ((1<<ISC11)|(0<<ISC10));
	
	//zeiger auf buffer speichern
	
	_delay_ms(120);
	nrf_config();
}

void nrf_config()										//Funkmodul init
{
	nrf_allocate_register(RF_CH, NRF_CHANNEL);			//Kanal einstellen
	nrf_allocate_register(RX_PW_P0, 0x00);				//Pipe wird nicht benutzt
	nrf_allocate_register(RX_PW_P1, NRF_PAYLOAD_LEN);
	nrf_allocate_register(RX_PW_P2, 0x00);				//Pipe wird nicht benutzt
	nrf_allocate_register(RX_PW_P3, 0x00);				//Pipe wird nicht benutzt
	nrf_allocate_register(RX_PW_P4, 0x00);				//Pipe wird nicht benutzt
	nrf_allocate_register(RX_PW_P5, 0x00);				//Pipe wird nicht benutzt
	
	nrf_allocate_register(RF_SETUP, (0<<RF_DR_HIGH)|((0x03)<<RF_PWR));
	nrf_allocate_register(CONFIG, (1<<EN_CRC)|(0<<CRCO));
	nrf_allocate_register(EN_AA,(1<<ENAA_P0)|(1<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));
	nrf_allocate_register(EN_RXADDR,(1<<ERX_P0)|(1<<ERX_P1)|(0<<ERX_P2)|(0<<ERX_P3)|(0<<ERX_P4)|(0<<ERX_P5));
	nrf_allocate_register(SETUP_RETR,(0x04<<ARD)|(0x0F<<ARC));
	nrf_allocate_register(DYNPD,(0<<DPL_P0)|(0<<DPL_P1)|(0<<DPL_P2)|(0<<DPL_P3)|(0<<DPL_P4)|(0<<DPL_P5));
	
	nrf_set_rx_adress(NRF_RX_ADRESS);
	nrf_set_tx_adress(NRF_TX_ADRESS);
}

uint8_t nrf_get_status(void)
{
	uint8_t status=0;
	NRF24_SELECT
	status = SPI_SendByte(NOP);
	NRF24_UNSELECT
	return(status);
}

void nrf_set_rx_adress(const uint8_t *Addr)
{
	NRF24_SELECT
	nrf_write_register(RX_ADDR_P1, Addr, NRF_ADDR_LEN);
	NRF24_UNSELECT
}

void nrf_set_tx_adress(const uint8_t *Addr)
{
	NRF24_SELECT
	nrf_write_register(RX_ADDR_P0, Addr, NRF_ADDR_LEN);
	nrf_write_register(TX_ADDR, Addr, NRF_ADDR_LEN);
	NRF24_UNSELECT
}

void nrf_powerdown()
{
	NRF24_DEACTIVATE_CE
	nrf_allocate_register(CONFIG, (1<<EN_CRC)|(0<<CRCO));
}

void nrf_powerup_tx(void)
{
	nrf_allocate_register(STATUS,((1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT)));						//alle Interrupts zu Beginn löschen
	nrf_allocate_register(CONFIG, ((1<<EN_CRC)|(0<<CRCO)|(1<<PWR_UP)|(0<<PRIM_RX)));
}

void nrf_powerup_rx(void)
{
	NRF24_SELECT
	SPI_SendByte(FLUSH_RX);		//RX FIFO leeren
	NRF24_UNSELECT
	
	nrf_allocate_register(STATUS,((1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT)));						//alle Interrupts zu Beginn löschen
	nrf_allocate_register(CONFIG, ((1<<EN_CRC)|(0<<CRCO)|(1<<PWR_UP)|(1<<PRIM_RX)));
	
	NRF24_ACTIVATE_CE			// start RX-Mode
}

void nrf_send_data(uint8_t *Tx_Buffer)
{
	NRF24_DEACTIVATE_CE
	_delay_us(10);
	nrf_powerup_tx();
	NRF24_SELECT
	SPI_SendByte(W_TX_PAYLOAD);
	nrf_transmit(Tx_Buffer, NRF_PAYLOAD_LEN);
	NRF24_UNSELECT
	NRF24_ACTIVATE_CE

}
void nrf_get_data(uint8_t *Rx_Buffer)
{
	NRF24_SELECT
	SPI_SendByte(R_RX_PAYLOAD);
	nrf_transfer_sync(Rx_Buffer, Rx_Buffer, NRF_PAYLOAD_LEN);
	NRF24_UNSELECT
	nrf_allocate_register(STATUS, (1<<RX_DR));	//Empfangsinterrut loeschen
	_delay_us(10);
}
//--------------------------------------------------------------------------------
//Standard Funktion zum Lesen und Schreiben von Registerinhalten:
//--------------------------------------------------------------------------------
void nrf_transmit(uint8_t *Buffer_Out, uint8_t Length)		//mehrere bytes in eine Registeradresse schreiben
{
	for(uint8_t i=0; i<Length; i++)
	{
		SPI_SendByte(Buffer_Out[i]);
	}
}
void nrf_transfer_sync(uint8_t *Buffer_Out, uint8_t *Buffer_In, uint8_t Length)			//mehrere bytes in eine Registeradresse schreiben und die antwort auslesen
{
	for(uint8_t i=0; i<Length; i++)
	{
		Buffer_In[i]=SPI_SendByte(Buffer_Out[i]);
	}
}

void nrf_allocate_register(uint8_t Reg_Addr, uint8_t Val)
{
	NRF24_SELECT
	SPI_SendByte(W_REGISTER | (REGISTER_MASK & Reg_Addr));
	_delay_us(10);
	SPI_SendByte(Val);
	NRF24_UNSELECT
	_delay_us(10);
}

void nrf_write_register(uint8_t Reg_Addr, uint8_t *Tx_Buffer, uint8_t Length)
{
	NRF24_SELECT											//Chip Select
	SPI_SendByte(W_REGISTER | (REGISTER_MASK & Reg_Addr));	//Schreibbefehl + Registeradresse
	nrf_transmit(Tx_Buffer, Length);
	NRF24_UNSELECT
	_delay_us(10);
}

void nrf_read_register(uint8_t Reg_Addr, uint8_t *TxRx_Buffer, uint8_t Length)
{
	NRF24_SELECT											
	SPI_SendByte(R_REGISTER | (REGISTER_MASK & Reg_Addr));	//Lesebefehl + Registeradresse
	nrf_transfer_sync(TxRx_Buffer, TxRx_Buffer, Length);
	NRF24_UNSELECT
	_delay_us(10);
}

//--------------------------------------------------------------------------------
//NRF24 state machine
//--------------------------------------------------------------------------------
uint8_t nrf_state_machine(void)
{
	switch(nrf_state) 
	{
		case idle: nrf_idle (); break;
		case setup_rx: nrf_setup_rx (); break;
		case wait_on_rx: nrf_wait_on_rx (); break;
		case collecting_data: nrf_collect_data (); break;
		case setup_tx: nrf_setup_tx (); break;
		case wait_on_tx: nrf_wait_on_tx(); break;
		case tx_failed: nrf_tx_failed(); break;
		case tx_complete: nrf_tx_complete(); break;
		default: break;
	}
	return 0;
}



void nrf_idle (void)
{
	if (RX_REQUEST_IS_SET) 
	{
		nrf_state = setup_rx;
	}
	if (TX_REQUEST_IS_SET)
	{
		nrf_state = setup_tx;
	}
}

void nrf_setup_rx (void)
{
	nrf_powerup_rx ();
	nrf_state = wait_on_rx;
}

void nrf_wait_on_rx (void)
{
	if (GIFR & (1<<NRF24_IRQ))
	{
		uint8_t status = 0;
		status = nrf_get_status();
		
		if (status & (1 << RX_DR)) 
		{
			nrf_state = collecting_data;
		}
		
		GIFR |= (1 << NRF24_IRQ);
	} 
	else 
	{
		if (RX_REQUEST_IS_CLEAR)
		{
			nrf_state = idle;
		}
		if (TX_REQUEST_IS_SET)
		{
			nrf_state = setup_tx;
		}
	}
}
void nrf_collect_data (void)
{
	nrf_get_data(NRF_Receive_Buffer);
	if (RX_REQUEST_IS_SET) 
	{
		nrf_state = wait_on_rx;
	}
	if (RX_REQUEST_IS_CLEAR)
	{
		nrf_state = idle;
	}
	SET_RX_COMPLETE
}

void nrf_setup_tx (void)
{
	nrf_set_tx_adress(NRF_TX_ADRESS);
	nrf_send_data(Tx_Data.Buffer);
	nrf_state = wait_on_tx;	
}

void nrf_wait_on_tx(void)
{
	if (GIFR & (1<<NRF24_IRQ))
	{
		uint8_t status = 0;
		status = nrf_get_status ();
		
		if (status & (1 << TX_DS))
		{
			nrf_state = tx_complete;
		}
		if (status & (1 << MAX_RT))
		{
			nrf_state = tx_failed;
		}
		GIFR |= (1 << NRF24_IRQ);
	}
}

void nrf_tx_complete (void)
{
	nrf_allocate_register(STATUS,(1 << TX_DS));
	if(TX_REQUEST_IS_SET)
	{
		nrf_state = setup_tx;
	}
	else
	{
		nrf_state = idle;	
	}
}

void nrf_tx_failed (void)
{
	nrf_allocate_register(STATUS,(1 << MAX_RT));
	nrf_state = idle;
	SET_TX_FAILED
	RESET_TX_REQUEST
}