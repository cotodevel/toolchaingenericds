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

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"

#include <ctype.h>
#include <_ansi.h>
#include <reent.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "dmaTGDS.h"
#include "biosTGDS.h"
#include "exceptionTGDS.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "global_settings.h"
#include "keypadTGDS.h"
#include "posixHandleTGDS.h"

#ifdef ARM9
#include "nds_cp15_misc.h"
#include "debugNocash.h"
#include "dldi.h"
#endif

void setupDefaultExceptionHandler(){
	
	//27FFD9Ch - RAM - NDS9 Debug Stacktop / Debug Vector (0=None)
	//380FFDCh - RAM - NDS7 Debug Stacktop / Debug Vector (0=None)
	//These addresses contain a 32bit pointer to the Debug Handler, and, memory below of the addresses is used as Debug Stack. 
	//The debug handler is called on undefined instruction exceptions, on data/prefetch aborts (caused by the protection unit), 
	//on FIQ (possibly caused by hardware debuggers). It is also called by accidental software-jumps to the reset vector, and by unused SWI numbers within range 0..1Fh.
	
	#ifdef EXCEPTION_VECTORS_0x00000000
	//todo: replace projects that their own exception vectors @ 0x00000000 methods for raising exceptions
	#endif
	
	#ifdef EXCEPTION_VECTORS_0xffff0000
	
	#ifdef ARM7
	*(uint32*)0x0380FFDC = (uint32)DebugException;
	#endif
	
	#ifdef ARM9
	*(uint32*)0x02FFFD9C = (uint32)DebugException;
	SendFIFOWords(TGDS_ARM7_SETUPEXCEPTIONHANDLER, (u32)&exceptionArmRegs[0]);
	#endif
	
	#endif
	
}

//setupCustomExceptionHandler(&HandlerFunction);
uint32 CustomHandler = 0;
void setupCustomExceptionHandler(uint32 * Handler){
	CustomHandler = (uint32)(uint32 *)Handler;
	
	#ifdef ARM7
	*(uint32*)0x0380FFDC = (uint32)CustomDebugException;
	#endif
	
	#ifdef ARM9
	*(uint32*)0x02FFFD9C = (uint32)CustomDebugException;
	#endif
}

#ifdef ARM7
uint8 * exceptionArmRegsShared = NULL;
#endif

#ifdef ARM9
uint32 exceptionArmRegs[0x20];
#endif


//crt0 wrong exit
//__attribute__((section(".itcm"))) //cant be at ITCM
void exception_sysexit(){
	#ifdef ARM7
	
	SendFIFOWords(EXCEPTION_ARM7, unexpectedsysexit_7);
	while(1){
		IRQWait(1, IRQ_VBLANK);
	}
	#endif
	
	#ifdef ARM9
	exception_handler((uint32)unexpectedsysexit_9);
	#endif
}

//__attribute__((section(".itcm")))	//cant be at ITCM
void generalARMExceptionHandler(){
	#ifdef ARM7
	
	SendFIFOWords(EXCEPTION_ARM7, generalARM7Exception);
	while(1==1){
		IRQVBlankWait();
	}
	#endif
	#ifdef ARM9
	exception_handler((uint32)generalARM9Exception);
	#endif
}

#ifdef ARM9
static bool GDBSession;

//__attribute__((section(".itcm"))) //cant be at ITCM
void exception_handler(uint32 arg){
	bool isTGDSCustomConsole = false;	//set default console or custom console: default console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	printf(" -- ");
	if(arg == (uint32)unexpectedsysexit_9){
		printf("ARM9 Exception:Unexpected main() exit.");
		while(1==1){
			IRQVBlankWait();
		}
	}
	
	else if(arg == (uint32)unexpectedsysexit_7){
		printf("ARM7 Exception:Unexpected main() exit.");
		while(1==1){
			IRQVBlankWait();
		}
	}
	
	else{
		if(arg == (uint32)generalARM7Exception){
			printf("ARM7: Hardware Exception. ");
			printf("ARM7 Debug Vector: ");
			coherent_user_range_by_size((uint32)&exceptionArmRegs[0], sizeof(exceptionArmRegs));
		}
		else if(arg == (uint32)generalARM9Exception){
			printf("ARM9: Hardware Exception. ");
			printf("ARM9 Debug Vector: ");
		}
		else{
			printf("?????????: Unhandled Exception.");
		}
		
		uint32 * debugVector = (uint32 *)&exceptionArmRegs[0]; //Shared buffer ARM7 / ARM9
		uint32 pc_abort = (uint32)exceptionArmRegs[0xf];
		
		if((debugVector[0xe] & 0x1f) == 0x17){
			debugVector[0xf] = pc_abort - 8;
		}
		
		//add support for GDB session.
		printf("R0[%x] R1[%X] R2[%X] ",debugVector[0],debugVector[1],debugVector[2]);
		printf("R3[%x] R4[%X] R5[%X] ",debugVector[3],debugVector[4],debugVector[5]);
		printf("R6[%x] R7[%X] R8[%X] ",debugVector[6],debugVector[7],debugVector[8]);
		printf("R9[%x] R10[%X] R11[%X] ",debugVector[9],debugVector[0xa],debugVector[0xb]);
		printf("R12[%x] R13[%X] R14[%X]  ",debugVector[0xc],debugVector[0xd],debugVector[0xe]);
		printf("R15[%x] SPSR[%x] CPSR[%X]  ",debugVector[0xf],debugVector[17],debugVector[16]);
		
		//red
		//BG_PALETTE_SUB[0] = RGB15(31,0,0);
		//BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		//green
		//BG_PALETTE_SUB[0] = RGB15(0,31,0);
		//BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		//blue
		BG_PALETTE_SUB[0] = RGB15(0,0,31);
		BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		printf("A: Enable GDB Debugging. ");
		printf("(check: toolchaingenericds-gdbstub-example project)");
		printf("B: Skip GDB Debugging");
		
		while(1){
			scanKeys();
			int isdaas = keysDown();
			if (isdaas&KEY_A)
			{
				GDBSession =  true;
				break;
			}
			if(isdaas&KEY_B)
			{
				GDBSession =  false;
				break;
			}
		}
	
		if(GDBSession == true){
			LeaveExceptionMode();	//code works in ITCM now
		}
		
	}
	
	while(1){
		
		IRQVBlankWait();
	}
}
#endif

int TGDSInitLoopCount=0;
#ifdef ARM9

char tempBuf[256];

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleDSInitError(int stage, u32 fwNo){
	bool isTGDSCustomConsole = false;	//set default console
	GUI_init(isTGDSCustomConsole);
	clrscr();
	
	printf(" ---- ");
	printf(" ---- ");
	printf(" ---- ");
	
	//Stage 0 = failed detecting DS model from firmware
	//Stage 1 = failed initializing ARM7 DLDI / NDS ARM9 memory allocator
	//Stage 2 = failed initializing DSWIFI (ARM9)
	//Stage 3 = failed detecting DS model from firmware (2).
	//Stage 4 = TWL Mode: SCFG_EXT7 locked. ToolchainGenericDS SDK needs it to run from SD in TWL mode.
	//Stage 5 = TWL Mode: SCFG_EXT9 locked. ToolchainGenericDS SDK needs it to run from SD in TWL mode.
	//Stage 6 = TGDS App has quit through exit(int status);
	
	
	
	
	sprintf(tempBuf, "TGDS boot fail: Stage %d, firmware model: %d\n", stage, fwNo);
	printf(tempBuf);
	
	if(stage == 1){
		printf("DLDI: [%s]", (char*)&dldiGet()->friendlyName[0]);
	}
	else if(stage == 4){
		printf("TWL Mode: SCFG_EXT7 locked. Unlaunch and TWiLightMenu++ only supported.");
	}
	else if(stage == 5){
		printf("TWL Mode: SCFG_EXT9 locked. Unlaunch and TWiLightMenu++ only supported.");
	}
	else if(stage == 6){
		if(exitValue != -10000){
			sprintf(tempBuf, "ToolchainGenericDS App has quit through exit(%d); .\n", exitValue);
		}
		else{
			sprintf(tempBuf, "ToolchainGenericDS App: abort(); .\n");
		}
		printf(tempBuf);
	}
	while(1==1){
		swiDelay(1);
	}
}
#endif