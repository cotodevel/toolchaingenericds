#ifdef ARM9
#include "typedefsTGDS.h"
#include "nand.h"
#include "dldi.h"
#include "libndsFIFO.h"
#include "utils.twl.h"
#include "nds_cp15_misc.h"
#include "debugNocash.h"

///////////////////////////////////TWL mode SD ARM9i DLDI Access///////////////////////////////////
//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool sdio_Startup() {
//---------------------------------------------------------------------------------
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	nocashMessage("TGDS:sdio_Startup():TWL Mode: If this gets stuck, most likely you're running a TGDS NTR Binary in TWL Mode.");
	//int retryCount = 0;
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	fifomsg[23] = (uint32)0xABCDABCD;
	sendByteIPC(IPC_STARTUP_ARM7_TWLSD_REQBYIRQ);
	while(fifomsg[23] == (uint32)0xABCDABCD){
		swiDelay(1);
		/*retryCount++;	//make it fail (slide) when either TWL SD Card failed to init due to: hardware SD failure, or TWL Mode reloading binaries (TGDS-MB TWL), 
						//as this layer ignores binary embedded DLDI, using the TWL one.
		
		if(retryCount > 9000){
			return false;
		}*/
	}
	return true;
	#endif
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool sdio_IsInserted() {
//---------------------------------------------------------------------------------
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	return true;
	#endif
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
__attribute__((section(".itcm")))
bool sdio_ReadSectors(sec_t sector, sec_t numSectors,void* buffer) {
//---------------------------------------------------------------------------------	
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	void * targetMem = (void *)((int)&ARM7SharedDLDI[0]	+ 0x400000); //TWL uncached EWRAM
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	fifomsg[20] = sector;
	fifomsg[21] = numSectors;
	fifomsg[22] = (uint32)targetMem;
	fifomsg[23] = (uint32)0xFFAAFFAA;
	sendByteIPC(IPC_READ_ARM7_TWLSD_REQBYIRQ);
	while(fifomsg[23] == (uint32)0xFFAAFFAA){
		swiDelay(333);
	}
	memcpy((uint16_t*)buffer, (uint16_t*)targetMem, (numSectors * 512));
	coherent_user_range_by_size((uint32)buffer, (numSectors * 512)); //make coherent writes to dest buffer
	return true;
	#endif
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
__attribute__((section(".itcm")))
bool sdio_WriteSectors(sec_t sector, sec_t numSectors, const void* buffer) {
//---------------------------------------------------------------------------------
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	void * targetMem = (void *)((int)&ARM7SharedDLDI[0] + 0x400000); //TWL uncached EWRAM
	coherent_user_range_by_size((uint32)buffer, (numSectors * 512)); //make coherent reads from src buffer
	memcpy((uint16_t*)targetMem, (uint16_t*)buffer, (numSectors * 512));
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	fifomsg[24] = (uint32)sector;
	fifomsg[25] = (uint32)numSectors;
	fifomsg[26] = (uint32)targetMem;
	fifomsg[27] = (uint32)0xFFBBCCAA;
	sendByteIPC(IPC_WRITE_ARM7_TWLSD_REQBYIRQ);
	while(fifomsg[27] == (uint32)0xFFBBCCAA){
		swiDelay(1);
	}
	return true;
	#endif
}


//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool sdio_ClearStatus() {
//---------------------------------------------------------------------------------
	return true;
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool sdio_Shutdown() {
//---------------------------------------------------------------------------------
	return true;
}


//__io_dsisd is mapped to DLDI format. Thus DLDI calls must be called from __io_dsisd in TWLmode rather than default DLDI
const struct DISC_INTERFACE_STRUCT __io_dsisd = {
	DEVICE_TYPE_DSI_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&sdio_Startup,
	(FN_MEDIUM_ISINSERTED)&sdio_IsInserted,
	(FN_MEDIUM_READSECTORS)&sdio_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&sdio_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&sdio_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&sdio_Shutdown
};

#endif