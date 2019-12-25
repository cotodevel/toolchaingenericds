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
#include "utilsTGDS.h"
#include "timerTGDS.h"

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "soundTGDS.h"
#endif

#ifdef ARM9
#include <stdbool.h>
#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm9.h"
#include "nds_cp15_misc.h"

#endif

//IPC
void sendByteIPC(uint8 inByte){
	REG_IPC_SYNC = ((REG_IPC_SYNC&0xfffff0ff) | (inByte<<8) | (1<<13) );	// (1<<13) Send IRQ to remote CPU      (0=None, 1=Send IRQ)
}


uint8 receiveByteIPC(){
	return (REG_IPC_SYNC&0xf);
}

void idleIPC(){
	sendByteIPC(0x0);
}

//Async FIFO Sender
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void SendFIFOWords(uint32 data0, uint32 data1){	//format: arg0: cmd, arg1: value
	REG_IPC_FIFO_TX = (uint32)data0;	//last message should always be command
	REG_IPC_FIFO_TX = (uint32)data1;
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
		switch (data0) {
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
			
			//ARM7 command handler
			#ifdef ARM7
			case((uint32)FIFO_INITSOUND):{
				initSound();
			}
			break;
			
			case((uint32)FIFO_PLAYSOUND):{
				uint32* fifomsg = (uint32*)data1;		//data1 == uint32 * fifomsg					
				int sampleRate = (uint32)fifomsg[0];
				u32* data = (uint32)fifomsg[1];
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
				powerON((uint16)data1);
			}
			break;
			case((uint32)FIFO_POWERCNT_OFF):{
				powerOFF((uint16)data1);
			}
			break;
			//Power Management: 
				//Supported mode(s): NTR
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
			//LID signaling open (ARM7 is hw triggered, but here, ARM9 is soft-triggered)
			case((uint32)FIFO_IRQ_LIDHASOPENED_SIGNAL):{
				screenLidHasOpenedhandlerUser();
			}
			break;
			case((uint32)FIFO_IRQ_LIDHASCLOSED_SIGNAL):{
				screenLidHasClosedhandlerUser();
			}
			break;
			
			//Process the packages (signal) that sent earlier FIFO_SEND_EXT
			case((uint32)READ_EXTARM_IPC):{
				//take orders only if we have one
				struct sSharedSENDCtx * ctx = (struct sSharedSENDCtx *)data1;
				if(getSENDCtxStatus(ctx) == READ_EXTARM_IPC_BUSY){
					REG_IME = 0;
					memcpy((u8*)ctx->targetAddr, (u8*)ctx->srcAddr, ctx->size);
					setSENDCtxStatus(READ_EXTARM_IPC_READY, ctx);
					REG_IME = 1;
				}
			}
			break;
			
			case((uint32)FIFO_FLUSHSOUNDCONTEXT):{
				int curChannelFreed = (int)data1;
				flushSoundContext(curChannelFreed);
			}
			break;
			
			#endif
			
		}
		HandleFifoNotEmptyWeakRef(data0, data1);	//this one follows: cmd, value order
	}
}


//targetAddr == the output buffer : bufStart + bufSize will get written to
//bufStart must be in EWRAM
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void SendBufferThroughFIFOIrqsAsync(u32 bufStart, u32 targetAddr, int bufSize, struct sSharedSENDCtx * ctx){		//todo, put some callback here to read the chunk
	setSENDCtx(bufStart, targetAddr, bufSize, ctx);
	setSENDCtxStatus(READ_EXTARM_IPC_BUSY, ctx); 
	SendFIFOWords(READ_EXTARM_IPC, (u32)ctx);
	#ifdef ARM9
	coherent_user_range_by_size(targetAddr, bufSize);
	#endif
	
	while(getSENDCtxStatus(ctx) == READ_EXTARM_IPC_BUSY){
		//some irq wait here...
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void setSENDCtxStatus(int val, struct sSharedSENDCtx * ctx){
	#ifdef ARM9
	coherent_user_range_by_size((u32)ctx, sizeof(struct sSharedSENDCtx));
	#endif
	ctx->status = val;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
int getSENDCtxStatus(struct sSharedSENDCtx * ctx){
	return ctx->status;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void setSENDCtx(u32 srcAddr, u32 targetAddr, int Size, struct sSharedSENDCtx * ctx){
	memset((u8*)ctx, 0, sizeof(struct sSharedSENDCtx));
	//must be in ewram
	ctx->targetAddr = targetAddr;
	//relative to ARM processor
	ctx->srcAddr = srcAddr;
	ctx->size = Size;
}
