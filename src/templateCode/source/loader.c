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

#include "loader.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "biosTGDS.h"

#ifdef ARM7

//TGDS-MB Bootcode ARM7 v2: Runs from VRAM
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void reloadNDSBootstub(){
	
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	{
		REG_IME = 0;
		REG_IF = 0;
		REG_IE = 0;
		
		//ARM7 reloads here (only if within 0x037f8000 range)
		int arm7BootCodeSize = getValueSafe(&fifomsg[1]);
		u32 arm7EntryAddress = getValueSafe(&fifomsg[0]);
		
		//ARM7 Payload copy from shared ewram into ARM7 iwram
		memcpy((void *)arm7EntryAddress,(const void *)ARM7_PAYLOAD, arm7BootCodeSize);
		
		//reload ARM7!
		setValueSafe(&fifomsg[0], (u32)0);
		typedef void (*t_bootAddr)();
		t_bootAddr bootARM7Payload = (t_bootAddr)arm7EntryAddress;
		bootARM7Payload();
	}
}

#endif
