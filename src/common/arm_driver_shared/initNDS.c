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

#include "initNDS.h"
#include "mem_handler_shared.h"
#include "toolchain_utils.h"
#include "dsregs.h"
#include "typedefs.h"
#include "InterruptsARMCores_h.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "dma.h"
#include "timer.h"

#ifdef ARM9
#include "devoptab_devices.h"
#include "ipcfifo.h"
#include "file.h"
#include "video.h"
#endif

void initHardware(void) {
//---------------------------------------------------------------------------------
	//Reset Both Cores
	resetMemory_ARMCores();
	
	#ifdef ARM7
	
	//Init Shared Address Region
	memset((uint32*)MyIPC, 0, sizeof(tMyIPC));
	
	//Read DHCP settings (in order)
	LoadFirmwareSettingsFromFlash();
	
	MyIPC->arm7startaddress = get_iwram_start();
	MyIPC->arm7endaddress = (uint32)(get_iwram_start() + get_iwram_size());
	//Init Shared FIFO Buffer
	setARM7ARM9SharedBuffer(NULL);
	
	#endif
	
	#ifdef ARM9
	powerON(POWERMAN_ARM9 | POWER_LCD | POWER_2D_A | POWER_2D_B | POWER_SWAP_LCDS);
	setupExceptionHandler();
	
	//PPU Engines Default
	SETDISPCNT_MAIN(0); 
	SETDISPCNT_SUB(0);
	REG_BG0CNT = REG_BG1CNT = REG_BG2CNT = REG_BG3CNT = 0;
	
	//Init Shared FIFO Buffer
	memset((uint32*)&arm7arm9sharedBuffer[0], 0, sizeof(arm7arm9sharedBuffer));
	setARM7ARM9SharedBuffer((uint32*)&arm7arm9sharedBuffer[0]);
	
	//Library init code
	
	//Newlib init
	//Stream Devices Init: see stream_devices.c for specific DS hardware IO driver access
	//setbuf(stdin, NULL);
	setbuf(stdout, NULL);	//printf ability to direct DS Framebuffer
	//setbuf(stderr, NULL);
	
	//devoptab structs so Newlib POSIX filesystem can work
	initdevoptab();
	
	//init file handles
	file_default_init();
	
	MyIPC->arm9startaddress = get_ewram_start();
	MyIPC->arm9endaddress = (uint32)(get_ewram_start() + get_ewram_size());
	
	#endif
	
}

void resetMemory_ARMCores()
{
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
    
	//0-1   ARM9/ARM7 (0-3 = 32K/0K, 2nd 16K/1st 16K, 1st 16K/2nd 16K, 0K/32K)
	//set WORKRAM 32K to ARM9 by default
	WRAM_CR = 0x00;
	#endif
	
}