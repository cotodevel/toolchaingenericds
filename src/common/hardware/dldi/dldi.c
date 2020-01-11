#include "dldi.h"
#include "typedefsTGDS.h"
#include "dmaTGDS.h"
#include "global_settings.h"
#include "busTGDS.h"
#include "ipcfifoTGDS.h"
#include <string.h>

// Common

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

#ifdef ARM9

static const uint32  DLDI_MAGIC_NUMBER = 0xBF8DA5ED;	
static const data_t dldiMagicString[12] = "\xED\xA5\x8D\xBF Chishm";	// Normal DLDI file
static const data_t dldiMagicLoaderString[12] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

//Coto: this one copies the DLDI section from EWRAM and relocates it to u32 DldiRelocatedAddress, so all DLDI code can be called and executed from there.
void fixAndRelocateDLDI(u32 dldiSourceInRam){
	//#ifdef ARM7
	
	#ifdef ARM7
	u32 DldiRelocatedAddress = (u32)DLDIARM7Address; //relocation offset
	#endif
	#ifdef ARM9
	u32 DldiRelocatedAddress = (u32)&_dldi_start; //relocation offset
	#endif
	
	bool clearBSS = false;
	bool patchStatus = dldiRelocateLoader(clearBSS, DldiRelocatedAddress, dldiSourceInRam);	//Coto: re-use DLDI code to patch the DLDI loaded to us (by the loader itself), so we can relocate the DLDI to whatever the address we want.
	if(patchStatus == true){
		//*((vu32*)0x06202000) = 0x4b4f5450; //PTOK
	}
	else{
		//*((vu32*)0x06202000) = 0x52455450; //PTER	//Coto: means the loader failed to give us a proper DLDI driver. Make sure to launch GBARunner2 through any loader that does DLDI patching.
		while(1==1);
	}
	//#endif
	
	/*
	#ifdef ARM9
	u32 DldiRelocatedAddress = (u32)0x06800000; //relocation offset
	bool clearBSS = false;
	bool patchStatus = dldiRelocateLoader(clearBSS, DldiRelocatedAddress, dldiSourceInRam);	//Coto: re-use DLDI code to patch the DLDI loaded to us (by the loader itself), so we can relocate the DLDI to whatever the address we want.
	if(patchStatus == true){
		*((vu32*)0x06202000) = 0x4b4f5450; //PTOK
	}
	else{
		*((vu32*)0x06202000) = 0x52455450; //PTER	//Coto: means the loader failed to give us a proper DLDI driver. Make sure to launch GBARunner2 through any loader that does DLDI patching.
		while(1==1);
	}
	#endif
	*/
}

bool dldiRelocateLoader(bool clearBSS, u32 DldiRelocatedAddress, u32 dldiSourceInRam)
{
	addr_t memOffset;			// Offset of DLDI after the file is loaded into memory
	addr_t patchOffset;			// Position of patch destination in the file
	addr_t relocationOffset;	// Value added to all offsets within the patch to fix it properly
	addr_t ddmemOffset;			// Original offset used in the DLDI file
	addr_t ddmemStart;			// Start of range that offsets can be in the DLDI file
	addr_t ddmemEnd;			// End of range that offsets can be in the DLDI file
	addr_t ddmemSize;			// Size of range that offsets can be in the DLDI file

	addr_t addrIter;

	data_t *pDH;
	data_t *pAH;

	size_t dldiFileSize = 0;
	
	// Target the DLDI we want to use as stub copy and then relocate it to a DldiRelocatedAddress address
	struct DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DldiRelocatedAddress;
	pDH = (data_t*)dldiInterface;
	pAH = (data_t *)dldiSourceInRam;
	
	dldiFileSize = 1 << pAH[DO_driverSize];

	// Copy the DLDI patch into the application
	//dmaCopyWords(0, (void*)pAH, (void*)pDH, dldiFileSize);			//dmaCopyWords (uint8 channel, const void *src, void *dest, uint32 size)
	dmaTransferWord(3, (uint32)pAH, (uint32)pDH, (uint32)dldiFileSize);	//void dmaTransferWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count)


	if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) {
		// No DLDI patch
		return false;
	}
	
	if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) {
		// Not enough space for patch
		return false;
	}
	
	memOffset = DldiRelocatedAddress;	//readAddr (pAH, DO_text_start);
	if (memOffset == 0) {
			memOffset = readAddr (pAH, DO_startup) - DO_code;
	}
	ddmemOffset = readAddr (pDH, DO_text_start);
	relocationOffset = memOffset - ddmemOffset;

	ddmemStart = readAddr (pDH, DO_text_start);
	ddmemSize = (1 << pDH[DO_driverSize]);
	ddmemEnd = ddmemStart + ddmemSize;

	// Remember how much space is actually reserved
	pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
	
	// Fix the section pointers in the DLDI @ VRAM header
	writeAddr (pDH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
	writeAddr (pDH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
	writeAddr (pDH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
	writeAddr (pDH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
	writeAddr (pDH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
	writeAddr (pDH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
	writeAddr (pDH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
	writeAddr (pDH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
	
	// Fix the function pointers in the header
	writeAddr (pDH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
	writeAddr (pDH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
	writeAddr (pDH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
	writeAddr (pDH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
	writeAddr (pDH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
	writeAddr (pDH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);
	
	if (pDH[DO_fixSections] & FIX_ALL) { 
		// Search through and fix pointers within the data section of the file
		for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}
	
	
	if (pDH[DO_fixSections] & FIX_GLUE) { 
		// Search through and fix pointers within the glue section of the file
		for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}
	
	if (pDH[DO_fixSections] & FIX_GOT) { 
		// Search through and fix pointers within the Global Offset Table section of the file
		for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}
	
	/*
	if (clearBSS && (pDH[DO_fixSections] & FIX_BSS)) { 
		// Initialise the BSS to 0, only if the disc is being re-inited
		for(int i = 0; i < (readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start)) / 4; i++)
		{
			((uint32_t*)&pAH[readAddr(pDH, DO_bss_start) - ddmemStart])[i] = 0;
		}
		
	}
	*/
	return true;
}

bool dldiPatchLoader (data_t *binData, u32 binSize)
{
	addr_t memOffset;			// Offset of DLDI after the file is loaded into memory
	addr_t patchOffset;			// Position of patch destination in the file
	addr_t relocationOffset;	// Value added to all offsets within the patch to fix it properly
	addr_t ddmemOffset;			// Original offset used in the DLDI file
	addr_t ddmemStart;			// Start of range that offsets can be in the DLDI file
	addr_t ddmemEnd;			// End of range that offsets can be in the DLDI file
	addr_t ddmemSize;			// Size of range that offsets can be in the DLDI file

	addr_t addrIter;

	data_t *pDH;
	data_t *pAH;

	size_t dldiFileSize = 0;
	
	struct DLDI_INTERFACE* dldiInterface = dldiGet();
	pDH = (data_t*)dldiInterface;
	
	if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) 
	{
		/*
		// no dldi present in DSO, lets load it from the file we found earlier if it's available	
		char *dldiPath = getDLDI();
		
		if(strlen(dldiPath) == 0) // No DLDI patch
			return false;
		
		// load and then present
		
		DRAGON_chdir(d_res);
		
		DRAGON_FILE *df = DRAGON_fopen(dldiPath, "r");
		
		u32 fLen = DRAGON_flength(df);
		pDH = safeMalloc(PATCH_SIZE);
		memset(pDH, 0, PATCH_SIZE);
		
		DRAGON_fread(pDH, 1, fLen, df);
		DRAGON_fclose(df);
		*/
		printf("DLDI section not found in NTR binary. ");
	}
	else{
		printf("DLDI section found in NTR binary. Patching... ");
	}
	
	// Find the DLDI reserved space in the file
	patchOffset = quickFind(binData, dldiMagicString, binSize, sizeof(dldiMagicString));

	if (patchOffset < 0) {
		// does not have a DLDI section
		//printf("ERR: NO DLDI SECTION");
		return false;
	}
	
	pAH = &(binData[patchOffset]);

	if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) 
	{
		// Not enough space for patch
		//printf("ERR: NOT ENOUGH SPACE");
		return false;
	}
	
	dldiFileSize = 1 << pDH[DO_driverSize];

	memOffset = readAddr (pAH, DO_text_start);
	if (memOffset == 0) {
			memOffset = readAddr (pAH, DO_startup) - DO_code;
	}
	ddmemOffset = readAddr (pDH, DO_text_start);
	relocationOffset = memOffset - ddmemOffset;

	ddmemStart = readAddr (pDH, DO_text_start);
	ddmemSize = (1 << pDH[DO_driverSize]);
	ddmemEnd = ddmemStart + ddmemSize;

	// Remember how much space is actually reserved
	pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
	// Copy the DLDI patch into the application
	memcpy (pAH, pDH, dldiFileSize);

	// Fix the section pointers in the header
	writeAddr (pAH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
	writeAddr (pAH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
	writeAddr (pAH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
	writeAddr (pAH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
	writeAddr (pAH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
	writeAddr (pAH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
	writeAddr (pAH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
	writeAddr (pAH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
	// Fix the function pointers in the header
	writeAddr (pAH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
	writeAddr (pAH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
	writeAddr (pAH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
	writeAddr (pAH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
	writeAddr (pAH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
	writeAddr (pAH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);

	if (pDH[DO_fixSections] & FIX_ALL) { 
		// Search through and fix pointers within the data section of the file
		for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GLUE) { 
		// Search through and fix pointers within the glue section of the file
		for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GOT) { 
		// Search through and fix pointers within the Global Offset Table section of the file
		for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	/*
	if (pDH[DO_fixSections] & FIX_BSS) { 
		// Initialise the BSS to 0
		memset (&pAH[readAddr(pDH, DO_bss_start) - ddmemStart] , 0, readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start));
	}
	*/

	return true;
}

#endif

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