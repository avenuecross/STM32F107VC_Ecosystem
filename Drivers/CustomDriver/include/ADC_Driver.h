#ifndef __ADC_DRIVER_H
#define __ADC_DRIVER_H

#include "stm32f10x.h"

typedef struct {
	ADC_TypeDef      	   *reg;            	// ADC peripheral register interface
	IRQn_Type            	irq_num;			// ADC IRQ number
	uint32_t 			 	rccRoot;
} const ADC_RESOURCES;

int32_t ADC1_Initialize(void);
int32_t ADC1_GetValue(uint16_t * data);
uint32_t ADC_GetCPUTemperature(uint16_t vsense);
double ADC_GetVoltageValue(uint16_t vsense);

#endif
