#include "system_time.h"

void system_time_init(void)
{
	TIMER_REGISTER |= PRESCALER;
	TCCR1A |= (0 << WGM13) | (1 << WGM12);		// CTC Mode
	OCR1A = TIMER_END;
	TIMSK |= 1 << OCIE1A;
}

uint16_t stop_watch(uint16_t *start_value)
{
	if (*start_value == 0xFFFF)
	{
		*start_value = TCNT1;
		return 0;
	}
	else
	{
		if (TCNT1 < *start_value)
		{
			uint32_t temp = ((((uint32_t)TIMER_END - (uint32_t)*start_value) + (uint32_t)TCNT1) *1000  / TIME_TO_CLK);
			uint16_t temp2 = temp;
			return temp2;
		}
		else
		{
			uint32_t temp = ((((uint32_t)TCNT1 - (uint32_t)*start_value)) *1000 / TIME_TO_CLK);
			uint16_t temp2 = temp;
			return temp2;
		}
	}
}

uint8_t achieved_time (uint16_t delay_ms, uint16_t *end_value)
{
	uint16_t temp = TCNT1;

	if (*end_value == 0xFFFF)
	{
		uint32_t tmp_end_value = ((uint32_t)TIME_TO_CLK * (uint32_t)delay_ms) / 1000;
		*end_value = tmp_end_value;
		if (temp & (1 << 15))
		{
			uint16_t diff = TIMER_END - temp;
			if (*end_value >= diff)
			{
				*end_value -= diff;
				*end_value &= ~(1 << 15);
			}
			else
			{
				*end_value += temp;
				if (*end_value >= TIMER_END - SAFETY_MARGIN)
				{
					*end_value = 0;
				}
			}
		}
		else
		{
			uint16_t diff = TIMER_MIDDLE - temp;
			if (*end_value >= diff)
			{
				*end_value -= diff;
				*end_value |= (1 << 15);
			}
			else
			{
				*end_value += temp;
				if (*end_value >= TIMER_MIDDLE - SAFETY_MARGIN)
				{
					*end_value = 0;
					*end_value |= (1 << 15);
				}
			}
		}
	}
	else
	{
		
		if ( (temp >= *end_value) && ((temp & (1 << 15)) == (*end_value & (1 <<15))))
		{
			*end_value = 0xFFFF;
			return 1;
		}
	}
	return 0;
}

ISR(TIMER1_COMPA_vect)
{
	TCNT1 = 0;
}