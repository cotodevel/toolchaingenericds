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

#ifdef ARM9
#include "consoleTGDS.h"
#endif

//Hardware Exceptions
#define generalARM7Exception	0xdead1000	//Memory Protection Unit signaled ARM7 Exception: accidental software-jumps to the reset vector / unused SWI numbers within range 0..1Fh. / undefined exception / prefetch abort
#define generalARM9Exception	0xdead1001	//Memory Protection Unit signaled ARM9 Exception: accidental software-jumps to the reset vector / unused SWI numbers within range 0..1Fh. / undefined exception / prefetch abort / MPU Data Abort

//Software Exceptions
#define unexpectedsysexit_9	(u32)(0xdeaddea9) 	//when ARM9's main() quits
#define unexpectedsysexit_7	(u32)(0xdeaddea7)	//when ARM7's main() quits
#define manualexception_9	(u32)(0xdead0001)	//ARM9 wrapper for handleDSInitError(int stage, u32 fwNo). Used by User Code exceptions triggered from ARM9
#define manualexception_7	(u32)(0xdead0002)	//ARM7 wrapper for handleDSInitError(int stage, u32 fwNo). Used by User Code exceptions triggered from ARM7

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void handleDSInitOutputMessage(char * msg);

#ifdef ARM7
extern uint8 * exceptionArmRegsShared;
extern void handleDSInitError7(int stage, u32 fwNo);
#endif

#ifdef ARM9
extern uint32 exceptionArmRegs[0x20];
extern char msgDebugException[MAX_TGDSFILENAME_LENGTH];
#endif

extern void setupDefaultExceptionHandler();
extern void setupDisabledExceptionHandler();
extern void exception_sysexit();
extern void generalARMExceptionHandler();

extern void exception_handler(uint32 arg, int stage, u32 fwNo);
extern void DebugException();

//custom handler
extern void setupCustomExceptionHandler(uint32 * Handler);
extern uint32 CustomHandler;
extern void CustomDebugException();

extern void handleDSInitError(int stage, u32 fwNo);
extern int TGDSInitLoopCount;

#ifdef ARM7
extern char * sharedStringExceptionMessageOutput;
#endif

#ifdef ARM9
extern char sharedStringExceptionMessage[256];
extern u32 sharedBufHandler[2];
extern void ds_malloc_abort(void);

extern void handleUninitializedTGDSMalloc9();
extern void handleUninitializedTGDSCalloc9();
extern void handleUninitializedTGDSFree9();
extern void handleUninitializedTGDSMallocFreeMemory9();

extern void TGDSStubbedUserHandlerIRQCallback_Exception();
#endif

#ifdef __cplusplus
}
#endif