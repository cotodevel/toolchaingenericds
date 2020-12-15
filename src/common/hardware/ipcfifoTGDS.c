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


#include "global_settings.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "utilsTGDS.h"
#include "timerTGDS.h"
#include "dmaTGDS.h"
#include "eventsTGDS.h"
#include "ARM7FS.h"
#include "posixHandleTGDS.h"

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
#include "wifi_arm9.h"
#include "nds_cp15_misc.h"
#include "dldi.h"
#include "consoleTGDS.h"
#endif

void Write8bitAddrExtArm(uint32 address, uint8 value){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[54] = address;
	fifomsg[55] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_8, (uint32)fifomsg);
}

void Write16bitAddrExtArm(uint32 address, uint16 value){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[56] = address;
	fifomsg[57] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_16, (uint32)fifomsg);
}

void Write32bitAddrExtArm(uint32 address, uint32 value){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[58] = address;
	fifomsg[59] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_32, (uint32)fifomsg);
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
		
		//Process IPC FIFO commands
		switch (data1) {
			// ARM7IO from ARM9
			//	||
			// ARM9IO from ARM7
			case((uint32)WRITE_EXTARM_8):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[54];
				uint8 value = (uint8)((uint32)(fifomsg[55]&0xff));
				*(uint8*)address = (uint8)(value);
				fifomsg[55] = fifomsg[54] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_16):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[56];
				uint16 value = (uint16)((uint32)(fifomsg[57]&0xffff));
				*(uint16*)address = (uint16)(value);
				fifomsg[57] = fifomsg[56] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_32):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				uint32* address = (uint32*)fifomsg[58];
				uint32 value = (uint32)fifomsg[59];
				*(uint32*)address = (uint32)(value);
				fifomsg[59] = fifomsg[58] = 0;
			}
			break;
			
			case((uint32)WIFI_SYNC):{
				Wifi_Sync();
			}
			break;
			
			//ARM7 command handler
			#ifdef ARM7
			
			//todo: once tgds-audioplayer irqs work correctly.
			/*
			//Sound Player Context / Mic
			case ARM7COMMAND_SOUND_SETLEN:{
				sampleLen = (data0);
			}
			break;
			case ARM7COMMAND_SOUND_SETRATE:{
				sndRate = (data0);
			}
			break;
			case ARM7COMMAND_SOUND_SETMULT:{
				multRate = (data0);
			}
			break;
			case ARM7COMMAND_START_SOUND:{
				if((u32)data0 == (u32)SRC_WAV){
					setupSound(data0);
				}
				else{
					setupSoundUser(data0);
				}
			}
			break;
			case ARM7COMMAND_STOP_SOUND:{
				if((u32)data0 == (u32)SRC_WAV){
					stopSound(data0);
				}
				else{
					stopSoundUser(data0);
				}
			}
			break;
			case ARM7COMMAND_SOUND_COPY:
			{
				s16 *lbuf = NULL;
				s16 *rbuf = NULL;
				
				if(!sndCursor)
				{
					lbuf = strpcmL0;
					rbuf = strpcmR0;
				}
				else
				{
					lbuf = strpcmL1;
					rbuf = strpcmR1;
				}
				
				u32 i;
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				struct soundPlayerContext * soundPlayerCtx = (struct soundPlayerContext *)&TGDSIPC->sndPlayerCtx;
				int vMul = soundPlayerCtx->volume;
				int lSample = 0;
				int rSample = 0;
				s16 *arm9LBuf = soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data;
				s16 *arm9RBuf = soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data;
				
				switch(multRate)
				{
					case 1:{
						for(i=0;i<sampleLen;++i)
						{
							lSample = ((*arm9LBuf++) * vMul) >> 2;
							rSample = ((*arm9RBuf++) * vMul) >> 2;
							
							*lbuf++ = checkClipping(lSample);
							*rbuf++ = checkClipping(rSample);
						}
					}	
					break;
					case 2:{
						for(i=0;i<sampleLen;++i)
						{
							lSample = ((*arm9LBuf++) * vMul) >> 2;
							rSample = ((*arm9RBuf++) * vMul) >> 2;
							
							int midLSample = (lastL + lSample) >> 1;
							int midRSample = (lastR + rSample) >> 1;
							
							*lbuf++ = checkClipping(midLSample);
							*rbuf++ = checkClipping(midRSample);
							*lbuf++ = checkClipping(lSample);
							*rbuf++ = checkClipping(rSample);
							
							lastL = lSample;
							lastR = rSample;
						}
					}	
					break;
					case 4:{
						// unrolling this one out completely because it's soo much slower
						
						for(i=0;i<sampleLen;++i)
						{
							lSample = ((*arm9LBuf++) * vMul) >> 2;
							rSample = ((*arm9RBuf++) * vMul) >> 2;
							
							int midLSample = (lastL + lSample) >> 1;
							int midRSample = (lastR + rSample) >> 1;
							
							int firstLSample = (lastL + midLSample) >> 1;
							int firstRSample = (lastR + midRSample) >> 1;
							
							int secondLSample = (midLSample + lSample) >> 1;
							int secondRSample = (midRSample + rSample) >> 1;
							
							*lbuf++ = checkClipping(firstLSample);
							*rbuf++ = checkClipping(firstRSample);
							*lbuf++ = checkClipping(midLSample);
							*rbuf++ = checkClipping(midRSample);
							*lbuf++ = checkClipping(secondLSample);
							*rbuf++ = checkClipping(secondRSample);
							*lbuf++ = checkClipping(lSample);
							*rbuf++ = checkClipping(rSample);							
							
							lastL = lSample;
							lastR = rSample;							
						}
					}	
					break;
				}
			}
			break;
			*/
			case((uint32)TGDS_ARM7_ENABLESOUNDSAMPLECTX):{
				EnableSoundSampleContext((int)data0);
			}
			break;
			case((uint32)TGDS_ARM7_DISABLESOUNDSAMPLECTX):{
				DisableSoundSampleContext();
			}
			break;
			
			case((uint32)TGDS_ARM7_INITSTREAMSOUNDCTX):{
				//initSoundStream(data0);
			}
			break;
			
			//fifomsg[41] = fifomsg[40] = fifomsg[39]; freed. Available for upcoming stuff
			
			case((uint32)TGDS_ARM7_SETUPARMCoresMALLOC):{	//ARM7
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				u32 ARM7MallocStartaddress = (u32)fifomsg[42];
				u32 ARM7MallocSize = (u32)fifomsg[43];
				bool customAllocator = (bool)fifomsg[44];
				
				initARM7Malloc(ARM7MallocStartaddress, ARM7MallocSize);
				
				fifomsg[45] = fifomsg[44] = fifomsg[43] = fifomsg[42] = 0;
			}
			break;
			
			case((uint32)TGDS_ARM7_SETUPEXCEPTIONHANDLER):{
				exceptionArmRegsShared = (uint8*)data0;		//data0 == ARM9's exceptionArmRegs
				memset(exceptionArmRegsShared, 0, 0x20);	//same as exceptionArmRegs[0x20]
				setupDefaultExceptionHandler();	//ARM7 TGDS Exception Handler
			}
			break;
			
			case((uint32)TGDS_ARM7_PRINTF7SETUP):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				printfBufferShared = (u8*)fifomsg[46];
				arm7debugBufferShared = (u8*)fifomsg[47];
				arm7ARGVBufferShared = (int*)fifomsg[48];
				//ARM7 print debugger
				arm7ARGVDebugBufferShared = (int*)fifomsg[49];
				
				fifomsg[49] = fifomsg[48] = fifomsg[47] = fifomsg[46] = 0;
			}
			break;
			
			case((uint32)FIFO_INITSOUND):{
				initSound();
			}
			break;
			
			case((uint32)FIFO_PLAYSOUND):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg					
				int sampleRate = (uint32)fifomsg[50];
				u32* data = (u32*)fifomsg[51];
				u32 bytes = (uint32)fifomsg[52];
				u32 packedSnd = (uint32)fifomsg[53];
	
				u8 channel = (u8)((packedSnd >> 24)&0xff);
				u8 vol = (u8)((packedSnd >> 16)&0xff);
				u8 pan = (u8)((packedSnd >> 8)&0xff);
				u8 format = (u8)(packedSnd&0xff);
				
				//Try to play the sample through the specified channel
				s32 chan = isFreeSoundChannel(channel);
				if(chan != -1){ //means free channel / or channel is not auto (-1)
					startSoundSample(sampleRate, (const void*)data, bytes, chan, vol, pan, format);
				}
				//Otherwise, use a random alloc'd channel
				else{
					chan = getFreeSoundChannel();
					if (chan >= 0){
						startSoundSample(sampleRate, (const void*)data, bytes, chan, vol, pan, format);
					}
				}
				fifomsg[50] = 0;
				fifomsg[51] = 0;
				fifomsg[52] = 0;
				fifomsg[53] = 0;
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
				uint32 cmd = (uint32)fifomsg[60];
				uint32 flags = (uint32)fifomsg[61];
				switch(cmd){
					//screen power write
					case(FIFO_SCREENPOWER_WRITE):{
						int PMBitsRead = PowerManagementDeviceRead((int)POWMAN_READ_BIT);
						PMBitsRead &= ~(POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT);
						PMBitsRead |= (int)(flags & (POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT));
						PowerManagementDeviceWrite(POWMAN_WRITE_BIT, (int)PMBitsRead);				
					}
					break;
					
					//Shutdown NDS hardware
					case(FIFO_SHUTDOWN_DS):{
						int PMBitsRead = PowerManagementDeviceRead((int)POWMAN_READ_BIT);
						PMBitsRead |= (int)(POWMAN_SYSTEM_PWR_BIT);
						PowerManagementDeviceWrite(POWMAN_WRITE_BIT, (int)PMBitsRead);				
					}
					break;
				}
				fifomsg[61] = fifomsg[60] = 0;
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
			/*
			case ARM9COMMAND_UPDATE_BUFFER:{
				updateSoundContextStreamPlayback((u32)data0);
			}	
			break;
			*/
			case((uint32)TGDS_ARM7_DETECTTURNOFFCONSOLE):{
				detectAndTurnOffConsole();
			}
			break;
			
			//ARM7 FS: read from ARM9 POSIX filehandle to ARM7
			case(TGDS_ARM7_ARM7FSREAD):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				u8* readbuf = (u8*)fifomsg[0];
				int readBufferSize = (int)fifomsg[1];
				int fileOffset = (int)fifomsg[2];
				switch(ARM7FS_HandleMethod){
					case(TGDS_ARM7FS_FILEHANDLEPOSIX):{
						int readSoFar = ARM7FS_ReadBuffer_ARM9CallbackPOSIX(readbuf, fileOffset, ARM7FS_TGDSFileDescriptorRead, readBufferSize);	//UpdateDPG_Audio();
					}
					break;
					case(TGDS_ARM7FS_TGDSFILEDESCRIPTOR):{
						int readSoFar = ARM7FS_ReadBuffer_ARM9TGDSFD(readbuf, fileOffset, ARM7FS_TGDSFileDescriptorRead, readBufferSize);
					}
					break;
				}
				coherent_user_range_by_size((uint32)readbuf, readBufferSize);
				fifomsg[3] = fifomsg[2] = fifomsg[1] = fifomsg[0] = 0;
			}
			break;
			
			//ARM7 FS: write from ARM7 to ARM9 POSIX filehandle
			case(TGDS_ARM7_ARM7FSWRITE):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				u8* readbuf = (u8*)fifomsg[4];
				int writeBufferSize = (int)fifomsg[5];
				int fileOffset = (int)fifomsg[6];
				coherent_user_range_by_size((uint32)readbuf, writeBufferSize);
				switch(ARM7FS_HandleMethod){
					case(TGDS_ARM7FS_FILEHANDLEPOSIX):{
						if(ARM7FS_TGDSFileDescriptorWrite != NULL){
							int writtenSoFar = ARM7FS_SaveBuffer_ARM9CallbackPOSIX(readbuf, fileOffset, ARM7FS_TGDSFileDescriptorWrite, writeBufferSize);
						}
					}
					break;
					case(TGDS_ARM7FS_TGDSFILEDESCRIPTOR):{
						if(ARM7FS_TGDSFileDescriptorWrite != NULL){
							int writtenSoFar = ARM7FS_SaveBuffer_ARM9TGDSFD(readbuf, fileOffset, ARM7FS_TGDSFileDescriptorWrite, writeBufferSize);
						}
					}
					break;
				}
				fifomsg[7] = fifomsg[6] = fifomsg[5] = fifomsg[4] = 0;
			}
			break;
			
			case((uint32)TGDS_ARM7_PRINTF7):{
				uint32* fifomsg = (uint32*)data0;		//data0 == uint32 * fifomsg
				u8 * printfBufferShared = (u8 *)fifomsg[36];		//uint32 * printfBufferShared
				int * arm7ARGVBufferShared = (int *)fifomsg[37];
				int argvCount = (int)fifomsg[38];
				fifomsg[38] = fifomsg[37] = fifomsg[36] = 0;
				printf7(printfBufferShared, arm7ARGVBufferShared, argvCount);
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
			
			//ARM7: Exception Handler
			case((uint32)EXCEPTION_ARM7):{
				switch((uint32)data0){
					case(generalARM7Exception):{
						exception_handler((uint32)generalARM7Exception);
					}
					break;
					case(unexpectedsysexit_7):{
						exception_handler((uint32)unexpectedsysexit_7);
					}
					break;
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

//Note: u32* srcMemory must be in EWRAM in both cases

//Allows to read (EWRAM) memory from source ARM Core to destination ARM Core; IRQ Safe and blocking
//u32 * targetMemory == EWRAM Memory source buffer to copy -FROM- u32 * srcMemory
//u32 * srcMemory == External ARM Core Base Address
void ReadMemoryExt(u32 * srcMemory, u32 * targetMemory, int bytesToRead){
	dmaFillWord(0, 0, (uint32)targetMemory, (uint32)bytesToRead);
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[28] = (uint32)srcMemory;
	fifomsg[29] = (uint32)targetMemory;
	fifomsg[30] = (uint32)bytesToRead;
	fifomsg[31] = (uint32)TGDS_ARM7_ARM7FSREAD;
	sendByteIPC(IPC_ARM7READMEMORY_REQBYIRQ);
	while((uint32)fifomsg[31] == (uint32)TGDS_ARM7_ARM7FSREAD){
		swiDelay(2);
	}
	#ifdef ARM9
	coherent_user_range_by_size((uint32)targetMemory, (sint32)bytesToRead);
	#endif
}

//Allows to save (EWRAM) memory from source ARM Core to destination ARM Core; IRQ Safe and blocking
//u32 * targetMemory == EWRAM Memory source buffer to copy -TO- u32 * srcMemory
//u32 * srcMemory == External ARM Core Base Address
void SaveMemoryExt(u32 * srcMemory, u32 * targetMemory, int bytesToRead){
	#ifdef ARM9
	coherent_user_range_by_size((uint32)targetMemory, (sint32)bytesToRead);
	#endif
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[32] = (uint32)srcMemory;
	fifomsg[33] = (uint32)targetMemory;
	fifomsg[34] = (uint32)bytesToRead;
	fifomsg[35] = (uint32)TGDS_ARM7_ARM7FSWRITE;
	sendByteIPC(IPC_ARM7SAVEMEMORY_REQBYIRQ);
	while((uint32)fifomsg[35] == (uint32)TGDS_ARM7_ARM7FSWRITE){
		swiDelay(2);
	}
}

void ReadFirmwareARM7Ext(u32 * srcMemory){	//512 bytes src always
	memset(srcMemory, 0, (uint32)512);
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[28] = (uint32)srcMemory;
	//fifomsg[29] = (uint32)targetMemory;
	//fifomsg[30] = (uint32)bytesToRead;
	fifomsg[31] = (uint32)TGDS_ARM7_READFLASHMEM;
	sendByteIPC(IPC_READ_FIRMWARE_REQBYIRQ);
	while((uint32)fifomsg[31] == (uint32)TGDS_ARM7_READFLASHMEM){
		swiDelay(2);
	}
}