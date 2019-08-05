/* STM32F107 RTC Time Function
 * Auther: Abner
 * Date: 2019/7/12
 */

#include "Rtc.h"


#define DAYS_IN_A_YEAR      (365U)
#define SECONDS_IN_A_MINUTE (60U)
#define SECONDS_IN_A_HOUR   (3600U)
#define SECONDS_IN_A_DAY    (86400U)
#define YEAR_RANGE_START    (1970U)
//#define YEAR_RANGE_END      (2099U)

void RTC_Initialize(void)
{
    RTC_TIME time;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    BKP_DeInit();
    RCC_LSEConfig(RCC_LSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();

    /* Wait RTC over */
    RTC_WaitForLastTask();
    RTC_SetPrescaler(32767);
    RTC_WaitForLastTask();

    /* Set time */
    time.year = 2019;
    time.month = 7;
    time.day = 12;
    time.hour = 13;
    time.minute = 13;
    time.second = 0;
    RTC_SetTime(&time);
}

void RTC_SetTime(RTC_TIME * time)
{
    /* Number of days from begin of the non Leap-year*/
    /* Number of days from begin of the non Leap-year*/
    uint16_t monthDays[] = {0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};
    uint32_t seconds;

    /* Compute number of days from 1970 till given year*/
    seconds = (time->year - 1970U) * DAYS_IN_A_YEAR;
    /* Add leap year days */
    seconds += ((time->year / 4) - (1970U / 4));
    /* Add number of days till given month*/
    seconds += monthDays[time->month];
    /* Add days in given month. We subtract the current day as it is
     * represented in the hours, minutes and seconds field*/
    seconds += (time->day - 1);
    /* For leap year if month less than or equal to Febraury, decrement day counter*/
    if ((!(time->year & 3U)) && (time->month <= 2U))
    {
        seconds--;
    }

    seconds = (seconds * SECONDS_IN_A_DAY) + (time->hour * SECONDS_IN_A_HOUR) +
              (time->minute * SECONDS_IN_A_MINUTE) + time->second;

    RTC_SetCounter(seconds);
    RTC_WaitForLastTask();
}

void RTC_GetTime(RTC_TIME * time)
{
    uint32_t x;
    uint32_t secondsRemaining, days;
    uint16_t daysInYear;
    uint32_t seconds;

    /* Table of days in a month for a non leap year. First entry in the table is not used,
     * valid months start from 1
     */
    uint8_t daysPerMonth[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

    seconds = RTC_GetCounter();

    /* Start with the seconds value that is passed in to be converted to date time format */
    secondsRemaining = seconds;

    /* Calcuate the number of days, we add 1 for the current day which is represented in the
     * hours and seconds field
     */
    days = secondsRemaining / SECONDS_IN_A_DAY + 1;

    /* Update seconds left*/
    secondsRemaining = secondsRemaining % SECONDS_IN_A_DAY;

    /* Calculate the datetime hour, minute and second fields */
    time->hour = secondsRemaining / SECONDS_IN_A_HOUR;
    secondsRemaining = secondsRemaining % SECONDS_IN_A_HOUR;
    time->minute = secondsRemaining / 60U;
    time->second = secondsRemaining % SECONDS_IN_A_MINUTE;

    /* Calculate year */
    daysInYear = DAYS_IN_A_YEAR;
    time->year = YEAR_RANGE_START;
    while (days > daysInYear)
    {
        /* Decrease day count by a year and increment year by 1 */
        days -= daysInYear;
        time->year++;

        /* Adjust the number of days for a leap year */
        if (time->year & 3U)
        {
            daysInYear = DAYS_IN_A_YEAR;
        }
        else
        {
            daysInYear = DAYS_IN_A_YEAR + 1;
        }
    }

    /* Adjust the days in February for a leap year */
    if (!(time->year & 3U))
    {
        daysPerMonth[2] = 29U;
    }

    for (x = 1U; x <= 12U; x++)
    {
        if (days <= daysPerMonth[x])
        {
            time->month = x;
            break;
        }
        else
        {
            days -= daysPerMonth[x];
        }
    }

    time->day = days;
}
