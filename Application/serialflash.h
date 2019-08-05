#ifndef __FLASH_DRIVER_H
#define __FLASH_DRIVER_H

#include "stm32f10x.h"

uint8_t  FLASH_ReadDeviceID(void);
uint32_t FLASH_ReadManufacturerID(void);
void     FLASH_WriteEnable(void);
void     FLASH_WaitWriteOver(void);
void     FLASH_SectorErase(uint32_t flash_addr);
void     FLASH_WritePage(char * tx_buff, uint32_t flash_addr, uint32_t data_length);
void     FLASH_Read(char * rx_buff, uint32_t flash_addr, uint32_t data_length);
void     FLASH_EraseChip(void);


#endif
