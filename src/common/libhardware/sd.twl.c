#include "typedefsTGDS.h"
#include "nand.h"
#include "dldi.h"
#include "utils.twl.h"
#include "debugNocash.h"
#include "ipcfifoTGDS.h"
#include "biosTGDS.h"

#ifdef TWLMODE
#include "sdmmc.h"
#endif

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
	
	#ifdef ARM7
	int ret = sdmmc_sd_startup();
	if(ret == 0){
		return true;
	}
	return false;
	#endif
	
	#ifdef ARM9
	nocashMessage("TGDS:sdio_Startup():TWL Mode: If this gets stuck, most likely you're running a TGDS NTR Binary in TWL Mode.");
	int retryCount = 0;
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[23], (uint32)IPC_STARTUP_ARM7_TWLSD_REQBYIRQ);
	sendByteIPC(IPC_SEND_TGDS_CMD);
	while( ((uint32)getValueSafe(&fifomsg[23])) != ((uint32)0) ){
		swiDelay(1);
		retryCount++;
		if(retryCount > 1048576){
			return false;
		}
	}
	return true;
	#endif
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
		#ifdef ARM7
		if(sdmmc_readsectors(&deviceSD, sector, numSectors, (void*)buffer) == 0){
			return true;
		}
		return false;
		#endif
		
		#ifdef ARM9
		void * targetMem = (void *)((int)&ARM7SharedDLDI[0]	+ 0x400000); //TWL uncached EWRAM
		uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
		setValueSafe(&fifomsg[20], (uint32)sector);
		setValueSafe(&fifomsg[21], (uint32)numSectors);
		setValueSafe(&fifomsg[22], (uint32)targetMem);
		setValueSafe(&fifomsg[23], (uint32)IPC_READ_ARM7_TWLSD_REQBYIRQ);
		sendByteIPC(IPC_SEND_TGDS_CMD);
		while( ((uint32)getValueSafe(&fifomsg[23])) != ((uint32)0) ){
			swiDelay(1);
		}
		memcpy((uint16_t*)buffer, (uint16_t*)targetMem, (numSectors * 512));
		coherent_user_range_by_size((uint32)buffer, (numSectors * 512)); //make coherent writes to dest buffer
		return ((bool)getValueSafe(&fifomsg[24]));
		#endif
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
		#ifdef ARM7
		if(sdmmc_writesectors(&deviceSD, sector, numSectors, (void*)buffer) == 0){
			return true;
		}
		return false;
		#endif
		
		#ifdef ARM9
		void * targetMem = (void *)((int)&ARM7SharedDLDI[0] + 0x400000); //TWL uncached EWRAM
		coherent_user_range_by_size((uint32)buffer, (numSectors * 512)); //make coherent reads from src buffer
		memcpy((uint16_t*)targetMem, (uint16_t*)buffer, (numSectors * 512));
		uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
		setValueSafe(&fifomsg[20], (uint32)sector);
		setValueSafe(&fifomsg[21], (uint32)numSectors);
		setValueSafe(&fifomsg[22], (uint32)targetMem);
		setValueSafe(&fifomsg[23], (uint32)IPC_WRITE_ARM7_TWLSD_REQBYIRQ);
		sendByteIPC(IPC_SEND_TGDS_CMD);
		while( ((uint32)getValueSafe(&fifomsg[23])) != ((uint32)0) ){
			swiDelay(1);
		}
		return ((bool)getValueSafe(&fifomsg[24]));
		#endif	
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
bool sdio_Shutdown() {
//---------------------------------------------------------------------------------
	#ifdef NTRMODE
	return false;
	#endif
	
	#ifdef TWLMODE
	return true;
	#endif
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