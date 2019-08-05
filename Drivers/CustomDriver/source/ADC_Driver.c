/* ADC driver
 * Auther: Abner
 * Date: 2018/9/17
 */

#include "ADC_Driver.h"
#include "stm32f10x.h"
#include "Driver_Common.h"
#include "Delay.h"

#define ADC_PRECISION     4096

/* STM32F107vc Temperature sensor characteristics
 * Symbol       Min   Type    Max
 * Avg_Slope    4.0    4.3    4.6  (mV/C)
 * V25         1.34   1.43   1.52     (V)
 * Tstart         4            10    (us)
 * Tsample                   17.1    (us)
 */
#define CPU_PARAM_V25  1.43
#define AVG_SLOPE      0.0043

static ADC_RESOURCES ADC1_Resources = {
	ADC1,
	ADC1_2_IRQn,
	RCC_APB2Periph_ADC1,
};

void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetFlagStatus(DMA1_FLAG_TC1) == SET)
	{
		DMA_ClearFlag(DMA1_FLAG_TC1);
	}
}

double ADC_GetVoltageValue(uint16_t vsense)
{
	return (((double)vsense / ADC_PRECISION) * 3.3);
}

uint32_t ADC_GetCPUTemperature(uint16_t vsense)
{
	double temperature;
	temperature = ADC_GetVoltageValue(vsense);
	return (((CPU_PARAM_V25 - temperature) / AVG_SLOPE) + 25);
}

static void ADC_DMAConfig(void * adc_data, uint32_t translen, ADC_RESOURCES * adc)
{
	DMA_InitTypeDef  DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* ADC DMA configure */
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(adc->reg->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = translen;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	//DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
}
static uint32_t potentio_meter;
int32_t ADCx_Initialize(ADC_RESOURCES * adc)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef   ADC_InitStructure;

	RCC_APB2PeriphClockCmd(adc->rccRoot | RCC_APB2Periph_GPIOC, ENABLE);

	/* Config ADC IN pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

    ADC_DMAConfig(&potentio_meter, 1, adc);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5);

	ADC_DMACmd(ADC1, ENABLE);

	ADC_Cmd(ADC1, ENABLE);
	//ADC_TempSensorVrefintCmd(ENABLE);

	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));

	/* Everytime power on, do ADC calibration */
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

	ADC_SoftwareStartConvCmd(adc->reg, ENABLE);
   
	return ARM_DRIVER_OK;
}

int32_t ADCx_GetValue(uint16_t * data, ADC_RESOURCES * adc)
{
    *data = potentio_meter;
	return ARM_DRIVER_OK;
}

int32_t ADC1_Initialize (void) {
	return ADCx_Initialize(&ADC1_Resources);
}

int32_t ADC1_GetValue(uint16_t * data) {
	return ADCx_GetValue(data, &ADC1_Resources);
}
