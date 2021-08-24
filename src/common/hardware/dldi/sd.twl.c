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
bool sdio_Startup() __attribute__ ((optnone)) {
//---------------------------------------------------------------------------------
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	nocashMessage("TGDS:sdio_Startup():TWL Mode: If this gets stuck, most likely you're running a TGDS NTR Binary in TWL Mode.");
	//int retryCount = 0;
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[23], (uint32)0xABCDABCD);
	sendByteIPC(IPC_STARTUP_ARM7_TWLSD_REQBYIRQ);
	while(getValueSafe(&fifomsg[23]) == (uint32)0xABCDABCD){
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
bool sdio_IsInserted() __attribute__ ((optnone)) {
//---------------------------------------------------------------------------------
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	return true;
	#endif
}

//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
bool sdio_ReadSectors(sec_t sector, sec_t numSectors,void* buffer) __attribute__ ((optnone)) {
//---------------------------------------------------------------------------------	
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	void * targetMem = (void *)((int)&ARM7SharedDLDI[0]	+ 0x400000); //TWL uncached EWRAM
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafeInt(&fifomsg[20], sector);
	setValueSafeInt(&fifomsg[21], numSectors);
	setValueSafe(&fifomsg[22], (uint32)targetMem);
	setValueSafe(&fifomsg[23], (uint32)0xFFAAFFAA);
	SendFIFOWords(FIFO_READ_TWLSD_REQBYIRQ);
	while(getValueSafe(&fifomsg[23]) == (uint32)0xFFAAFFAA){
		swiDelay(333);
	}
	memcpy((uint16_t*)buffer, (uint16_t*)targetMem, (numSectors * 512));
	coherent_user_range_by_size((uint32)buffer, (numSectors * 512)); //make coherent writes to dest buffer
	return true;
	#endif
}

//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
bool sdio_WriteSectors(sec_t sector, sec_t numSectors, const void* buffer) __attribute__ ((optnone)) {
//---------------------------------------------------------------------------------
	#ifdef NTRMODE
	return false;
	#endif
	#ifdef TWLMODE
	void * targetMem = (void *)((int)&ARM7SharedDLDI[0] + 0x400000); //TWL uncached EWRAM
	coherent_user_range_by_size((uint32)buffer, (numSectors * 512)); //make coherent reads from src buffer
	memcpy((uint16_t*)targetMem, (uint16_t*)buffer, (numSectors * 512));
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[24], (uint32)sector);
	setValueSafe(&fifomsg[25], (uint32)numSectors);
	setValueSafe(&fifomsg[26], (uint32)targetMem);
	setValueSafe(&fifomsg[27], (uint32)0xFFBBCCAA);
	sendByteIPC(IPC_WRITE_ARM7_TWLSD_REQBYIRQ);
	while(getValueSafe(&fifomsg[27]) == (uint32)0xFFBBCCAA){
		swiDelay(1);
	}
	return true;
	#endif
}


//---------------------------------------------------------------------------------
bool sdio_ClearStatus() __attribute__ ((optnone)) {
//---------------------------------------------------------------------------------
	return true;
}

//---------------------------------------------------------------------------------
bool sdio_Shutdown() __attribute__ ((optnone)) {
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