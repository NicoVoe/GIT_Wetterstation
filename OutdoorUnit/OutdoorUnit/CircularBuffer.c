#include "CircularBuffer.h"

//--------------------------------------------------------------------------------
//Circular Buffer Functions:
//--------------------------------------------------------------------------------
uint8_t cb_push(circular_buffer *cb, uint8_t *data, uint8_t length) 
{
	length -= 1;
	//*******************************************
	if(cb->read_index == cb->write_index)			//Buffer is empty
	{
		if(length > (cb->size-1))
		{
			return 0;						//not able to write data into the buffer
		}
		else
		{
			for(uint8_t i=0; i<length; i++) 
			{
				if(cb->write_index >= cb->size)	//Check for Buffer end
				{
					cb->write_index = 0;
					cb->buffer[cb->write_index] = data[i];
				}
				else
				{
					cb->buffer[cb->write_index] = data[i];
				}
				cb->write_index++;
			}
			return 1;					//success
		}
	}
	//*******************************************
	else if(cb->write_index > cb->read_index)
	{
		uint8_t left_space = 0;
		if (cb->read_index == 0) 
		{
			left_space = cb->size - cb->write_index - 1;
		}
		else
		{
			left_space = (cb->size - cb->write_index) + (cb->read_index-1);
		}
		
		if(length > left_space)
		{
			return 0;			//not enough free space in buffer
		}
		else
		{
			for (int i=0; i<length; i++)
			{
				if(cb->write_index >= cb->size)	//Check for Buffer end
				{
					cb->write_index = 0;
					cb->buffer[cb->write_index] = data[i];
				}
				else
				{
					cb->buffer[cb->write_index] = data[i];
				}
				cb->write_index++;
			}
			return 1;
		}
	}
	//*******************************************
	else if(cb->write_index < cb->read_index)
	{
		uint8_t left_space = cb->read_index - cb->write_index -1;
		if(length > left_space) 
		{
			return 0;			//not enough free space in buffer
		}
		else
		{
			for (int i=0; i<length; i++)
			{
				 cb->buffer[cb->write_index] = data[i];
				 cb->write_index++;
			}
			return 1;
		}
	}
	//*******************************************
return 0;
}		

uint8_t cb_pop(circular_buffer *cb, uint8_t *data, uint8_t length) 
{
	uint8_t i=0;
	
	if(cb->read_index == cb->write_index) 
	{
		return 0;
	}
	else
	{
		while((cb->read_index != cb->write_index) && (i<(length-1))) 
		{
			if(cb->buffer[cb->read_index]!=0) 
			{	
				data[i]=cb->buffer[cb->read_index];
			}
			if(cb->read_index >= (cb->size-1)) 
			{
				cb->read_index=0;
			}
			else
			{
				cb->read_index++;
			}
			i++;
		}	
	}
	data[i]=0;
return i;
}

uint8_t cb_is_empty(circular_buffer *cb)
{
	return (cb->read_index == cb->write_index); 
}
