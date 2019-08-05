#ifndef __LED_DRIVER_H
#define __LED_DRIVER_H

#include "stm32f10x.h"

struct LED
{
    GPIO_TypeDef   *port;
    uint16_t        pin;
};

#define	LED_0		0x01	
#define	LED_1		0x02	
#define	LED_2		0x04	
#define	LED_3		0x08	
#define	LED_ALL	    0x0F

void    LED_Initialize(void);
void    LED_On(uint16_t val);
void    LED_Off(uint16_t val);
void    LED_Toogle(uint16_t val);

#endif 
