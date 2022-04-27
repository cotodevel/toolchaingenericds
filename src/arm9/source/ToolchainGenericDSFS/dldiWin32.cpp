#ifdef WIN32

#include <stdio.h>
#include <string.h>
#include "dldiWin32.h"

#if defined(WIN32) || defined(ARM9)
#include "fatfslayerTGDS.h"
#endif

#if defined(WIN32)
FILE * virtualDLDIDISKImg = NULL;
u8 _io_dldi_stub[16384];
#endif

const uint32  DLDI_MAGIC_NUMBER = 
	0xBF8DA5ED;	
	
// Stored backwards to prevent it being picked up by DLDI patchers
const sint8 DLDI_MAGIC_STRING_BACKWARDS [DLDI_MAGIC_STRING_LEN] =
	{'\0', 'm', 'h', 's', 'i', 'h', 'C', ' '} ;


// 2/2 : once DLDI has been setup
struct DLDI_INTERFACE* dldiGet(void) {
	#if defined(WIN32)
	return (struct DLDI_INTERFACE*)&_io_dldi_stub[0];
	#endif
}

#if defined(WIN32)
bool __dsimode = false; //always NTR mode for debugging purposes
#endif

#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_init() {
	#if defined(WIN32)
	fseek(virtualDLDIDISKImg, 0, SEEK_SET);
	int res = ftell(virtualDLDIDISKImg);
	if(res != 0){
		return false;
	}
	#endif	
	return true;
}

#ifdef ARM9
__attribute__ ((optnone))
#endif
void dldi_handler_deinit() {
	
}

//////////////////////////////////////////////// RAM Disk DLDI Implementation End ///////////////////////////////////////////
//future optimization, make it EWRAM-only so we can DMA directly!
#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer) {
	#if defined(WIN32)
	fseek(virtualDLDIDISKImg, 512*sector, SEEK_SET);
	int fetch = fread(buffer, 1, 512*numSectors, virtualDLDIDISKImg);
	if(fetch != (512*numSectors)){
		return false;
	}
	#endif
	return true;
}

#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer) {
	#if defined(WIN32)
	fseek(virtualDLDIDISKImg, 512*sector, SEEK_SET);
	int fetch = fwrite((void*)buffer, 1, 512*numSectors, virtualDLDIDISKImg);
	if(fetch != (512*numSectors)){
		return false;
	}
	#endif
	return true;
}
#endif