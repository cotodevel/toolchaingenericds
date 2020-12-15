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

#include "initNDSTGDS.h"
#include "utilsTGDS.h"
#include "dsregs.h"
#include "typedefsTGDS.h"
#include "InterruptsARMCores_h.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "dmaTGDS.h"
#include "timerTGDS.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "ipcfifoTGDS.h"
#include "soundTGDS.h"
#include "global_settings.h"
#include "eventsTGDS.h"
#include "posixHandleTGDS.h"
#include "keypadTGDS.h"

#ifdef ARM9
#include "devoptab_devices.h"
#include "videoTGDS.h"
#include "wifi_arm9.h"
#include "fatfslayerTGDS.h"
#include "dldi.h"
#endif

void resetMemory_ARMCores(u8 DSHardware){
	
	//cmp r0, #0xFF		@DS Phat
	//beq FirmwareARM7OK
	//cmp r0, #0x20		@DS Lite
	//beq FirmwareARM7OK
	//cmp r0, #0x57		@DSi
	//beq FirmwareARM7OK
	//cmp r0, #0x43		@iQueDS
	//beq FirmwareARM7OK
	//cmp r0, #0x63		@iQueDS Lite
	
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
		//while(REG_VCOUNT!=191){}
	 	register int i;
	    //clear out ARM9 DMA channels
		for (i=0; i<4; i++) {
			DMAXCNT(i) = 0;
			DMAXSAD(i) = 0;
			DMAXDAD(i) = 0;
			TIMERXCNT(i) = 0;
			TIMERXDATA(i) = 0;
		}
	
		#ifdef ARM9
		VRAM_CR = 0x80808080;
		VRAM_E_CR = 0x80;
		VRAM_F_CR = 0x80;
		VRAM_G_CR = 0x80;
		VRAM_H_CR = 0x80;
		VRAM_I_CR = 0x80;
	
		// clear vram
		uint16 * vram = (uint16 *)0x06800000;
		memset(vram, 0, 656 * 1024);
	
		// clear video palette
		memset(BG_PALETTE, 0, 2048 );	//BG_PALETTE[0] = RGB15(1,1,1);
		memset(BG_PALETTE_SUB, 0, 2048 );	//BG_PALETTE[0] = RGB15(1,1,1);
	
		// clear video object attribution memory
		memset(OAM, 0, 2048 );	//BG_PALETTE[0] = RGB15(1,1,1);
		memset(OAM_SUB, 0, 2048 );	//BG_PALETTE[0] = RGB15(1,1,1);
	
		// clear video object data memory
		memset(SPRITE_GFX, 0, 128 * 1024 );	//BG_PALETTE[0] = RGB15(1,1,1);
		memset(SPRITE_GFX_SUB, 0, 128 * 1024 );	//BG_PALETTE[0] = RGB15(1,1,1);
	
		// clear main display registers
		memset((void*)0x04000000, 0, 0x6c );	//BG_PALETTE[0] = RGB15(1,1,1);
	
		// clear sub display registers
		memset((void*)0x04001000, 0, 0x6c );	//BG_PALETTE[0] = RGB15(1,1,1);
	
		// clear maths registers
		memset((void*)0x04000280, 0, 0x40 );	//BG_PALETTE[0] = RGB15(1,1,1);
	
		REG_DISPSTAT = 0;
		SETDISPCNT_MAIN(0);
		SETDISPCNT_SUB(0);
		VRAM_A_CR = 0;
		VRAM_B_CR = 0;
		VRAM_C_CR = 0;
		VRAM_D_CR = 0;
		VRAM_E_CR = 0;
		VRAM_F_CR = 0;
		VRAM_G_CR = 0;
		VRAM_H_CR = 0;
		VRAM_I_CR = 0;
		VRAM_CR   = 0x03000000;
		REG_POWERCNT  = 0x820F;
    
		//set WORKRAM 32K to ARM9 by default
		WRAM_CR = WRAM_32KARM9_0KARM7;
		#endif
	}
	
	//TWL 
	else if(DSHardware == 0x57){
		  
		#ifdef ARM7
		//TWL Hardware ARM7 Init code goes here...
		#endif
		
		#ifdef ARM9
		//TWL Hardware ARM9 Init code goes here...
		#endif
		
	}
}

void initHardware(u8 DSHardware) {
//---------------------------------------------------------------------------------
	#ifdef ARM7
	//Init Shared Address Region and get NDS Heade
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
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
	#endif
	
	#ifdef ARM9
	
	//Disable mpu
	CP15ControlRegisterDisable(CR_M);
	
	//Hardware ARM9 Init
	resetMemory_ARMCores(DSHardware);
	IRQInit(DSHardware);
	
	//Enable mpu
	CP15ControlRegisterEnable(CR_M);
	
	//Library init code
	
	//Newlib init
	//Stream Devices Init: see devoptab_devices.c
	//setbuf(stdin, NULL);
	setbuf(stdout, NULL);	//iprintf directs to DS Framebuffer (printf already does that)
	//setbuf(stderr, NULL);
	
	printf7Setup();
	TryToDefragmentMemory();
	
	#ifdef ARM9_DLDI
	//Setup SLOT-1 if ARM9 DLDI
	setDLDIARM7Address((u32 *)dldiGet());
	#endif
	
	//Enable TSC
	setTouchScreenEnabled(true);	
	
	//Enable TGDS Event handling + Set timeout to turn off screens if idle.
	setAndEnableSleepModeInSeconds(SLEEPMODE_SECONDS);
	
	//TGDS Sound stream: Default volume
	setVolume(4);
	
	#endif
	
}
