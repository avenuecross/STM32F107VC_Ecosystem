/* W25X16 Serial Flash Application Driver
 * Auther: Abner
 * Date: 2018/8/31
 */

#include "serialflash.h"
#include "SPI_Driver.h"
#include "Driver_Common.h"

/* CS control of FLASH */
#define FLASH_CS_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define FLASH_CS_LOW()   GPIO_ResetBits(GPIOB, GPIO_Pin_9)

/* W25X16 command */
#define FLASH_WR_ENABLE_CMD       0x06
#define FLASH_WR_DISABLE_CMD      0x04
#define FLASH_RD_REGISTER_CMD     0x05
#define FLASH_WR_REGISTER_CMD     0x01
#define FLASH_RD_DATA_CMD         0x03
#define FLASH_FAST_RD_CMD         0x0B
#define FLASH_FAST_RD_DUAL_CMD    0x3B
#define FLASH_PAGE_PROGRAM_CMD    0x02
#define FLASH_ERASE_BLOCK_CMD     0xD8
#define FLASH_ERASE_SECTOR_CMD    0x20
#define FLASH_ERASE_CHIP_CMD      0xC7
#define FLASH_POWER_DOWN_CMD      0xB9
#define FLASH_DEVICEID_CMD        0xAB
#define FLASH_DEVICEID_CMD        0xAB
#define FLASH_MANUID_CMD          0x90
#define FLASH_JEDECID_CMD         0x9F

/* FLASH Status Register */
#define FLASH_STATUS_BUSY         0x01    // Read only bit which is set when executing write operation
#define FLASH_STATUS_WEL          0x02    // 0 : disable 1 : enable , it will be set 0 after power on or write operation
#define FLASH_STATUS_BP0          0x04
#define FLASH_STATUS_BP1          0x08
#define FLASH_STATUS_BP2          0x10
#define FLASH_STATUS_TB           0x20
#define FLASH_STATUS_RESERVE      0x40    // Use for future
#define FLASH_STATUS_SRP          0x80


#define SPI_DUMMY_DATA            0x00
#define FLASH_PAGE_SIZE           256     // Bytes
#define FLASH_PAGE_NUM            8192

extern SPI_DRIVER  Driver_SPI1;
#define ptrSPI     (&Driver_SPI1)

int32_t SPI_TransferMultiByte(uint8_t * tx_buff, uint8_t * rx_buff, uint32_t translen)
{
	return ptrSPI->Transfer(tx_buff, rx_buff, translen);
}

uint8_t SPI_TransferSingleByte(uint8_t tx_data)
{
	uint8_t rx_data = 0;
	ptrSPI->Transfer(&tx_data, &rx_data, 1);
	return rx_data;
}

uint8_t FLASH_ReadDeviceID(void)
{
	uint8_t device_id = 0;
	uint8_t tx_buff[5] = {FLASH_DEVICEID_CMD, SPI_DUMMY_DATA, SPI_DUMMY_DATA, SPI_DUMMY_DATA, SPI_DUMMY_DATA};
	uint8_t rx_buff[5] = {0};

	/* Enable spi tranfer */
	FLASH_CS_LOW();

	/* Tranfer command and get info */
	SPI_TransferMultiByte(tx_buff, rx_buff, 5);
	device_id = rx_buff[4];

	/* Disable spi tranfer */
	FLASH_CS_HIGH();

	return device_id;
}

uint32_t FLASH_ReadManufacturerID(void)
{
	uint32_t manufacturer = 0, mem_type = 0, capacity = 0;
	uint8_t tx_buff[4] = {FLASH_JEDECID_CMD, SPI_DUMMY_DATA, SPI_DUMMY_DATA, SPI_DUMMY_DATA};
	uint8_t rx_buff[4] = {0};

	/* Enable spi tranfer */
	FLASH_CS_LOW();

	/* Tranfer command and get info */
	SPI_TransferMultiByte(tx_buff, rx_buff, 4);
	manufacturer = rx_buff[1];
	mem_type = rx_buff[2];
	capacity = rx_buff[3];
	
	/* Disable spi tranfer */
	FLASH_CS_HIGH();

	return (manufacturer << 16) | (mem_type << 8) | capacity;
}

/* Everytime try to write, enable flash write status first */
void FLASH_WriteEnable(void)
{
	FLASH_CS_LOW();
	SPI_TransferSingleByte(FLASH_WR_ENABLE_CMD);
	FLASH_CS_HIGH();
}

/* Check busy flag of status register to make sure transfering is over */
void FLASH_WaitWriteOver(void)
{
	uint8_t status = 0;
	
	FLASH_CS_LOW();
	
	SPI_TransferSingleByte(FLASH_RD_REGISTER_CMD);
	do
	{
		status = SPI_TransferSingleByte(SPI_DUMMY_DATA);
	} while((status & FLASH_STATUS_BUSY) == SET);
	
	FLASH_CS_HIGH();
}

void FLASH_SectorErase(uint32_t flash_addr)
{
	FLASH_WriteEnable();
	FLASH_WaitWriteOver();
	
	/* Enable spi tranfer */
	FLASH_CS_LOW();

	/* Tranfer command first */
	SPI_TransferSingleByte(FLASH_ERASE_SECTOR_CMD);
	SPI_TransferSingleByte((flash_addr & 0xFF0000) >> 16);
	SPI_TransferSingleByte((flash_addr & 0xFF00) >> 8);
	SPI_TransferSingleByte(flash_addr & 0xFF);
	
	/* Disable spi tranfer */
	FLASH_CS_HIGH();
	
	FLASH_WaitWriteOver();
}

void FLASH_WritePage(char * tx_buff, uint32_t flash_addr, uint32_t data_length)
{
	FLASH_WriteEnable();

	/* Enable spi tranfer */
	FLASH_CS_LOW();

	/* Tranfer command first */
	SPI_TransferSingleByte(FLASH_PAGE_PROGRAM_CMD);
	SPI_TransferSingleByte((flash_addr & 0xFF0000) >> 16);
	SPI_TransferSingleByte((flash_addr & 0xFF00) >> 8);
	SPI_TransferSingleByte(flash_addr & 0xFF);

	if (data_length > FLASH_PAGE_SIZE)
	{
		data_length = FLASH_PAGE_SIZE;
	}

	SPI_TransferMultiByte((uint8_t *)tx_buff, NULL, data_length);
	
	/* Disable spi tranfer */
	FLASH_CS_HIGH();
	
	FLASH_WaitWriteOver();
}

void FLASH_Read(char * rx_buff, uint32_t flash_addr, uint32_t data_length)
{
	/* Enable spi tranfer */
	FLASH_CS_LOW();

	/* Tranfer command first */
	SPI_TransferSingleByte(FLASH_RD_DATA_CMD);
	SPI_TransferSingleByte((flash_addr & 0xFF0000) >> 16);
	SPI_TransferSingleByte((flash_addr & 0xFF00) >> 8);
	SPI_TransferSingleByte(flash_addr & 0xFF);

	SPI_TransferMultiByte(NULL, (uint8_t *)rx_buff, data_length);

	/* Disable spi tranfer */
	FLASH_CS_HIGH();
}

void FLASH_EraseChip(void)
{
	/* Enable spi tranfer */
	FLASH_CS_LOW();

	/* Tranfer erase command */
	SPI_TransferSingleByte(FLASH_ERASE_CHIP_CMD);

	/* Disable spi tranfer */
	FLASH_CS_HIGH();
}
