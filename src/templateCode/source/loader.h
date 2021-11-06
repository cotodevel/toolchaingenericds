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

#ifndef __loader_common_h__
#define __loader_common_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "utilsTGDS.h"
#include "biosTGDS.h"

#define FIFO_TGDSMBRELOAD_SETUP (u32)(0xFFFFABC8)
#define FIFO_ARM7_RELOAD_OK (u32)(0xFFFFABC9)
#define FIFO_ARM7_RELOAD (u32)(0xFFFFABCA)
#define ARM7_PAYLOAD (u32)((int)0x02400000 - 0x18000)

static inline void reloadARMCore(u32 targetAddress){
	REG_IF = REG_IF;	// Acknowledge interrupt
	REG_IE = 0;
	REG_IME = IME_DISABLE;	// Disable interrupts
	swiDelay(800);
	swiSoftResetByAddress(targetAddress);	// Jump to boot loader
}

#endif


#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
extern void reloadNDSBootstub();
#endif


#ifdef __cplusplus
}
#endif