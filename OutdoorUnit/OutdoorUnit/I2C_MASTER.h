//
//		I2C Library
//
//		date: 11.02.2018
//////////////////////////////////////////////////////////////////////////////////
#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#include <avr/io.h>
#include "system_time.h"

#define I2C_BUFFER_SIZE 10
#define I2C_TIME_OUT_MS 10

#define I2C_STATUS (TWSR & 0xF8)
#define I2C_SET_START_CONDITON (TWCR|=(1<<TWSTA));
#define I2C_CLEAR_START_CONDITON (TWCR&=~(1<<TWSTA));
#define I2C_SET_STOP_CONDITON (TWCR|=(1<<TWSTO));
#define I2C_CLEAR_STOP_CONDITON (TWCR&=~(1<<TWSTO));
#define I2C_CLEAR_INT (TWCR|=(1<<TWINT));
#define I2C_SET_W_FLAG (i2c_flags |= (1<<0));
#define I2C_CLEAR_W_FLAG (i2c_flags &=~ (1<<0));
#define I2C_SET_R_FLAG (i2c_flags |= (1<<1));
#define I2C_CLEAR_R_FLAG (i2c_flags &=~ (1<<1));

#define I2C_INT_IS_SET (TWCR&(1<<TWINT))
#define I2C_W_FLAG_IS_SET (i2c_flags&(1<<0))

enum {i2c_state_idle=1, i2c_state_start, i2c_state_wait_for_free_bus, i2c_state_wait_for_w_ack, i2c_state_write_data, i2c_state_wait_for_r_ack, i2c_state_error};
extern uint8_t i2c_flags;

uint8_t i2c_send_data(uint8_t adress, uint8_t *data, uint8_t length);

void i2c_init(void);
void i2c_state_machine(void);
void i2c_idle(void);
void i2c_start_condition(void);
void i2c_wait_for_free_bus(void);
void i2c_start_writing(void);
void i2c_wait_for_w_ack(void);
void i2c_write_data(void);
void i2c_wait_for_r_ack(void);

void i2c_error(void);



#endif /* I2C_MASTER_H_ */