//
//		I2C Library
//
//		date: 11.02.2018
//////////////////////////////////////////////////////////////////////////////////
#include "I2C_MASTER.h"

uint8_t i2c_state = 1;
uint8_t i2c_flags = 0;
uint8_t i2c_buffer[I2C_BUFFER_SIZE] = {0};
uint8_t i2c_current_adress = 0;
uint16_t i2c_time_out = 0xFFFF;

void i2c_init(void)
{
	TWCR |= (0<<TWIE)		//TWI Interrupt Enable
			|(1<<TWEN);		//TWI Enable
	//SCL Frequency[Hz] = F_CPU/(16+2*(TWBR)*4^TWPS) | TWPS := Prescaler 
	TWBR = 150;										
	TWSR |= (0<<TWPS1)|(0<<TWPS0);	//prescaler = 1
}

uint8_t i2c_send_data(uint8_t adress, uint8_t *data, uint8_t length)
{
	if((i2c_state==i2c_state_idle)&&(!I2C_W_FLAG_IS_SET))
	{
		I2C_SET_W_FLAG
		i2c_current_adress = adress;
		for(int i=0; i<length; i++)
		{
			i2c_buffer[i] = data[i];
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

void i2c_state_machine(void)
{
	switch(i2c_state) 
	{
		case i2c_state_idle:				i2c_idle();break;
		case i2c_state_start:				i2c_start_condition();break;
		case i2c_state_wait_for_free_bus:	i2c_wait_for_free_bus();break;
		case i2c_state_wait_for_w_ack:		i2c_wait_for_w_ack();break;
		case i2c_state_write_data:			i2c_write_data();break;
		case i2c_state_error:				i2c_error();break;
		default:break;
	}
}

void i2c_idle(void)
{
	if(I2C_W_FLAG_IS_SET)
	{
		i2c_state = i2c_state_start;
	}
}

void i2c_error(void)
{
	
}

void i2c_start_condition(void)
{
	I2C_CLEAR_INT
	I2C_SET_START_CONDITON
	i2c_state = i2c_state_wait_for_free_bus;
}

void i2c_wait_for_free_bus(void)
{ 
	if(I2C_INT_IS_SET && (I2C_STATUS == 0x08)) 
	{
		//PORTC |= (1<<PC2);
		TWDR = (i2c_current_adress<<1);
		if(I2C_W_FLAG_IS_SET) 
		{
			TWDR &=~ (1<<0);
			i2c_state = i2c_state_wait_for_w_ack;
			I2C_CLEAR_INT
			PORTC |= (1<<PC0);
		}
		else
		{
			TWDR |= (1<<0);
			i2c_state = i2c_state_wait_for_r_ack;
			I2C_CLEAR_INT
		}
	}
	else
	{
		if (achieved_time(I2C_TIME_OUT_MS, &i2c_time_out)) 
		{
			i2c_state = i2c_state_error;
		}
	}
}

void i2c_wait_for_w_ack(void)
{
	if(I2C_INT_IS_SET) 
	{
		PORTC |= (1<<PC2);
		if(I2C_STATUS == 0x18)			//everything OK; go on with data transmit 
		{
			i2c_state = i2c_state_write_data;
		}
		else if(I2C_STATUS == 0x20)		//slave doesnt answer
		{
			PORTC |= (1<<PC3);
			i2c_state = i2c_state_error;
		}
		else if(I2C_STATUS == 0x38)		//arbitration lost; only in multi master
		{
			i2c_state = i2c_state_error;
		}
		else
		{
			PORTC |=(1<<PC0);
			i2c_state = i2c_state_error;
		}
	}
	else
	{
		if (achieved_time(I2C_TIME_OUT_MS, &i2c_time_out)) 
		{
			i2c_state = i2c_state_error;
		}
	}
}

void i2c_write_data(void)
{
	TWDR = 0xD0;
	I2C_CLEAR_INT
	i2c_state = i2c_state_error;
}

void i2c_wait_for_r_ack(void)
{
	if(I2C_STATUS == 0x38)		//arbitration lost; only in multi master
	{
		i2c_state = i2c_state_error;
	}
	else if(I2C_STATUS == 0x40)	//everything OK; go on with data reception 
	{
		
	}
	else if(I2C_STATUS == 0x48)	//slave doesnt answer
	{
		i2c_state = i2c_state_error;
	}
	else
	{
		if (achieved_time(I2C_TIME_OUT_MS, &i2c_time_out)) 
		{
			i2c_state = i2c_state_error;
		}
	}
}

