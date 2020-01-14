#include "dldi.h"
#include "typedefsTGDS.h"
#include "dmaTGDS.h"
#include "global_settings.h"
#include "busTGDS.h"
#include "ipcfifoTGDS.h"
#include <string.h>

#ifdef ARM7
#endif

#ifdef ARM9
#include "nds_cp15_misc.h"
#endif

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


//ARM7 DLDI implementation
#ifdef ARM7_DLDI

struct sTGDSDLDIARM7Context * getsTGDSDLDIARM7Context(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((u32)&TGDSIPC->dldi7TGDSCtx, sizeof(struct sTGDSDLDIARM7Context));
	#endif
	return (struct sTGDSDLDIARM7Context *)(&TGDSIPC->dldi7TGDSCtx);
}

//Stage 1
//ARM9 only allowed to init TGDS DLDI @ ARM7.
//Trigger comes from ARM7
#ifdef ARM9

bool dldiARM7InitStatus = false;

void TGDSDLDIARM7SetupStage1(u32 targetDLDI7Address){
	struct sTGDSDLDIARM7Context * sharedDLDIInitCtx = getsTGDSDLDIARM7Context();
	memset(sharedDLDIInitCtx, 0, sizeof(struct sTGDSDLDIARM7Context)); 
	
	//Perform relocation, and pass the DLDI context to ARM7 Init code
	u8* relocatedARM7DLDIBinary = (u8*)malloc(16*1024);
	
	//DldiRelocatedAddress == target DLDI relocated address
	//dldiSourceInRam == physical DLDI section having a proper DLDI driver used as donor 
	//dldiOutWriteAddress == new physical DLDI out buffer, except, relocated to a new DldiRelocatedAddress!
	bool clearBSS = false;
	bool status = dldiRelocateLoader(clearBSS, targetDLDI7Address, (u32)&_dldi_start, (u32)( (u32)(relocatedARM7DLDIBinary) | 0x400000));
	if(status == true){
		//OK
	}
	else{
		while(1==1);	//Error
	}
	
	sharedDLDIInitCtx->DLDISourceAddress = (u32)relocatedARM7DLDIBinary;
	sharedDLDIInitCtx->DLDISize = 16*1024;
	sharedDLDIInitCtx->dldiCmdSharedCtx = NULL;
	setDLDIInitStatus(TGDS_DLDI_ARM7_STATUS_STAGE1);
	while(getDLDIInitStatus() == TGDS_DLDI_ARM7_STATUS_STAGE1){
		swiDelay(1);	//This delay is required!
	}
	free((u8*)relocatedARM7DLDIBinary);
}

void ARM9DeinitDLDI(){
	SendFIFOWords(TGDS_DLDI_ARM7_STATUS_DEINIT, (u32)0);
}

#endif

//Stage 0
#ifdef ARM7
void TGDSDLDIARM7SetupStage0(u32 targetAddrDLDI7){
	setDLDIARM7Address((u32*)targetAddrDLDI7);
	setDLDIInitStatus(TGDS_DLDI_ARM7_STATUS_STAGE0);
	SendFIFOWords(TGDS_DLDI_ARM7_STATUS_INIT, (u32)targetAddrDLDI7);
	while(getDLDIInitStatus() == TGDS_DLDI_ARM7_STATUS_STAGE0){
		swiDelay(1);	//This delay is required!
	}
	
	//TGDS_DLDI_ARM7_STATUS_BUSY_STAGE1 means DLDI code was relocated.
	//Stage 1:
	struct sTGDSDLDIARM7Context * sharedDLDIInitCtx = getsTGDSDLDIARM7Context();
	memcpy((u8*)targetAddrDLDI7, (u8*)sharedDLDIInitCtx->DLDISourceAddress, (int)sharedDLDIInitCtx->DLDISize);
	
	setDLDIInitStatus(TGDS_DLDI_ARM7_STATUS_STAGE0);	//free ARM9
	//Init DLDI
	if(dldi_handler_init() == true){
		SendFIFOWords(TGDS_DLDI_ARM7_INIT_OK, 0);
		//while(1);
	}
	else{
		SendFIFOWords(TGDS_DLDI_ARM7_INIT_ERROR, 0);
		while(1);
	}
}
#endif

#endif

///////////////////////////////////////////////////////////////////////////DLDI ARM7 CODE START/////////////////////////////////////////////////////////////////////

//ARM7 DLDI implementation
#ifdef ARM7_DLDI

#ifdef ARM7
u32 * DLDIARM7Address = NULL;

void setDLDIARM7Address(u32 * address){
	DLDIARM7Address = address;
}

u32 * getDLDIARM7Address(){
	return DLDIARM7Address;
}
#endif
 
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

void dldi_handler_deinit(){
	struct DLDI_INTERFACE* dldiInit = dldiGet();	//ensures SLOT-1 / SLOT-2 is mapped to ARM7/ARM9 now
	dldiInit->ioInterface.clearStatus();
	dldiInit->ioInterface.shutdown();
}


#ifdef ARM9

static const uint32  DLDI_MAGIC_NUMBER = 0xBF8DA5ED;	
static const data_t dldiMagicString[12] = "\xED\xA5\x8D\xBF Chishm";	// Normal DLDI file
static const data_t dldiMagicLoaderString[12] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

//DldiRelocatedAddress == target DLDI relocated address
//dldiSourceInRam == physical DLDI section having a proper DLDI driver used as donor 
//dldiOutWriteAddress == new physical DLDI out buffer, except, relocated to a new DldiRelocatedAddress!
bool dldiRelocateLoader(bool clearBSS, u32 DldiRelocatedAddress, u32 dldiSourceInRam, u32 dldiOutWriteAddress){
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
	pDH = (data_t*)dldiOutWriteAddress;
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

//ARM7 DLDI implementation
#ifdef ARM7_DLDI

#ifdef ARM9
struct sTGDSDLDIARM7DLDICmd sTGDSDLDIARM7DLDICmdSharedRead;
struct sTGDSDLDIARM7DLDICmd sTGDSDLDIARM7DLDICmdSharedWrite;

u8 ARM7DLDIBuf[64*512];	//Up to 64KB per cluster, should allow 64K and below 
#endif

#endif

////////////////////////////////////////////// DLDI ARM 7 CODE end ////////////////////////////////////////////// 