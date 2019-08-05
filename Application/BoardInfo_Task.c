/* Real-time information of the system
 * Auther: Abner
 * Date: 2018/9/11
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x.h"
#include "ADC_Driver.h"
#include "Rtc.h"

extern void USART_Transfer(uint8_t * data_out, uint8_t * data_in, uint32_t data_length);
extern void Debug_Message(char * str, ...);


void GetPlatInfo(void)
{
	uint16_t flash_size;
	uint32_t unique_id[3];

	/* Following is platform info */

	/* CPU internal flash size */
	flash_size = *(volatile uint16_t *)0x1FFFF7E0;
	Debug_Message("Internal flash size: %d K\n", flash_size);
	
	unique_id[0] = *(volatile uint16_t *)0x1FFFF7E8;
	unique_id[1] = *(volatile uint16_t *)0x1FFFF7EC;
	unique_id[2] = *(volatile uint16_t *)0x1FFFF7F0;
	Debug_Message("Chip unique ID: %X %X %X\n", unique_id[0], unique_id[1], unique_id[2]);
}

void BoardInfo_Task(void* pvParameters)
{
    uint16_t adc_value;
    RTC_TIME time = {0};

    /* Display board information */
    GetPlatInfo();

    /* Initial RTC */
    RTC_Initialize();

    while (1)
    {
        RTC_GetTime(&time);
		Debug_Message("Date: %d-%d-%d  %02d:%02d:%02d\n", time.year, time.month, time.day, time.hour, time.minute, time.second);

        ADC1_GetValue(&adc_value);
		Debug_Message("ADC value = %d\n", adc_value);
		vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
