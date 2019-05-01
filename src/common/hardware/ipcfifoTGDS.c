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

//TGDS IPC Version: 1.3

//Coto: Use them as you want , just make sure you read WELL the descriptions below.

#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "memoryHandleTGDS.h"
#include "notifierProcessor.h"
#include "timerTGDS.h"

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#endif

#ifdef ARM9
#include <stdbool.h>
#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm9.h"
#include "nds_cp15_misc.h"

#endif

//Software FIFO calls, Rely on Hardware FIFO calls so it doesnt matter if they are in different maps 
volatile int FIFO_SOFT_PTR = 0;

//useful for checking if something is pending
inline __attribute__((always_inline)) 
int GetSoftFIFOCount(){
	return FIFO_SOFT_PTR;
}

//GetSoftFIFO: Stores up to FIFO_NDS_HW_SIZE. Exposed to usercode for fetching up to 64 bytes (in 4 bytes each) sent from other core, until it returns false (empty buffer).
//Example: 
//uint32 n = 0;
//while(GetSoftFIFO(&n)== true){
//	//n has 4 bytes from the other ARM Core.
//}
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
bool GetSoftFIFO(uint32 * var){
	if(FIFO_SOFT_PTR >= 1){
		FIFO_SOFT_PTR--;
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		*var = (uint32)TGDSIPC->FIFO_BUF_SOFT[FIFO_SOFT_PTR];
		TGDSIPC->FIFO_BUF_SOFT[FIFO_SOFT_PTR] = (uint32)0;
		return true;
	}
	else
		return false;
}

//SetSoftFIFO == false means FULL
#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
//returns ammount of inserted uint32 blocks into FIFO hardware regs
inline __attribute__((always_inline)) 
bool SetSoftFIFO(uint32 value)
{
	if(FIFO_SOFT_PTR < (int)(FIFO_NDS_HW_SIZE/4)){
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		TGDSIPC->FIFO_BUF_SOFT[FIFO_SOFT_PTR] = value;
		FIFO_SOFT_PTR++;
		return true;
	}
	else
		return false;
}

//Software FIFO Handler receiver, add it wherever its required. (This is a FIFO that runs on software logic, rather than NDS FIFO).
//Supports multiple arguments, in FIFO format.
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Handle_SoftFIFORECV()
{
	uint32 msg = 0;
	while(GetSoftFIFO((uint32*)&msg) == true){
		//process incoming packages
		switch(msg){
			
			
		}
	}
}

//Software FIFO Sender. Send from external ARM Core, receive from the above function. Uses hardware FIFO.
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void SoftFIFOSEND(uint32 value0,uint32 value1,uint32 value2,uint32 value3){
	//todo: needs hardware IPC FIFO implementation.
}


//non blocking IPC FIFO
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void SendFIFOWords(uint32 data0, uint32 data1){
	REG_IPC_FIFO_TX = (uint32)data0;
	REG_IPC_FIFO_TX = (uint32)data1;
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void HandleFifoEmpty(){
	HandleFifoEmptyWeakRef((uint32)0,(uint32)0);
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void HandleFifoNotEmpty(){

	volatile uint32 data0 = 0,data1 = 0;
	while(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		data0 = REG_IPC_FIFO_RX;
		data1 = REG_IPC_FIFO_RX;
		
		//Do ToolchainGenericDS IPC handle here
		switch (data0) {
			//Shared 
			case((uint32)notifierProcessorRunThread):{
				//0 cmd: 1: index, 2: (u32)struct notifierDescriptor * getNotifierDescriptorByIndex(index)
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				
				uint32 * fifomsg = (uint32 *)&TGDSIPC->ipcmsg[0];
				int index = (int)fifomsg[0];
				struct notifierDescriptor * notifierDescriptorInst = (struct notifierDescriptor *)fifomsg[1];
				
				//run the thread here, grab message and acknowledge it
				struct notifierProcessorHandlerQueued notifierProcessorHandlerQueuedOut = RunNotifierProcessorThread(notifierDescriptorInst);
				SendFIFOWords(notifierProcessorRunAsyncAcknowledge, (uint32)fifomsg);	//acknowledge we just ran!: //0 cmd: 1: index, 2: (u32)struct notifierDescriptor * getNotifierDescriptorByIndex(index)
			}
			break;
			case(notifierProcessorRunAsyncAcknowledge):{
				//a thread async has ran! format: //0 cmd: 1: index, 2: (u32)struct notifierDescriptor * getNotifierDescriptorByIndex(index)
				//data0: cmd
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->ipcmsg[0];
				int index = (int)fifomsg[0];
				struct notifierDescriptor * notifierDescriptorInst = (struct notifierDescriptor *)fifomsg[1];
				fifomsg[0] = fifomsg[1] = 0;
				
				//printf("processor ran!");
			}
			break;
			// ARM7IO from ARM9
			//	||
			// ARM9IO from ARM7
			case((uint32)WRITE_EXTARM_8):{
				uint32* fifomsg = (uint32*)data1;		//data1 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[0];
				uint8 value = (uint8)((uint32)(fifomsg[1]&0xff));
				*(uint8*)address = (uint8)(value);
				fifomsg[1] = fifomsg[0] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_16):{
				uint32* fifomsg = (uint32*)data1;		//data1 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[0];
				uint16 value = (uint16)((uint32)(fifomsg[1]&0xffff));
				*(uint16*)address = (uint16)(value);
				fifomsg[1] = fifomsg[0] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_32):{
				uint32* fifomsg = (uint32*)data1;		//data1 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[0];
				uint32 value = (uint32)fifomsg[1];
				*(uint32*)address = (uint32)(value);
				fifomsg[1] = fifomsg[0] = 0;
			}
			break;
			
			case((uint32)WIFI_SYNC):{
				Wifi_Sync();
			}
			break;
			//Process the packages (signal) that sent earlier FIFO_SEND_EXT
			case((uint32)FIFO_SOFTFIFO_READ_EXT):{
			
			}
			break;
			case((uint32)FIFO_SOFTFIFO_WRITE_EXT):{
				SetSoftFIFO(data1);
			}
			break;
			
			//ARM7 command handler
			#ifdef ARM7
			case((uint32)FIFO_POWERCNT_ON):{
				powerON((uint16)data1);
			}
			break;
			case((uint32)FIFO_POWERCNT_OFF):{
				powerOFF((uint16)data1);
			}
			break;
			//Power Management: supported model so far: DS Phat.
			//Todo add: DSLite/DSi
			case((uint32)FIFO_POWERMGMT_WRITE):{
				
				uint32* fifomsg = (uint32*)data1;		//data1 == uint32 * fifomsg
				uint32 cmd = (uint32)fifomsg[0];
				uint32 flags = (uint32)fifomsg[1];
				fifomsg[1] = fifomsg[0] = 0;
				switch(cmd){
					//screen power write
					case(FIFO_SCREENPOWER_WRITE):{
						int PMBitsRead = PowerManagementDeviceRead((int)POWMAN_READ_BIT);
						PMBitsRead &= ~(POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT);
						PMBitsRead |= (int)(flags & (POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT));
						PowerManagementDeviceWrite(POWMAN_WRITE_BIT, (int)PMBitsRead);				
					}
					break;
				}
				
			}
			break;
			//arm9 wants to send a WIFI context block address / userdata is always zero here
			case((uint32)WIFI_INIT):{
				//	wifiAddressHandler( void * address, void * userdata )
				wifiAddressHandler((Wifi_MainStruct *)(uint32)data1, 0);
			}
			break;
			// Deinit WIFI
			case((uint32)WIFI_DEINIT):{
				DeInitWIFI();
			}
			break;
			#endif
			
			//ARM9 command handler
			#ifdef ARM9
			//exception handler from arm7
			case((uint32)EXCEPTION_ARM7):{
				if((uint32)data1 == (uint32)unexpectedsysexit_7){
					exception_handler((uint32)unexpectedsysexit_7);	//r0 = EXCEPTION_ARM7 / r1 = unexpectedsysexit_7
				}
			}
			break;
			//printf ability from ARM7
			case((uint32)FIFO_PRINTF_7):{
				clrscr();
				char * printfBuf7 = (char*)getPrintfBuffer();
				//Prevent Cache problems.
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				coherent_user_range_by_size((uint32)printfBuf7, (int)sizeof(TGDSIPC->arm7PrintfBuf));
				//printf("ARM7:%s",printfBuf7);
			}
			break;
			//IRQ_SCREENLID signal from ARM7
			case((uint32)FIFO_IRQ_SCREENLID_SIGNAL):{
				ScreenlidhandlerUser();
			}
			break;
			#endif
		}
		HandleFifoNotEmptyWeakRef(data0,data1);
	}
}


int getnotifierProcessorNewInstance(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	int freeIndex = TGDSIPC->notifierInternalIndex;	//this index == indexNotifierDescriptor;
	if(freeIndex < notifierProcessorInstancesTop){
		freeIndex+=1;
		TGDSIPC->notifierInternalIndex = freeIndex;
		return freeIndex - 1;
	}
	return notifierProcessorInstanceInvalid;
}

void deletenotifierProcessorInstance(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	int curIndex = TGDSIPC->notifierInternalIndex;
	if( (curIndex - 1) >= 0){
		curIndex-=1;
		TGDSIPC->notifierInternalIndex = curIndex;
	}
}