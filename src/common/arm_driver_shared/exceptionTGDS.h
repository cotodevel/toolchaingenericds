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
#include "typedefsTGDS.h"

#include "ipcfifoTGDS.h"

#include <ctype.h>
#include <stdlib.h>

#include <sys/reent.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <_ansi.h>
#include <reent.h>
#include <sys/lock.h>
#include <fcntl.h>


#ifdef ARM7
#endif

#ifdef ARM9
#include "consoleTGDS.h"
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

#ifdef ARM7
extern void printf7(char *fmt, ...);
#endif

extern void setupDefaultExceptionHandler();

extern uint32 exceptionArmRegs[0x20];
extern void exception_sysexit();
extern void exception_data_abort();

extern void exception_handler(uint32 arg);
extern void DebugException();

//custom handler
extern void setupCustomExceptionHandler(uint32 * Handler);
extern uint32 CustomHandler;
extern void CustomDebugException();

extern int _vfprintf_r(struct _reent * reent, FILE *fp,const sint8 *fmt, va_list list);
extern void printfCoords(int x, int y, const char *format, ...);


extern void LeaveExceptionMode();

#ifdef __cplusplus
}
#endif