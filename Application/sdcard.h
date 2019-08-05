#ifndef _SDCARD_H
#define _SDCARD_H

#include "diskio.h"

typedef enum
{
	SD_INSERT = 0,
	SD_NOT_INSERT
} SD_InsertState;

DSTATUS SD_initialize(BYTE pdrv);
DSTATUS SD_status(BYTE pdrv);
DRESULT SD_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
DRESULT SD_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
DRESULT SD_ioctl(BYTE pdrv, BYTE cmd, void *buff);

#endif
