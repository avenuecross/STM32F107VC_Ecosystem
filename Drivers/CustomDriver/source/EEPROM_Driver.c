#include "EEPROM_Driver.h"

#define EEPROM_ADDR 0xA0
#define EEPROM_PAGE_SIZE 128

void I2C1_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
//	I2C_InitTypeDef I2C_InitStructure;
	
	/* Enable peripheral clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	/* Config Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_AF_OD;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);

//	I2C_InitStructure.I2C_ClockSpeed = 100000;
//	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
//	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
//	I2C_InitStructure.I2C_OwnAddress1 = 0x0A;
//	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
//	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

//	I2C_DeInit(I2C1);
//	I2C_Init(I2C1, &I2C_InitStructure);
//	I2C_Cmd(I2C1, ENABLE);
}

void EEPROM_Page_Write(char * buff, uint8_t addr, uint32_t data_byte_num)
{
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C1, ENABLE);
	
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Transmitter);
	
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
	I2C_SendData(I2C1, addr);
	
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	while(data_byte_num--)
	{
		I2C_SendData(I2C1, addr);
	
		buff++;
		
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
	I2C_GenerateSTOP(I2C1, ENABLE);
}

void I2C_WaitStandbyState(void)
{
	do
	{
		I2C_GenerateSTART(I2C1, ENABLE);
		I2C_ReadRegister(I2C1, I2C_Register_SR1);
		I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Transmitter);
	} while (!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002));

	I2C_ClearFlag(I2C1, I2C_FLAG_AF);
	I2C_GenerateSTOP(I2C1, ENABLE);
}

void EEPROM_Write(char * buff, uint8_t addr, uint32_t data_byte_num)
{
	uint8_t align_status, page_num, over_page_data;
	
	align_status = addr % EEPROM_PAGE_SIZE;
	page_num = data_byte_num / EEPROM_PAGE_SIZE;
	over_page_data = data_byte_num % EEPROM_PAGE_SIZE;

	/* Addr is not page aligned */
	if (align_status)
	{
		
	}
	/* Addr is page aligned */
	else
	{
		if (page_num == 0)
		{
			EEPROM_Page_Write(buff, addr, over_page_data);
			I2C_WaitStandbyState();
		}
		else
		{
			while (page_num--)
			{
				EEPROM_Page_Write(buff, addr, EEPROM_PAGE_SIZE);
				I2C_WaitStandbyState();
				buff += EEPROM_PAGE_SIZE;
				addr += EEPROM_PAGE_SIZE;
			}
			if (over_page_data != 0)
			{
				EEPROM_Page_Write(buff, addr, over_page_data);
				I2C_WaitStandbyState();
			}
		}
	}
}

void EEPROM_Read(char * buff, uint8_t addr, uint32_t data_byte_num)
{
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
	I2C_Cmd(I2C1, ENABLE);
	
	I2C_SendData(I2C1, addr);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Receiver);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	
	while(data_byte_num--)
	{
		if (data_byte_num == 1)
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE);
			I2C_GenerateSTOP(I2C1, ENABLE);
		}
		
		if (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		{
			*buff = I2C_ReceiveData(I2C1);
			buff++;
			data_byte_num--;
		}
	}
	I2C_AcknowledgeConfig(I2C1, ENABLE);
}
