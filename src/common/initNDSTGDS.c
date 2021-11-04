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
#include "posixHandleTGDS.h"
#include "keypadTGDS.h"
#include "biosTGDS.h"

#ifdef ARM9
#include "devoptab_devices.h"
#include "videoTGDS.h"
#include "fatfslayerTGDS.h"
#include "dldi.h"
#endif

#ifdef TWLMODE
#include "utils.twl.h"
#endif

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void resetMemory_ARMCores(u8 DSHardware) {
	
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void initHardware(u8 DSHardware) {
//---------------------------------------------------------------------------------
	swiDelay(15000);
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
	
	#ifdef TWLMODE
	u32 SFGEXT7 = *(u32*)0x04004008;
	
	//0     Revised ARM7 DMA Circuit       (0=NITRO, 1=Revised) = NTR
	//1     Revised Sound DMA              (0=NITRO, 1=Revised) = NTR
	//2     Revised Sound                  (0=NITRO, 1=Revised) = NTR
	//3-6   Unused (0)
	//7     Revised Card Interface Circuit (0=NITRO, 1=Revised) (set via ARM9) (R)
	//8     Extended ARM7 Interrupts      (0=NITRO, 1=Extended) (4000218h) = TWL
	//9     Extended SPI Clock (8MHz)     (0=NITRO, 1=Extended) (40001C0h) = TWL
	//10    Extended Sound DMA        ?   (0=NITRO, 1=Extended) (?) = NTR

	//Revised ARM7 DMA Circuit       (0=NITRO, 1=Revised)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 0)) | (0x0 << 0);
	
	//Revised Sound DMA              (0=NITRO, 1=Revised)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 1)) | (0x0 << 1);
	
	//Revised Sound                  (0=NITRO, 1=Revised)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 2)) | (0x0 << 2);
	
	//Extended ARM7 Interrupts      (0=NITRO, 1=Extended) (4000218h)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 8)) | (0x1 << 8);
	
	//Extended SPI Clock (8MHz)     (0=NITRO, 1=Extended) (40001C0h)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 9)) | (0x1 << 9);
	
	//Extended Sound DMA        ?   (0=NITRO, 1=Extended) (?)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 10)) | (0x0 << 10);
	
	//Access to New DMA Controller   (0=Disable, 1=Enable) (40041xxh)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 16)) | (0x1 << 16);
	
	//Access to 2nd NDS Cart Slot   (0=Disable, 1=Enable)	(set via ARM7)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 24)) | (0x0 << 24);
	
	// 25    Access to New Shared WRAM     (0=Disable, 1=Enable)  (set via ARM7) (R)
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 25)) | (0x0 << 25);
	
	*(u32*)0x04004008 = SFGEXT7;
	#endif
	
	handleARM7InitSVC();
	#endif
	
	#ifdef ARM9
	
	#ifdef TWLMODE
	u32 SFGEXT9 = *(u32*)0x04004008;
	
	//7     Revised Card Interface Circuit (0=NITRO, 1=Revised) = TWL
	//8     Extended ARM9 Interrupts       (0=NITRO, 1=Extended) = TWL
	//12    Extended LCD Circuit           (0=NITRO, 1=Extended) = NTR
	//13    Extended VRAM Access           (0=NITRO, 1=Extended) = NTR
	//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger) = 4MB + (2) mirrors NTR mode
	//16    Access to New DMA Controller   (0=Disable, 1=Enable) (40041xxh) = TWL
	
	//Revised Card Interface Circuit (0=NITRO, 1=Revised) (set via ARM9) (R)
	SFGEXT9 = (SFGEXT9 & ~(0x1 << 7)) | (0x1 << 7);
	
	//Extended ARM9 Interrupts       (0=NITRO, 1=Extended)
	SFGEXT9 = (SFGEXT9 & ~(0x1 << 8)) | (0x1 << 8);
	
	//Extended LCD Circuit           (0=NITRO, 1=Extended)
	SFGEXT9 = (SFGEXT9 & ~(0x1 << 12)) | (0x0 << 12);
	
	//Extended VRAM Access           (0=NITRO, 1=Extended)
	SFGEXT9 = (SFGEXT9 & ~(0x1 << 13)) | (0x0 << 13);
	
	//EWRAM: 4MB + (2) mirrors NTR mode
	SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x0 << 14);
	
	//Access to New DMA Controller   (0=Disable, 1=Enable) (40041xxh)
	SFGEXT9 = (SFGEXT9 & ~(0x1 << 16)) | (0x1 << 16);
	
	*(u32*)0x04004008 = SFGEXT9;
	
	setCpuClock(false);	//true: 133Mhz / false: 66Mhz (TWL Mode only)
	#endif
	
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
	
	TryToDefragmentMemory();
	
	//Enable TSC
	setTouchScreenEnabled(true);	
	
	handleARM9InitSVC();	
	#endif
	
}
