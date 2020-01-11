#include "dldi.h"
#include "typedefsTGDS.h"
#include "dmaTGDS.h"
#include "global_settings.h"
#include "busTGDS.h"
#include "ipcfifoTGDS.h"
#include <string.h>

// Common
static const uint32  DLDI_MAGIC_NUMBER = 0xBF8DA5ED;	
static const data_t dldiMagicString[12] = "\xED\xA5\x8D\xBF Chishm";	// Normal DLDI file
static const data_t dldiMagicLoaderString[12] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

// Stored backwards to prevent it being picked up by DLDI patchers
const char DLDI_MAGIC_STRING_BACKWARDS [DLDI_MAGIC_STRING_LEN] =
	{'\0', 'm', 'h', 's', 'i', 'h', 'C', ' '} ;

struct DLDI_INTERFACE* dldiGet(void) {
	#ifdef ARM7
	struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
	if (dldiInterface->ioInterface.features & FEATURE_SLOT_GBA) {
		SetBusSLOT1ARM9SLOT2ARM7();
	}
	if (dldiInterface->ioInterface.features & FEATURE_SLOT_NDS) {
		SetBusSLOT1ARM7SLOT2ARM9();
	}
	#endif
	#ifdef ARM9
	struct DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)&_dldi_start;
	if (dldiInterface->ioInterface.features & FEATURE_SLOT_GBA) {
		SetBusSLOT1ARM7SLOT2ARM9();
	}
	if (dldiInterface->ioInterface.features & FEATURE_SLOT_NDS) {
		SetBusSLOT1ARM9SLOT2ARM7();
	}
	#endif
	return (struct DLDI_INTERFACE*)dldiInterface;
}


///////////////////////////////////////////////////////////////////////////DLDI ARM7 CODE START/////////////////////////////////////////////////////////////////////


#ifdef ARM7
u32 * DLDIARM7Address = NULL;

void setDLDIARM7Address(u32 * address){
	DLDIARM7Address = address;
}

void initDLDIARM7(u32 srcDLDIAddr){	//implementation
	
	//FIFO Code must be sent from ARM7 here.
	//ARM9 must receive this ptr, perform relocation to (EWRAM) temp memory, create (EWRAM) ptr temp memory. This must be done because the ARM7 IWRAM is scarse and extra DLDI code can't fit
	//ARM7 must receive ptr temp memory and copy it to srcDLDIAddr
	/*
	fixAndRelocateDLDI((u32)srcDLDIAddr);
	*/
	
	
	//Then ARM7 can proceed like this
	//memcpy ((char*)&_dldi_start, (u8*)srcDLDIAddr, 16*1024);
	bool status = dldi_handler_init();
	if(status == false){
		SendFIFOWords(0x11ff00ff, 0);
		while(1);
	}
	else{
		SendFIFOWords(0x22ff11ff, 0);
		//while(1);
	}
}
#endif

bool dldi_handler_init(){
	bool status = false;
	struct DLDI_INTERFACE* dldiInit = dldiGet();	//ensures SLOT-1 / SLOT-2 is mapped to ARM7/ARM9 now
	if( (!dldiInit->ioInterface.startup()) || (!dldiInit->ioInterface.isInserted()) ){
		status = false;
	}
	else{
		status = true;	//init OK!
	}
	return status;
}


// WIP
/*

#ifdef ARM7_DLDI

//ARM9 resorts to callbacks between FIFO irqs to fecth DLDI data
#ifdef ARM9
//buffer should be in main memory
PUT_IN_VRAM __attribute__ ((noinline)) __attribute__((aligned(4)))	void read_sd_sectors_safe(sec_t sector, sec_t numSectors, void* buffer){
	//remote procedure call on arm7
	//assume buffer is in the vram cd block
	//uint32_t arm7_address = ((uint32_t)buffer) - ((uint32_t)vram_cd) + 0x06000000;
	//dc_wait_write_buffer_empty();
	//dc_invalidate_all();
	//dc_flush_range(vram_cd, 256 * 1024);
	//dc_flush_all();
	//map cd to arm7
	//REG_VRAMCNT_CD = VRAM_CD_ARM7;
	//REG_VRAMCNT_C = VRAM_C_ARM7;
	
	REG_SEND_FIFO = 0xAA5500DF;
	REG_SEND_FIFO = sector;
	REG_SEND_FIFO = numSectors;
	REG_SEND_FIFO = (uint32_t)MAIN_MEMORY_ADDRESS_SDCACHE;	//original: void * buffer -> arm7_address
	//wait for response
	do
	{
		while(*((vu32*)0x04000184) & (1 << 8));
	} while(REG_RECV_FIFO != 0x55AAAA55);
	//REG_VRAMCNT_C = VRAM_C_ARM9;
	//REG_VRAMCNT_CD = VRAM_CD_ARM9;
	//invalidate
	//dc_invalidate_all();
	
	//void * buffer is in VRAM...
	//fetched sectors are in MAIN_MEMORY_ADDRESS_SDCACHE, must be dma'd back to buffer
	dc_flush_range((void*)MAIN_MEMORY_ADDRESS_SDCACHE, numSectors * 512);
	memcpy((uint16_t*)buffer, (uint16_t*)MAIN_MEMORY_ADDRESS_SDCACHE, (numSectors * 512));	
}

//buffer should be in main memory
PUT_IN_VRAM __attribute__((noinline)) __attribute__((aligned(4)))	void write_sd_sectors_safe(sec_t sector, sec_t numSectors, const void* buffer){

	//remote procedure call on arm7
	//assume buffer is in the vram cd block
	//uint32_t arm7_address = ((uint32_t)buffer) - ((uint32_t)vram_cd) + 0x06000000;
	//dc_wait_write_buffer_empty();
	//dc_invalidate_all();
	//dc_flush_range(vram_cd, 256 * 1024);
	//dc_flush_all();
	//map cd to arm7
	//REG_VRAMCNT_CD = VRAM_CD_ARM7;
	//REG_VRAMCNT_C = VRAM_C_ARM7;
	
	//void * buffer is in VRAM...
	//fetched sectors are in MAIN_MEMORY_ADDRESS_SDCACHE, must be dma'd back to buffer
	dc_flush_range((void*)MAIN_MEMORY_ADDRESS_SDCACHE, numSectors * 512);
	memcpy((uint16_t*)MAIN_MEMORY_ADDRESS_SDCACHE, (uint16_t*)buffer, (numSectors * 512));
	
	REG_SEND_FIFO = 0xAA5500F0;
	REG_SEND_FIFO = sector;
	REG_SEND_FIFO = numSectors;
	REG_SEND_FIFO = (uint32_t)MAIN_MEMORY_ADDRESS_SDCACHE;	//original: void * buffer -> arm7_address
									 //wait for response
	do
	{
		while (*((vu32*)0x04000184) & (1 << 8));
	} while (REG_RECV_FIFO != 0x55AAAA55);
	//REG_VRAMCNT_C = VRAM_C_ARM9;
	//REG_VRAMCNT_CD = VRAM_CD_ARM9;
	//invalidate
	//dc_invalidate_range(buffer, numSectors * 512);
	//dc_invalidate_all();
}
#endif

#endif
*/
////////////////////////////////////////////// DLDI ARM 7 CODE end ////////////////////////////////////////////// 