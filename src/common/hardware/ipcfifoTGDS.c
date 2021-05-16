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
#include "posixHandleTGDS.h"
#include "biosTGDS.h"
#include "libndsFIFO.h"
#include "loader.h"	//TGDS-multiboot reload NDS Binary ability
#include "dldi.h"

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#include "spiTGDS.h"
#include "spifwTGDS.h"
#include "spitscTGDS.h"
#include "powerTGDS.h"
#include "soundTGDS.h"
#endif

#ifdef ARM9
#include <stdbool.h>
#include "dsregs.h"
#include "dsregs_asm.h"
#include "wifi_arm9.h"
#include "nds_cp15_misc.h"
#include "consoleTGDS.h"
#endif

//arg 0: channel
//arg 1: arg0: handler, arg1: userdata
u32 fifoFunc[FIFO_CHANNELS][2];	//context is only passed on callback prototype stage, because, the channel index generates the callee callback

void Write8bitAddrExtArm(uint32 address, uint8 value) __attribute__ ((optnone)) {
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[54] = address;
	fifomsg[55] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_8);
}

void Write16bitAddrExtArm(uint32 address, uint16 value) __attribute__ ((optnone)) {
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[56] = address;
	fifomsg[57] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_16);
}

void Write32bitAddrExtArm(uint32 address, uint32 value) __attribute__ ((optnone)) {
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[58] = address;
	fifomsg[59] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_32);
}

//Async FIFO Sender
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void SendFIFOWords(uint32 data0) __attribute__ ((optnone)) {	//format: arg0: cmd, arg1: value
	REG_IPC_FIFO_TX = (uint32)data0;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmpty() __attribute__ ((optnone)) {
	HandleFifoEmptyWeakRef((uint32)0,(uint32)0);
}
	
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmpty() __attribute__ ((optnone)) {
	volatile uint32 data0 = 0;	
		
	while(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		//Process IPC FIFO commands
		data0 = REG_IPC_FIFO_RX;
		switch (data0) {
			// ARM7IO from ARM9
			//	||
			// ARM9IO from ARM7
			case((uint32)WRITE_EXTARM_8):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				uint32* address = (uint32*)fifomsg[54];
				uint8 value = (uint8)((uint32)(fifomsg[55]&0xff));
				*(uint8*)address = (uint8)(value);
				fifomsg[55] = fifomsg[54] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_16):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				uint32* address = (uint32*)fifomsg[56];
				uint16 value = (uint16)((uint32)(fifomsg[57]&0xffff));
				*(uint16*)address = (uint16)(value);
				fifomsg[57] = fifomsg[56] = 0;
			}
			break;
			case((uint32)WRITE_EXTARM_32):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
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
			case(ARM7COMMAND_RELOADNDS):{
				runBootstrapARM7();	//ARM7 Side
			}
			break;
			
			case (TGDS_ARM7_RELOADFLASH):{
				//Init Shared Address Region and get NDS Heade
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				
				memcpy((u8*)&TGDSIPC->DSHeader,(u8*)0x027FFE00, sizeof(TGDSIPC->DSHeader));
				
				//Read DHCP settings (in order)
				LoadFirmwareSettingsFromFlash();
				
				//Hardware ARM7 Init
				u8 DSHardwareReadFromFlash = TGDSIPC->DSFWHEADERInst.stub[0x1d];
				resetMemory_ARMCores(DSHardwareReadFromFlash);
				IRQInit(DSHardwareReadFromFlash);
				
				//Init SoundSampleContext
				initSoundSampleContext();
				initSound();
				setValueSafe(&fifomsg[58], (u32)0);
			}
			break;
			
			case ARM7COMMAND_START_SOUND:{
				setupSound();
			}
			break;
			case ARM7COMMAND_STOP_SOUND:{
				stopSound();
			}
			break;
			case ARM7COMMAND_SOUND_SETRATE:{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				sndRate = fifomsg[60];
			}
			break;
			case ARM7COMMAND_SOUND_SETLEN:{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				sampleLen = fifomsg[61];
			}
			break;
			case ARM7COMMAND_SOUND_SETMULT:{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				multRate = fifomsg[62];
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
				int vMul = TGDSIPC->soundIPC.volume;
				int lSample = 0;
				int rSample = 0;
				s16 *arm9LBuf = TGDSIPC->soundIPC.arm9L;
				s16 *arm9RBuf = TGDSIPC->soundIPC.arm9R;
				
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
				VblankUser();
			}
			break;
			case ARM7COMMAND_SOUND_DEINTERLACE:
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
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
				s16 *iSrc = TGDSIPC->soundIPC.interlaced;
				u32 i = 0;
				int vMul = TGDSIPC->soundIPC.volume;
				int lSample = 0;
				int rSample = 0;
				
				switch(multRate)
				{
					case 1:{
						if(TGDSIPC->soundIPC.channels == 2)
						{
							for(i=0;i<sampleLen;++i)
							{					
								lSample = *iSrc++;
								rSample = *iSrc++;
								
								*lbuf++ = checkClipping((lSample * vMul) >> 2);
								*rbuf++ = checkClipping((rSample * vMul) >> 2);
							}
						}
						else
						{
							for(i=0;i<sampleLen;++i)
							{					
								lSample = *iSrc++;
								
								lSample = checkClipping((lSample * vMul) >> 2);
								
								*lbuf++ = lSample;
								*rbuf++ = lSample;
							}
						}
					}	
					break;
					case 2:{
						for(i=0;i<sampleLen;++i)
						{					
							if(TGDSIPC->soundIPC.channels == 2)
							{
								lSample = *iSrc++;
								rSample = *iSrc++;
							}
							else
							{
								lSample = *iSrc++;
								rSample = lSample;
							}
							
							lSample = ((lSample * vMul) >> 2);
							rSample = ((rSample * vMul) >> 2);
							
							int midLSample = (lastL + lSample) >> 1;
							int midRSample = (lastR + rSample) >> 1;
							
							lbuf[(i << 1)] = checkClipping(midLSample);
							rbuf[(i << 1)] = checkClipping(midRSample);
							lbuf[(i << 1) + 1] = checkClipping(lSample);
							rbuf[(i << 1) + 1] = checkClipping(rSample);
							
							lastL = lSample;
							lastR = rSample;							
						}
					}	
					break;
					case 4:{
						for(i=0;i<sampleLen;++i)
						{				
							if(TGDSIPC->soundIPC.channels == 2)
							{
								lSample = *iSrc++;
								rSample = *iSrc++;
							}
							else
							{
								lSample = *iSrc++;
								rSample = lSample;
							}
							
							lSample = ((lSample * vMul) >> 2);
							rSample = ((rSample * vMul) >> 2);
							
							int midLSample = (lastL + lSample) >> 1;
							int midRSample = (lastR + rSample) >> 1;
							
							int firstLSample = (lastL + midLSample) >> 1;
							int firstRSample = (lastR + midRSample) >> 1;
							
							int secondLSample = (midLSample + lSample) >> 1;
							int secondRSample = (midRSample + rSample) >> 1;
							
							lbuf[(i << 2)] = checkClipping(firstLSample);
							rbuf[(i << 2)] = checkClipping(firstRSample);
							lbuf[(i << 2) + 1] = checkClipping(midLSample);
							rbuf[(i << 2) + 1] = checkClipping(midRSample);
							lbuf[(i << 2) + 2] = checkClipping(secondLSample);
							rbuf[(i << 2) + 2] = checkClipping(secondRSample);
							lbuf[(i << 2) + 3] = checkClipping(lSample);
							rbuf[(i << 2) + 3] = checkClipping(rSample);							
							
							lastL = lSample;
							lastR = rSample;							
						}
					}	
					break;
				}
				VblankUser();
			}
			break;
			case ARM7COMMAND_PSG_COMMAND:
			{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
				SCHANNEL_CR(TGDSIPC->soundIPC.psgChannel) = TGDSIPC->soundIPC.cr;
				SCHANNEL_TIMER(TGDSIPC->soundIPC.psgChannel) = TGDSIPC->soundIPC.timer;
			}
			break;
			
			case((uint32)TGDS_ARM7_ENABLESOUNDSAMPLECTX):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				EnableSoundSampleContext((int)fifomsg[60]);
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
			
			case((uint32)TGDS_ARM7_SETUPARMCPUMALLOCANDDLDI):{	//ARM7
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				u32 ARM7MallocStartaddress = (u32)getValueSafe(&fifomsg[42]);
				u32 ARM7MallocSize = (u32)getValueSafe(&fifomsg[43]);
				//bool customAllocator = (bool)getValueSafe(&fifomsg[44]);
				u32 dldiStartAddress = (u32)getValueSafe(&fifomsg[45]);
				u32 ARM7DLDISetting = (u32)getValueSafe(&fifomsg[46]);
				if(ARM7DLDISetting == TGDS_ARM7DLDI_ENABLED){
					ARM7DLDIEnabled = true;
				}
				else{
					ARM7DLDIEnabled = false;
				}
				
				if(ARM7DLDIEnabled == true){
					//init DLDI 7 Here
					DLDIARM7Address = (u32*)dldiStartAddress;
					bool DLDIARM7InitStatus = dldi_handler_init();
					if(DLDIARM7InitStatus == true){
						//setValueSafe(&fifomsg[45], (uint32)0xFAFAFAFA);
						//after this (if ret status true) it's safe to call dldi read and write sectors from ARM9 (ARM7 DLDI mode)
					}
					else{
						//setValueSafe(&fifomsg[45], (uint32)0xFCFCFCFC);
					}
					initARM7Malloc(ARM7MallocStartaddress, ARM7MallocSize);
				}
				
				setValueSafe(&fifomsg[42], (uint32)0);
				setValueSafe(&fifomsg[43], (uint32)0);
				setValueSafe(&fifomsg[44], (uint32)0);
				setValueSafe(&fifomsg[46], (uint32)0);
				setValueSafe(&fifomsg[45], (uint32)0);
			}
			break;
			
			case((uint32)TGDS_ARM7_SETUPEXCEPTIONHANDLER):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				exceptionArmRegsShared = (uint8*)fifomsg[60];		//data0 == ARM9's exceptionArmRegs
				memset(exceptionArmRegsShared, 0, 0x20);	//same as exceptionArmRegs[0x20]
				setupDefaultExceptionHandler();	//ARM7 TGDS Exception Handler
			}
			break;
			
			case((uint32)TGDS_ARM7_PRINTF7SETUP):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
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
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
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
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				powerON((uint16)fifomsg[60]);
			}
			break;
			case((uint32)FIFO_POWERCNT_OFF):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				powerOFF((uint16)fifomsg[60]);
			}
			break;
			//Power Management: 
				//Supported mode(s): NTR
			case((uint32)FIFO_POWERMGMT_WRITE):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
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
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				//	wifiAddressHandler( void * address, void * userdata )
				wifiAddressHandler((Wifi_MainStruct *)fifomsg[60], 0);
			}
			break;
			// Deinit WIFI
			case((uint32)WIFI_DEINIT):{
				DeInitWIFI();
			}
			break;
			
			case((uint32)ARM7COMMAND_RELOADARM7):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				u32 ARM7Entrypoint = getValueSafe(&fifomsg[64]);
				reloadARMCore(ARM7Entrypoint);
			}
			break;
			
			case(TGDS_DLDI_ARM7_STATUS_DEINIT):{
				dldi_handler_deinit();
			}
			break;
			
			case TGDS_ARM7_ENABLE_SLEEPMODE_TIMEOUT:{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				setTurnOffScreensTimeout((int)fifomsg[60]);
			}
			break;
			
			case TGDS_ARM7_SET_EVENT_HANDLING:{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				TGDSSetEvent((int)fifomsg[60]);
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
			case ARM9COMMAND_UPDATE_BUFFER:{
				updateRequested = true;
					
				// check for formats that can handle not being on an interrupt (better stability)
				// (these formats are generally decoded faster)
				switch(soundData.sourceFmt)
				{
					case SRC_MP3:
						// mono sounds are slower than stereo for some reason
						// so we force them to update faster
						if(soundData.channels != 1)
							return;
						
						break;
					case SRC_WAV:
					case SRC_FLAC:
					case SRC_STREAM_MP3:
					case SRC_STREAM_AAC:
					case SRC_SID:
						// these will be played next time it hits in the main screen
						// theres like 4938598345 of the updatestream checks in the 
						// main code
						return;
				}
				
				// call immediately if the format needs it
				updateStream();
			}	
			break;
			case((uint32)TGDS_ARM7_DETECTTURNOFFCONSOLE):{
				detectAndTurnOffConsole();
			}
			break;
			
			case((uint32)TGDS_ARM7_PRINTF7):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				u8 * printfBufferShared = (u8 *)fifomsg[36];		//uint32 * printfBufferShared
				int * arm7ARGVBufferShared = (int *)fifomsg[37];
				int argvCount = (int)fifomsg[38];
				fifomsg[38] = fifomsg[37] = fifomsg[36] = 0;
				printf7(printfBufferShared, arm7ARGVBufferShared, argvCount);
			}
			break;
			
			//ARM7: Exception Handler
			case((uint32)EXCEPTION_ARM7):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				switch((uint32)fifomsg[60]){
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
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];				
				int curChannelFreed = (int)fifomsg[60];
				flushSoundContext(curChannelFreed);
			}
			break;
			
			#endif
			
			//Handle Libnds FIFO receive handlers
			case(TGDS_LIBNDSFIFO_COMMAND):{
				int channel = (int)receiveByteIPC();
				sendByteIPCNOIRQ((uint8)0);	//clean
				//Run: FifoDatamsgHandlerFunc newhandler -> arg: void * userdata by a given channel				
				//arg 0: channel
				//arg 1: arg0: handler, arg1: userdata
				FifoHandlerFunc fn = (FifoHandlerFunc)fifoFunc[channel][0];
				if((int)fn != 0){
					fn(fifoCheckDatamsgLength(channel), fifoFunc[channel][1]);
				}
			}
			break;
			
		}
		HandleFifoNotEmptyWeakRef(data0);	//this one follows: cmd, value order
	}
}

#ifdef ARM7

static s32 xscale, yscale;
static s32 xoffset, yoffset;
static u8 last_time_touched = 0;

static int LastTSCPosX = 0;
static int LastTSCPosY = 0;

__attribute__ ((noinline))
struct xyCoord readTSC()  __attribute__ ((optnone)) {
	struct xyCoord tscCoords;
	//Handle Touchscreen
	//Set Chip Select LOW to invoke the command & Transmit the instruction byte: TSC CNT Differential Mode: X Raw TSC 
	REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_TSCCNT | BIT_SPICLK_1MHZ;
	volatile uint8 resultx4to0 = RWSPICNT(BIT_TSCCNT_START_CTRL|BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL| BIT_TSCCNT_REFSEL_DIFFERENTIAL | BIT_TSCCNT_CONVMODE_12bit | BIT_TSCCNT_TOUCHXPOS);
	volatile uint8 resultx11to5 = RWSPICNT(0);	//0-11-10-9-8-7-6-5
	volatile uint16 read_raw_x = ((resultx11to5 & 0x7F) << 5) | (resultx4to0 & 0x1F);
	SPICSHIGH();
	swiDelay(111);
	//Set Chip Select LOW to invoke the command & Transmit the instruction byte: TSC CNT Differential Mode: Y Raw TSC 
	REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_TSCCNT | BIT_SPICLK_1MHZ;
	volatile uint8 resulty4to0 = RWSPICNT(BIT_TSCCNT_START_CTRL|BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL| BIT_TSCCNT_REFSEL_DIFFERENTIAL | BIT_TSCCNT_CONVMODE_12bit | BIT_TSCCNT_TOUCHYPOS);
	volatile uint8 resulty11to5 = RWSPICNT(0);	//4-3-2-1-0-0-0-0
	volatile uint16 read_raw_y = ((resulty11to5 & 0x7F) << 5) | (resulty4to0 & 0x1F);
	SPICSHIGH();
	swiDelay(111);
	tscCoords.x = read_raw_x;
	tscCoords.y = read_raw_y;
	return tscCoords;
}

__attribute__ ((noinline))
void XYReadScrPos(struct XYTscPos * StouchScrPosInst)  __attribute__ ((optnone)) {
	struct xyCoord coord = readTSC();		
	uint16 read_raw_x = coord.x;
	uint16 read_raw_y = coord.y;
	
	//Touchscreen Position (pixel TFT X Y Coordinates conversion)
	//Read the X and Y positions in 12bit differential mode, then convert the touchscreen values (adc) to screen/pixel positions (scr), as such:
	//scr.x = (adc.x-adc.x1) * (scr.x2-scr.x1) / (adc.x2-adc.x1) + (scr.x1-1)
	//scr.y = (adc.y-adc.y1) * (scr.y2-scr.y1) / (adc.y2-adc.y1) + (scr.y1-1)
	struct sIPCSharedTGDS * sIPCSharedTGDSInst = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
	struct sDSFWSETTINGS * DSFWSettingsInst = (struct sDSFWSETTINGS *)&sIPCSharedTGDSInst->DSFWSETTINGSInst;
	
	uint16 adc_x1 = (((DSFWSettingsInst->tsc_adcposx1y112bit[1] << 8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx1y112bit[0];
	uint16 adc_y1 = (((DSFWSettingsInst->tsc_adcposx1y112bit[3] << 8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx1y112bit[2];
	
	uint8 scr_x1  = (DSFWSettingsInst->tsc_tsccalx1y18bit[0]);
	uint8 scr_y1  = (DSFWSettingsInst->tsc_tsccalx1y18bit[1]);
	
	uint16 adc_x2 = (((DSFWSettingsInst->tsc_adcposx2y212bit[1]<<8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx2y212bit[0];
	uint16 adc_y2 = (((DSFWSettingsInst->tsc_adcposx2y212bit[3]<<8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx2y212bit[2];
	
	uint8 scr_x2  = (DSFWSettingsInst->tsc_tsccalx2y28bit[0]);
	uint8 scr_y2  = (DSFWSettingsInst->tsc_tsccalx2y28bit[1]);
	
	//sint32 scrx = (read_raw_x-adc_x1) * (scr_x2-scr_x1) / (adc_x2-adc_x1) + (scr_x1-1);
	//sint32 scry = (read_raw_y-adc_y1) * (scr_y2-scr_y1) / (adc_y2-adc_y1) + (scr_y1-1);
	
	xscale = ((scr_x2 - scr_x1) << 19) / ((adc_x2) - (adc_x1));
	yscale = ((scr_y2 - scr_y1) << 19) / ((adc_y2) - (adc_y1));

	xoffset = ((adc_x1 + adc_x2) * xscale  - ((scr_x1 + scr_x2) << 19) ) / 2;
	yoffset = ((adc_y1 + adc_y2) * yscale  - ((scr_y1 + scr_y2) << 19) ) / 2;

	s16 px = ( read_raw_x * xscale - xoffset + xscale/2 ) >>19;
	s16 py = ( read_raw_y * yscale - yoffset + yscale/2 ) >>19;

	if ( px < 0) px = 0;
	if ( py < 0) py = 0;
	if ( px > (SCREEN_WIDTH -1)) px = SCREEN_WIDTH -1;
	if ( py > (SCREEN_HEIGHT -1)) py = SCREEN_HEIGHT -1;
	
	//TFT x/y pixel
	StouchScrPosInst->rawx    = read_raw_x;
	StouchScrPosInst->touchXpx = px;
	StouchScrPosInst->rawy    = read_raw_y;
	StouchScrPosInst->touchYpx = py;
	
	LastTSCPosX = px;
	LastTSCPosY = py;
	
	//todo? maybe we don't need them for UI controls
	StouchScrPosInst->z1   =   0;
	StouchScrPosInst->z2   =   0;
}

#endif

//Requires VCOUNT irq calls
void XYReadScrPosUser(struct XYTscPos * StouchScrPosInst)  __attribute__ ((optnone)) {
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
	StouchScrPosInst->rawx    = TGDSIPC->rawx;
	StouchScrPosInst->touchXpx = TGDSIPC->touchXpx;
	StouchScrPosInst->rawy    = TGDSIPC->rawy;
	StouchScrPosInst->touchYpx = TGDSIPC->touchYpx;
	StouchScrPosInst->z1   =   TGDSIPC->touchZ1;
	StouchScrPosInst->z2   =   TGDSIPC->touchZ2;
}

//Note: u32* srcMemory must be in EWRAM in both cases

//Allows to read (EWRAM) memory from source ARM Core to destination ARM Core; IRQ Safe and blocking
//u32 * targetMemory == EWRAM Memory source buffer to copy -FROM- u32 * srcMemory
//u32 * srcMemory == External ARM Core Base Address
void ReadMemoryExt(u32 * srcMemory, u32 * targetMemory, int bytesToRead){
	dmaFillWord(0, 0, (uint32)targetMemory, (uint32)bytesToRead);
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[28], (uint32)srcMemory);
	setValueSafe(&fifomsg[29], (uint32)targetMemory);
	setValueSafe(&fifomsg[30], (uint32)bytesToRead);
	setValueSafe(&fifomsg[31], (uint32)0xFFFFFFFF);
	sendByteIPC(IPC_ARM7READMEMORY_REQBYIRQ);
	while((uint32)fifomsg[31] != (uint32)0){
		swiDelay(1);
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
	setValueSafe(&fifomsg[32], (uint32)srcMemory);
	setValueSafe(&fifomsg[33], (uint32)targetMemory);
	setValueSafe(&fifomsg[34], (uint32)bytesToRead);
	setValueSafe(&fifomsg[35], (uint32)0xFFFFFFFF);
	sendByteIPC(IPC_ARM7SAVEMEMORY_REQBYIRQ);
	while((u32)getValueSafe(&fifomsg[35]) != (u32)0){
		swiDelay(1);
	}
}

void ReadFirmwareARM7Ext(u32 * srcMemory){	//512 bytes src always
	memset(srcMemory, 0, (uint32)512);
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
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

#ifdef ARM9
//Reloads ARM7 Flash memory and returns DS hardware model
u8 ARM7ReloadFlashSync(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[58], (u32)0xFFFFFFFF);
	SendFIFOWords(TGDS_ARM7_RELOADFLASH);
	while(getValueSafe(&fifomsg[58]) != 0){
		swiDelay(2);
	}
	coherent_user_range_by_size((uint32)&TGDSIPC->DSFWHEADERInst.stub[0], 32);
	return TGDSIPC->DSFWHEADERInst.stub[0x1d];
}
#endif