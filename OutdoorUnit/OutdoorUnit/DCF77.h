#ifndef DCF77_H_
#define DCF77_H_

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

//#include <string.h>
#include <avr/io.h>
#include <string.h>

//DCF Signal Format
#define DCF_ONE					200
#define DCF_ZERO				100
#define DCF_ONE_PAUSE			800
#define DCF_ZERO_PAUSE			900
#define DCF_MARGIN				40
#define DCF_FIRST_PULSE			1850
#define DCF_MARGIN_FIRST_PULSE	150

#define DCF_INT0_NEGATIVE_EDGE	MCUCR |= (1 << ISC01); MCUCR &= ~(1 << ISC00);
#define DCF_INT0_POSITIVE_EDGE	MCUCR |= (1 << ISC00) | (1 << ISC01);
#define DCF_CLEAR_INT0			GIFR |= (1 << INTF0); 
#define DCF_INTF0_IS_SET		GIFR & (1 << INTF0)
#define DCF_INTF0_IS_CLEAR		!(GIFR & (1 << INTF0))

#define DCF_STOPWATCH_RESET		dcf_stoptime_counter = 0xFFFF;

void dcf_state_machine (void);
void dcf_idle (void);
void dcf_setup (void);
void dcf_search_begining (void);
void dcf_next_bit (void);
void dcf_measure_pw (void);
void dcf_next_pause (void);
void dcf_one_detected (void);
void dcf_zero_detected (void);
void dcf_minute_eval (void);
void dcf_hour_eval (void);
void dcf_day_eval (void);
void dcf_month_eval (void);
void dcf_weekday_eval (void);
void dcf_year_eval (void);

void dcf_init (void);
uint8_t data_interpretation (uint8_t val, uint8_t data_length);
uint8_t power (uint8_t basis, uint8_t exponent);

typedef struct
{
	volatile uint8_t bit_number, parity, state;
	volatile uint8_t raw_data[6];
}DCF_Data;

typedef struct
{
	volatile uint8_t second, minute, hour, day, month;
	volatile uint16_t year;
	char weekday[11];
}Time_Data;

enum {dcf_state_idle=1, dcf_state_setup, dcf_state_search_for_start, dcf_state_wait_for_next_bit, dcf_state_wait_for_next_pause, dcf_state_measure_pulse_width, dcf_state_one_detected, dcf_state_zero_detected, dcf_state_minute_eval, dcf_state_hour_eval, dcf_state_day_eval, dcf_state_month_eval, dcf_state_weekday_eval, dcf_state_year_eval};
	
#endif