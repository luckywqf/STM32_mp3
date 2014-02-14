/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "sdcard.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/*-----------------------------------------------------------------------*/

#define SECTOR_SIZE		512U

#define ATA		0
#define MMC		1
#define USB		2



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	SD_CardInfo SDCardInfo;
	SD_Error Status = SD_OK;
	
	Status = SD_Init();

  //Read CSD/CID MSD registers
	if (Status == SD_OK) {
    Status = SD_GetCardInfo(&SDCardInfo);
  }

  //Select Card
	if (Status == SD_OK) {
    Status = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
  }

//	if (Status == SD_OK) {
//    Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
//  }

	//Set Device Transfer Mode to INTERRUPT 
  if (Status == SD_OK) {  
		Status = SD_SetDeviceMode(SD_INTERRUPT_MODE);
   }
	
	if (Status == SD_OK) {
		return RES_OK;
	} else {
		return RES_ERROR;
	}
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	SD_Error Status = SD_OK;
	
	if (count == 1) {
		Status = SD_ReadBlock(sector << 9 ,(u32 *)(&buff[0]),SECTOR_SIZE);
	} else {
		Status = SD_ReadMultiBlocks(sector << 9 ,(u32 *)(&buff[0]),SECTOR_SIZE,count);	
	}

	if (Status == SD_OK) {
		return RES_OK;
	} else {
		return RES_ERROR;
	}
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* The FatFs module will issue multiple sector transfer request
/  (count > 1) to the disk I/O layer. The disk function should process
/  the multiple sector transfer properly Do. not translate it into
/  multiple single sector transfers to the media, or the data read/write
/  performance may be drasticaly decreased. */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	SD_Error Status = SD_OK;
	
	if (count == 1) {
		Status = SD_WriteBlock(sector << 9 ,(u32 *)(&buff[0]),SECTOR_SIZE);
	} else {
		Status = SD_WriteMultiBlocks(sector << 9 ,(u32 *)(&buff[0]),SECTOR_SIZE,count);	
	}

	if (Status == SD_OK) {
		return RES_OK;
	} else {
		return RES_ERROR;
	}
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_OK;
}


/*******************************************************************************
* Function Name  : rtc_gettime
* Description    : populates structure from HW-RTC, takes DST into account
* Input          : None
* Output         : time-struct gets modified
* Return         : always true/not used
*******************************************************************************/
DWORD get_fattime (void)
{
	return 0;
}


