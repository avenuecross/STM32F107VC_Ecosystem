#ifndef __USART_DRIVER_H
#define __USART_DRIVER_H

#include "stm32f10x.h"

typedef struct
{
	USART_TypeDef      	    *reg;      // USART peripheral register interface
	IRQn_Type                irq_num;
	uint32_t 			 	 rccRoot;
} const USART_RESOURCES;

typedef struct
{
	int32_t    (*Initialize) (void);
	int32_t    (*Configure)  (void);
	int32_t    (*Transfer)   (void * data_out, void *data_in, uint32_t translen);
} USART_DRIVER;

#endif
