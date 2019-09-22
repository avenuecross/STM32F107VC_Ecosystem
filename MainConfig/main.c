#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LED_driver.h"
#include "Application_Config.h"
#include "GUI.h"

TaskHandle_t  thUITask = NULL;

extern void Debug_Message(char * str, ...);
extern void TEST_Task(void* pvParameters);
extern void GUI_Task(void* pvParameters);
extern void BSP_Init(void);
extern void GUI_Refresh_Task(void* pvParameters);
extern void WIFI_Task(void* pvParameters);
extern void BoardInfo_Task(void* pvParameters);

extern void delay_us(uint32_t delay_us);

static void vMainTask( void *pvParameters )
{
    BaseType_t rc = 0;

    Debug_Message("========================================\n\n\
                   STM32F107VC6 Ecosystem Application Start\n\n\
                   ========================================\n\n");

    rc = xTaskCreate( TEST_Task, "TEST", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    if (rc != pdPASS) for (;;);

	rc = xTaskCreate( GUI_Task, "GUI", GUI_STACK_SIZE, NULL, GUI_TASK_PRIORITY, &thUITask);
	if (rc != pdPASS) for (;;);

	rc = xTaskCreate( WIFI_Task, "WIFI", configMINIMAL_STACK_SIZE, NULL, WIFI_TASK_PRIORITY, NULL);
	if (rc != pdPASS) for (;;);

	rc = xTaskCreate( BoardInfo_Task, "BDINFO", configMINIMAL_STACK_SIZE, NULL, WIFI_TASK_PRIORITY, NULL);
	if (rc != pdPASS) for (;;);

	vTaskDelete(NULL);
}

int main(void)
{
    BaseType_t rc = 0;

	BSP_Init();

    rc = xTaskCreate( vMainTask,
                      "MAIN",
                      configMINIMAL_STACK_SIZE,
                      NULL,
                      2,
                      NULL );
    if (rc != pdPASS) for (;;);

    vTaskStartScheduler();

    while (1);
}

void vApplicationMallocFailedHook(void)
{
	while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
	while (1);
}	
