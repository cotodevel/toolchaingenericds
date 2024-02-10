/*
 dldi.h

 Copyright (c) 2006 Michael "Chishm" Chisholm
	
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NDS_DLDI_INCLUDE
#define NDS_DLDI_INCLUDE

#if defined (ARM7) || defined(ARM9)
#include "typedefsTGDS.h"
#endif

#if defined (MSDOS) || defined(WIN32)
#include "fatfslayerTGDS.h"
#include "..\misc\vs2012TGDS-FS\TGDSFSVS2012\TGDSFSVS2012\TGDSTypes.h"
#endif

typedef unsigned int sec_t;
typedef signed int addr_t;
typedef unsigned char data_t;

#define FEATURE_MEDIUM_CANREAD		0x00000001
#define FEATURE_MEDIUM_CANWRITE		0x00000002
#define FEATURE_SLOT_GBA			0x00000010
#define FEATURE_SLOT_NDS			0x00000020

typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(uint32 sector, uint32 numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(uint32 sector, uint32 numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;

struct  DISC_INTERFACE_STRUCT{
	unsigned long			ioType ;
	unsigned long			features ;
	FN_MEDIUM_STARTUP		startup ;
	FN_MEDIUM_ISINSERTED	isInserted ;
	FN_MEDIUM_READSECTORS	readSectors ;
	FN_MEDIUM_WRITESECTORS	writeSectors ;
	FN_MEDIUM_CLEARSTATUS	clearStatus ;
	FN_MEDIUM_SHUTDOWN		shutdown ;
} ;


#define FIX_ALL						0x01
#define FIX_GLUE					0x02
#define FIX_GOT						0x04
#define FIX_BSS						0x08
#define DEVICE_TYPE_DLDI 			0x49444C44
#define DLDI_MAGIC_STRING_LEN 		8
#define DLDI_FRIENDLY_NAME_LEN 		48

// I/O interface with DLDI extensions
typedef struct DLDI_INTERFACE {
	uint32 	magicNumber;
	sint8	magicString [DLDI_MAGIC_STRING_LEN];
	uint8		versionNumber;
	uint8		driverSize;			// log-2 of driver size in bytes
	uint8		fixSectionsFlags;
	uint8		allocatedSize;		// log-2 of the allocated space in bytes

	sint8	friendlyName [DLDI_FRIENDLY_NAME_LEN];
	
	// Pointers to sections that need address fixing
	void*	dldiStart;
	void*	dldiEnd;
	void*	interworkStart;
	void*	interworkEnd;
	void*	gotStart;
	void*	gotEnd;
	void*	bssStart;
	void*	bssEnd;
	
	// Original I/O interface data
	struct DISC_INTERFACE_STRUCT ioInterface;
} DLDI_INTERFACE;

#define DEVICE_TYPE_DSI_SD ('_') | ('S' << 8) | ('D' << 16) | ('_' << 24)

//ARM7DLDI operating mode: Internal SD / DLDI bits used by int TWLModeInternalSDAccess
#define TWLModeDLDIAccessDisabledInternalSDDisabled ((int)-1) //TWL Mode: neither DLDI or SDIO access (default at startup)
#define TWLModeDLDIAccessEnabledInternalSDDisabled ((int)1) //TWL Mode: DLDI access only
#define TWLModeDLDIAccessDisabledInternalSDEnabled ((int)2) //TWL Mode: SDIO access only
#define TWLModeDLDIAccessEnabledInternalSDEnabled ((int)3) //TWL Mode: DLDI + SDIO access

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern const uint32  DLDI_MAGIC_NUMBER;
extern bool __dsimode;

#ifdef ARM9
extern struct DLDI_INTERFACE _io_dldi_stub;
#endif
#if defined(WIN32)

/*
Pointer to the internal DLDI, not directly usable by libfat.
You'll need to set the bus permissions appropriately before using.
*/
// The only built in driver
extern u8 _io_dldi_stub[16384];
extern FILE * virtualDLDIDISKImg;
#endif

/* pointer to DLDI_INTERFACE (DLDI handle) */
extern struct DLDI_INTERFACE* dldiGet(void);

extern bool dldi_handler_init();
extern void dldi_handler_deinit();
extern bool dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer);
extern bool dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer);

#ifdef ARM7
extern u32 * DLDIARM7Address;
#endif

#ifdef ARM9
extern u8 ARM7SharedDLDI[32768];

//Coto: RAM Disk DLDI Implementation. DLDI RW Works (32MB @ 0x08000000) in emulators now!
extern bool _DLDI_isInserted(void);
extern bool _DLDI_clearStatus(void);
extern bool _DLDI_shutdown(void);
extern bool _DLDI_startup(void);
extern bool _DLDI_readSectors(uint32 sector, uint32 sectorCount, uint8* buffer);
extern bool _DLDI_writeSectors(uint32 sector, uint32 sectorCount, const uint8* buffer);
extern bool dldiRelocateLoader(bool clearBSS, u32 DldiRelocatedAddress, u32 dldiSourceInRam, u32 dldiOutWriteAddress);
extern bool dldiPatchLoader(data_t *binData, u32 binSize, u32 physDLDIAddress);
#endif

///////////////////////////////////TWL mode SD ARM9i DLDI Access///////////////////////////////////
extern const struct DISC_INTERFACE_STRUCT __io_dsisd;
extern const struct DISC_INTERFACE_STRUCT* get_io_dsisd (void);
extern bool sdio_Startup();
extern bool sdio_IsInserted();
extern bool sdio_ReadSectors(sec_t sector, sec_t numSectors,void* buffer);
extern bool sdio_WriteSectors(sec_t sector, sec_t numSectors,const void* buffer);
extern bool sdio_ClearStatus();
extern bool sdio_Shutdown();
extern int TWLModeInternalSDAccess;
extern const data_t dldiMagicString[12];
extern addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen);

#ifdef __cplusplus
}
#endif

