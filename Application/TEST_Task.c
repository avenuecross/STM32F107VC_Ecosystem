#include "LED_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Delay.h"
#include "EEPROM_Driver.h"
#include "SerialFlash.h"
#include "MULTIEDIT.h"
#include "string.h"
#include "GUI_task.h"
#include "ADC_Driver.h"
#include "SdCard.h"
#include "ff.h"


extern void Debug_Message(char * str, ...);
extern MULTIEDIT_HANDLE _hMEdit;
extern void UI_ShowMessage(char * message, ...);
extern int32_t ADCx_GetValueDMA(uint16_t * data);

char data_buff[20] = "GG Hellosddsadasd";
char data_buff1[20] = {0};
char * mess = "Abner put this message";

TaskStatus_t TaskStatus[4];

void RTOS_OutputStatus(void)
{
	UBaseType_t  task_num;
	uint8_t      i;

	task_num = uxTaskGetNumberOfTasks();
	if (task_num <= sizeof(TaskStatus))
	{
		uxTaskGetSystemState(TaskStatus, task_num, NULL);
		Debug_Message("\n====FreeRTOS Task Status====\n\n");
		for (i = 0; i < task_num; i++)
		{
			Debug_Message("%s: %d--%d\n", TaskStatus[i].pcTaskName, TaskStatus[i].pxStackBase, TaskStatus[i].usStackHighWaterMark);
		}
		Debug_Message("\n============END=============\n\n");
	}
	else
	{
		Debug_Message("Current task number is over the size of array!\n\n");
	}
}

void TEST_Task(void* pvParameters)
{
	//__REV
//	uint8_t id;
//	uint16_t data[2] = {0};
//	FATFS fs;
//	FIL file;
//	FRESULT fr;
	
//	id = FLASH_ReadDeviceID();
//	Debug_Message("Serial Flash Device ID: %d\n", id);

//	f_mount(&fs, "", 0);
//	fr = f_open(&file, "Abner" ,FA_READ);
//	if (fr == FR_OK)
//	{
//		f_close(&file);
//	}

    while (1)
    {
//		FLASH_SectorErase(0x0);
//		FLASH_WritePage(data_buff, 0x0, 20);
//		FLASH_Read(data_buff1, 0x0, 20);
//		Debug_Message("Str: %s\n", data_buff1);
		
		//adc->GetValue(data);
		//Debug_Message("Pometer: %.2fV\nCPU temperature: %dC\n", ADC_GetVoltageValue(data[0]), ADC_GetCPUTemperature(data[1]));
		//UI_ShowMessage("Pometer: %.2fV\nCPU temperature: %dC\n", ADC_GetVoltageValue(data[0]), ADC_GetCPUTemperature(data[1]));
		
		//SD_initialize(0);
		
		/* LED Toggle for test */
		LED_On(LED_0);
		vTaskDelay(pdMS_TO_TICKS(250));
		LED_Off(LED_0);

		LED_On(LED_1);
		vTaskDelay(pdMS_TO_TICKS(250));
		LED_Off(LED_1);

		LED_On(LED_2);
		vTaskDelay(pdMS_TO_TICKS(250));
		LED_Off(LED_2);

		LED_On(LED_3);
		vTaskDelay(pdMS_TO_TICKS(250));
		LED_Off(LED_3);

		//RTOS_OutputStatus();
	}
}
