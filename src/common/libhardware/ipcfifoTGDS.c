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

//Coto: Use them as you want, just make sure you read WELL the descriptions below.

#include "global_settings.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "utilsTGDS.h"
#include "timerTGDS.h"
#include "dmaTGDS.h"
#include "posixHandleTGDS.h"
#include "biosTGDS.h"
#include "dldi.h"
#include "loader.h"

#ifdef ARM7
#include <string.h>
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "soundTGDS.h"
#endif

#ifdef ARM9
#include <stdbool.h>
#include "dsregs.h"
#include "dsregs_asm.h"
#include "nds_cp15_misc.h"
#include "dldi.h"
#include "consoleTGDS.h"
#endif

//arg 0: channel
//arg 1: arg0: handler, arg1: userdata
u32 fifoFunc[FIFO_CHANNELS][2];	//context is only passed on callback prototype stage, because, the channel index generates the callee callback

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void Write8bitAddrExtArm(uint32 address, uint8 value){
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[54] = address;
	fifomsg[55] = (uint32)value;
	SendFIFOWordsITCM(WRITE_EXTARM_8, (uint32)fifomsg);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void Write16bitAddrExtArm(uint32 address, uint16 value){
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[56] = address;
	fifomsg[57] = (uint32)value;
	SendFIFOWordsITCM(WRITE_EXTARM_16, (uint32)fifomsg);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void Write32bitAddrExtArm(uint32 address, uint32 value){
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[58] = address;
	fifomsg[59] = (uint32)value;
	SendFIFOWordsITCM(WRITE_EXTARM_32, (uint32)fifomsg);
}

//Hardware IPC struct packed 
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
struct sIPCSharedTGDS* getsIPCSharedTGDS(){
	struct sIPCSharedTGDS* getsIPCSharedTGDSInst = (__attribute__((aligned (4))) struct sIPCSharedTGDS*)0x027FF000;
	return getsIPCSharedTGDSInst;
}

//Async FIFO Sender
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void SendFIFOWordsITCM(uint32 data0, uint32 data1){	//format: arg0: cmd, arg1: value
	REG_IPC_FIFO_TX = (uint32)data1;	
	REG_IPC_FIFO_TX = (uint32)data0;	//last message should always be command
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
void HandleFifoEmpty(){
	HandleFifoEmptyWeakRef((uint32)0,(uint32)0);
}
	
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void HandleFifoNotEmpty(){
	REG_IF=IRQ_RECVFIFO_NOT_EMPTY;
	volatile uint32 data0 = 0, data1 = 0;
	while(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		
		data0 = (u32)REG_IPC_FIFO_RX;
		data1 = (u32)REG_IPC_FIFO_RX;
		
		//Execute ToolchainGenericDS FIFO commands
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
			
			
			//ARM7 command handler
			#ifdef ARM7
			
			case (TGDS_ARM7_RELOADFLASH):{
				//Init Shared Address Region and get NDS Header
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				memcpy((u8*)&TGDSIPC->DSHeader,(u8*)0x027FFE00, sizeof(TGDSIPC->DSHeader)); //save Header flags
				
				//Read DHCP settings (in order)
				LoadFirmwareSettingsFromFlash();
				
				//Hardware ARM7 Init
				u8 DSHardwareReadFromFlash = TGDSIPC->DSFWHEADERInst.stub[0x1d];
				
				memcpy((u8*)0x027FFE00, (u8*)&TGDSIPC->DSHeader, sizeof(TGDSIPC->DSHeader));	//restore Header flags
				setValueSafe(&fifomsg[58], (u32)DSHardwareReadFromFlash);
			}
			break;
			
			case ARM7COMMAND_START_SOUND:{
				if(SoundStreamSetupSoundARM7LibUtilsCallback != NULL){
					SoundStreamSetupSoundARM7LibUtilsCallback();
				}
			}
			break;
			case ARM7COMMAND_STOP_SOUND:{
				if(SoundStreamStopSoundARM7LibUtilsCallback != NULL){
					SoundStreamStopSoundARM7LibUtilsCallback();
				}
			}
			break;
			case ARM7COMMAND_SOUND_SETRATE:{
				uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
				sndRate = fifomsg[60];
			}
			break;
			case ARM7COMMAND_SOUND_SETLEN:{
				uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
				sampleLen = fifomsg[61];
			}
			break;
			case ARM7COMMAND_SOUND_SETMULT:{
				uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
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
			
			case ARM7COMMAND_SND_COMMAND:{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
				SCHANNEL_CR(TGDSIPC->soundIPC.psgChannel) = TGDSIPC->soundIPC.cr;
				SCHANNEL_TIMER(TGDSIPC->soundIPC.psgChannel) = TGDSIPC->soundIPC.timer;
				SCHANNEL_SOURCE(TGDSIPC->soundIPC.psgChannel) = (u32)TGDSIPC->soundIPC.arm9L;
				SCHANNEL_LENGTH(TGDSIPC->soundIPC.psgChannel) = (TGDSIPC->soundIPC.volume >> 2); //volume == size
				TGDSIPC->soundIPC.volume = 0;
			}
			break;
			
			case((uint32)TGDS_ARM7_SETUPEXCEPTIONHANDLER):{
				exceptionArmRegsShared = (uint8*)data0;		//data0 == ARM9's exceptionArmRegs
				memset(exceptionArmRegsShared, 0, 0x20);	//same as exceptionArmRegs[0x20]
				setupDefaultExceptionHandler();	//ARM7 TGDS Exception Handler
			}
			break;
			
			case((uint32)FIFO_INITSOUND):{
				initSound();
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
				
				struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
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
			
			//ARM7 TGDS-Multiboot loader code here
						case(FIFO_ARM7_RELOAD):{	
							struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
							uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
							u32 arm7EntryAddressPhys = getValueSafe(&fifomsg[0]);
							int arm7BootCodeSize = getValueSafe(&fifomsg[1]);
							u32 arm7entryaddress = getValueSafe(&fifomsg[2]);
							memcpy((void *)arm7entryaddress,(const void *)arm7EntryAddressPhys, arm7BootCodeSize);
							reloadARMCore((u32)arm7entryaddress);	//Run Bootstrap7 
						}
						break;
						case(FIFO_TGDSMBRELOAD_SETUP):{
							reloadNDSBootstub();
						}
						break;
						
			
			case(TGDS_ARM7_SETUPMALLOCDLDI):{	//ARM7
				struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				u32 ARM7MallocStartaddress = fifomsg[0];
				u32 ARM7MallocSize = fifomsg[1];
				//bool customAllocator = (bool)getValueSafe(&fifomsg[2]);
				u32 dldiStartAddress = fifomsg[3];
				u32 TargetARM7DLDIAddress = fifomsg[4];
				
				setupLibUtils(); //ARM7 libUtils Setup
				
				//DSi in NTR mode throws false positives about TWL mode, enforce DSi SD initialization to define, NTR or TWL mode.
				int detectedTWLModeInternalSDAccess = 0;
				bool DLDIARM7FSInitStatus = false;
					
				if( (!sdio_Startup()) || (!sdio_IsInserted()) ){
					detectedTWLModeInternalSDAccess = TWLModeDLDIAccessDisabledInternalSDDisabled;
					__dsimode = false;
				}
				else{
					detectedTWLModeInternalSDAccess = TWLModeDLDIAccessDisabledInternalSDEnabled;
					__dsimode = true;
					DLDIARM7FSInitStatus = true;
				}
				
				//NTR mode: define DLDI initialization and ARM7DLDI operating mode
				if((detectedTWLModeInternalSDAccess == TWLModeDLDIAccessDisabledInternalSDDisabled) && (TargetARM7DLDIAddress != 0)){
					DLDIARM7Address = (u32*)TargetARM7DLDIAddress; 
					memcpy (DLDIARM7Address, dldiStartAddress, 16*1024);
					DLDIARM7FSInitStatus = dldi_handler_init();
					if(DLDIARM7FSInitStatus == true){
						detectedTWLModeInternalSDAccess = TWLModeDLDIAccessEnabledInternalSDDisabled;
					}
					else{
						detectedTWLModeInternalSDAccess = TWLModeDLDIAccessDisabledInternalSDDisabled;
					}					
				}
				
				//ARM7 custom Malloc libutils implementation
				if(initMallocARM7LibUtilsCallback != NULL){
					initMallocARM7LibUtilsCallback(ARM7MallocStartaddress, ARM7MallocSize);
				}
				
				fifomsg[0] = 0;
				fifomsg[1] = 0;
				fifomsg[2] = (u32)__dsimode;
				fifomsg[3] = TWLModeInternalSDAccess = detectedTWLModeInternalSDAccess;
				fifomsg[4] = DLDIARM7FSInitStatus;
			}
			break;
			
			case TGDS_ARMCORES_REPORT_PAYLOAD_MODE:{
				uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
				reportTGDSPayloadMode(data0);
				setValueSafe(&fifomsg[45], (uint32)0);
			}
			break;
			
			#ifdef TWLMODE
			case TGDS_ARM7_REQ_SLOT1_DISABLE:{
				disableSlot1();
			}
			break;
			
			case TGDS_ARM7_REQ_SLOT1_ENABLE:{
				enableSlot1();
			}
			break;
			
			case TGDS_ARM7_TWL_SET_TSC_NTRMODE:{
				TWLSetTouchscreenNTRMode();	//resets TSC controller
			}
			break;
			
			case TGDS_ARM7_TWL_SET_TSC_TWLMODE:{
				TWLSetTouchscreenTWLMode(); //resets TSC controller
			}
			break;
			
			#endif
			
			#endif
			
			//ARM9 command handler
			#ifdef ARM9
			//tgds-mb loader code here (ARM9)
						case(FIFO_ARM7_RELOAD_OK):{
							reloadStatus = 0;
						}
						break;
			
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
				if(SoundStreamUpdateSoundStreamARM9LibUtilsCallback != NULL){
					SoundStreamUpdateSoundStreamARM9LibUtilsCallback();
				}
			}	
			break;
			case((uint32)TGDS_ARM7_DETECTTURNOFFCONSOLE):{
				detectAndTurnOffConsole();
			}
			break;
			
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
			
			#endif
		}
		
		//Libutils FIFO not empty API
		if(libutilisFifoNotEmptyCallback != NULL){
			libutilisFifoNotEmptyCallback(data1, data0);	//Format: cmd, value order
		}
		
		HandleFifoNotEmptyWeakRef(data1, data0);	//Format: cmd, value order
		
		//FIFO Full / Error? Discard
		if((REG_IPC_FIFO_CR & IPC_FIFO_ERROR) == IPC_FIFO_ERROR){
			REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR | FIFO_IPC_ERROR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
			break;
		}
	}
}

//Allows to read (EWRAM) memory from source ARM Core to destination ARM Core; IRQ Safe and blocking
//u32 * targetMemory == EWRAM Memory source buffer to copy -FROM- u32 * srcMemory
//u32 * srcMemory == External ARM Core Base Address
void ReadMemoryExt(u32 * srcMemory, u32 * targetMemory, int bytesToRead){
	dmaFillWord(0, 0, (uint32)targetMemory, (uint32)bytesToRead);
	#ifdef ARM7
	uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
	#endif
	#ifdef ARM9
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	#endif
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
	#ifdef ARM7
	uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
	#endif
	#ifdef ARM9
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	#endif
	setValueSafe(&fifomsg[32], (uint32)srcMemory);
	setValueSafe(&fifomsg[33], (uint32)targetMemory);
	setValueSafe(&fifomsg[34], (uint32)bytesToRead);
	setValueSafe(&fifomsg[35], (uint32)0xFFFFFFFF);
	sendByteIPC(IPC_ARM7SAVEMEMORY_REQBYIRQ);
	while((u32)getValueSafe(&fifomsg[35]) != (u32)0){
		swiDelay(1);
	}
}

#ifdef ARM9
void XYReadScrPosUser(struct touchPosition * StouchScrPosInst)   {
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 
	struct touchPosition * sTouchPosition = (struct touchPosition *)&TGDSIPC->tscIPC;
	
	StouchScrPosInst->rawx    = sTouchPosition->rawx;
	StouchScrPosInst->px = sTouchPosition->px;
	StouchScrPosInst->rawy    = sTouchPosition->rawy;
	StouchScrPosInst->py = sTouchPosition->py;
	StouchScrPosInst->z1   =   sTouchPosition->z1;
	StouchScrPosInst->z2   =   sTouchPosition->z2;
}

//Reloads ARM7 Flash memory and returns DS hardware model by FIFO IRQ
u8 ARM7ReadFWVersionFromFlashByFIFOIRQ(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[58], (u32)0xFFFFFFFF);
	SendFIFOWords(TGDS_ARM7_RELOADFLASH, 0xFF);
	while(getValueSafe(&fifomsg[58]) == 0xFFFFFFFF){
		swiDelay(2);
	}
	return (u8)fifomsg[58];
}

#endif