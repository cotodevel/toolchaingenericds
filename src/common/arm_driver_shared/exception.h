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

#ifndef __exceptions_h__
#define __exceptions_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"

#ifdef ARM7
#endif

#ifdef ARM9
#include "console.h"
#endif

#define unexpectedsysexit_9	0xdeaddea9
#define unexpectedsysexit_7	0xdeaddea7

#ifdef ARM9
#define dataabort_9	0xdead1000
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void setupExceptionHandler();

extern uint32 exceptionArmRegs[0x20];
extern void exception_sysexit();
extern void exception_data_abort();

extern void exception_handler(uint32 arg);
extern void DebugException();

#ifdef __cplusplus
}
#endif
