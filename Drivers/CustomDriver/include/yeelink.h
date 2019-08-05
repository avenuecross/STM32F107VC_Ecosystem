#ifndef __YEELINK_H
#define __YEELINK_H 

#include "stm32f1xx_hal.h"


void adddata(void);
void putATinstruction(void);
void putDHT11_T(void);
void transmit2yeelink(void);
void putDHT11_T(void);
void change(unsigned char, unsigned char *);
void getSTR(void);


#endif


