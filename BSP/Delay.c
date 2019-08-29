#include "Delay.h"

extern uint32_t SystemCoreClock;
extern volatile uint32_t Time_ms;

#define US_TO_INSTRUCTION_COUNT(freq) (freq / 1000000)

__asm void delay_loop(uint32_t delay_count)
{
loop
    SUBS R0, #1
    BNE  loop
    BX   LR
}

void delay_us(uint32_t delay_us)
{
    uint32_t count;

    count = delay_us * US_TO_INSTRUCTION_COUNT(SystemCoreClock);
    count /= 2;
    delay_loop(count);
}

void delay_ms(uint32_t delay_ms)
{
    Time_ms = delay_ms;
    while(Time_ms);
}
