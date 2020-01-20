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

#ifdef ARM9
#include "devoptab_devices.h"
#include "videoTGDS.h"
#endif

void initHardware(void) {
//---------------------------------------------------------------------------------
	//Reset Both Cores
	resetMemory_ARMCores();
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	
	#ifdef ARM7
	//Init Shared Address Region
	memset((uint32*)TGDSIPC, 0, TGDSIPCSize);
	
	//Read DHCP settings (in order)
	LoadFirmwareSettingsFromFlash();
	TGDSIPC->arm7startaddress = get_iwram_start();
	TGDSIPC->arm7endaddress = (uint32)(get_iwram_start() + get_iwram_size());
	
	//Init SoundSampleContext
	initSoundSampleContext();
	initSound();
	
	#endif
	
	#ifdef ARM9
	powerON(POWER_2D_A | POWER_2D_B | POWER_SWAP_LCDS);
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT|POWMAN_BACKLIGHT_BOTTOM_BIT);
	setupDefaultExceptionHandler();
	
	//PPU Engines Default
	SETDISPCNT_MAIN(0); 
	SETDISPCNT_SUB(0);
	REG_BG0CNT = REG_BG1CNT = REG_BG2CNT = REG_BG3CNT = 0;
	
	//Library init code
	
	//Newlib init
	//Stream Devices Init: see devoptab_devices.c
	//setbuf(stdin, NULL);
	setbuf(stdout, NULL);	//iprintf directs to DS Framebuffer (printf already does that)
	//setbuf(stderr, NULL);
	
	TGDSIPC->arm9startaddress = get_ewram_start();
	TGDSIPC->arm9endaddress = (uint32)(get_ewram_start() + get_ewram_size());
	
	TryToDefragmentMemory();
	#endif
	
}
