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

#ifdef ARM7
#include "wifi_arm7.h"
#include "soundTGDS.h"
#endif

#ifdef ARM9
#include "wifi_arm9.h"
#include "dswnifi_lib.h"
#include "nds_cp15_misc.h"
#endif

#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDS.h"
#include "linkerTGDS.h"
#include "eventsTGDS.h"
#include "biosTGDS.h"
#include "ARM7FS.h"
#include "dmaTGDS.h"

void IRQInit(u8 DSHardware){
	
	//NTR
	if(
		(DSHardware == 0xFF)
		||
		(DSHardware == 0x20)
		||
		(DSHardware == 0x43)
		||
		(DSHardware == 0x63)
	){
		//FIFO IRQ Init
		REG_IME = 0;
		
		int i = 0;
		for(i = 0; i <32 ; i++){
			if((REG_IF & (1 << i)) == (1 << i)){
				REG_IF = (1 << i);
			}
		}
		
		REG_IE = 0;
		
		//FIFO IRQ Init
		REG_IPC_SYNC = (1 << 14);	//14    R/W  Enable IRQ from remote CPU  (0=Disable, 1=Enable)
		REG_IPC_FIFO_CR = IPC_FIFO_SEND_CLEAR | RECV_FIFO_IPC_IRQ  | FIFO_IPC_ENABLE;
		
		//Set up PPU IRQ: HBLANK/VBLANK/VCOUNT
		REG_DISPSTAT = (DISP_HBLANK_IRQ | DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
		
		//Set up PPU IRQ Vertical Line
		setVCountIRQLine(TGDS_VCOUNT_LINE_INTERRUPT);
		
		volatile uint32 interrupts_to_wait_armX = 0;
		
		#ifdef ARM7
		interrupts_to_wait_armX = IRQ_TIMER1 | IRQ_HBLANK | IRQ_VBLANK | IRQ_VCOUNT | IRQ_IPCSYNC | IRQ_RECVFIFO_NOT_EMPTY | IRQ_SCREENLID;
		#endif
		
		#ifdef ARM9
		interrupts_to_wait_armX = IRQ_HBLANK| IRQ_VBLANK | IRQ_VCOUNT | IRQ_IPCSYNC | IRQ_RECVFIFO_NOT_EMPTY;
		#endif
		
		REG_IE = interrupts_to_wait_armX; 
		
		INTERRUPT_VECTOR = (uint32)&NDS_IRQHandler;
		REG_IME = 1;
		
		#ifdef ARM9
		//DrainWrite
		DrainWriteBuffer();
		#endif
	}
	//TWL 
	else if(DSHardware == 0x57){
		  
		#ifdef ARM7
		//TWL ARM7 IRQ Init code goes here...
		#endif
		
		#ifdef ARM9
		//TWL ARM9 IRQ Init code goes here...
		#endif
	}
}



//Software bios irq more or less emulated. (replaces default NDS bios for some parts)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void NDS_IRQHandler(){
	volatile uint32 REG_IE_SET = (volatile uint32)((REG_IF & REG_IE) | SWI_CHECKBITS);
	
	////			Common
	if(REG_IE_SET & IRQ_TIMER0){
		Timer0handlerUser();
		REG_IF = IRQ_TIMER0;
	}
	
	if(REG_IE_SET & IRQ_TIMER1){
		Timer1handlerUser();
		REG_IF = IRQ_TIMER1;
	}
	
	if(REG_IE_SET & IRQ_TIMER2){
		Timer2handlerUser();
		REG_IF = IRQ_TIMER2;
	}
	
	if(REG_IE_SET & IRQ_TIMER3){
		#ifdef ARM7
		//Audio playback handler
		setSwapChannel();
		SendFIFOWordsITCM(ARM9COMMAND_UPDATE_BUFFER, srcFrmt);
		#endif
		#ifdef ARM9
		//wifi arm9 irq
		Timer_10ms();
		#endif
		Timer3handlerUser();
		REG_IF = IRQ_TIMER3;
	}
	
	if(REG_IE_SET & IRQ_HBLANK){
		HblankUser();
		REG_IF = IRQ_HBLANK;
	}
	
	if(REG_IE_SET & IRQ_VBLANK){
		#ifdef ARM7
		Wifi_Update();
		#endif
		
		#ifdef ARM9
		//handles DS-DS Comms
		sint32 currentDSWNIFIMode = doMULTIDaemonStage1();
		#endif
		VblankUser();
		REG_IF = IRQ_VBLANK;
	}
	
	if(REG_IE_SET & IRQ_VCOUNT){
		#ifdef ARM7
		doSPIARM7IO();
		TGDSEventHandler();
		updateSoundContextSamplePlayback();
		#endif
		VcounterUser();
		REG_IF = IRQ_VCOUNT;
	}
	
	if(REG_IE_SET & IRQ_SENDFIFO_EMPTY){
		HandleFifoEmpty();
		REG_IF = IRQ_SENDFIFO_EMPTY;
	}
	
	if(REG_IE_SET & IRQ_RECVFIFO_NOT_EMPTY){
		HandleFifoNotEmpty();
		REG_IF=IRQ_RECVFIFO_NOT_EMPTY;
	}
	
	if(REG_IE_SET & IRQ_IPCSYNC){
		uint8 ipcByte = receiveByteIPC();
		switch(ipcByte){
			case(IPC_ARM7READMEMORY_REQBYIRQ):{
				struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				uint32 srcMemory = fifomsg[28];
				uint32 targetMemory = fifomsg[29];
				int bytesToRead = (int)fifomsg[30];
				memcpy((u8*)targetMemory,(u8*)srcMemory, bytesToRead);
				fifomsg[31] = fifomsg[30] = fifomsg[29] = fifomsg[28] = (uint32)0;
			}
			break;
			case(IPC_ARM7SAVEMEMORY_REQBYIRQ):{
				struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				uint32 srcMemory = fifomsg[32];
				uint32 targetMemory = fifomsg[33];
				int bytesToRead = (int)fifomsg[34];
				#ifdef ARM9
				dmaFillWord(0, 0, (uint32)srcMemory, (uint32)bytesToRead);
				#endif
				memcpy((u8*)srcMemory, (u8*)targetMemory, bytesToRead);
				fifomsg[35] = fifomsg[34] = fifomsg[33] = fifomsg[32] = (uint32)0;
			}
			break;
			#ifdef ARM7
			
			case(IPC_READ_FIRMWARE_REQBYIRQ):{
				struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				uint32 srcMemory = fifomsg[28];
				//uint32 targetMemory = fifomsg[29];
				//int bytesToRead = (int)fifomsg[30];
				
				//Read DHCP settings (in order)
				LoadFirmwareSettingsFromFlash();
				memcpy((u8*)srcMemory, (u8*)&TGDSIPC->DSFWHEADERInst.stub[0], sizeof(TGDSIPC->DSFWHEADERInst.stub));	//512 bytes
				
				fifomsg[31] = fifomsg[30] = fifomsg[29] = fifomsg[28] = (uint32)0;
			}
			break;
			
			//ARM7 FS Init
			case(IPC_ARM7INIT_ARM7FS):{	//ARM7
				struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				char *  ARM7FS_ARM9Filename = (char *)fifomsg[9];
				int fileHandleSize = (int)fifomsg[10];
				int splitBufferSize = (int)fifomsg[11];
				int curARM7FS_HandleMethod = (int)fifomsg[12];
				u32 * debugVar = (u32*)fifomsg[13];
				u32 testCase = (u32)fifomsg[14]; 
				initARM7FS((char*)ARM7FS_ARM9Filename, curARM7FS_HandleMethod);
				if(testCase == (u32)0xc070c070){
					performARM7MP2FSTestCase(ARM7FS_ARM9Filename, splitBufferSize, debugVar);	//ARM7 Setup
				}
				fifomsg[15] = fifomsg[14] = fifomsg[13] = fifomsg[12] = fifomsg[11] = fifomsg[10] = fifomsg[9] = 0;
			}
			break;
			
			case(IPC_ARM7DEINIT_ARM7FS):{	//ARM7
				struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				
				//ARM7(FS) de-init
				deinitARM7FS();
				
				fifomsg[8] = 0;
			}
			break;
			
			#ifdef ARM7_DLDI
			case(IPC_INIT_ARM7DLDI_REQBYIRQ):{
				//Init DLDI ARM7
				ARM7DLDIInit();
			}
			break;
			
			case((uint32)IPC_READ_ARM7DLDI_REQBYIRQ):{
				struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				uint32 sector = getValueSafe(&fifomsg[20]);
				uint32 numSectors = getValueSafe(&fifomsg[21]);
				uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[22]);
				dldiInterface->ioInterface.readSectors(sector, numSectors, targetMem);
				setValueSafe(&fifomsg[20], (u32)0);
				setValueSafe(&fifomsg[21], (u32)0);
				setValueSafe(&fifomsg[22], (u32)0);
				setValueSafe(&fifomsg[23], (u32)0);
			}
			break;
			
			case((uint32)IPC_WRITE_ARM7DLDI_REQBYIRQ):{
				struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
				uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
				uint32 sector = getValueSafe(&fifomsg[24]);
				uint32 numSectors = getValueSafe(&fifomsg[25]);
				uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[26]);
				dldiInterface->ioInterface.writeSectors(sector, numSectors, targetMem);
				setValueSafe(&fifomsg[24], (u32)0);
				setValueSafe(&fifomsg[25], (u32)0);
				setValueSafe(&fifomsg[26], (u32)0);
				setValueSafe(&fifomsg[27], (u32)0);
			}
			break;
			
			#endif
			
			#endif
			
			#ifdef ARM9
			case (IPC_ARM7INIT_ARM7FS):	//ARM9 
			{
				
			}
			break;
			
			case(IPC_ARM7DEINIT_ARM7FS):{	//ARM9 
				
			}
			break;
			
			#endif
			
			default:{
				IpcSynchandlerUser(ipcByte);//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
			}
			break;
		}
		REG_IF=IRQ_IPCSYNC;
	}
	
	#ifdef ARM7
	//arm7 wifi irq
	if(REG_IE_SET & IRQ_WIFI){
		Wifi_Interrupt();
		REG_IF = IRQ_WIFI;
	}
	
	//clock //could cause freezes
	if(REG_IE_SET & IRQ_RTCLOCK){
		REG_IF = IRQ_RTCLOCK;
	}
	
	if(REG_IE_SET & IRQ_SCREENLID){
		isArm7ClosedLid = false;
		SendFIFOWordsITCM(FIFO_IRQ_LIDHASOPENED_SIGNAL, 0);
		screenLidHasOpenedhandlerUser();
		REG_IF = IRQ_SCREENLID;
	}
	#endif
	
	//Update BIOS flags
	SWI_CHECKBITS = REG_IF;
}


void EnableIrq(uint32 IRQ){
	REG_IE	|=	IRQ;
}

void DisableIrq(uint32 IRQ){
	REG_IE	&=	~(IRQ);
}
