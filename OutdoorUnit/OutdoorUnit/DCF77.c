#include "DCF77.h"

DCF_Data DCF = {0, 1, 1, {0}};
Time_Data time = {0, 0, 0, 0, 0, 2000, "Montag"};
uint8_t dcf_request = 0;
uint8_t dcf_ready=0;
uint16_t dcf_stoptime_counter = 0;

void dcf_init(void)
{
	
}

void dcf_state_machine()
{
	
	switch (DCF.State) 
	{
		case idle_dcf: dcf_idle(); break;
		case setup_dcf: dcf_setup(); break; 
		case search_for_start: dcf_search_begining(); break;
		case wait_for_next_bit: dcf_next_bit(); break;
		case measure_pulse_width: dcf_measure_pw(); break;
		case one_detected:  dcf_one_detected(); break;
		case zero_detected: dcf_zero_detected(); break;
		case wait_for_next_pause: dcf_next_pause(); break;
		case minute_eval: dcf_minute_eval(); break;
		case hour_eval: dcf_hour_eval(); break;
		case day_eval: dcf_day_eval(); break;
		case month_eval: dcf_month_eval(); break;
		case weekday_eval: dcf_weekday_eval(); break;
		case year_eval: dcf_year_eval(); break;
		default: break;
	}
}

void dcf_idle (void)
{
	
	PORTC |= 1<<PC0;
	
	if (dcf_request == 1)
	{
		DCF.State = setup_dcf;
	}
}

void dcf_setup (void) 
{
	INT0_NEGATIVE_EDGE
	if (DCF_INTF0_IS_SET)
	{
		CLEAR_INT0
		STOPWATCH_RESET
		DCF.State = search_for_start;
		DCF.Bit_Number = 0;
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
			CLEAR_INT0
			DCF.State = setup_dcf;
		}
		
		if ((stoptime > (DCF_FIRST_PULSE-DCF_MARGIN_FIRST_PULSE)) && (stoptime < (DCF_FIRST_PULSE + DCF_MARGIN_FIRST_PULSE))) 
		{
			DCF.State = wait_for_next_bit;
		}
	
	}
	if (stoptime > (DCF_FIRST_PULSE + DCF_MARGIN_FIRST_PULSE)) 
	{
		DCF.State = setup_dcf;
	}
}

void dcf_next_bit (void)
{
	if (DCF.Bit_Number < 59) 
	{
		INT0_POSITIVE_EDGE
		if (DCF_INTF0_IS_SET)
		{
			STOPWATCH_RESET
			CLEAR_INT0
			DCF.State = measure_pulse_width;
		}
	}
	else
	{
		DCF.State = minute_eval;
	}
}

void dcf_measure_pw (void)
{
	INT0_NEGATIVE_EDGE
	uint16_t stoptime = stop_watch(&dcf_stoptime_counter);
	if (DCF_INTF0_IS_SET)
	{	
		CLEAR_INT0
		INT0_POSITIVE_EDGE

		if (stoptime < (DCF_ZERO - DCF_MARGIN))
		{
			DCF.State = setup_dcf;
			return;
		}
		if ((stoptime > (DCF_ZERO - DCF_MARGIN)) && (stoptime < (DCF_ZERO + DCF_MARGIN))) 
		{
			DCF.State = zero_detected;
			STOPWATCH_RESET
			stop_watch(&dcf_stoptime_counter);
		}
		if ((stoptime > (DCF_ONE - DCF_MARGIN)) && (stoptime < (DCF_ONE + DCF_MARGIN)))
		{
			DCF.State = one_detected;
			STOPWATCH_RESET
		}		
	}
	if (stoptime > (DCF_ONE + DCF_MARGIN)) 
	{
		DCF.State = setup_dcf;
	}
}

void dcf_one_detected (void)
{
	DCF.State = wait_for_next_bit;
	
	if (DCF.Bit_Number < 59)
	{
		if((DCF.Bit_Number>=21)&&(DCF.Bit_Number<=28)) 
		{
			DCF.RawData[0] |= (1 << (DCF.Bit_Number - 21));
		}
		else if ((DCF.Bit_Number >= 29) && (DCF.Bit_Number <= 35)) 
		{	
			DCF.RawData[1] |= (1 << (DCF.Bit_Number - 29));
		}
		else if ((DCF.Bit_Number >= 36) && (DCF.Bit_Number <= 41)) 
		{	
			DCF.RawData[2] |= (1 << (DCF.Bit_Number - 36));
		}
		else if ((DCF.Bit_Number >= 42) && (DCF.Bit_Number <= 44)) 
		{	
			DCF.RawData[3] |= (1 << (DCF.Bit_Number - 42));
		}
		else if ((DCF.Bit_Number >= 45) && (DCF.Bit_Number <= 49)) 
		{	
			DCF.RawData[4] |= (1 << (DCF.Bit_Number - 45));
		}
		else if ((DCF.Bit_Number >= 50) && (DCF.Bit_Number <= 58)) 
		{	
			DCF.RawData[5] |= (1 << (DCF.Bit_Number - 50));
		}
		DCF.Bit_Number++;	
	}
}

void dcf_zero_detected (void)
{
	if (DCF.Bit_Number < 59)
	{
		DCF.Bit_Number++;
		DCF.State = wait_for_next_bit;
	}

}

void dcf_next_pause (void)
{
	uint16_t stoptime = stop_watch(&dcf_stoptime_counter);
	if (DCF_INTF0_IS_SET)	
	{
		if (stoptime <= DCF_ONE_PAUSE - DCF_MARGIN) 
		{
			DCF.State = setup_dcf;
		}
		if ((stoptime >= DCF_ONE_PAUSE - DCF_MARGIN) && (stoptime <= DCF_ZERO_PAUSE + DCF_MARGIN)) 
		{
			DCF.State = wait_for_next_bit;
		}
	}
	if (DCF.Bit_Number < 59) 
	{
		if (stoptime >= DCF_ZERO_PAUSE + DCF_MARGIN) 
		{
			DCF.State = setup_dcf;
		} 
	}
	else
	{
		DCF.State = minute_eval;		
	}
}

void dcf_minute_eval (void)
{
	time.minute = data_interpretation(DCF.RawData[0], 7);
	if (DCF.Parity)
	{
		DCF.State = hour_eval;
	}
	else
	{
		DCF.State = setup_dcf;
	}
}

void dcf_hour_eval (void)
{
	time.hour = data_interpretation(DCF.RawData[1], 6);
	if (DCF.Parity)
	{
		DCF.State = day_eval;
	}
	else
	{
		DCF.State = setup_dcf;
	}
}

void dcf_day_eval (void)
{
	time.day = data_interpretation(DCF.RawData[2], 6);
	DCF.State = weekday_eval;
}

void dcf_weekday_eval (void)
{
	switch(DCF.RawData[3])
	{
		case 0: DCF.Parity == (1<<0);
		case 1: strcpy(time.weekday,"Montag"); DCF.Parity == (0<<0); break;
		case 2: strcpy(time.weekday,"Dienstag"); DCF.Parity == (0<<0); break;
		case 3: strcpy(time.weekday,"Mittwoch"); DCF.Parity == (1<<0); break;
		case 4: strcpy(time.weekday,"Donnerstag"); DCF.Parity == (0<<0); break;
		case 5: strcpy(time.weekday,"Freitag"); DCF.Parity == (1<<0); break;
		case 6: strcpy(time.weekday,"Samstag"); DCF.Parity == (1<<0); break;
		case 7: strcpy(time.weekday,"Sonntag"); DCF.Parity == (0<<0); break;
		default:break;
	}
	DCF.State = month_eval; 
}

void dcf_month_eval (void)
{
	time.month = data_interpretation(DCF.RawData[4], 5);
	DCF.State = year_eval;
}

void dcf_year_eval (void)
{
	time.year = 2000;
	time.year += data_interpretation(DCF.RawData[5], 8);
	DCF.State = idle_dcf;
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
		DCF.Parity == (0<<0);
	}
	else
	{
		DCF.Parity == (1<<0);
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