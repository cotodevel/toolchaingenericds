#ifndef __DLDI_H__
#define __DLDI_H__

#include <string.h>
#include "typedefsTGDS.h"
#define FEATURE_MEDIUM_CANREAD		0x00000001
#define FEATURE_MEDIUM_CANWRITE		0x00000002
#define FEATURE_SLOT_GBA			0x00000010
#define FEATURE_SLOT_NDS			0x00000020

typedef uint32_t sec_t;

typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(sec_t sector, sec_t numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(sec_t sector, sec_t numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;

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

typedef signed int addr_t;
typedef unsigned char data_t;

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

#define LOAD_DEFAULT_NDS 0
#define FIX_ALL						0x01
#define FIX_GLUE					0x02
#define FIX_GOT						0x04
#define FIX_BSS						0x08
#define DEVICE_TYPE_DLDI 0x49444C44

#define DLDI_MAGIC_STRING_LEN 		8
#define DLDI_FRIENDLY_NAME_LEN 		48

// I/O interface with DLDI extensions
struct DLDI_INTERFACE {
	uint32 	magicNumber;
	char	magicString [DLDI_MAGIC_STRING_LEN];
	uint8		versionNumber;
	uint8		driverSize;			// log-2 of driver size in bytes
	uint8		fixSectionsFlags;
	uint8		allocatedSize;		// log-2 of the allocated space in bytes

	char	friendlyName [DLDI_FRIENDLY_NAME_LEN];
	
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
};

extern u32 * DLDIARM7Address;

static inline addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static inline void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

static inline addr_t quickFindVRAM(const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
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

#ifdef __cplusplus
extern "C" {
#endif

extern bool dldi_handler_init();
extern void dldi_handler_deinit();

//ARM7DLDI:
//ARM7: ARM7 physical DLDI target location address
//ARM9: Uses this as a pointer to: ARM7 physical DLDI target location address

//ARM9DLDI:
//ARM7: NULL ptr
//ARM9: Global Physical DLDI section (rather than &_dldi_start, since it's discarded at TGDS init)
extern u32 * getDLDIARM7Address();				//	/
extern void setDLDIARM7Address(u32 * address);	//	| Must be defined/standardized by the TGDS project at runtime. This way we ensure IWRAM 64K compatibility + DLDI at ARM7
extern void initDLDIARM7(u32 srcDLDIAddr);		//	/

//ARM7 DLDI implementation
#ifdef ARM7_DLDI
	#ifdef ARM9
	extern void ARM7DLDIInit(u32 targetDLDI7Address);
	extern void ARM9DeinitDLDI();
	#endif
#endif

#ifdef ARM7
extern void ARM7DLDIInit();
#endif

//DldiRelocatedAddress == target DLDI relocated address
//dldiSourceInRam == physical DLDI section having a proper DLDI driver used as donor 
//dldiOutWriteAddress == new physical DLDI out buffer, except, relocated to a new DldiRelocatedAddress!
extern bool dldiRelocateLoader(bool clearBSS, u32 DldiRelocatedAddress, u32 dldiSourceInRam, u32 dldiOutWriteAddress);

#ifdef __cplusplus
}
#endif

extern struct DLDI_INTERFACE _io_dldi_stub;
static inline struct DLDI_INTERFACE* dldiGet(void) {
	struct DLDI_INTERFACE * dldiInterface = &_io_dldi_stub;
	return (struct DLDI_INTERFACE*)dldiInterface;
}

extern const uint32  DLDI_MAGIC_NUMBER;	
extern const data_t dldiMagicString[12];	// Normal DLDI file
extern const data_t dldiMagicLoaderString[12];	// Different to a normal DLDI file

#endif
