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
#include "linker_ext.h"
#include "common_shared.h"

#endif
#ifdef __cplusplus
extern "C"{
#endif

//project specific here

#ifdef ARM7
extern uint32 interrupts_to_wait_arm7;
#endif

#ifdef ARM9
extern volatile uint32 interrupts_to_wait_arm9;
#endif

// Common

//weak symbols : the implementation of these is project-defined
#ifdef ARM7
extern __attribute__((weak))	void Timer1handler();
#endif
#ifdef ARM9
extern __attribute__((weak))	void Timer_50ms();
#endif

extern __attribute__((weak))	void Hblank();
extern __attribute__((weak))	void Vblank();
extern __attribute__((weak))	void Vcounter();
extern __attribute__((weak))	void HandleFifoEmpty();
extern __attribute__((weak))	void HandleFifoNotEmpty();
extern __attribute__((weak))	int main(int _argc, sint8 **_argv);

//weak symbols end

extern uint32 getIRQs();
extern void EnableIrq(uint32 IRQ);
extern void DisableIrq(uint32 IRQ);
extern void NDS_IRQHandler();
extern void IRQWait(uint32 reentrant,uint32 irqstowait);
extern void IRQVBlankWait();
extern void IRQInit();
extern void InterruptServiceRoutineARMCores();	//Actual Interrupt Handler

extern uint32 GLOBAL_IME;
extern uint32 enterSafeInterrupts();
extern void leaveSafeInterrupts();

#ifdef __cplusplus
}
#endif
