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
#include "debugNocash.h"

#ifdef ARM7
#include <string.h>
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "soundTGDS.h"

#ifdef TWLMODE
#include "i2c.h"
#endif
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

#ifdef ARM9
//EWRAM
u32 sharedbufferIOReadWrite = 0;
void Write16bitAddrExtArm(u32 address, u16 bits){
	sharedbufferIOReadWrite = (u32)(bits & 0x0000FFFF);
	SaveMemoryExt((u32*)address, (u32 *)&sharedbufferIOReadWrite, 2);
}
#endif

//Hardware IPC struct packed 
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
struct sIPCSharedTGDS* getsIPCSharedTGDS(){
	struct sIPCSharedTGDS* getsIPCSharedTGDSInst = (__attribute__((aligned (4))) struct sIPCSharedTGDS*)TGDSIPCStartAddress;
	return getsIPCSharedTGDSInst;
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
			case(FIFO_SEND_TGDS_CMD):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
				uint32 FIFO_SEND_TGDS_CMD_in = (uint32)getValueSafe(&fifomsg[7]);
				switch(FIFO_SEND_TGDS_CMD_in){
					
					//Shared
					case(TGDS_GETEXTERNALPAYLOAD_MODE):{
						setValueSafe(&fifomsg[8], (u32)__dsimode);
					}
					break;
					
					case(TGDS_GETEXTERNALCPU_COMMAND_MODE):{
						u32 which = (u32)getValueSafe(&fifomsg[8]);
						bool ExtCoreResult = queryTGDSARMBinaryFeaturesCurrentCore(which);
						setValueSafe(&fifomsg[8], (u32)ExtCoreResult);
					}
					break;
					
					#ifdef ARM7
						//ARM7 TGDS-Multiboot loader 
						case(FIFO_ARM7_RELOAD):{	//TGDS-MB v3 VRAM Loader's tgds_multiboot_payload.bin: void executeARM7Payload(u32 arm7entryaddress, int arm7BootCodeSize);
							u32 arm7EntryAddressPhys = getValueSafe(&fifomsg[0]);
							int arm7BootCodeSize = getValueSafe(&fifomsg[1]);
							u32 arm7entryaddress = getValueSafe(&fifomsg[2]);
							if(arm7EntryAddressPhys != ((u32)0) ){
								memcpy((void *)arm7entryaddress,(const void *)arm7EntryAddressPhys, arm7BootCodeSize);
							}
							setValueSafe((u32*)0x02FFFE34, (u32)arm7entryaddress);
							swiSoftReset();	// Jump to boot loader
						}
						break;
			
						case(BOOT_FILE_TGDSMB):{	//TGDS-MB v3 VRAM Loader's tgds_multiboot_payload.bin: arm9 bootloader: char * homebrewToBoot
							bootfile();
						}break;
						
						//TGDS ARM7 Sound streaming 
						case ARM7COMMAND_START_SOUND:{
							if(SoundStreamSetupSoundARM7LibUtilsCallback != NULL){
								u32 SoundBuffARM7 = getValueSafe(&fifomsg[0]);
								SoundStreamSetupSoundARM7LibUtilsCallback(SoundBuffARM7);	//data0 == ARM7 Sound Buffer source for streaming
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
							sndRate = getValueSafe(&fifomsg[0]);
						}
						break;
						case ARM7COMMAND_SOUND_SETLEN:{
							sampleLen = getValueSafe(&fifomsg[0]);
						}
						break;
						case ARM7COMMAND_SOUND_SETMULT:{
							multRate = getValueSafe(&fifomsg[0]);
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
						}
						break;
						
						case((uint32)TGDS_ARM7_SETUPEXCEPTIONHANDLER):{
							u32 * sharedBuf = (uint32*)getValueSafe(&fifomsg[0]); //arg0 == ARM9's sharedBuf
							exceptionArmRegsShared = (uint8*)(getValueSafe(sharedBuf+0));
							memset(exceptionArmRegsShared, 0, 0x20);	//same as exceptionArmRegs[0x20]
							sharedStringExceptionMessageOutput = (char*)(getValueSafe(sharedBuf+1));
							setupDefaultExceptionHandler();	//ARM7 TGDS Exception Handler
						} break;
						
						case((uint32)TGDS_ARM7_SETUPDISABLEDEXCEPTIONHANDLER):{
							setupDisabledExceptionHandler();	//ARM7 TGDS Exception Handler
						}
						break;
						
						
						case((uint32)FIFO_INITSOUND):{
							initSound();
						}
						break;
						
						case((uint32)FIFO_POWERCNT_ON):{
							powerON((uint16)(getValueSafe(&fifomsg[0]) & 0xFFFF));
						}
						break;
						case((uint32)FIFO_POWERCNT_OFF):{
							powerOFF((uint16)(getValueSafe(&fifomsg[0]) & 0xFFFF));
						}
						break;
						//Power Management: 
							//Supported mode(s): NTR / TWL
						case((uint32)FIFO_POWERMGMT_WRITE):{
							uint32 cmd = (uint32)getValueSafe(&fifomsg[8]);
							uint32 flags = (uint32)getValueSafe(&fifomsg[9]);
							switch(cmd){
								//Screen power write (NTR/TWL)
								case(FIFO_SCREENPOWER_WRITE):{
									int PMBitsRead = PowerManagementDeviceRead((int)POWMAN_READ_BIT);
									PMBitsRead &= ~(POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT);
									PMBitsRead |= (int)(flags & (POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT));
									PowerManagementDeviceWrite(POWMAN_WRITE_BIT, (int)PMBitsRead);						
								}
								break;
								
								//Shutdown NDS hardware (NTR/TWL)
								case(FIFO_SHUTDOWN_DS):{
									shutdownNDSHardware();
								}
								break;
								
								//Reset TWL hardware (only)
								case(FIFO_RESET_DS):{
									resetNDSHardware();
								}
								break;
							}
							fifomsg[61] = fifomsg[60] = 0;
						}
						break;
						
						case TGDS_ARMCORES_REPORT_PAYLOAD_MODE:{
							reportTGDSPayloadMode7(getValueSafe(&fifomsg[0]));
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
					
					#ifdef ARM9
						case((uint32)TGDS_ARM7_DETECTTURNOFFCONSOLE):{
							detectAndTurnOffConsole();
						}
						break;
						
						//ARM9 Side -> ARM7: Exception Handler
						case((uint32)EXCEPTION_ARM7):{
							uint32 exception_cmd = (uint32)getValueSafe(&fifomsg[0]);
							switch((uint32)exception_cmd){
								case(generalARM7Exception):{
									exception_handler((uint32)generalARM7Exception, 0, 0);
								}
								break;
								case(unexpectedsysexit_7):{
									exception_handler((uint32)unexpectedsysexit_7, 0, 0);
								}
								break;
								case(manualexception_7):{
									int stage = (int)getValueSafe(&fifomsg[1]);
									u8 fwNo = (u8)getValueSafe(&fifomsg[2]);
									exception_handler((uint32)manualexception_7, stage, fwNo);
								}
								break;
							}
						}
						break;
						
						case((uint32)TGDS_ARM7_STAGE4_ERROR):{
							u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
							int stage = 4;
							handleDSInitError(stage, (u32)fwNo);		
						}
						break;
					#endif
					
				}
				
				setValueSafe(&fifomsg[7], (u32)0);
			}break;
			
			
			//ARM7 hardware FIFO commands
			#ifdef ARM7
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
			#endif
			
			//ARM9 hardware FIFO commands
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
				
				// call immediately if the format needs it (decode on demand)
				if(SoundStreamUpdateSoundStreamARM9LibUtilsCallback != NULL){
					SoundStreamUpdateSoundStreamARM9LibUtilsCallback();
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
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[20], (uint32)srcMemory);
	setValueSafe(&fifomsg[21], (uint32)targetMemory);
	setValueSafe(&fifomsg[22], (uint32)bytesToRead);
	setValueSafe(&fifomsg[23], (uint32)IPC_ARM7READMEMORY_REQBYIRQ);
	sendByteIPC(IPC_SEND_TGDS_CMD);
	while( ( ((uint32)getValueSafe(&fifomsg[23])) != ((u32)0)) ){
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
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[20], (uint32)srcMemory);
	setValueSafe(&fifomsg[21], (uint32)targetMemory);
	setValueSafe(&fifomsg[22], (uint32)bytesToRead);
	setValueSafe(&fifomsg[23], (uint32)IPC_ARM7SAVEMEMORY_REQBYIRQ);
	sendByteIPC(IPC_SEND_TGDS_CMD);
	while( ( ((uint32)getValueSafe(&fifomsg[23])) != ((u32)0)) ){
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

bool getNTRorTWLModeFromExternalProcessor(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[7], (u32)TGDS_GETEXTERNALPAYLOAD_MODE);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
	return (bool)getValueSafe(&fifomsg[8]);
}

#endif