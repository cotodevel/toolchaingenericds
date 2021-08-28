#include <stdio.h>
#include <string.h>
#include "dldi.h"

#if defined(WIN32) || defined(ARM9)
#include "fatfslayerTGDS.h"
#endif

#ifdef ARM9
#include "dsregs.h"
#include "dmaTGDS.h"
#include "busTGDS.h"
#include "biosTGDS.h"
#include "ipcfifoTGDS.h"
#include "global_settings.h"
#include "utilsTGDS.h"
#endif

#ifdef TWLMODE

#ifdef ARM7
#include "sdmmc.h"
#endif

#include "memory.h"
#include "utils.twl.h"
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

#ifdef ARM9
const DLDI_INTERFACE* io_dldi_data = &_io_dldi_stub;
#endif

#if defined(WIN32)
const DLDI_INTERFACE* io_dldi_data = (const DLDI_INTERFACE*)&_io_dldi_stub[0];
#endif

// 2/2 : once DLDI has been setup
struct DLDI_INTERFACE* dldiGet(void) {
	
	#ifdef ARM7
	struct DLDI_INTERFACE* arm7DLDI = (DLDI_INTERFACE*)DLDIARM7Address;
	return arm7DLDI;
	#endif
	
	#ifdef ARM9
	return &_io_dldi_stub;
	#endif

	#if defined(WIN32)
	return (struct DLDI_INTERFACE*)&_io_dldi_stub[0];
	#endif
	
}

#ifdef ARM7
//DLDI bits
u32 * DLDIARM7Address = NULL;
#endif

#ifdef ARM9
u8 ARM7SharedDLDI[32768];	//Required by ARM7 DLDI read/write dldi calls
#endif

#if defined(WIN32)
bool __dsimode = false; //always NTR mode for debugging purposes
#endif

#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_init() {
	//DS
	if(__dsimode == false){
		#if defined(ARM7) || defined(ARM9)
		struct DLDI_INTERFACE* dldiInit = dldiGet();	//NTR Mode + Emu = ARM9DLDI: Ensures SLOT-1 / SLOT-2 is mapped to ARM7/ARM9 now
		if( (!dldiInit->ioInterface.startup()) || (!dldiInit->ioInterface.isInserted()) ){
			return false;
		}
		#endif
		#if defined(WIN32)
		fseek(virtualDLDIDISKImg, 0, SEEK_SET);
		int res = ftell(virtualDLDIDISKImg);
		if(res != 0){
			return false;
		}
		#endif
	}
	//DSi
	else{
		#ifdef TWLMODE
		#ifdef ARM9
		if( (!sdio_Startup()) || (!sdio_IsInserted()) ){
			return false;
		}
		#endif
		#endif
	}
	return true;
}

#ifdef ARM9
__attribute__ ((optnone))
#endif
void dldi_handler_deinit() {
	//DS
	if(__dsimode == false){
		struct DLDI_INTERFACE* dldiInit = dldiGet();	//ensures SLOT-1 / SLOT-2 is mapped to ARM7/ARM9 now
		dldiInit->ioInterface.clearStatus();
		dldiInit->ioInterface.shutdown();
	}
	//DSi
	else{
		#ifdef TWLMODE
		#ifdef ARM9
		sdio_ClearStatus();
		sdio_Shutdown();
		#endif
		#endif
	}
}

/////////////////////////////////////////////////// (EWRAM DLDI) RAM Disk DLDI Implementation ////////////////////////////////////////////
#ifdef ARM9
__attribute__((section(".dldiSection")))
bool _DLDI_isInserted(void)
{
	return true;	//Always True
}

__attribute__((section(".dldiSection")))
bool _DLDI_clearStatus(void)
{
    return true;	//Always True
}

__attribute__((section(".dldiSection")))
bool _DLDI_shutdown(void)
{
    return true;	//Always True
}

__attribute__((section(".dldiSection")))
bool _DLDI_startup(void)
{
    return true;	//Always True
} 

__attribute__((section(".dldiSection")))
bool _DLDI_writeSectors(uint32 sector, uint32 sectorCount, const uint8* buffer)
{
	int sectorSize = 512;
	int curSector = 0;
	while(sectorCount > 0)
	{
        memcpy(((u8*)0x08000000 + ((sector+curSector)*sectorSize)), (buffer + (curSector*sectorSize)), sectorSize);
		curSector++;
		--sectorCount;
	}
    return true;
}

__attribute__((section(".dldiSection")))
bool _DLDI_readSectors(uint32 sector, uint32 sectorCount, uint8* buffer)
{
	int sectorSize = 512;
	int curSector = 0;
	while(sectorCount > 0)
	{
        memcpy(buffer + (curSector*sectorSize), ((u8*)0x08000000 + ((sector+curSector)*sectorSize)), sectorSize);
		curSector++;
		--sectorCount;
	}
    return true;
}
#endif
//////////////////////////////////////////////// RAM Disk DLDI Implementation End ///////////////////////////////////////////
//future optimization, make it EWRAM-only so we can DMA directly!
#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer) {
	//ARM7 DLDI implementation
	//NTR hardware: ARM7DLDI
	if(__dsimode == false){
		#ifdef ARM7
		struct  DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DLDIARM7Address;
		return dldiInterface->ioInterface.readSectors(sector, numSectors, buffer);
		#endif
		#ifdef ARM9
		void * targetMem = (void *)((int)&ARM7SharedDLDI[0] + 0x400000); //Uncached NTR, TWL is implemented below
		uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
		fifomsg[20] = (uint32)sector;
		fifomsg[21] = (uint32)numSectors;
		fifomsg[22] = (uint32)targetMem;
		fifomsg[23] = (uint32)0xFFFFFFFF;
		sendByteIPC(IPC_READ_ARM7DLDI_REQBYIRQ);
		while(fifomsg[23] != (uint32)0){
			swiDelay(1);
		}
		memcpy((uint16_t*)buffer, (uint16_t*)targetMem, (numSectors * 512));
		return true;
		#endif
		#if defined(WIN32)
		fseek(virtualDLDIDISKImg, 512*sector, SEEK_SET);
		int fetch = fread(buffer, 1, 512*numSectors, virtualDLDIDISKImg);
		if(fetch != (512*numSectors)){
			return false;
		}
		#endif
	}
	
	//TWL hardware: ARM7DLDI
	else{
		#ifdef ARM7
			#ifdef TWLMODE
			sdmmc_readsectors(&deviceSD, sector, numSectors, buffer);
			return true;
			#endif
			
			#ifndef TWLMODE
			return false;
			#endif
		#endif
		
		#ifdef ARM9			
			#ifdef TWLMODE
			sdio_ReadSectors(sector, numSectors, buffer);
			return true;
			#endif
			
			#ifndef TWLMODE
			return false;
			#endif
		#endif
	}
	return true;
}

#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer) {
	//NTR hardware: ARM7DLDI
	if(__dsimode == false){
		#ifdef ARM7
		struct  DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DLDIARM7Address;
		return dldiInterface->ioInterface.writeSectors(sector, numSectors, buffer);
		#endif
		#ifdef ARM9
		void * targetMem = (void *)((int)&ARM7SharedDLDI[0] + 0x400000);	//Uncached NTR, TWL is implemented below
		memcpy((uint16_t*)targetMem, (uint16_t*)buffer, (numSectors * 512));
		uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
		fifomsg[24] = (uint32)sector;
		fifomsg[25] = (uint32)numSectors;
		fifomsg[26] = (uint32)targetMem;
		fifomsg[27] = (uint32)0xFFFFFFFF;
		sendByteIPC(IPC_WRITE_ARM7DLDI_REQBYIRQ);
		while(fifomsg[27] != (uint32)0){
			swiDelay(1);
		}
		return true;
		#endif
		#if defined(WIN32)
		fseek(virtualDLDIDISKImg, 512*sector, SEEK_SET);
		int fetch = fwrite((void*)buffer, 1, 512*numSectors, virtualDLDIDISKImg);
		if(fetch != (512*numSectors)){
			return false;
		}
		#endif
	}
	
	//TWL hardware: ARM7DLDI
	else{
		#ifdef ARM7
			#ifdef TWLMODE
			sdmmc_writesectors(&deviceSD, sector, numSectors, (void*)buffer);
			return true;
			#endif
			
			#ifndef TWLMODE
			return false;
			#endif
		#endif
		
		#ifdef ARM9			
			#ifdef TWLMODE
			sdio_WriteSectors(sector, numSectors, buffer);
			return true;
			#endif
			#ifndef TWLMODE
			return false;
			#endif
		#endif
	}
	return true;
}

static const data_t dldiMagicString[12] = "\xED\xA5\x8D\xBF Chishm";	// Normal DLDI file
//static const data_t dldiMagicLoaderString[12] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

enum DldiOffsets {
	DO_magicString = 0x00,			// "\xED\xA5\x8D\xBF Chishm"
	DO_magicToken = 0x00,			// 0xBF8DA5ED
	DO_magicShortString = 0x04,		// " Chishm"
	DO_version = 0x0C,
	DO_driverSize = 0x0D,
	DO_fixSections = 0x0E,
	DO_allocatedSpace = 0x0F,

	DO_friendlyName = 0x10,

	DO_text_start = 0x40,			// Data start
	DO_data_end = 0x44,				// Data end
	DO_glue_start = 0x48,			// Interworking glue start	-- Needs address fixing
	DO_glue_end = 0x4C,				// Interworking glue end
	DO_got_start = 0x50,			// GOT start					-- Needs address fixing
	DO_got_end = 0x54,				// GOT end
	DO_bss_start = 0x58,			// bss start					-- Needs setting to zero
	DO_bss_end = 0x5C,				// bss end

	// IO_INTERFACE data
	DO_ioType = 0x60,
	DO_features = 0x64,
	DO_startup = 0x68,	
	DO_isInserted = 0x6C,	
	DO_readSectors = 0x70,	
	DO_writeSectors = 0x74,
	DO_clearStatus = 0x78,
	DO_shutdown = 0x7C,
	DO_code = 0x80
};

static inline addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static inline void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

static inline addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
	const int* dataChunk = (const int*) data;
	int searchChunk = ((const int*)search)[0];
	addr_t i;
	addr_t dataChunkEnd = (addr_t)(dataLen / sizeof(int));

	for ( i = 0; i < dataChunkEnd; i++) {
		if (dataChunk[i] == searchChunk) {
			if ((i*sizeof(int) + searchLen) > dataLen) {
				return -1;
			}
			if (memcmp (&data[i*sizeof(int)], search, searchLen) == 0) {
				return i*sizeof(int);
			}
		}
	}
	return -1;
}

//DldiRelocatedAddress == target DLDI relocated address
//dldiSourceInRam == physical DLDI section having a proper DLDI driver used as donor 
//dldiOutWriteAddress == new physical DLDI out buffer, except, relocated to a new DldiRelocatedAddress!
bool dldiRelocateLoader(bool clearBSS, u32 DldiRelocatedAddress, u32 dldiSourceInRam, u32 dldiOutWriteAddress){
	addr_t memOffset;			// Offset of DLDI after the file is loaded into memory
	//addr_t patchOffset;			// Position of patch destination in the file
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
	pDH = (data_t*)dldiOutWriteAddress;
	pAH = (data_t *)dldiSourceInRam;
	dldiFileSize = 1 << pAH[DO_driverSize];
	// Copy the DLDI patch into the application
	#if defined(WIN32)
	memcpy((void *)pDH, (const void *)pAH, dldiFileSize);
	#endif
	#ifdef ARM9
	dmaTransferWord(3, (uint32)pAH, (uint32)pDH, (uint32)dldiFileSize);	//void dmaTransferWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count)
	#endif
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

//arg0: binary data address to patch
//arg1: binary data size to patch
//arg2: Physical valid DLDI address (containing the DLDI binary to-be used as donor)

bool dldiPatchLoader(data_t *binData, u32 binSize, u32 physDLDIAddress)
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
	pDH = (data_t*)physDLDIAddress;
	//DS DLDI
	struct  DLDI_INTERFACE* dldiInterface = (struct  DLDI_INTERFACE*)pDH;
	
	if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) 
	{
		#ifdef ARM9
		printf("DLDI section not found in NTR binary. ");
		#endif
	}
	//TGDS DLDI RAMDISK? No need to patch because the offsets are outside the DLDI stub and kept in TGDS NDS Binary anyway to be reused later
	if(strncmp((char *)&dldiInterface->friendlyName[0], "TGDS RAMDISK", 12) == 0){
		return false;
	}
	else{
		#ifdef ARM9
		printf("DLDI section found in NTR binary. Patching... ");
		#endif
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

#ifdef TWLMODE
#ifdef ARM9
extern const struct DISC_INTERFACE_STRUCT __io_dsisd;
const struct DISC_INTERFACE_STRUCT* get_io_dsisd (void) {
	return (isDSiMode() && __NDSHeader->unitCode ) ? &__io_dsisd : NULL;
}
#endif
#endif