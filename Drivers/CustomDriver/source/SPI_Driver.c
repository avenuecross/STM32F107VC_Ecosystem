#include "SPI_Driver.h"
#include "Driver_Common.h"


#define SPI1_DR 0x4001300C

#define SPI_DMA_SENDRECEIVE  0
#define SPI_DMA_ONLYSEND     1
#define SPI_DMA_ONLYRECEIVE  2

uint8_t spi_trans_over = 0;

static SPI_RESOURCES SPI1_Resources = {
	SPI1,
	SPI1_IRQn,
	//SPI1_IRQ_PRIORITY,
	RCC_APB2Periph_SPI1,
//	&SPI1_Ctrl,
//	&SPI1_Xfer_Info,
};

void DMA1_Channel2_IRQHandler(void)
{
	if (DMA_GetFlagStatus(DMA1_FLAG_TC2) == SET)
	{
		spi_trans_over = 1;
		DMA_ClearFlag(DMA1_FLAG_TC2);
	}
}

static void SPI_PinConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Config SCLK MOSI MISO pin of SPI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Config CS of Flash */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_9);

	/* Config CS of SD card */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

static void SPI_DMAConfig(void * tx_buff_addr, void * rx_buff_addr, uint32_t translen, uint32_t mode)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_Cmd(DMA1_Channel3, DISABLE);
	DMA_Cmd(DMA1_Channel2, DISABLE);

	/* DMA NVIC */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* SPI1 Tx DMA configure */
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1_DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tx_buff_addr;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = translen;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	if (mode == SPI_DMA_ONLYRECEIVE)
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	else
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel3, ENABLE);
	
	/* SPI1 Rx DMA configure */
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1_DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rx_buff_addr;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = translen;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	if (mode == SPI_DMA_ONLYSEND)
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	else
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel2, ENABLE);
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
}

int32_t SPIx_Initialize(SPI_RESOURCES * spi)
{
	/* Enable peripheral clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(spi->rccRoot, ENABLE);

	SPI_PinConfig();

	return ARM_DRIVER_OK;
}

int32_t SPIx_Configure(SPI_RESOURCES * spi)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* Configure SPI attribute */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(spi->reg, ENABLE);

	return ARM_DRIVER_OK;
}

int32_t SPIx_TransferDMA(void * data_out, void * data_in, uint32_t translen, SPI_RESOURCES * spi)
{
	uint32_t dma_mode = SPI_DMA_SENDRECEIVE;
	uint8_t  dummy_data = 0xFF;
	
	if (((data_out == NULL) && (data_in == NULL)) || (translen == 0))  { return ARM_DRIVER_ERROR_PARAMETER; }
	
	if (data_out == NULL)
	{
		data_out = &dummy_data;
		dma_mode = SPI_DMA_ONLYRECEIVE;
	}
	if (data_in == NULL)
	{
		data_in = &dummy_data;
		dma_mode = SPI_DMA_ONLYSEND;
	}

	SPI_DMAConfig(data_out, data_in, translen, dma_mode);

	/* Enable DMA transfer when Rx buff is empty or Rx buff is full */
	spi->reg->CR2 |= SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx;

	while(!spi_trans_over);
	spi_trans_over = 0;
	
	while((spi->reg->SR & SPI_SR_BSY) | (!(spi->reg->SR & SPI_SR_TXE)));
	
	return ARM_DRIVER_OK;
}

static int32_t SPI1_Initialize (void) {
	return SPIx_Initialize(&SPI1_Resources);
}

static int32_t SPI1_Configure(void) {
	return SPIx_Configure(&SPI1_Resources);
}

static int32_t SPI1_Transfer(void * data_out, void *data_in, uint32_t translen) {
	return SPIx_TransferDMA(data_out, data_in, translen, &SPI1_Resources);
}

SPI_DRIVER Driver_SPI1 = {
    SPI1_Initialize,
	SPI1_Configure,
    SPI1_Transfer
};
