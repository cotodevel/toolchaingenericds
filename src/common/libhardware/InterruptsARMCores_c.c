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
#include "soundTGDS.h"
#endif

#ifdef ARM9
#include "nds_cp15_misc.h"
#include "exceptionTGDS.h"
#endif

#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDS.h"
#include "linkerTGDS.h"
#include "biosTGDS.h"
#include "dmaTGDS.h"
#include "soundTGDS.h"
#include "dldi.h"
#include "debugNocash.h"
#include "utilsTGDS.h"
#include "timerTGDS.h"

#ifdef TWLMODE
#include "utils.twl.h"

#ifdef ARM7
#include "i2c.h"
#include "sdmmc.h"
#endif

#endif

void IRQInit(u8 DSHardware)  {
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	//default TGDS Project -> internal logger
	DrainWriteBuffer();
	setTGDSARM9LoggerCallback((loggerARM9LibUtils_fn)&nocashMessage);
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
	
	REG_IE = interrupts_to_wait_armX; 
	
	INTERRUPT_VECTOR = (uint32)&NDS_IRQHandler;
	REG_IME = 1;
	
	//NTR
	if(
		(DSHardware == 0xFF)	//DS Phat
		||
		(DSHardware == 0x20)	//DS Lite normal
		||
		(DSHardware == 0x35)	//DS Lite rare fw #1
		||
		(DSHardware == 0x43)	//Other DS hardware..
		||
		(DSHardware == 0x63)	//..
	){
		__dsimode = false;
		#ifdef ARM9
		loggerARM9LibUtilsCallback("TGDS:IRQInit():NTR Mode!");
		#endif
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
			//TGDS-Projects -> TWL TSC
			TWLSetTouchscreenNTRMode();
			#endif
			
			#ifdef ARM9
			//TWL ARM9 IRQ Init
			#endif
		#endif
		#ifdef ARM9
		loggerARM9LibUtilsCallback("TGDS:IRQInit():TWL Mode!");
		#endif
	}
	else{
		#ifdef ARM9
		int stage = 3;
		handleDSInitError(stage, (u32)DSHardware);
		#endif
	}
	
	
}

#ifdef ARM7
static bool penDown = false;
#endif

//Software bios irq more or less emulated. (replaces default NDS bios for some parts)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
__attribute__((target("arm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void NDS_IRQHandler(){
	volatile uint32 REG_IE_SET = (volatile uint32)(REG_IF & REG_IE);
	
	#ifdef TWLMODE
	u32 handledIRQAUX = REG_AUXIE & REG_AUXIF;
	#endif
	
	////			Common
	if(REG_IE_SET & IRQ_TIMER0){
		Timer0handlerUser();
		REG_IF = IRQ_TIMER0;
	}
	
	if(REG_IE_SET & IRQ_TIMER1){
		#ifdef ARM7
		if(SoundStreamTimerHandlerARM7LibUtilsCallback != NULL){
			SoundStreamTimerHandlerARM7LibUtilsCallback(); //Audio playback handler
		}
		#endif
		Timer1handlerUser();
		REG_IF = IRQ_TIMER1;
	}
	
	if(REG_IE_SET & IRQ_TIMER2){
		#ifdef ARM7
		if(MicInterruptARM7LibUtilsCallback != NULL){
			MicInterruptARM7LibUtilsCallback(); //Microphone recording handler
		}
		#endif
		Timer2handlerUser();
		REG_IF = IRQ_TIMER2;
	}
	
	if(REG_IE_SET & IRQ_TIMER3){
		#ifdef ARM9
		if(timerWifiInterruptARM9LibUtilsCallback != NULL){
			timerWifiInterruptARM9LibUtilsCallback();	//wifi arm9 irq
		}
		timerTicks+=timerUnitsPerTick;
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
		if(wifiUpdateVBLANKARM7LibUtilsCallback != NULL){
			wifiUpdateVBLANKARM7LibUtilsCallback();
		}
		#endif
		VblankUser();
		REG_IF = IRQ_VBLANK;
	}
	
	if(REG_IE_SET & IRQ_VCOUNT){
		#ifdef ARM7		
		struct sIPCSharedTGDS * sIPCSharedTGDSInst = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
		struct sEXTKEYIN * sEXTKEYINInst = (struct sEXTKEYIN *)&sIPCSharedTGDSInst->EXTKEYINInst;
		struct touchPosition * sTouchPosition = (struct touchPosition *)&sIPCSharedTGDSInst->tscIPC;
		
		//ARM7 Keypad has access to X/Y/Hinge/Pen down bits
		sIPCSharedTGDSInst->KEYINPUT7 = (uint16)REG_KEYINPUT;
		
		u16 keys= REG_KEYXY;	
		#ifdef TWLMODE
		keys |= (1 << 6);
		if(touchPenDown() == true){
			keys &= ~(1 << 6);
		}
		#endif
		/*
		4000136h - NDS7 - EXTKEYIN - Key X/Y Input (R)
		0      Button X     (0=Pressed, 1=Released)
		1      Button Y     (0=Pressed, 1=Released)
		3      DEBUG button (0=Pressed, 1=Released/None such)
		6      Pen down     (0=Pressed, 1=Released/Disabled) (always 0 in DSi mode)
		7      Hinge/folded (0=Open, 1=Closed)
		2,4,5  Unknown / set
		8..15  Unknown / zero
		*/
		if(keys & KEY_TOUCH){
			penDown = false;
		}
		else{	
			//reset state
			sTouchPosition->rawy    = 0;
			sTouchPosition->py = 0;
			sTouchPosition->rawx    = 0;
			sTouchPosition->px = 0;
			sTouchPosition->z1 = 0;
			sTouchPosition->z2 = 0;
			
			if(penDown){
				keys |= KEY_TOUCH;	//tsc event must be before coord handling to give priority over touch events
				
				touchPosition tempPos = {0};
				touchReadXY(&tempPos);
				
				if(tempPos.rawx && tempPos.rawy){
					sTouchPosition->rawy    = tempPos.rawy;
					sTouchPosition->py = tempPos.py;
					sTouchPosition->rawx    = tempPos.rawx;
					sTouchPosition->px = tempPos.px;
					sTouchPosition->z1 = tempPos.z1;
					sTouchPosition->z2 = tempPos.z2;
				}
				else{
					penDown = false;
				}
				
			}
			else{
				penDown = true;
			}
			
			//handle re-click
			
			#ifdef NTRMODE
			if( !(((uint16)REG_KEYINPUT) & KEY_TOUCH) ){
				penDown = true;
			}
			#endif
			
			#ifdef TWLMODE
			if(touchPenDown() == false){
				penDown = true;
			}
			#endif
		}
		
		sIPCSharedTGDSInst->buttons7	= keys;
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
	}
	
	if(REG_IE_SET & IRQ_IPCSYNC){
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
					case(IPC_READ_ARM7_TWLSD_REQBYIRQ):{
						uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
						int sector = getValueSafeInt(&fifomsg[20]);
						int numSectors = getValueSafeInt(&fifomsg[21]);
						uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[22]);
						bool retval = sdio_ReadSectors(sector, numSectors, (void*)targetMem);
						setValueSafe(&fifomsg[23], (u32)retval);	//last value has ret status & release ARM9 dldi cmd
					}
					break;
					
					case(IPC_WRITE_ARM7_TWLSD_REQBYIRQ):{
						uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
						uint32 sector = getValueSafe(&fifomsg[24]);
						uint32 numSectors = getValueSafe(&fifomsg[25]);
						uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[26]);
						bool retval = sdio_WriteSectors(sector, numSectors, (void*)targetMem);
						setValueSafe(&fifomsg[27], (u32)retval);	//last value has ret status & release ARM9 dldi cmd
					}
					break;
					
					case(IPC_STARTUP_ARM7_TWLSD_REQBYIRQ):{
						uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
						bool result = sdio_Startup();
						setValueSafe(&fifomsg[23], (u32)result);	//last value has ret status	
					}
					break;
					
					case(IPC_SD_IS_INSERTED_ARM7_TWLSD_REQBYIRQ):{
						uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
						bool result = sdio_IsInserted();
						setValueSafe(&fifomsg[23], (u32)result);	//last value has ret status
					}
					break;
				#endif
				
			#endif
			
			default:{
				IpcSynchandlerUser(ipcByte);//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
			}
			break;
		}
		REG_IF=IRQ_IPCSYNC;
	}
	
	#ifdef ARM7
	if(REG_IE_SET & IRQ_WIFI){
		if(wifiInterruptARM7LibUtilsCallback != NULL){
			wifiInterruptARM7LibUtilsCallback();	//arm7 wifi irq
		}
		REG_IF=IRQ_WIFI;
	}
	
	//clock 
	if(REG_IE_SET & IRQ_RTCLOCK){
		REG_IF = IRQ_RTCLOCK;
	}
	
	if(REG_IE_SET & IRQ_SCREENLID){
		isArm7ClosedLid=false;
		TurnOnScreens();
		screenLidHasOpenedhandlerUser();
		REG_IF = IRQ_SCREENLID;
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
	
	//Update BIOS flags
	SWI_CHECKBITS = REG_IE_SET;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void irqEnable(uint32 IRQ){
	REG_IE	|=	IRQ;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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