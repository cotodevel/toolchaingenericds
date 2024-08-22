
/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

//TGDS required version: IPC Version: 1.3

//IPC FIFO Description: 
//		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 														// Access to TGDS internal IPC FIFO structure. 		(ipcfifoTGDS.h)
//		struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;		// Access to TGDS Project (User) IPC FIFO structure	(ipcfifoTGDSUser.h)

#include "ipcfifoTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "biosTGDS.h"
#include "loader.h"
#include "dmaTGDS.h"
#include "libutilsShared.h"
#include "microphoneShared.h"

#ifdef ARM7
#include <string.h>

#include "main.h"
#include "wifi_arm7.h"
#include "spifwTGDS.h"

#endif

#ifdef ARM9

#include <stdbool.h>
#include "main.h"
#include "wifi_arm9.h"
#include "dswnifi_lib.h"

#endif

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
struct sIPCSharedTGDSSpecific* getsIPCSharedTGDSSpecific(){
	struct sIPCSharedTGDSSpecific* sIPCSharedTGDSSpecificInst = (struct sIPCSharedTGDSSpecific*)(TGDSIPCUserStartAddress);
	return sIPCSharedTGDSSpecificInst;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void HandleFifoNotEmptyWeakRef(uint32 cmd1, uint32 cmd2){
	switch (cmd1) {
		//NDS7: 
		#ifdef ARM7
		
		#endif
		
		//NDS9: 
		#ifdef ARM9
		
		#endif
	}
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2){
}

//project specific stuff

#ifdef ARM9

void updateStreamCustomDecoder(u32 srcFrmt){

}

void freeSoundCustomDecoder(u32 srcFrmt){

}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void fcopy(FILE *f1, FILE *f2){
    char            buffer[BUFSIZ];
    size_t          n;
    while ((n = fread(buffer, sizeof(char), sizeof(buffer), f1)) > 0){
        if (fwrite(buffer, sizeof(char), n, f2) != n){
            
		}
    }
}

#endif

//Libutils setup: TGDS project doesn't use any libutils extensions.

//Note: IGNORELIBS flag exists because default ARM7 TGDS-multiboot payload has remoteboot, and requires DSWIFI
void setupLibUtils(){
	//libutils:
	
	//Stage 0
	#ifdef ARM9
	initializeLibUtils9(
		(HandleFifoNotEmptyWeakRefLibUtils_fn)&libUtilsFIFONotEmpty, //ARM7 & ARM9
		(timerWifiInterruptARM9LibUtils_fn)&Timer_50ms, //ARM9 
		(SoundStreamStopSoundStreamARM9LibUtils_fn)&stopSoundStream,	//ARM9: bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType)
		(SoundStreamUpdateSoundStreamARM9LibUtils_fn)&updateStream, //ARM9: void updateStream() 
		(wifiDeinitARM7ARM9LibUtils_fn)&DeInitWIFI, //ARM7 & ARM9: DeInitWIFI()
		(wifiswitchDsWnifiModeARM9LibUtils_fn)&switch_dswnifi_mode //ARM9: bool switch_dswnifi_mode(sint32 mode)
	);
	#endif
	
	//Stage 1
	#ifdef ARM7
	initializeLibUtils7(
		(HandleFifoNotEmptyWeakRefLibUtils_fn)&libUtilsFIFONotEmpty, //ARM7 & ARM9
		(wifiUpdateVBLANKARM7LibUtils_fn)&Wifi_Update, //ARM7
		(wifiInterruptARM7LibUtils_fn)&Wifi_Interrupt,  //ARM7
		(SoundStreamTimerHandlerARM7LibUtils_fn)&TIMER1Handler, //ARM7: void TIMER1Handler()
		(SoundStreamStopSoundARM7LibUtils_fn)&stopSound, 	//ARM7: void stopSound()
		(SoundStreamSetupSoundARM7LibUtils_fn)&setupSound,	//ARM7: void setupSound()
		(initMallocARM7LibUtils_fn)&initARM7Malloc, //ARM7: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);
		(wifiDeinitARM7ARM9LibUtils_fn)&DeInitWIFI,  //ARM7 & ARM9: DeInitWIFI()
		(MicInterruptARM7LibUtils_fn)&micInterrupt, //ARM7: micInterrupt()
		(DeInitWIFIARM7LibUtils_fn)&DeInitWIFI, //ARM7: DeInitWIFI()
		(wifiAddressHandlerARM7LibUtils_fn)&wifiAddressHandler	//ARM7: void wifiAddressHandler( void * address, void * userdata )
	);
	#endif
}