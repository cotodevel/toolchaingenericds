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
#include "wifi_shared.h"

#ifdef TWLMODE
#include "utils.twl.h"

#ifdef ARM7
#include "i2c.h"
#include "sdmmc.h"
#endif

#endif

void IRQInit(u8 DSHardware)  {
	#ifdef ARM9
	
	DrainWriteBuffer();
	setTGDSARM9PrintfCallback((printfARM9LibUtils_fn)&TGDSDefaultPrintf2DConsole); //Setup default TGDS Project: console 2D printf 
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
	
	int isNTRTWLBinary = isThisPayloadNTROrTWLMode();
	if(isNTRTWLBinary == isTWLBinary){
		__dsimode = true;
		#ifdef ARM9
		nocashMessage("TGDS:IRQInit():TWL Mode!");
		#endif
	}
	else{
		__dsimode = false;
		#ifdef ARM9
		nocashMessage("TGDS:IRQInit():NTR Mode!");
		#endif
	}
	#ifdef TWLMODE
		#ifdef ARM7
		//TWL ARM7 IRQ Init
		REG_AUXIE = 0;
		REG_AUXIF = ~0;
		irqEnableAUX(IRQ_I2C);
		
		//TGDS-Projects -> TWL TSC
		TWLSetTouchscreenNTRMode();
		#endif
		
		#ifdef ARM9
		//TWL ARM9 IRQ Init
		#endif
	#endif
}

#ifdef ARM7
static bool penDown = false;
#endif

//Software bios irq more or less emulated. (replaces default NDS bios for some parts)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void NDS_IRQHandler(){
	volatile uint32 REG_IE_SET = (volatile uint32)(REG_IF & REG_IE);
	
	#ifdef TWLMODE
	#ifdef ARM7
	u32 handledIRQAUX = REG_AUXIE & REG_AUXIF;
	#endif
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
			case(IPC_SEND_TGDS_CMD):{
				uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
				uint32 TGDS_CMD = (uint32)getValueSafe(&fifomsg[23]);
				switch(TGDS_CMD){
					#ifdef ARM7
					//ARM7_DLDI
					//Slot-1 or slot-2 access
					case(IPC_READ_ARM7DLDI_REQBYIRQ):{
						struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
						uint32 sector = getValueSafe(&fifomsg[20]);
						uint32 numSectors = getValueSafe(&fifomsg[21]);
						uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[22]);
						dldiInterface->ioInterface.readSectors(sector, numSectors, targetMem);
					}break;
					case(IPC_WRITE_ARM7DLDI_REQBYIRQ):{
						struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
						uint32 sector = getValueSafe(&fifomsg[20]);
						uint32 numSectors = getValueSafe(&fifomsg[21]);
						uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[22]);
						dldiInterface->ioInterface.writeSectors(sector, numSectors, targetMem);
					}break;
					
						//TWL SD Hardware
						#ifdef TWLMODE
						case(IPC_READ_ARM7_TWLSD_REQBYIRQ):{
							int sector = getValueSafeInt(&fifomsg[20]);
							int numSectors = getValueSafeInt(&fifomsg[21]);
							uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[22]);
							bool retval = sdio_ReadSectors(sector, numSectors, (void*)targetMem);
							setValueSafe(&fifomsg[24], (u32)retval);	//last value has ret status & release ARM9 dldi cmd
						}
						break;
						
						case(IPC_WRITE_ARM7_TWLSD_REQBYIRQ):{
							int sector = getValueSafeInt(&fifomsg[20]);
							int numSectors = getValueSafeInt(&fifomsg[21]);
							uint32 * targetMem = (uint32*)getValueSafe(&fifomsg[22]);
							bool retval = sdio_WriteSectors(sector, numSectors, (void*)targetMem);
							setValueSafe(&fifomsg[24], (u32)retval);	//last value has ret status & release ARM9 dldi cmd
						}
						break;
						
						case(IPC_STARTUP_ARM7_TWLSD_REQBYIRQ):{
							bool result = sdio_Startup();
						}
						break;
						#endif
					
					case(IPC_ARM7DISABLE_WIFI_REQBYIRQ):{
						// Deinit WIFI
						if(DeInitWIFIARM7LibUtilsCallback != NULL){
							DeInitWIFIARM7LibUtilsCallback();
						}		
					}break;			
					
					//arm9 wants to send a WIFI context block address / userdata is always zero here
					case(IPC_ARM7ENABLE_WIFI_REQBYIRQ):{
						if(wifiAddressHandlerARM7LibUtilsCallback != NULL){
							//	wifiAddressHandler( void * address, void * userdata )
							wifiAddressHandlerARM7LibUtilsCallback((Wifi_MainStruct *)getValueSafe(&fifomsg[22]), 0);
						}			
					}
					break;
					
					
					case(IPC_ARM7READMEMORY_REQBYIRQ):{
						uint32 srcMemory = getValueSafe(&fifomsg[20]);
						uint32 targetMemory = getValueSafe(&fifomsg[21]);
						int bytesToRead = (int)getValueSafe(&fifomsg[22]);
						memcpy((u8*)targetMemory,(u8*)srcMemory, bytesToRead);
					}
					break;
					case(IPC_ARM7SAVEMEMORY_REQBYIRQ):{
						uint32 srcMemory = getValueSafe(&fifomsg[20]);
						uint32 targetMemory = getValueSafe(&fifomsg[21]);
						int bytesToRead = (int)getValueSafe(&fifomsg[22]);
						dmaFillWord(0, 0, (uint32)srcMemory, (uint32)bytesToRead);
						memcpy((u8*)srcMemory, (u8*)targetMemory, bytesToRead);
					}
					break;
					
					#endif
				}
				setValueSafe(&fifomsg[23], (uint32)0);
			}
			break;
			
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
			REG_AUXIF = IRQ_I2C;
		}
		
		if(handledIRQAUX & IRQ_SDMMC){
			//todo
			REG_AUXIF = IRQ_SDMMC;
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
	void i2cIRQHandler() {
	//---------------------------------------------------------------------------------
		int cause = (i2cReadRegister(I2C_PM, I2CREGPM_PWRIF) & 0xb);

		switch (cause & 0xb) {
			//bit 0 & bit 3
			case 1:
			case 8:{
				i2cWriteRegister(I2C_PM, I2CREGPM_RESETFLAG, 1);
			}break;
			case 2:{ //bit 1
				i2cWriteRegister(I2C_PM, I2CREGPM_PWRCNT, 1);
			}break;
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