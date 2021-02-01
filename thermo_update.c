/*
 * thermo_update.c
 *
 *  Created on: 12-Oct-2020
 *      Author: sanjay
 */
#include"thermo.h"
#include <stdlib.h>
int bitMasks[10] = {
		0b1111110, // 0
		0b0001100, // 1
		0b0110111, // 2
		0b0011111, // 3
		0b1001101, // 4
		0b1011011, // 5
		0b1111011, // 6
		0b0001110, // 7
		0b1111111, // 8
		0b1011111  // 9
};

int set_temp_from_ports(temp_t *temp)
{
	if(THERMO_SENSOR_PORT/64>=1000 && THERMO_SENSOR_PORT%64>0)
	{
		return 1;
	}
	int temperature = -500;
	temperature = temperature+(THERMO_SENSOR_PORT/64);
	if(THERMO_SENSOR_PORT%64 >= 32)
	{
		temperature = temperature+1;
	}
	temp->is_fahrenheit = THERMO_STATUS_PORT & 1;;

	if(temp->is_fahrenheit)
	{
		temperature = (temperature*9)/5;
		int rem = temperature%10;
		temperature = temperature/10 + 32;
		temperature = temperature*10+rem;
	}

	temp->tenths_degrees = temperature;
	return 0;
}
int set_display_from_temp(temp_t temp, int *display)
{
	if(temp.is_fahrenheit == 0 && (temp.tenths_degrees>500 || temp.tenths_degrees < -500) )
	{
		return 1;
	}
	if(temp.is_fahrenheit == 1 && (temp.tenths_degrees>1220 ||  temp.tenths_degrees < -580) )
	{
		return 1;
	}
	if(temp.is_fahrenheit != 0 && temp.is_fahrenheit != 1)
	{
		return 1;
	}

	int temperature = abs(temp.tenths_degrees);
	*display=0b0;

	// Set Fahrenheit/Celcius
	int isfahrenheitOrCelcius;
	if(temp.is_fahrenheit)
	{
		isfahrenheitOrCelcius = 0b10;
	}
	else
	{
		isfahrenheitOrCelcius = 0b01;
	}
	isfahrenheitOrCelcius = isfahrenheitOrCelcius<<28;
	*display = *display | isfahrenheitOrCelcius;

	// Set 10Th digit
	int temp_tenth=0b0;
	temp_tenth = bitMasks[temperature%10];
	*display = *display | temp_tenth;
	temperature=temperature/10;

	// Set 1st digit
	int temp_one=0b0;
	temp_one = bitMasks[temperature%10];
	temp_one = temp_one<<7;
	*display = *display | temp_one;
	temperature = temperature/10;

	// If 10 digit available
	if(temperature>0)
	{
		// Set 10 digit
		int temp_ten=0b0;
		temp_ten = bitMasks[temperature%10];
		temp_ten = temp_ten<<14;
		*display = *display | temp_ten;
		temperature = temperature/10;

		// Set 100 digit
		if(temperature>0)
		{
			int temp_hundread=0b0;
			temp_hundread = bitMasks[temperature%10];
			temp_hundread = temp_hundread<<21;
			*display = *display | temp_hundread;
		}

		if(temp.tenths_degrees<0)
		{
			int signNegative = 0b0000001;
			signNegative = signNegative<<21;
			*display = *display | signNegative;
		}
	}
	// 10 digit is not available
	else
	{
		// Set 100 digit
		if(temp.tenths_degrees<0)
		{
			int signNegative = 0b0000001;
			signNegative = signNegative<<14;
			*display = *display | signNegative;
		}
	}
	return 0;
}
int thermo_update()
{
	temp_t temp = {};
	set_temp_from_ports(&temp);
	int display = 0;
	set_display_from_temp(temp, &display);
	THERMO_DISPLAY_PORT = display;
	return 0;
}


