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

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#endif

#ifdef ARM9
#include <stdbool.h>

#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm9.h"

#endif

//Coto: Hardware IPC struct packed 
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
struct sIPCSharedTGDS* getsIPCSharedTGDS(){
	struct sIPCSharedTGDS* getsIPCSharedTGDSInst = (__attribute__((aligned (4))) struct sIPCSharedTGDS*)(getToolchainIPCAddress());
	return getsIPCSharedTGDSInst;
}

//Software FIFO calls, Rely on Hardware FIFO calls so it doesnt matter if they are in different maps 
volatile int FIFO_SOFT_PTR = 0;

//useful for checking if something is pending
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
bool GetSoftFIFO(uint32 * var)
{
	if(FIFO_SOFT_PTR >= 1){
		FIFO_SOFT_PTR--;
		*var = (uint32)getsIPCSharedTGDS()->FIFO_BUF_SOFT[FIFO_SOFT_PTR];
		getsIPCSharedTGDS()->FIFO_BUF_SOFT[FIFO_SOFT_PTR] = (uint32)0;
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
bool SetSoftFIFO(uint32 value)
{
	if(FIFO_SOFT_PTR < (int)(FIFO_NDS_HW_SIZE/4)){
		getsIPCSharedTGDS()->FIFO_BUF_SOFT[FIFO_SOFT_PTR] = value;
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
void SoftFIFOSEND(uint32 value0,uint32 value1,uint32 value2,uint32 value3){
	//todo: needs hardware IPC FIFO implementation.
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void SendMultipleWordACK(uint32 data0, uint32 data1, uint32 data2, uint32 * buffer_shared){
	SendMultipleWordByFifo(data0, data1, data2, buffer_shared);
}

//command0, command1, command2, buffer to pass to other core.
//IPC FIFO non blocking
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void SendMultipleWordByFifo(uint32 data0, uint32 data1, uint32 data2, uint32 * buffer_shared)
{
	REG_IPC_FIFO_TX = (uint32)data0;
	REG_IPC_FIFO_TX = (uint32)data1;			
	REG_IPC_FIFO_TX = (uint32)data2;
	REG_IPC_FIFO_TX = (uint32)(uint32*)buffer_shared;
	REG_IPC_FIFO_TX =	(uint32)FIFO_IPC_MESSAGE;
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmpty(){
	HandleFifoEmptyWeakRef((uint32)0,(uint32)0,(uint32)0,(uint32)0);
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmpty(){
	volatile uint32 data0 = 0,data1 = 0,data2 = 0,data3 = 0,data4 = 0;
	while(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		data0 = REG_IPC_FIFO_RX;
		data1 = REG_IPC_FIFO_RX;
		data2 = REG_IPC_FIFO_RX;
		data3 = REG_IPC_FIFO_RX;
		data4 = REG_IPC_FIFO_RX;	
		if((uint32)FIFO_IPC_MESSAGE == (uint32)data4){
			//Do ToolchainGenericDS IPC handle here
			switch (data0) {
				//Shared 
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
				case ((uint32)FIFO_POWERMGMT_WRITE):{
					PowerManagementDeviceWrite(PM_SOUND_AMP, (int)data1>>16);  // void * data == command2
				}
				break;
				//arm9 wants to send a WIFI context block address / userdata is always zero here
				case((uint32)WIFI_STARTUP):{
					//	wifiAddressHandler( void * address, void * userdata )
					wifiAddressHandler((Wifi_MainStruct *)(uint32)data1, 0);
				}
				break;
				#endif
				
				//ARM9 command handler
				#ifdef ARM9
				//exception handler: arm7
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
					coherent_user_range_by_size((uint32)printfBuf7, (int)sizeof(getsIPCSharedTGDS()->arm7PrintfBuf));
					printf("ARM7:%s",printfBuf7);
				}
				break;
				#endif
			}
			HandleFifoNotEmptyWeakRef(data0,data1,data2,data3);
		}
		//clear fifo inmediately
		REG_IPC_FIFO_CR |= (1<<3);
	}
}
