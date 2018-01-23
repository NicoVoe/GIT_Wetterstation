#ifndef DCF77_H_
#define DCF77_H_

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <string.h>
#include <avr/io.h>

//DCF Signal Format
#define DCF_ONE					200
#define DCF_ZERO				100
#define DCF_ONE_PAUSE			800
#define DCF_ZERO_PAUSE			900
#define DCF_MARGIN				40
#define DCF_FIRST_PULSE			1850
#define DCF_MARGIN_FIRST_PULSE	150

#define INT0_NEGATIVE_EDGE	MCUCR |= (1 << ISC01); MCUCR &= ~(1 << ISC00);
#define INT0_POSITIVE_EDGE	MCUCR |= (1 << ISC00) | (1 << ISC01);
#define CLEAR_INT0			GIFR |= (1 << INTF0); 
#define DCF_INTF0_IS_SET	GIFR & (1 << INTF0)
#define DCF_INTF0_IS_CLEAR	!(GIFR & (1 << INTF0))

#define STOPWATCH_RESET		dcf_stoptime_counter = 0xFFFF;

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
	volatile uint8_t Bit_Number, Parity, State;
	volatile uint8_t RawData[6];
}DCF_Data;

typedef struct
{
	volatile uint8_t second, minute, hour, day, month;
	volatile uint16_t year;
	char weekday[11];
}Time_Data;

enum {idle_dcf=1, setup_dcf, search_for_start, wait_for_next_bit, wait_for_next_pause, measure_pulse_width, one_detected, zero_detected, minute_eval, hour_eval, day_eval, month_eval, weekday_eval, year_eval};
	
#endif