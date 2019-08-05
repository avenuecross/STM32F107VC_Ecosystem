#include "stm32f10x.h"

void EEPROM_Write(char * buff, uint8_t addr, uint32_t data_byte_num);
void EEPROM_Read(char * buff, uint8_t addr, uint32_t data_byte_num);
