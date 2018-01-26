#include "DCF77.h"

DCF_Data dcf = {0, 1, 1, {0}};
Time_Data time = {0, 0, 0, 0, 0, 2000, "Montag"};
uint8_t dcf_request = 0;
uint8_t dcf_ready=0;
uint16_t dcf_stoptime_counter = 0;

void dcf_init(void)
{
	
}

void dcf_state_machine()
{
	
	switch (dcf.state) 
	{
		case dcf_state_idle: dcf_idle(); break;
		case dcf_state_setup: dcf_setup(); break; 
		case dcf_state_search_for_start: dcf_search_begining(); break;
		case dcf_state_wait_for_next_bit: dcf_next_bit(); break;
		case dcf_state_measure_pulse_width: dcf_measure_pw(); break;
		case dcf_state_one_detected:  dcf_one_detected(); break;
		case dcf_state_zero_detected: dcf_zero_detected(); break;
		case dcf_state_wait_for_next_pause: dcf_next_pause(); break;
		case dcf_state_minute_eval: dcf_minute_eval(); break;
		case dcf_state_hour_eval: dcf_hour_eval(); break;
		case dcf_state_day_eval: dcf_day_eval(); break;
		case dcf_state_month_eval: dcf_month_eval(); break;
		case dcf_state_weekday_eval: dcf_weekday_eval(); break;
		case dcf_state_year_eval: dcf_year_eval(); break;
		default: break;
	}
}

void dcf_idle (void)
{
	PORTC |= 1<<PC0;
	
	if (dcf_request == 1)
	{
		dcf.state = dcf_state_setup;
	}
}

void dcf_setup (void) 
{
	DCF_INT0_NEGATIVE_EDGE
	if (DCF_INTF0_IS_SET)
	{
		DCF_CLEAR_INT0
		DCF_STOPWATCH_RESET
		dcf.state = dcf_state_search_for_start;
		dcf.bit_number = 0;
	}
}

void dcf_search_begining (void)
{ 
	MCUCR |= ((1 << ISC00) | (1 << ISC01));					//INT0_POSITIVE_EDGE
	uint16_t stoptime = stop_watch(&dcf_stoptime_counter);
	if (DCF_INTF0_IS_SET) 
	{
		//CLEAR_INT0
		if (stoptime < (1700))
		{
			DCF_CLEAR_INT0
			dcf.state = dcf_state_setup;
		}
		
		if ((stoptime > (DCF_FIRST_PULSE-DCF_MARGIN_FIRST_PULSE)) && (stoptime < (DCF_FIRST_PULSE + DCF_MARGIN_FIRST_PULSE))) 
		{
			dcf.state = dcf_state_wait_for_next_bit;
		}
	
	}
	if (stoptime > (DCF_FIRST_PULSE + DCF_MARGIN_FIRST_PULSE)) 
	{
		dcf.state = dcf_state_setup;
	}
}

void dcf_next_bit (void)
{
	if (dcf.bit_number < 59) 
	{
		DCF_INT0_POSITIVE_EDGE
		if (DCF_INTF0_IS_SET)
		{
			DCF_STOPWATCH_RESET
			DCF_CLEAR_INT0
			dcf.state = dcf_state_measure_pulse_width;
		}
	}
	else
	{
		dcf.state = dcf_state_minute_eval;
	}
}

void dcf_measure_pw (void)
{
	DCF_INT0_NEGATIVE_EDGE
	uint16_t stoptime = stop_watch(&dcf_stoptime_counter);
	if (DCF_INTF0_IS_SET)
	{	
		DCF_CLEAR_INT0
		DCF_INT0_POSITIVE_EDGE

		if (stoptime < (DCF_ZERO - DCF_MARGIN))
		{
			dcf.state = dcf_state_setup;
			return;
		}
		if ((stoptime > (DCF_ZERO - DCF_MARGIN)) && (stoptime < (DCF_ZERO + DCF_MARGIN))) 
		{
			dcf.state = dcf_state_zero_detected;
			DCF_STOPWATCH_RESET
			stop_watch(&dcf_stoptime_counter);
		}
		if ((stoptime > (DCF_ONE - DCF_MARGIN)) && (stoptime < (DCF_ONE + DCF_MARGIN)))
		{
			dcf.state = dcf_state_one_detected;
			DCF_STOPWATCH_RESET
		}		
	}
	if (stoptime > (DCF_ONE + DCF_MARGIN)) 
	{
		dcf.state = dcf_state_setup;
	}
}

void dcf_one_detected (void)
{
	dcf.state = dcf_state_wait_for_next_bit;
	
	if (dcf.bit_number < 59)
	{
		if((dcf.bit_number>=21)&&(dcf.bit_number<=28)) 
		{
			dcf.raw_data[0] |= (1 << (dcf.bit_number - 21));
		}
		else if ((dcf.bit_number >= 29) && (dcf.bit_number <= 35)) 
		{	
			dcf.raw_data[1] |= (1 << (dcf.bit_number - 29));
		}
		else if ((dcf.bit_number >= 36) && (dcf.bit_number <= 41)) 
		{	
			dcf.raw_data[2] |= (1 << (dcf.bit_number - 36));
		}
		else if ((dcf.bit_number >= 42) && (dcf.bit_number <= 44)) 
		{	
			dcf.raw_data[3] |= (1 << (dcf.bit_number - 42));
		}
		else if ((dcf.bit_number >= 45) && (dcf.bit_number <= 49)) 
		{	
			dcf.raw_data[4] |= (1 << (dcf.bit_number - 45));
		}
		else if ((dcf.bit_number >= 50) && (dcf.bit_number <= 58)) 
		{	
			dcf.raw_data[5] |= (1 << (dcf.bit_number - 50));
		}
		dcf.bit_number++;	
	}
}

void dcf_zero_detected (void)
{
	if (dcf.bit_number < 59)
	{
		dcf.bit_number++;
		dcf.state = dcf_state_wait_for_next_bit;
	}

}

void dcf_next_pause (void)
{
	uint16_t stoptime = stop_watch(&dcf_stoptime_counter);
	if (DCF_INTF0_IS_SET)	
	{
		if (stoptime <= DCF_ONE_PAUSE - DCF_MARGIN) 
		{
			dcf.state = dcf_state_setup;
		}
		if ((stoptime >= DCF_ONE_PAUSE - DCF_MARGIN) && (stoptime <= DCF_ZERO_PAUSE + DCF_MARGIN)) 
		{
			dcf.state = dcf_state_wait_for_next_bit;
		}
	}
	if (dcf.bit_number < 59) 
	{
		if (stoptime >= DCF_ZERO_PAUSE + DCF_MARGIN) 
		{
			dcf.state = dcf_state_setup;
		} 
	}
	else
	{
		dcf.state = dcf_state_minute_eval;		
	}
}

void dcf_minute_eval (void)
{
	time.minute = data_interpretation(dcf.raw_data[0], 7);
	if (dcf.parity)
	{
		dcf.state = dcf_state_hour_eval;
	}
	else
	{
		dcf.state = dcf_state_setup;
	}
}

void dcf_hour_eval (void)
{
	time.hour = data_interpretation(dcf.raw_data[1], 6);
	if (dcf.parity)
	{
		dcf.state = dcf_state_day_eval;
	}
	else
	{
		dcf.state = dcf_state_setup;
	}
}

void dcf_day_eval (void)
{
	time.day = data_interpretation(dcf.raw_data[2], 6);
	dcf.state = dcf_state_weekday_eval;
}

void dcf_weekday_eval (void)
{
	switch(dcf.raw_data[3])
	{
		case 0: dcf.parity == (1<<0);
		case 1: strcpy(time.weekday,"Montag"); dcf.parity == (0<<0); break;
		case 2: strcpy(time.weekday,"Dienstag"); dcf.parity == (0<<0); break;
		case 3: strcpy(time.weekday,"Mittwoch"); dcf.parity == (1<<0); break;
		case 4: strcpy(time.weekday,"Donnerstag"); dcf.parity == (0<<0); break;
		case 5: strcpy(time.weekday,"Freitag"); dcf.parity == (1<<0); break;
		case 6: strcpy(time.weekday,"Samstag"); dcf.parity == (1<<0); break;
		case 7: strcpy(time.weekday,"Sonntag"); dcf.parity == (0<<0); break;
		default:break;
	}
	dcf.state = dcf_state_month_eval; 
}

void dcf_month_eval (void)
{
	time.month = data_interpretation(dcf.raw_data[4], 5);
	dcf.state = dcf_state_year_eval;
}

void dcf_year_eval (void)
{
	time.year = 2000;
	time.year += data_interpretation(dcf.raw_data[5], 8);
	dcf.state = dcf_state_idle;
	dcf_request = 0;
	dcf_ready = 1;
}

uint8_t data_interpretation (uint8_t dcf_value, uint8_t data_length) 
{
	
	uint8_t parity = 0, data = 0;
	for(uint8_t i = 0; i < data_length; i++)
	{
		parity += (dcf_value & (1 << i));
		if (dcf_value & (1 << i)) 
		{
			if (i < 4)
			{
				data += power(2,i);
			}
			else
			{
				data += 10 * power(2,(i-4));
			}
		}
	}
	parity += dcf_value & (1<<data_length);
	if (parity & (1<<0)) 
	{
		dcf.parity == (0<<0);
	}
	else
	{
		dcf.parity == (1<<0);
	}
return (data);
}

uint8_t power (uint8_t basis, uint8_t exponent) 
{
	uint8_t ergebnis = 1;
	
	for(uint8_t i = 1; i <= exponent; i++) 
	{
		ergebnis *= basis;
	}

return (ergebnis);
}