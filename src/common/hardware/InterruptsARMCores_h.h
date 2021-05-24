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

#ifndef __interruptsARMCores_h__
#define __interruptsARMCores_h__

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "linkerTGDS.h"
#include "ipcfifoTGDS.h"

static inline void setVCountIRQLine(int VCountIRQ){
	REG_DISPSTAT = (u16)((REG_DISPSTAT & ~(0xFF<<8)) | ((VCountIRQ & 0xFF)<<8));
}

#define TGDS_VCOUNT_LINE_INTERRUPT (int)(159)

#endif
#ifdef __cplusplus
extern "C"{
#endif


// Common

//weak symbols : the implementation of these is project-defined
extern 	void HblankUser();
extern 	void VblankUser();
extern 	void VcounterUser();

extern 	void Timer0handlerUser();
extern 	void Timer1handlerUser();
extern 	void Timer2handlerUser();
extern 	void Timer3handlerUser();
extern 	void screenLidHasOpenedhandlerUser();
extern 	void screenLidHasClosedhandlerUser();
extern 	void IpcSynchandlerUser(uint8 ipcByte);
//weak symbols end

extern uint32 getIRQs();
extern void EnableIrq(uint32 IRQ);
extern void DisableIrq(uint32 IRQ);
extern void NDS_IRQHandler();	//Actual Interrupt Handler
extern void IRQWait(uint32 irqstowait);
extern void IRQVBlankWait();
extern void IRQInit(u8 DSHardware);

#ifdef TWLMODE
	#ifdef ARM7
	extern void i2cIRQHandler();
	#endif
	extern void irqDisableAUX(uint32 irq);
	extern void irqEnableAUX(uint32 irq);
#endif

#ifdef __cplusplus
}
#endif
