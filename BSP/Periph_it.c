#include "stm32f10x.h"
#include "LED_Driver.h"

volatile unsigned int Time_ms;
volatile unsigned int Time_SD;
volatile unsigned int Time_SD_Init;

void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		LED_Toogle(LED_3);
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
}

void TIM6_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		Time_ms--;
		if (Time_SD > 0)
		{
			Time_SD--;
		}
		if (Time_SD_Init > 0)
		{
			Time_SD_Init--;
		}
	}
}


void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
//		if (Time_SD > 0)
//		{
//			Time_SD--;
//		}
	}
}
