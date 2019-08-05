/* GUI Task
 * Auther: Abner
 * Date: 2018/9/11
 * Note: STM32F107 only has 64K RAM, can't assign emWin too much resource, so I use GUI_Exec() manually rather than one single task.
 */


#include "GUI_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "MULTIEDIT.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

MULTIEDIT_HANDLE _hMEdit = NULL;
WM_HWIN _hMainDialog = NULL;

extern GUI_CONST_STORAGE GUI_BITMAP bmnaruto;
extern TaskHandle_t  thUITask;

struct haha_lihai
{
    char a;
	int  b;
};

struct haha_lihai haha1 __attribute__((section("haha1_23"), zero_init));

char ui_message[50] = {0};

void UI_ShowMessage(char * message, ...)
{
	va_list param_list;

	memset(ui_message, 0, sizeof(ui_message));
	va_start(param_list, message);
	vsnprintf(ui_message, sizeof(ui_message), message, param_list);
	va_end(param_list);

	if (thUITask)
		xTaskNotify(thUITask, UI_MESSAGE_EVENT, eSetBits);
}

void GUI_Task(void* pvParameters)
{
	uint32_t events;

    GUI_Init();

    /* Draw start-up picture */
    //GUI_DrawBitmap(&bmnaruto, 0, 0);

	GUI_SetBkColor(GUI_BLACK);
	vTaskDelay(pdMS_TO_TICKS(1500));

    GUI_SetColor(GUI_RED);
    GUI_SetFont(&GUI_Font32B_ASCII);
    GUI_DispStringAt("Abner's board", 60, 50);

//	_hMEdit = MULTIEDIT_CreateEx(10, 10, 300, 220, WM_HBKWIN, WM_CF_SHOW, MULTIEDIT_CF_READONLY, UI_ID_MULTIPLYEDIT, 50, NULL);
//	MULTIEDIT_SetBkColor(_hMEdit, MULTIEDIT_CI_READONLY, GUI_WHITE);
//	MULTIEDIT_SetTextColor(_hMEdit, MULTIEDIT_CI_READONLY, GUI_BLACK);
//	MULTIEDIT_AddText(_hMEdit, "GUI Task Ready\n");
	GUI_Exec();

    while (1)
    {
		xTaskNotifyWait( 0, 0xffffffff, &events, portMAX_DELAY);
		
		if (events & UI_MESSAGE_EVENT)
		{
			MULTIEDIT_AddText(_hMEdit, ui_message);
			GUI_Exec();
		}
    }
}
