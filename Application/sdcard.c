/* SD card basic interface with SPI
 * Auther: Abner
 * Date: 2018/9/17
 */

#include "SdCard.h"
#include "stm32f10x.h"
#include "diskio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SPI_Driver.h"


#define SD_CS_HIGH()  GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define SD_CS_LOW()   GPIO_ResetBits(GPIOA, GPIO_Pin_4)

#define SD_OK               0
#define SD_TIMEOUT          1
#define SPI_DUMMY_DATA      0xFF

/* MMC/SD command */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		     /* MMC ver 3 */
#define CT_SD1		0x02		     /* SD ver 1 */
#define CT_SD2		0x04		     /* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	 /* SD */
#define CT_BLOCK	0x08		     /* Block addressing */


extern SPI_DRIVER  Driver_SPI1;
#define ptrSPI     (&Driver_SPI1)

extern int32_t SPI_TransferMultiByte(uint8_t * tx_buff, uint8_t * rx_buff, uint32_t translen);
extern uint8_t SPI_TransferSingleByte(uint8_t tx_data);

extern volatile unsigned int Time_SD;
extern volatile unsigned int Time_SD_Init;

static BYTE CardType;			/* Card type flags */

extern void Debug_Message(char * str, ...);

/**
  * @brief  Gets Time from RTC 
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
    return 0;
}

//==================================================

/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* Wait for card goes ready or timeout */
/* 0:Ready, 1:Timeout */
static int wait_ready(UINT time_out)
{
	BYTE  data;

	Time_SD = time_out;
	do {
		data = SPI_TransferSingleByte(SPI_DUMMY_DATA);
		vTaskDelay(pdMS_TO_TICKS(1));
	} while (data != 0xFF && Time_SD);

	return (data == 0xFF) ? SD_OK : SD_TIMEOUT;
}

/* 1:OK, 0:Error */
static int rcvr_datablock (BYTE *buff, UINT btr/* Data block length (byte) */ )
{
	BYTE token;

	Time_SD = 200;
	do {							/* Wait for DataStart token in timeout of 200ms */
		token = SPI_TransferSingleByte(0xFF);
		/* This loop will take a time. Insert rot_rdq() here for multitask envilonment. */
	} while ((token == 0xFF) && Time_SD);
	if(token != 0xFE) return 0;		/* Function fails if invalid DataStart token or timeout */

	SPI_TransferMultiByte(NULL, buff, btr);		/* Store trailing data to the buffer */
	SPI_TransferSingleByte(0xFF); SPI_TransferSingleByte(0xFF);			/* Discard CRC */

	return 1;						/* Function succeeded */
}

//#if _USE_WRITE
static int xmit_datablock (	/* 1:OK, 0:Failed */
	const BYTE *buff,	/* Ponter to 512 byte data to be sent */
	BYTE token			/* Token */)
{
	BYTE resp;

	if (wait_ready(500)) return 0;		/* Wait for card ready */

	SPI_TransferSingleByte(token);					/* Send token */
	if (token != 0xFD) {				/* Send data if token is other than StopTran */
		SPI_TransferMultiByte((uint8_t *)buff, NULL, 512);		/* Data */
		SPI_TransferSingleByte(0xFF); SPI_TransferSingleByte(0xFF);	/* Dummy CRC */

		resp = SPI_TransferSingleByte(0xFF);				/* Receive data resp */
		if ((resp & 0x1F) != 0x05) return 0;	/* Function fails if the data packet was not accepted */
	}
	return 1;
}
//#endif

void SD_ChipDeselect(void)
{
	SD_CS_HIGH();
}

int SD_ChipSelect(void)
{
	SD_CS_LOW();
	if (wait_ready(500) == SD_OK) return SD_OK;	/* Wait for card ready */

	SD_ChipDeselect();
	return SD_TIMEOUT;
}

/* Return value: R1 resp (bit7==1:Failed to send) */
static BYTE SD_SendCmd(BYTE cmd, DWORD arg)
{
	BYTE n, res;
	uint8_t rx_buff[10] = {0};

	if (cmd & 0x80)
	{
		/* Send a CMD55 prior to ACMD<n> */
		cmd &= 0x7F;
		res = SD_SendCmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Send command packet */
	SPI_TransferSingleByte(0x40 | cmd);				/* Start + command index */
	SPI_TransferSingleByte((BYTE)(arg >> 24));		/* Argument[31..24] */
	SPI_TransferSingleByte((BYTE)(arg >> 16));		/* Argument[23..16] */
	SPI_TransferSingleByte((BYTE)(arg >> 8));		/* Argument[15..8] */
	SPI_TransferSingleByte((BYTE)arg);				/* Argument[7..0] */

	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	SPI_TransferSingleByte(n);

	/* Receive command resp */
	if (cmd == CMD12) SPI_TransferSingleByte(0xFF);/* Discard following one byte when CMD12 */

	/* Wait for response (10 bytes max) */
//	n = 10;
//	do {
//		res = SPI_TransferSingleByte(0xFF);
//	} while ((res & 0x80) && --n);
	SPI_TransferMultiByte(NULL, rx_buff, 10);
	res = 0xFF;
	for (n = 0; n < 10; n++)
	{
		if (!(rx_buff[n] & 0x80))
		{
			res = rx_buff[n];
			break;
		}
	}

	return res;
}

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_initialize(BYTE pdrv)
{
	int i;
	BYTE cmd, sd_type = 0, ocr[4];

	if (pdrv) return STA_NOINIT;			/* Supports only drive 0 */
	if (Stat & STA_NODISK) return Stat;     /* Is card existing in the soket? */

	Stat = STA_NOINIT;

	/* Following is the init step of SD card */

	/* Wait at least 1ms after power up */
	vTaskDelay(pdMS_TO_TICKS(2));

	//FCLK_SLOW();

	for(i = 0; i < 10; i++) { SPI_TransferSingleByte(SPI_DUMMY_DATA); }	/* Send >= 74 dummy clocks */

	/* Select the card and wait for ready */
	if (SD_ChipSelect() != SD_OK) return RES_NOTRDY;

	/* Put the card SPI/Idle state */
	if (SD_SendCmd(CMD0, 0) == 0x01)
	{
		/* Initialization timeout = 1 sec */
		Time_SD_Init = 1500;//1000;
		Debug_Message("Send CMD8\n");
		if (SD_SendCmd(CMD8, 0x1AA) == 0x01)
		{
			Debug_Message("CMD8 OK\n");
			/* SD v2.0+ card */
			for (i = 0; i < 4; i++)  ocr[i] = SPI_TransferSingleByte(SPI_DUMMY_DATA);	/* Get 32 bit return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)
			{
				/* Is the card supports vcc of 2.7-3.6V? */
				while (Time_SD_Init && SD_SendCmd(ACMD41, 1UL << 30));	/* Wait for end of initialization with ACMD41(HCS) */
				if (Time_SD_Init && SD_SendCmd(CMD58, 0) == 0)
				{
					/* Check CCS bit in the OCR */
					for (i = 0; i < 4; i++)  ocr[i] = SPI_TransferSingleByte(SPI_DUMMY_DATA);
					/* Card id SDv2 */
					sd_type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		}
		else
		{
			Debug_Message("CMD8 fail\n");
			/* Not SD v2.0+ card */
			if (SD_SendCmd(ACMD41, 0) <= 1)
			{
				/* SDv1 or MMC? */
				sd_type = CT_SD1; cmd = ACMD41;	/* SDv1 (ACMD41(0)) */
			}
			else
			{
				Debug_Message("MMCv3\n");
				sd_type = CT_MMC; cmd = CMD1;	/* MMCv3 (CMD1(0)) */
			}
			while (Time_SD_Init && SD_SendCmd(cmd, 0));		/* Wait for end of initialization */
			if (!Time_SD_Init || SD_SendCmd(CMD16, 512) != 0)	/* Set block length: 512 */
				sd_type = 0;
		}
	}
	Debug_Message("sd_type: %d\n", sd_type);
	CardType = sd_type;
	SD_ChipDeselect();

	if (sd_type)
	{
		//FCLK_FAST();			/* Set fast clock */
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT flag */
	} else {
		Stat = STA_NOINIT;
	}

	return Stat;
}

/**
  * @brief  Gets Disk Status 
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_status(BYTE pdrv)
{
	if (pdrv) return STA_NOINIT;
    return Stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT SD_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
	if (pdrv || !count) return RES_PARERR;		/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA ot BA conversion (byte addressing cards) */

	if (count == 1) {	/* Single sector read */
		if ((SD_SendCmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512)) {
			count = 0;
		}
	}
	else {				/* Multiple sector read */
		if (SD_SendCmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			SD_SendCmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	SD_ChipDeselect();

	return count ? RES_ERROR : RES_OK;	/* Return result */
}

/**
  * @brief  Writes Sector(s)  
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
//#if _USE_WRITE == 1
DRESULT SD_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
	if (pdrv || !count) return RES_PARERR;		/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check drive status */
	if (Stat & STA_PROTECT) return RES_WRPRT;	/* Check write protect */

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA ==> BA conversion (byte addressing cards) */

	if (count == 1) {	/* Single sector write */
		if ((SD_SendCmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE)) {
			count = 0;
		}
	}
	else {				/* Multiple sector write */
		if (CardType & CT_SDC) SD_SendCmd(ACMD23, count);	/* Predefine number of sectors */
		if (SD_SendCmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD)) count = 1;	/* STOP_TRAN token */
		}
	}
	SD_ChipDeselect();

	return count ? RES_ERROR : RES_OK;	/* Return result */
}
//#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation  
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
//#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	DRESULT res;
	BYTE n, csd[16];
	DWORD *dp, st, ed, csize;


	if (pdrv) return RES_PARERR;					/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		if (SD_ChipSelect() == SD_OK) res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
		if ((SD_SendCmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(DWORD*)buff = csize << 10;
			} else {					/* SDC ver 1.XX or MMC ver 3 */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		if (CardType & CT_SD2) {	/* SDC ver 2.00 */
			if (SD_SendCmd(ACMD13, 0) == 0) {	/* Read SD status */
				SPI_TransferSingleByte(0xFF);
				if (rcvr_datablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) SPI_TransferSingleByte(0xFF);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDC ver 1.XX or MMC */
			if ((SD_SendCmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
				if (CardType & CT_SD1) {	/* SDC ver 1.XX */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMC */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case CTRL_TRIM :	/* Erase a block of sectors (used when _USE_ERASE == 1) */
		if (!(CardType & CT_SDC)) break;				/* Check if the card is SDC */
		if (disk_ioctl(pdrv, MMC_GET_CSD, csd)) break;	/* Get CSD */
		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card */
		dp = buff; st = dp[0]; ed = dp[1];				/* Load sector block */
		if (!(CardType & CT_BLOCK))
		{
			st *= 512; ed *= 512;
		}
		if (SD_SendCmd(CMD32, st) == 0 && SD_SendCmd(CMD33, ed) == 0 && SD_SendCmd(CMD38, 0) == 0 && (!wait_ready(30000))) {	/* Erase sector block */
			res = RES_OK;	/* FatFs does not check result of this command */
		}
		break;

	default:
		res = RES_PARERR;
	}

	SD_ChipDeselect();

	return res;
}
//#endif /* _USE_IOCTL == 1 */

SD_InsertState CheckSdCardInsert(void)
{
	uint8_t insert_flag;
	insert_flag = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
	return (insert_flag ? SD_NOT_INSERT : SD_INSERT);
}
