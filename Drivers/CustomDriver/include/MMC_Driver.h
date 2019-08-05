
#ifndef SD_SPI_H
#define SD_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "ff.h"
#include "ff_gen_drv.h"

extern uint8_t retUSER; /* Return value for USER */
extern char USER_Path[4]; /* USER logical drive path */

void MX_FATFS_Init(void);

#ifdef __cplusplus
}
#endif

#endif
