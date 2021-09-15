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
#include "dmaTGDS.h"
#include "soundTGDS.h"
#include "dldi.h"
#include "debugNocash.h"

#ifdef TWLMODE
#include "utils.twl.h"

#ifdef ARM7
#include "i2c.h"
#include "sdmmc.h"
#endif

#endif

void IRQInit(u8 DSHardware)  {
	
	#ifdef ARM9
	//DrainWrite
	DrainWriteBuffer();
	#endif
	
	//FIFO IRQ Init
	REG_IE = 0;
	REG_IF = ~0;
	REG_IME = 0;
	
	//FIFO IRQ Init
	REG_IPC_SYNC = (1 << 14);	//14    R/W  Enable IRQ from remote CPU  (0=Disable, 1=Enable)
	REG_IPC_FIFO_CR = IPC_FIFO_SEND_CLEAR | RECV_FIFO_IPC_IRQ  | FIFO_IPC_ENABLE;
	
	//Set up PPU IRQ: HBLANK/VBLANK/VCOUNT
	#ifdef ARM7
	REG_DISPSTAT = (DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
	#endif
	#ifdef ARM9
	REG_DISPSTAT = (DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
	#endif
	
	//Set up PPU IRQ Vertical Line
	setVCountIRQLine(TGDS_VCOUNT_LINE_INTERRUPT);
	
	volatile uint32 interrupts_to_wait_armX = 0;	
	#ifdef ARM7
	interrupts_to_wait_armX = IRQ_TIMER1 | IRQ_VBLANK | IRQ_VCOUNT | IRQ_IPCSYNC | IRQ_RECVFIFO_NOT_EMPTY | IRQ_SCREENLID;
	#endif
	
	#ifdef ARM9
	interrupts_to_wait_armX = IRQ_VBLANK | IRQ_VCOUNT | IRQ_IPCSYNC | IRQ_RECVFIFO_NOT_EMPTY;
	#endif
	
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
		__dsimode = false;
		nocashMessage("TGDS:IRQInit():NTR Mode!");
	}
	//TWL 
	else if(DSHardware == 0x57){
		__dsimode = true;
		#ifdef TWLMODE
			#ifdef ARM7
			//TWL ARM7 IRQ Init
			REG_AUXIE = 0;
			REG_AUXIF = ~0;
			irqEnableAUX(GPIO33_2);
			#endif
			
			#ifdef ARM9
			//TWL ARM9 IRQ Init
			#endif
		#endif
		nocashMessage("TGDS:IRQInit():TWL Mode!");
	}
	
	REG_IE = interrupts_to_wait_armX; 
	
	INTERRUPT_VECTOR = (uint32)&NDS_IRQHandler;
	REG_IME = 1;
}

#ifdef ARM7
static bool penDown = false;
#endif

//Software bios irq more or less emulated. (replaces default NDS bios for some parts)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void NDS_IRQHandler()  {
	u32 handledIRQ = REG_IF & REG_IE;
	
	#ifdef TWLMODE
	u32 handledIRQAUX = REG_AUXIE & REG_AUXIF;
	#endif
	
	if(handledIRQ & IRQ_HBLANK){
		HblankUser();
	}
	//vblank on ARM7 may be best for tasks NOT required while audio playback takes place
	if(handledIRQ & IRQ_VBLANK){
		#ifdef ARM7
		doSPIARM7IO();
		Wifi_Update();
		TGDSEventHandler();
		updateSoundContextSamplePlayback();
		#endif
		#ifdef ARM9
		//handles DS-DS Comms
		sint32 currentDSWNIFIMode = doMULTIDaemonStage1();
		#endif
		VblankUser();
	}
	if(handledIRQ & IRQ_VCOUNT){
		#ifdef ARM7
		struct sIPCSharedTGDS * sIPCSharedTGDSInst = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
		//ARM7 Keypad has access to X/Y/Hinge/Pen down bits
		sIPCSharedTGDSInst->ARM7REG_KEYINPUT = (uint16)REG_KEYINPUT;
		
		u16 keys= REG_KEYXY;	
		if(keys & KEY_TOUCH){
			penDown = false;
		}
		else{	
			//reset state
			sIPCSharedTGDSInst->rawy    = 0;
			sIPCSharedTGDSInst->touchYpx = 0;
			sIPCSharedTGDSInst->rawx    = 0;
			sIPCSharedTGDSInst->touchXpx = 0;
			sIPCSharedTGDSInst->touchZ1 = 0;
			sIPCSharedTGDSInst->touchZ2 = 0;
			
			if(penDown){
				keys |= KEY_TOUCH;	//tsc event must be before coord handling to give priority over touch events
				
				touchPosition tempPos = {0};
				touchReadXY(&tempPos);
				
				if(tempPos.rawx && tempPos.rawy){
					sIPCSharedTGDSInst->rawy    = tempPos.rawy;
					sIPCSharedTGDSInst->touchYpx = tempPos.py;
					sIPCSharedTGDSInst->rawx    = tempPos.rawx;
					sIPCSharedTGDSInst->touchXpx = tempPos.px;
					sIPCSharedTGDSInst->touchZ1 = tempPos.z1;
					sIPCSharedTGDSInst->touchZ2 = tempPos.z2;
				}
				else{
					penDown = false;
				}
				
			}
			else{
				penDown = true;
			}
			
			//handle re-click
			if( !(((uint16)REG_KEYINPUT) & KEY_TOUCH) ){
				penDown = true;
			}
		}
		
		sIPCSharedTGDSInst->ARM7REG_KEYXY	= keys;
		#endif
		VcounterUser();
	}
	if(handledIRQ & IRQ_TIMER0){
		Timer0handlerUser();
	}
	if(handledIRQ & IRQ_TIMER1){
		#ifdef ARM7
		TIMER1Handler();	//Audio playback handler
		#endif
		Timer1handlerUser();
	}
	if(handledIRQ & IRQ_TIMER2){
		Timer2handlerUser();
	}
	if(handledIRQ & IRQ_TIMER3){
		#ifdef ARM9
		//wifi arm9 irq
		Timer_50ms();
		#endif
		Timer3handlerUser();
	}
	
	if(handledIRQ & IRQ_SENDFIFO_EMPTY){
		HandleFifoEmpty();
	}
	if(handledIRQ & IRQ_RECVFIFO_NOT_EMPTY){
		HandleFifoNotEmpty();
	}
	
	if(handledIRQ & IRQ_IPCSYNC){
		uint8 ipcByte = receiveByteIPC();
		switch(ipcByte){
			case(IPC_ARM7READMEMORY_REQBYIRQ):{
				uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
				uint32 srcMemory = getValueSafe(&fifomsg[28]);
				uint32 targetMemory = getValueSafe(&fifomsg[29]);
				int bytesToRead = (int)getValueSafe(&fifomsg[30]);
				memcpy((u8*)targetMemory,(u8*)srcMemory, bytesToRead);
				setValueSafe(&fifomsg[28], (uint32)0);
				setValueSafe(&fifomsg[29], (uint32)0);
				setValueSafe(&fifomsg[30], (uint32)0);
				setValueSafe(&fifomsg[31], (uint32)0);
			}
			break;
			case(IPC_ARM7SAVEMEMORY_REQBYIRQ):{
				uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
				uint32 srcMemory = getValueSafe(&fifomsg[32]);
				uint32 targetMemory = getValueSafe(&fifomsg[33]);
				int bytesToRead = (int)getValueSafe(&fifomsg[34]);
				#ifdef ARM9
				dmaFillWord(0, 0, (uint32)srcMemory, (uint32)bytesToRead);
				#endif
				memcpy((u8*)srcMemory, (u8*)targetMemory, bytesToRead);
				setValueSafe(&fifomsg[32], (uint32)0);
				setValueSafe(&fifomsg[33], (uint32)0);
				setValueSafe(&fifomsg[34], (uint32)0);
				setValueSafe(&fifomsg[35], (uint32)0);
			}
			break;
			#ifdef ARM7
			
			case(IPC_READ_FIRMWARE_REQBYIRQ):{
				struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
				uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
				uint32 srcMemory = fifomsg[28];
				//uint32 targetMemory = fifomsg[29];
				//int bytesToRead = (int)fifomsg[30];
				
				//Read DHCP settings (in order)
				LoadFirmwareSettingsFromFlash();
				memcpy((u8*)srcMemory, (u8*)&TGDSIPC->DSFWHEADERInst.stub[0], sizeof(TGDSIPC->DSFWHEADERInst.stub));	//512 bytes
				
				fifomsg[31] = fifomsg[30] = fifomsg[29] = fifomsg[28] = (uint32)0;
			}
			break;			
				//ARM7_DLDI
				//Slot-1 or slot-2 access
				case(IPC_READ_ARM7DLDI_REQBYIRQ):{
					struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
					uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
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
				
				
				case(IPC_WRITE_ARM7DLDI_REQBYIRQ):{
					struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
					uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
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
				
				//TWL SD Hardware
				#ifdef TWLMODE
				
				case(IPC_STARTUP_ARM7_TWLSD_REQBYIRQ):{
					uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
					int result = sdmmc_sd_startup();
					if(result == 0){ //success?
						
					}
					else{
						while(1==1){
							*(u32*)0x02000000 = 0xEAEAEAEA;
							swiDelay(1);
						}
					}
					setValueSafe(&fifomsg[23], (u32)result);	//last value has ret status
					
				}
				break;
				
				case(IPC_SD_IS_INSERTED_ARM7_TWLSD_REQBYIRQ):{
					uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
					setValueSafe(&fifomsg[23], (u32)0);	//last value has ret status
				}
				break;
				
				case(IPC_READ_ARM7_TWLSD_REQBYIRQ):{
					uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
					int sector = getValueSafeInt(&fifomsg[20]);
					int numSectors = getValueSafeInt(&fifomsg[21]);
					uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[22]);
					u32 retval = (u32)sdmmc_readsectors(&deviceSD, sector, numSectors, (void*)targetMem);
					setValueSafe(&fifomsg[23], (u32)retval);	//last value has ret status & release ARM9 dldi cmd
				}
				break;
				
				case(IPC_WRITE_ARM7_TWLSD_REQBYIRQ):{
					uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
					uint32 sector = getValueSafe(&fifomsg[24]);
					uint32 numSectors = getValueSafe(&fifomsg[25]);
					uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[26]);
					u32 retval = (u32)sdmmc_writesectors(&deviceSD, sector, numSectors, (void*)targetMem);
					setValueSafe(&fifomsg[27], (u32)retval);	//last value has ret status & release ARM9 dldi cmd
				}
				break;
				#endif
				
			#endif
			
			#ifdef ARM9
			
			#endif
			
			default:{
				IpcSynchandlerUser(ipcByte);//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
			}
			break;
		}
	}

	#ifdef ARM7
	//arm7 wifi irq
	if(handledIRQ & IRQ_WIFI){
		Wifi_Interrupt();
	}
	
	if(handledIRQ & IRQ_SCREENLID){
		if(isArm7ClosedLid == true){
			SendFIFOWords(FIFO_IRQ_LIDHASOPENED_SIGNAL);
			screenLidHasOpenedhandlerUser();
			isArm7ClosedLid = false;
		}
	}
	
	#ifdef TWLMODE
	if(handledIRQAUX & IRQ_I2C){
		i2cIRQHandler();
	}
	
	if(handledIRQAUX & IRQ_SDMMC){
		
	}
	
	if(handledIRQAUX & GPIO33_2){
		
	}
	
	#endif
	
	#endif
	REG_IF = handledIRQ;
	SWI_CHECKBITS |= handledIRQ;
	
	#ifdef TWLMODE
	REG_AUXIF = handledIRQAUX;
	#endif
}

void irqEnable(uint32 IRQ){
	REG_IE	|=	IRQ;
}

void irqDisable(uint32 IRQ){
	REG_IE	&=	~(IRQ);
}

#ifdef TWLMODE
	#ifdef ARM7
	//---------------------------------------------------------------------------------
	TWL_CODE void i2cIRQHandler() {
	//---------------------------------------------------------------------------------
		int cause = (i2cReadRegister(I2C_PM, I2CREGPM_PWRIF) & 0x3) | (i2cReadRegister(I2C_GPIO, 0x02)<<2);

		switch (cause & 3) {
			case 1:{
				if (/*__powerbuttonCB*/ 1 == 1) {
					//__powerbuttonCB();
				} 
				else {
					i2cWriteRegister(I2C_PM, I2CREGPM_RESETFLAG, 1);
					i2cWriteRegister(I2C_PM, I2CREGPM_PWRCNT, 1);
				}
			}
			break;
			case 2:{
				shutdownNDSHardware(); //todo: maybe this call doesn't work in TWL mode? writePowerManagement(PM_CONTROL_REG,PM_SYSTEM_PWR);
			}
			break;
		}
	}
	
	//---------------------------------------------------------------------------------
	TWL_CODE void irqDisableAUX(uint32 irq) {
	//---------------------------------------------------------------------------------
		REG_AUXIE &= ~irq;
	}

	//---------------------------------------------------------------------------------
	TWL_CODE void irqEnableAUX(uint32 irq) {
	//---------------------------------------------------------------------------------
		REG_AUXIE |= irq;
	}
	#endif
#endif