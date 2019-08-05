#include "Delay.h"

extern  volatile unsigned int Time_us;
extern  volatile unsigned int Time_ms;

void delay_10us(unsigned int t)
{ 
	//Time_us = t;
	//while(Time_us);
}

void delay_ms(unsigned int t)
{
	Time_ms = t;
	while(Time_ms);
}
