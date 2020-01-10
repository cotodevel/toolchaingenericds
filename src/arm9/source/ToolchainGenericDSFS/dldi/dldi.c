#include "typedefsTGDS.h"
#include "dldi.h"
#include <string.h>

#include "dmaTGDS.h"

static const uint32  DLDI_MAGIC_NUMBER = 0xBF8DA5ED;	
static const data_t dldiMagicString[12] = "\xED\xA5\x8D\xBF Chishm";	// Normal DLDI file
static const data_t dldiMagicLoaderString[12] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

// Stored backwards to prevent it being picked up by DLDI patchers
const char DLDI_MAGIC_STRING_BACKWARDS [DLDI_MAGIC_STRING_LEN] =
	{'\0', 'm', 'h', 's', 'i', 'h', 'C', ' '} ;

const DLDI_INTERFACE* io_dldi_data = (const DLDI_INTERFACE*)&_dldi_start;

struct DLDI_INTERFACE* dldiGet(void) {
	#ifdef ARM7
	if (_dldi_start.ioInterface.features & FEATURE_SLOT_GBA) {
		SetBusSLOT1ARM9SLOT2ARM7();
	}
	if (_dldi_start.ioInterface.features & FEATURE_SLOT_NDS) {
		SetBusSLOT1ARM7SLOT2ARM9();
	}
	#endif
	#ifdef ARM9
	if (_dldi_start.ioInterface.features & FEATURE_SLOT_GBA) {
		SetBusSLOT1ARM7SLOT2ARM9();
	}
	if (_dldi_start.ioInterface.features & FEATURE_SLOT_NDS) {
		SetBusSLOT1ARM9SLOT2ARM7();
	}
	#endif
	return (struct DLDI_INTERFACE*)&_dldi_start;
}


///////////////////////////////////////////////////////////////////////////DLDI ARM7 CODE START/////////////////////////////////////////////////////////////////////


#ifdef ARM7
void initDLDIARM7(u32 srcDLDIAddr){	//implementation
	
	fixAndRelocateDLDI((u32)srcDLDIAddr);

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

bool dldi_handler_init()
{
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

bool dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer){
	return _dldi_start.ioInterface.readSectors(sector, numSectors, buffer);
}

bool dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer){
	return _dldi_start.ioInterface.writeSectors(sector, numSectors, buffer);
}

////////////////////////////////////////////// DLDI ARM 9 CODE end ////////////////////////////////////////////// 


////////////////////////////////////////////// DLDI ARM 7 CODE ////////////////////////////////////////////// 

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

////////////////////////////////////////////// DLDI ARM 7 CODE end ////////////////////////////////////////////// 