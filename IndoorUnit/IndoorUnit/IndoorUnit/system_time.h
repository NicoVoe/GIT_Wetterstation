#ifndef system_time_H_
#define system_time_H_

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#define PRESCALER ((1 << CS10) | (0 << CS11) | (1<< CS12))		// 1024
#define TIMER_REGISTER TCCR1B
#define TIME_TO_CLK 7812
#define TIMER_END 0xFFFE
#define TIMER_MIDDLE 0x7FFF
#define SAFETY_MARGIN 12

void system_time_init (void);
uint8_t achieved_time (uint16_t delay_ms, uint16_t *end_value);
uint16_t stop_watch(uint16_t *start_value);

#endif
