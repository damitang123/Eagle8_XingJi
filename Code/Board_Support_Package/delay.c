#include "system_misc.h"

void delay_us(unsigned short time)
{
	unsigned short i = 0;
	while(time--)
	{
		i = 6;
		while(i--);
	}
}

void delay_ms(unsigned short time)
{
	volatile unsigned long nStartTime;
	nStartTime = Get_System_Time();
	while(Get_System_Time() < nStartTime + time);
}

