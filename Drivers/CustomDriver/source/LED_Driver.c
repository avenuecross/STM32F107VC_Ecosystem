#include "LED_Driver.h"

const struct LED led[] =
{
    GPIOD, GPIO_Pin_2,
    GPIOD, GPIO_Pin_3,
    GPIOD, GPIO_Pin_4,
    GPIOD, GPIO_Pin_7,
};

#define NUM_LEDS (sizeof(led)/sizeof(struct LED))

/*
 * Example: LED_On(LED_0 | LED_1)
 */
void LED_On(uint16_t val)
{
    uint8_t n;

    for(n = 0; n < NUM_LEDS; n++)
    {
        if (val & (1 << n)) 
        {
            GPIO_SetBits(led[n].port, led[n].pin);
        }
    }
}

void LED_Off(uint16_t val)
{
    uint8_t n;

    for(n = 0; n < NUM_LEDS; n++)
    {
        if (val & (1 << n)) 
        {
			GPIO_ResetBits(led[n].port, led[n].pin);      
        }
    }
}

void LED_Toogle(uint16_t val)
{
    uint8_t n;
	uint8_t bit_value;

    for(n = 0; n < NUM_LEDS; n++)
    {
        if (val & (1 << n)) 
        {
			bit_value = GPIO_ReadOutputDataBit(led[n].port, led[n].pin);
			if (bit_value) GPIO_ResetBits(led[n].port, led[n].pin);
			else           GPIO_SetBits(led[n].port, led[n].pin);
        }
    }
}
