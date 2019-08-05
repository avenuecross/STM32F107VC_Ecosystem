#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "USART_Driver.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


extern USART_DRIVER Driver_USART1;
#define usart1 (&Driver_USART1)

static SemaphoreHandle_t _xSemphrUsart = NULL;
static char usart_message[80] = {0};

static void _DebugMessageLock(void)
{
	if (_xSemphrUsart)
	{
		xSemaphoreTake(_xSemphrUsart, portMAX_DELAY);
	}
	else
	{
		_xSemphrUsart = xSemaphoreCreateMutex();
	}
}

static void _DebugMessageUnock(void)
{
	if (_xSemphrUsart)
	{
		xSemaphoreGive(_xSemphrUsart);
	}
}

void Debug_Message(char * str, ...)
{
	int str_len = 0;
	va_list param_list;

	_DebugMessageLock();
	memset(usart_message, 0, sizeof(usart_message));
	va_start(param_list, str);
	str_len = vsnprintf(usart_message, sizeof(usart_message), str, param_list);
	va_end(param_list);

	usart1->Transfer(usart_message, NULL, str_len);
	_DebugMessageUnock();
}
