#ifndef __SPI_DRIVER_H
#define __SPI_DRIVER_H

#include "stm32f10x.h"

typedef struct {
	SPI_TypeDef      	   *reg;            	// SPI peripheral register interface
	IRQn_Type            	irq_num;			// SPI IRQ number
	//uint32_t				spi_priority;		// SPI interrupt priority
	uint32_t 			 	rccRoot;
	//SPI_CTRL			   *ctrl;				// SPI control information
	//SPI_TRANSFER_INFO	   *xfer;				// SPI transfer information
} const SPI_RESOURCES;

typedef struct SPI_DRIVER_STRUCT
{
	int32_t    (*Initialize) (void);
	int32_t    (*Configure)  (void);
	int32_t    (*Transfer)   (void * data_out, void *data_in, uint32_t translen);
} SPI_DRIVER;

#endif
