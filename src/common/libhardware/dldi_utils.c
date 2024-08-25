#include <stdio.h>
#include <string.h>
#include "dldi.h"
#include "typedefsTGDS.h"
#include "debugNocash.h"
#include "utilsTGDS.h"
#include "ipcfifoTGDS.h"

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
#include "nds_cp15_misc.h"
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

#ifdef ARM7
__attribute__((optimize("O0")))
bool ARM7InitDLDI(u32 ARM7MallocStartaddress, int ARM7MallocSize, u32 TargetARM7DLDIAddress){
	u32 dldiStartAddress = getValueSafe((u32*)0x02FFDFE8);	//ARM7_ARM9_DLDI_STATUS
	
	setupLibUtils(); //ARM7 libUtils Setup
	
	//DSi in NTR mode throws false positives about TWL mode, enforce DSi SD initialization to define, NTR or TWL mode.
	int detectedTWLModeInternalSDAccess = 0;
	bool DLDIARM7FSInitStatus = false;
	
	if( (!sdio_Startup()) || (!sdio_IsInserted()) ){
		detectedTWLModeInternalSDAccess = TWLModeDLDIAccessDisabledInternalSDDisabled;
	}
	else{
		detectedTWLModeInternalSDAccess = TWLModeDLDIAccessDisabledInternalSDEnabled;
		DLDIARM7FSInitStatus = true;
	}
	
	//NTR mode: define DLDI initialization and ARM7DLDI operating mode
	if((detectedTWLModeInternalSDAccess == TWLModeDLDIAccessDisabledInternalSDDisabled) && (TargetARM7DLDIAddress != 0)){
		DLDIARM7Address = (u32*)TargetARM7DLDIAddress;
		dldiRelocateLoader((u32)DLDIARM7Address, dldiStartAddress);
		DLDIARM7FSInitStatus = dldi_handler_init();
		if(DLDIARM7FSInitStatus == true){
			detectedTWLModeInternalSDAccess = TWLModeDLDIAccessEnabledInternalSDDisabled;
		}
		else{
			detectedTWLModeInternalSDAccess = TWLModeDLDIAccessDisabledInternalSDDisabled;
		}					
	}
	
	//ARM7 custom Malloc libutils implementation
	if(initMallocARM7LibUtilsCallback != NULL){
		initMallocARM7LibUtilsCallback(ARM7MallocStartaddress, ARM7MallocSize);
	}
	
	TWLModeInternalSDAccess = detectedTWLModeInternalSDAccess;
	setValueSafe((u32*)0x02FFDFE8, TWLModeInternalSDAccess); //arm9's FS_init(); go @ARM7_ARM9_DLDI_STATUS
	
	return DLDIARM7FSInitStatus;
}

#endif

const uint32  DLDI_MAGIC_NUMBER = 
	0xBF8DA5ED;	
	
// Stored backwards to prevent it being picked up by DLDI patchers
const sint8 DLDI_MAGIC_STRING_BACKWARDS [DLDI_MAGIC_STRING_LEN] =
	{'\0', 'm', 'h', 's', 'i', 'h', 'C', ' '} ;

int TWLModeInternalSDAccess = 0;

// 2/2 : once DLDI has been setup
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool dldi_handler_init() {	
	//NTR Mode
	#if defined(ARM9)
	struct DLDI_INTERFACE* dldiInit = dldiGet();
	#elif defined(ARM7) 
	struct DLDI_INTERFACE* dldiInit = (struct DLDI_INTERFACE*)DLDIARM7Address;
	#endif
	
	#if !defined(WIN32)
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
	return true;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void dldi_handler_deinit() {
	//DLDI & SDIO: Deinit
	sdio_ClearStatus();
	sdio_Shutdown();
	
	#ifdef ARM7
	struct DLDI_INTERFACE* dldiInit = dldiGet();	//ensures SLOT-1 / SLOT-2 is mapped to ARM7/ARM9 now
	dldiInit->ioInterface.clearStatus();
	dldiInit->ioInterface.shutdown();
	#endif
	
	#ifdef ARM9
	//send dldi deinit cmd		
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	fifomsg[27] = (uint32)0xFFFFFFFF;
	SendFIFOWords(TGDS_DLDI_ARM7_STATUS_DEINIT, 0);
	while(fifomsg[27] != (uint32)0){
		swiDelay(1);
	}
	#endif
}


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
bool dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer) {
	//DLDI Mode
	if(TWLModeInternalSDAccess == TWLModeDLDIAccessEnabledInternalSDDisabled){
		#ifdef ARM7
		struct DLDI_INTERFACE* dldiInterface = dldiGet();
		return dldiInterface->ioInterface.readSectors(sector, numSectors, buffer);
		#endif
		#ifdef ARM9
		void * targetMem = (void *)((int)&ARM7SharedDLDI[0]);
		//Get TWL RAM Setting
		u32 SFGEXT9 = *(u32*)0x04004008;
		//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
		u8 TWLRamSetting = (SFGEXT9 >> 14) & 0x3;
		if((TWLRamSetting == 0)||(TWLRamSetting == 1)){
			targetMem = (void *)((int)targetMem + 0x400000);
		}
		else{
			targetMem = (void *)((int)targetMem + 0x0A000000);
		}
		uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
		setValueSafe(&fifomsg[20], (uint32)sector);
		setValueSafe(&fifomsg[21], (uint32)numSectors);
		setValueSafe(&fifomsg[22], (uint32)targetMem);
		setValueSafe(&fifomsg[23], (uint32)IPC_READ_ARM7DLDI_REQBYIRQ);
		sendByteIPC(IPC_SEND_TGDS_CMD);
		while( ((uint32)getValueSafe(&fifomsg[23])) != ((uint32)0) ){
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
		return true;
		#endif
	}
	
	//SDIO Mode
	else if (TWLModeInternalSDAccess == TWLModeDLDIAccessDisabledInternalSDEnabled){
		#ifdef TWLMODE
		return sdio_ReadSectors(sector, numSectors, buffer);
		#endif		
	}
	return false;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
bool dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer) {
	//DLDI Mode
	if(TWLModeInternalSDAccess == TWLModeDLDIAccessEnabledInternalSDDisabled){
		#ifdef ARM7
		struct  DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DLDIARM7Address;
		return dldiInterface->ioInterface.writeSectors(sector, numSectors, buffer);
		#endif
		#ifdef ARM9
		void * targetMem = (void *)((int)&ARM7SharedDLDI[0]);
		//Get TWL RAM Setting
		u32 SFGEXT9 = *(u32*)0x04004008;
		//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
		u8 TWLRamSetting = (SFGEXT9 >> 14) & 0x3;
		if((TWLRamSetting == 0)||(TWLRamSetting == 1)){
			targetMem = (void *)((int)targetMem + 0x400000);
		}
		else{
			targetMem = (void *)((int)targetMem + 0x0A000000);
		}
		memcpy((uint16_t*)targetMem, (uint16_t*)buffer, (numSectors * 512));
		uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
		setValueSafe(&fifomsg[20], (uint32)sector);
		setValueSafe(&fifomsg[21], (uint32)numSectors);
		setValueSafe(&fifomsg[22], (uint32)targetMem);
		setValueSafe(&fifomsg[23], (uint32)IPC_WRITE_ARM7DLDI_REQBYIRQ);
		sendByteIPC(IPC_SEND_TGDS_CMD);
		while( ((uint32)getValueSafe(&fifomsg[23])) != ((uint32)0) ){
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
	
	//SDIO Mode
	else if (TWLModeInternalSDAccess == TWLModeDLDIAccessDisabledInternalSDEnabled){	
		#ifdef TWLMODE
		return sdio_WriteSectors(sector, numSectors, buffer);
		#endif		
	}
	return false;
}

const data_t dldiMagicString[12] = "\xED\xA5\x8D\xBF Chishm";	// Normal DLDI file
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
static inline addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
static inline void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
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
bool dldiRelocateLoader(u32 DldiRelocatedAddress, u32 dldiSourceInRam){
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
	pDH = (data_t*)DldiRelocatedAddress;
	pAH = (data_t *)dldiSourceInRam;
	dldiFileSize = 1 << pAH[DO_driverSize];
	
	// Copy the DLDI patch into the application
	memcpy((void *)pDH, (const void *)pAH, dldiFileSize);
	if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) {
		// No DLDI patch
		return false;
	}
	
	if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) {
		// Not enough space for patch
		return false;
	}
	
	u32 savedpAHDO_text_start = readAddr(pAH, DO_text_start);
	
	writeAddr (pAH, DO_text_start, (addr_t)DldiRelocatedAddress);
	writeAddr (pDH, DO_text_start, (addr_t)DldiRelocatedAddress);
	memOffset = DldiRelocatedAddress; //readAddr(pAH, DO_text_start);
	
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
	
	// Search through and fix pointers within the data section of the file
	for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
		if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
			writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
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
	
	writeAddr (pAH, DO_text_start, (addr_t)savedpAHDO_text_start);
	return true;
}

//arg0: binary data address to patch
//arg1: binary data size to patch
//arg2: Physical valid DLDI address (containing the DLDI binary to-be used as donor)
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
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
	
	if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) 
	{
		#ifdef ARM9
		nocashMessage("DLDI section not found in NTR binary. ");
		#endif
	}
	
	// Find the DLDI reserved space in the file
	patchOffset = quickFind(binData, dldiMagicString, binSize, sizeof(dldiMagicString));

	if (patchOffset < 0) {
		// does not have a DLDI section
		#ifdef ARM9
		nocashMessage("ERR: NO DLDI SECTION");
		#endif
		return false;
	}
	
	pAH = &(binData[patchOffset]);

	if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) 
	{
		// Not enough space for patch
		#ifdef ARM9
		nocashMessage("ERR: NOT ENOUGH SPACE");
		#endif
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
	#ifdef ARM9
	coherent_user_range_by_size((uint32)binData, (int)binSize); //Make ARM9 changes coherent
	#endif
	return true;
}

#ifdef TWLMODE
#ifdef ARM9
extern const struct DISC_INTERFACE_STRUCT __io_dsisd;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
const struct DISC_INTERFACE_STRUCT* get_io_dsisd (void) {
	return (isDSiMode() && __NDSHeader->unitCode ) ? &__io_dsisd : NULL;
}
#endif
#endif