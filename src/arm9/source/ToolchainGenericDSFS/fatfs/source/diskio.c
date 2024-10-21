/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "posixHandleTGDS.h"
#include "videoTGDS.h"

#if defined(WIN32)
#include "dldiWin32.h"
#include "TGDSTypes.h"
#endif

#if defined(ARM9)
#include "dldi.h"
#include <stdbool.h>
#include "global_settings.h"
#include "exceptionTGDS.h"
#endif

#include "ff.h"
/* Definitions of physical drive number for each media */
//#define SDCARD        0
//#define CTRNAND       1
#define DLDICART        0

//coto: support for dldi driver (:

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
#ifdef ARM9
__attribute__((unused))
#endif
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS ret = 0; //init OK!
	
	TWLModeInternalSDAccess = getValueSafe((u32*)0x02FFDFE8); //ARM7DLDI mode: TWL SD @ ARM7 or DLDI SD @ ARM7: @ARM7_ARM9_DLDI_STATUS
	if(TWLModeInternalSDAccess == TWLModeDLDIAccessDisabledInternalSDDisabled){
		
		//Throw exception always
		u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
		int TGDSDebuggerStage = 10;
		sprintf((char*)ConsolePrintfBuf, "ARM9: disk_initialize(): failed.");
		handleDSInitOutputMessage((char*)ConsolePrintfBuf);
		handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
		
		ret = STA_NOINIT;
	}
	return ret;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive number to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if(dldi_handler_read_sectors(sector, count, buff) == true){
		return RES_OK;
	}
	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if(dldi_handler_write_sectors(sector, count, buff) == true){
		return RES_OK;
	}
	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_PARERR;
}

