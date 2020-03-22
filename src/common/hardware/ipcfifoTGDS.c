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

#include "global_settings.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "utilsTGDS.h"
#include "timerTGDS.h"
#include "dldi.h"
#include "dmaTGDS.h"
#include "eventsTGDS.h"

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "soundTGDS.h"
#include "posixHandleTGDS.h"
#endif

#ifdef ARM9
#include <stdbool.h>
#include "dsregs.h"
#include "dsregs_asm.h"
#include "wifi_arm9.h"
#include "nds_cp15_misc.h"
#endif

//IPC
void sendMultipleByteIPC(uint8 inByte0, uint8 inByte1, uint8 inByte2, uint8 inByte3){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint8 * ipcMsg = (uint8 *)&TGDSIPC->ipcMesaggingQueue[0];
	ipcMsg[0] = (u8)inByte0;
	ipcMsg[1] = (u8)inByte1;
	ipcMsg[2] = (u8)inByte2;
	ipcMsg[3] = (u8)inByte3;
	sendByteIPC(IPC_SEND_MULTIPLE_CMDS);
}


//Async FIFO Sender
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void SendFIFOWords(uint32 data0, uint32 data1){	//format: arg0: cmd, arg1: value
	REG_IPC_FIFO_TX = (uint32)data1;	
	REG_IPC_FIFO_TX = (uint32)data0;	//last message should always be command
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void HandleFifoEmpty(){
	HandleFifoEmptyWeakRef((uint32)0,(uint32)0);
}
	
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void HandleFifoNotEmpty(){
	volatile uint32 data0 = 0, data1 = 0;
	while(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		data0 = (u32)REG_IPC_FIFO_RX;
		data1 = (u32)REG_IPC_FIFO_RX;
		//Do ToolchainGenericDS IPC handle here
		switch (data1) {
			// ARM7IO from ARM9
			//	||
			// ARM9IO from ARM7
			case((uint32)WRITE_EXTARM_8):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[0];
				uint8 value = (uint8)((uint32)(fifomsg[1]&0xff));
				*(uint8*)address = (uint8)(value);
				fifomsg[1] = fifomsg[0] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_16):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[0];
				uint16 value = (uint16)((uint32)(fifomsg[1]&0xffff));
				*(uint16*)address = (uint16)(value);
				fifomsg[1] = fifomsg[0] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_32):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
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
			
			//ARM7 command handler
			#ifdef ARM7
			
			case((uint32)TGDS_ARM7_PRINTF7SETUP):{
				printfBufferShared = (u8*)data0;	//data0 == uint32 * printfBufferShared
			}
			break;
			
			case((uint32)FIFO_INITSOUND):{
				initSound();
			}
			break;
			
			case((uint32)FIFO_PLAYSOUND):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg					
				int sampleRate = (uint32)fifomsg[0];
				u32* data = (u32*)fifomsg[1];
				u32 bytes = (uint32)fifomsg[2];
				u32 packedSnd = (uint32)fifomsg[3];
	
				u8 channel = (u8)((packedSnd >> 24)&0xff);
				u8 vol = (u8)((packedSnd >> 16)&0xff);
				u8 pan = (u8)((packedSnd >> 8)&0xff);
				u8 format = (u8)(packedSnd&0xff);
				
				//Try to play the sample through the specified channel
				s32 chan = isFreeSoundChannel(channel);
				if(chan != -1){ //means free channel / or channel is not auto (-1)
					startSound(sampleRate, (const void*)data, bytes, chan, vol, pan, format);
				}
				//Otherwise, use a random alloc'd channel
				else{
					chan = getFreeSoundChannel();
					if (chan >= 0){
						startSound(sampleRate, (const void*)data, bytes, chan, vol, pan, format);
					}
				}
				fifomsg[0] = 0;
				fifomsg[1] = 0;
				fifomsg[2] = 0;
				fifomsg[3] = 0;
			}
			break;
			
			case((uint32)FIFO_POWERCNT_ON):{
				powerON((uint16)data0);
			}
			break;
			case((uint32)FIFO_POWERCNT_OFF):{
				powerOFF((uint16)data0);
			}
			break;
			//Power Management: 
				//Supported mode(s): NTR
			case((uint32)FIFO_POWERMGMT_WRITE):{
				
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
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
				wifiAddressHandler((Wifi_MainStruct *)(uint32)data0, 0);
			}
			break;
			// Deinit WIFI
			case((uint32)WIFI_DEINIT):{
				DeInitWIFI();
			}
			break;
			
			//ARM7 DLDI implementation
			#ifdef ARM7_DLDI
			case(TGDS_DLDI_ARM7_STATUS_DEINIT):{
				dldi_handler_deinit();
			}
			break;
			#endif
			
			case TGDS_ARM7_ENABLE_SLEEPMODE_TIMEOUT:{
				setTurnOffScreensTimeout((int)data0);
			}
			break;
			
			case TGDS_ARM7_SET_EVENT_HANDLING:{
				TGDSSetEvent((int)data0);
			}
			break;
			
			case TGDS_ARM7_ENABLE_EVENT_HANDLING:{
				enableTGDSEventHandling();
			}
			break;
			
			case TGDS_ARM7_DISABLE_EVENT_HANDLING:{
				disableTGDSEventHandling();
			}
			break;
			
			case TGDS_ARM7_TURNON_BACKLIGHT:{
				TurnOnScreens();
			}
			break;
			case TGDS_ARM7_TURNOFF_BACKLIGHT:{
				TurnOffScreens();
			}
			break;
			case TGDS_ARM7_DISABLE_SLEEPMODE:{
				disableSleepMode();
			}
			break;
			case TGDS_ARM7_ENABLE_SLEEPMODE:{
				enableSleepMode();
			}
			break;
			
			#endif
			
			//ARM9 command handler
			#ifdef ARM9
			
			case((uint32)TGDS_ARM7_PRINTF7):{
				printf((sint8*)data0);		//data0 == uint32 * printfBufferShared
			}
			break;
			
				//ARM7 DLDI implementation
				#ifdef ARM7_DLDI
				case(TGDS_DLDI_ARM7_INIT_OK):{
					//printf("DLDI 7 INIT OK!");
				}
				break;
				
				case(TGDS_DLDI_ARM7_INIT_ERROR):{
					//printf("DLDI 7 INIT ERROR!");
				}
				break;
				
				case(TGDS_DLDI_ARM7_STATUS_INIT):{
					
				}
				break;
				#endif
			
			//exception handler from arm7
			case((uint32)EXCEPTION_ARM7):{
				if((uint32)data0 == (uint32)unexpectedsysexit_7){
					exception_handler((uint32)unexpectedsysexit_7);	//r0 = EXCEPTION_ARM7 / r1 = unexpectedsysexit_7
				}
			}
			break;
			//LID signaling open (ARM7 is hw triggered, but here, ARM9 is soft-triggered)
			case((uint32)FIFO_IRQ_LIDHASOPENED_SIGNAL):{
				screenLidHasOpenedhandlerUser();
			}
			break;
			case((uint32)FIFO_IRQ_LIDHASCLOSED_SIGNAL):{
				screenLidHasClosedhandlerUser();
			}
			break;
			
			case((uint32)FIFO_FLUSHSOUNDCONTEXT):{
				int curChannelFreed = (int)data0;
				flushSoundContext(curChannelFreed);
			}
			break;
			
			#endif
		}
		HandleFifoNotEmptyWeakRef(data1, data0);	//this one follows: cmd, value order
	}
}

#ifdef ARM9
//Note: Must be in EWRAM
//Allows to read any memory mapped from ARM7 to ARM9 directly; IRQ Safe and blocking (ARM9 issues call -> ARM7 does it -> ARM9 receives memory)
void ReadMemoryExt(u32 * srcMemory, u32 * targetMemory, int bytesToRead){
	coherent_user_range_by_size((uint32)targetMemory, (sint32)bytesToRead);
	dmaFillWord(3, 0, (uint32)targetMemory, (uint32)bytesToRead);
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (uint32)srcMemory;
	fifomsg[1] = (uint32)targetMemory;
	fifomsg[2] = (uint32)bytesToRead;
	fifomsg[7] = (uint32)ARM7READMEMORY_BUSY;
	sendByteIPC(IPC_ARM7READMEMORY_REQBYIRQ);
	while(fifomsg[7] == ARM7READMEMORY_BUSY){
		swiDelay(2);
	}
}
#endif