/* ESP8266 WiFi Application
 * Auther: Abner
 * Date: 2018/9/11
 */

#include "FreeRTOS.h"
#include "task.h"

extern void USART_Transfer(uint8_t * data_out, uint8_t * data_in, uint32_t data_length);
extern void Debug_Message(char * str, ...);

unsigned char senAT1[30] = "AT+CWMODE=3\r\n";
unsigned char recAT1[30] = {0};

void WIFI_Task(void* pvParameters)
{
    //BaseType_t rc = 0;
    //uint32_t   event = 0;
	
    while (1)
    {
        //rc = xTaskNotifyWait(0x0, 0x0, &event, pdMS_TO_TICKS(500));
        
        //USART_Transfer(senAT1, recAT1, sizeof(senAT1));
		//Debug_Message("WiFI rec:%s\n", recAT1);
		vTaskDelay(pdMS_TO_TICKS(1500));
    }
}
