#include "stm32f1xx_hal.h"
//#ifdef HAL_FATFS_ENABLED


#define TRUE  1
#define FALSE 0
#define bool BYTE


#include "diskio.h"
#include "fatfs_sd.h"


extern SPI_HandleTypeDef 	hspi1;
extern SPI_HandleTypeDef 	hspi2;

//#define HSPI_SDCARD		 	&hspi1
//#define	SD_CS_PORT			GPIOB
//#define SD_CS_PIN			GPIO_PIN_12
static GPIO_TypeDef * SD_CS_PORT[2] = { GPIOB,       GPIOB };
static uint32_t SD_CS_PIN[2]  = { GPIO_PIN_12, GPIO_PIN_2};
static SPI_HandleTypeDef * SD_HSPI[2] = { &hspi1, &hspi2};


volatile uint16_t Timer1, Timer2; /* 1ms Timer Counter */

static volatile DSTATUS SD_STAT[2] = { STA_NOINIT, STA_NOINIT} ;	/* Disk Status */
static uint8_t SD_TYPE[2];           /* Type 0:MMC, 1:SDC, 2:Block addressing */
static uint8_t SD_POWER[2] = {0, 0}; /* Power flag */

/***************************************
 * SPI functions
 **************************************/

/* slave select */
static void SELECT(int pdrv)
{
	HAL_GPIO_WritePin(SD_CS_PORT[pdrv], SD_CS_PIN[pdrv], GPIO_PIN_RESET);
	HAL_Delay(1);
}

/* slave deselect */
static void DESELECT(int pdrv)
{
	HAL_GPIO_WritePin(SD_CS_PORT[pdrv], SD_CS_PIN[pdrv], GPIO_PIN_SET);
	HAL_Delay(1);
}

/* SPI transmit a byte */
static void SPI_TxByte(int pdrv, uint8_t data)
{
	while(!__HAL_SPI_GET_FLAG(SD_HSPI[pdrv], SPI_FLAG_TXE));
	HAL_SPI_Transmit(SD_HSPI[pdrv], &data, 1, SPI_TIMEOUT);
}

/* SPI transmit buffer */
static void SPI_TxBuffer(int pdrv, uint8_t *buffer, uint16_t len)
{
	while(!__HAL_SPI_GET_FLAG(SD_HSPI[pdrv], SPI_FLAG_TXE));
	HAL_SPI_Transmit(SD_HSPI[pdrv], buffer, len, SPI_TIMEOUT);
}

/* SPI receive a byte */
static uint8_t SPI_RxByte(int pdrv)
{
	uint8_t dummy, data;
	dummy = 0xFF;

	while(!__HAL_SPI_GET_FLAG(SD_HSPI[pdrv], SPI_FLAG_TXE));
	HAL_SPI_TransmitReceive(SD_HSPI[pdrv], &dummy, &data, 1, SPI_TIMEOUT);

	return data;
}

/* SPI receive a byte via pointer */
static void SPI_RxBytePtr(int pdrv, uint8_t *buff)
{
	*buff = SPI_RxByte(pdrv);
}

/***************************************
 * SD functions
 **************************************/

/* wait SD ready */
static uint8_t SD_ReadyWait(int pdrv)
{
	uint8_t res;

	/* timeout 500ms */
	uint16_t Timer2 = 500;

	/* if SD goes ready, receives 0xFF */
	do {
		res = SPI_RxByte(pdrv);
	} while ((res != 0xFF) && Timer2);

	return res;
}

/* power on */
static void SD_PowerOn(int pdrv)
{
	uint8_t args[6];
	uint32_t cnt = 0x1FFF;

	/* transmit bytes to wake up */
	DESELECT(pdrv);
	for(int i = 0; i < 10; i++)
	{
		SPI_TxByte(pdrv, 0xFF);
	}

	/* slave select */
	SELECT(pdrv);

	/* make idle state */
	args[0] = CMD0;		/* CMD0:GO_IDLE_STATE */
	args[1] = 0;
	args[2] = 0;
	args[3] = 0;
	args[4] = 0;
	args[5] = 0x95;		/* CRC */

	SPI_TxBuffer(pdrv, args, sizeof(args));

	/* wait response */
	while ((SPI_RxByte(pdrv) != 0x01) && cnt)
	{
		cnt--;
	}

	DESELECT(pdrv);
	SPI_TxByte(pdrv, 0XFF);

	SD_POWER[pdrv] = 1;
}

/* power off */
static void SD_PowerOff(int pdrv)
{
	SD_POWER[pdrv] = 0;
}

/* check power flag */
static uint8_t SD_CheckPower(int pdrv)
{
	return SD_POWER[pdrv];
}

/* receive data block */
static bool SD_RxDataBlock(int pdrv, BYTE *buff, UINT len)
{
	uint8_t token;

	/* timeout 200ms */
	uint16_t Timer1 = 200;

	/* loop until receive a response or timeout */
	do {
		token = SPI_RxByte(pdrv);
	} while((token == 0xFF) && Timer1);

	/* invalid response */
	if(token != 0xFE) return FALSE;

	/* receive data */
	do {
		SPI_RxBytePtr(pdrv, buff++);
	} while(--len);

	/* discard CRC */
	SPI_RxByte(pdrv);
	SPI_RxByte(pdrv);

	return TRUE;
}

/* transmit data block */
#if _USE_WRITE == 1
static bool SD_TxDataBlock(int pdrv, const uint8_t *buff, BYTE token)
{
	uint8_t resp;
	uint8_t i = 0;

	/* wait SD ready */
	if (SD_ReadyWait(pdrv) != 0xFF) return FALSE;

	/* transmit token */
	SPI_TxByte(pdrv, token);

	/* if it's not STOP token, transmit data */
	if (token != 0xFD)
	{
		SPI_TxBuffer(pdrv, (uint8_t*)buff, 512);

		/* discard CRC */
		SPI_RxByte(pdrv);
		SPI_RxByte(pdrv);

		/* receive response */
		while (i <= 64)
		{
			resp = SPI_RxByte(pdrv);

			/* transmit 0x05 accepted */
			if ((resp & 0x1F) == 0x05) break;
			i++;
		}

		/* recv buffer clear */
		while (SPI_RxByte(pdrv) == 0);
	}

	/* transmit 0x05 accepted */
	if ((resp & 0x1F) == 0x05) return TRUE;

	return FALSE;
}
#endif /* _USE_WRITE */

/* transmit command */
static BYTE SD_SendCmd(int pdrv, BYTE cmd, uint32_t arg)
{
	uint8_t crc, res;

	/* wait SD ready */
	if (SD_ReadyWait(pdrv) != 0xFF) return 0xFF;

	/* transmit command */
	SPI_TxByte(pdrv, cmd); 					/* Command */
	SPI_TxByte(pdrv, (uint8_t)(arg >> 24)); 	/* Argument[31..24] */
	SPI_TxByte(pdrv, (uint8_t)(arg >> 16)); 	/* Argument[23..16] */
	SPI_TxByte(pdrv, (uint8_t)(arg >> 8)); 	/* Argument[15..8] */
	SPI_TxByte(pdrv, (uint8_t)arg); 			/* Argument[7..0] */

	/* prepare CRC */
	if(cmd == CMD0) crc = 0x95;	/* CRC for CMD0(0) */
	else if(cmd == CMD8) crc = 0x87;	/* CRC for CMD8(0x1AA) */
	else crc = 1;

	/* transmit CRC */
	SPI_TxByte(pdrv, crc);

	/* Skip a stuff byte when STOP_TRANSMISSION */
	if (cmd == CMD12) SPI_RxByte(pdrv);

	/* receive response */
	uint8_t n = 10;
	do {
		res = SPI_RxByte(pdrv);
	} while ((res & 0x80) && --n);

	return res;
}

/***************************************
 * user_diskio.c functions
 **************************************/

/* initialize SD */
DSTATUS SD_disk_initialize(BYTE pdrv)
{
	uint8_t n, type, ocr[4];

	/* single drive, drv should be 0 */
	if(pdrv > 1) return STA_NOINIT;

	/* no disk */
	if(SD_STAT[pdrv] & STA_NODISK) return SD_STAT[pdrv];

	/* power on */
	SD_PowerOn(pdrv);

	/* slave select */
	SELECT(pdrv);

	/* check disk type */
	type = 0;

	/* send GO_IDLE_STATE command */
	if (SD_SendCmd(pdrv, CMD0, 0) == 1)
	{
		/* timeout 1 sec */
		uint16_t Timer1 = 1000;

		/* SDC V2+ accept CMD8 command, http://elm-chan.org/docs/mmc/mmc_e.html */
		if (SD_SendCmd(pdrv, CMD8, 0x1AA) == 1)
		{
			/* operation condition register */
			for (n = 0; n < 4; n++)
			{
				ocr[n] = SPI_RxByte(pdrv);
			}

			/* voltage range 2.7-3.6V */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)
			{
				/* ACMD41 with HCS bit */
				do {
					if (SD_SendCmd(pdrv, CMD55, 0) <= 1 && SD_SendCmd(pdrv, CMD41, 1UL << 30) == 0) break;
				} while (Timer1);

				/* READ_OCR */
				if (Timer1 && SD_SendCmd(pdrv, CMD58, 0) == 0)
				{
					/* Check CCS bit */
					for (n = 0; n < 4; n++)
					{
						ocr[n] = SPI_RxByte(pdrv);
					}

					/* SDv2 (HC or SC) */
					type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		}
		else
		{
			/* SDC V1 or MMC */
			type = (SD_SendCmd(pdrv, CMD55, 0) <= 1 && SD_SendCmd(pdrv, CMD41, 0) <= 1) ? CT_SD1 : CT_MMC;

			do
			{
				if (type == CT_SD1)
				{
					if (SD_SendCmd(pdrv, CMD55, 0) <= 1 && SD_SendCmd(pdrv, CMD41, 0) == 0) break; /* ACMD41 */
				}
				else
				{
					if (SD_SendCmd(pdrv, CMD1, 0) == 0) break; /* CMD1 */
				}

			} while (Timer1);

			/* SET_BLOCKLEN */
			if (!Timer1 || SD_SendCmd(pdrv, CMD16, 512) != 0) type = 0;
		}
	}

	SD_TYPE[pdrv] = type;

	/* Idle */
	DESELECT(pdrv);
	SPI_RxByte(pdrv);

	/* Clear STA_NOINIT */
	if (type)
	{
		SD_STAT[pdrv] &= ~STA_NOINIT;
	}
	else
	{
		/* Initialization failed */
		SD_PowerOff(pdrv);
	}

	return SD_STAT[pdrv];
}

/* return disk status */
DSTATUS SD_disk_status(BYTE pdrv)
{
	if (pdrv > 1) return STA_NOINIT;
	return SD_STAT[pdrv];
}

/* read sector */
DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
	/* pdrv should be 0 */
	if (pdrv > 1 || !count) return RES_PARERR;

	/* no disk */
	if (SD_STAT[pdrv] & STA_NOINIT) return RES_NOTRDY;

	/* convert to byte address */
	if (!(SD_TYPE[pdrv] & CT_SD2)) sector *= 512;

	SELECT(pdrv);

	if (count == 1)
	{
		/* READ_SINGLE_BLOCK */
		if ((SD_SendCmd(pdrv, CMD17, sector) == 0) && SD_RxDataBlock(pdrv, buff, 512)) count = 0;
	}
	else
	{
		/* READ_MULTIPLE_BLOCK */
		if (SD_SendCmd(pdrv, CMD18, sector) == 0)
		{
			do {
				if (!SD_RxDataBlock(pdrv, buff, 512)) break;
				buff += 512;
			} while (--count);

			/* STOP_TRANSMISSION */
			SD_SendCmd(pdrv, CMD12, 0);
		}
	}

	/* Idle */
	DESELECT(pdrv);
	SPI_RxByte(pdrv);

	return count ? RES_ERROR : RES_OK;
}

/* write sector */
#if _USE_WRITE == 1
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
	/* pdrv should be 0 */
	if (pdrv > 1 || !count) return RES_PARERR;

	/* no disk */
	if (SD_STAT[pdrv] & STA_NOINIT) return RES_NOTRDY;

	/* write protection */
	if (SD_STAT[pdrv] & STA_PROTECT) return RES_WRPRT;

	/* convert to byte address */
	if (!(SD_TYPE[pdrv] & CT_SD2)) sector *= 512;

	SELECT(pdrv);

	if (count == 1)
	{
		/* WRITE_BLOCK */
		if ((SD_SendCmd(pdrv, CMD24, sector) == 0) && SD_TxDataBlock(pdrv, buff, 0xFE))
			count = 0;
	}
	else
	{
		/* WRITE_MULTIPLE_BLOCK */
		if (SD_TYPE[pdrv] & CT_SD1)
		{
			SD_SendCmd(pdrv, CMD55, 0);
			SD_SendCmd(pdrv, CMD23, count); /* ACMD23 */
		}

		if (SD_SendCmd(pdrv, CMD25, sector) == 0)
		{
			do {
				if(!SD_TxDataBlock(pdrv, buff, 0xFC)) break;
				buff += 512;
			} while (--count);

			/* STOP_TRAN token */
			if(!SD_TxDataBlock(pdrv, 0, 0xFD))
			{
				count = 1;
			}
		}
	}

	/* Idle */
	DESELECT(pdrv);
	SPI_RxByte(pdrv);

	return count ? RES_ERROR : RES_OK;
}
#endif /* _USE_WRITE */

/* ioctl */
DRESULT SD_disk_ioctl(BYTE pdrv, BYTE ctrl, void *buff)
{
	DRESULT res;
	uint8_t n, csd[16], *ptr = buff;
	WORD csize;

	/* pdrv should be 0 */
	if (pdrv > 1) return RES_PARERR;
	res = RES_ERROR;

	if (ctrl == CTRL_POWER)
	{
		switch (*ptr)
		{
		case 0:
			SD_PowerOff(pdrv);		/* Power Off */
			res = RES_OK;
			break;
		case 1:
			SD_PowerOn(pdrv);		/* Power On */
			res = RES_OK;
			break;
		case 2:
			*(ptr + 1) = SD_CheckPower(pdrv);
			res = RES_OK;		/* Power Check */
			break;
		default:
			res = RES_PARERR;
		}
	}
	else
	{
		/* no disk */
		if (SD_STAT[pdrv] & STA_NOINIT) return RES_NOTRDY;

		SELECT(pdrv);

		switch (ctrl)
		{
		case GET_SECTOR_COUNT:
			/* SEND_CSD */
			if ((SD_SendCmd(pdrv, CMD9, 0) == 0) && SD_RxDataBlock(pdrv, csd, 16))
			{
				if ((csd[0] >> 6) == 1)
				{
					/* SDC V2 */
					csize = csd[9] + ((WORD) csd[8] << 8) + 1;
					*(DWORD*) buff = (DWORD) csize << 10;
				}
				else
				{
					/* MMC or SDC V1 */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10) + 1;
					*(DWORD*) buff = (DWORD) csize << (n - 9);
				}
				res = RES_OK;
			}
			break;
		case GET_SECTOR_SIZE:
			*(WORD*) buff = 512;
			res = RES_OK;
			break;
		case CTRL_SYNC:
			if (SD_ReadyWait(pdrv) == 0xFF) res = RES_OK;
			break;
		case MMC_GET_CSD:
			/* SEND_CSD */
			if (SD_SendCmd(pdrv, CMD9, 0) == 0 && SD_RxDataBlock(pdrv, ptr, 16)) res = RES_OK;
			break;
		case MMC_GET_CID:
			/* SEND_CID */
			if (SD_SendCmd(pdrv, CMD10, 0) == 0 && SD_RxDataBlock(pdrv, ptr, 16)) res = RES_OK;
			break;
		case MMC_GET_OCR:
			/* READ_OCR */
			if (SD_SendCmd(pdrv, CMD58, 0) == 0)
			{
				for (n = 0; n < 4; n++)
				{
					*ptr++ = SPI_RxByte(pdrv);
				}
				res = RES_OK;
			}
		default:
			res = RES_PARERR;
		}

		DESELECT(pdrv);
		SPI_RxByte(pdrv);
	}

	return res;
}
//#endif
