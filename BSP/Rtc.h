#ifndef __DELAY_H
#define __DELAY_H 

#include "stm32f10x.h"

typedef struct rtc_time {
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
} RTC_TIME;

void RTC_Initialize(void);
void RTC_SetTime(RTC_TIME * time);
void RTC_GetTime(RTC_TIME * time);

#endif
