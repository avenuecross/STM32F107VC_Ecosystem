#include "USART_Driver.h"
#include "Driver_Common.h"

uint32_t usart1_trans_over = 0;

static USART_RESOURCES USART1_Resources = {
	USART1,
	USART1_IRQn,
	RCC_APB2Periph_USART1,
};

void DMA1_Channel4_IRQHandler(void)
{
	if (DMA_GetFlagStatus(DMA1_FLAG_TC4) == SET)
	{
		usart1_trans_over = 1;
		DMA_ClearFlag(DMA1_FLAG_TC4);
	}
}

static void USART_PinConfig(USART_RESOURCES * usart)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	switch ((uint32_t)usart->reg)
	{
		case USART1_BASE:
			/* Enable peripheral clock */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

			/* Tx of USART */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);

			/* Rx of USART */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case USART3_BASE:
			/* Enable peripheral clock */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

			/* Tx of USART */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

			/* Rx of USART */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			break;
		default:
			break;
	}
}

static void USART1_DMAConfig(void * tx_buff_addr, void * rx_buff_addr, uint32_t translen, USART_RESOURCES * usart)
{
	DMA_InitTypeDef   DMA_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, DISABLE);

	/* DMA NVIC */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* SPI1 Tx DMA configure */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(usart->reg->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tx_buff_addr;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = translen;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel4, ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
}

int32_t USARTx_Initialize(USART_RESOURCES * usart)
{
	/* Enable peripheral clock */
	RCC_APB2PeriphClockCmd(usart->rccRoot, ENABLE);

	USART_PinConfig(usart);

	return ARM_DRIVER_OK;
}

int32_t USARTx_Configure(USART_RESOURCES * usart)
{
	USART_InitTypeDef  USART_InitStructure;

	switch ((uint32_t)usart->reg)
	{
		case USART1_BASE:
			USART_InitStructure.USART_BaudRate = 115200;
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
			USART_InitStructure.USART_StopBits = USART_StopBits_1;
			USART_InitStructure.USART_Parity = USART_Parity_No;
			USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
			USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			break;
		case USART3_BASE:
			USART_InitStructure.USART_BaudRate = 115200;
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
			USART_InitStructure.USART_StopBits = USART_StopBits_1;
			USART_InitStructure.USART_Parity = USART_Parity_No;
			USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
			USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			break;
		default:
			break;
	}

	USART_Init(usart->reg, &USART_InitStructure);
	USART_Cmd(usart->reg, ENABLE);

	return ARM_DRIVER_OK;
}

int32_t USARTx_TransferDMA(uint8_t * data_out, uint8_t * data_in, uint32_t translen, USART_RESOURCES * usart)
{
	USART1_DMAConfig(data_out, data_in, translen, usart);

	usart->reg->CR3 |= USART_CR3_DMAT;

	while (!usart1_trans_over);
	usart1_trans_over = 0;

	while(!(usart->reg->SR & USART_SR_TC));
	usart->reg->SR = ~(USART_SR_TC);
	usart->reg->CR3 &= ~USART_CR3_DMAT;
	
	return ARM_DRIVER_OK;
}

static int32_t USART1_Initialize (void) {
	return USARTx_Initialize(&USART1_Resources);
}

static int32_t USART1_Configure(void) {
	return USARTx_Configure(&USART1_Resources);
}

static int32_t USART1_Transfer(void * data_out, void * data_in, uint32_t translen) {
	return USARTx_TransferDMA(data_out, data_in, translen, &USART1_Resources);
}

USART_DRIVER Driver_USART1 = {
    USART1_Initialize,
	USART1_Configure,
    USART1_Transfer
};

static int32_t USART3_Initialize (void) {
	return USARTx_Initialize(&USART1_Resources);
}

static int32_t USART3_Configure(void) {
	return USARTx_Configure(&USART1_Resources);
}

static int32_t USART3_Transfer(void * data_out, void * data_in, uint32_t translen) {
	return USARTx_TransferDMA(data_out, data_in, translen, &USART1_Resources);
}

USART_DRIVER Driver_USART3 = {
    USART3_Initialize,
	USART3_Configure,
    USART3_Transfer
};
