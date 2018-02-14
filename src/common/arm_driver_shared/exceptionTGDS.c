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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include <_ansi.h>
#include <reent.h>


#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "dmaTGDS.h"
#include "biosTGDS.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "exceptionTGDS.h"
#include "ipcfifoTGDS.h"

#include "InterruptsARMCores_h.h"
#include "memoryHandleTGDS.h"
#include "global_settings.h"



//File IO is stubbed even in buffered writes, so as a workaround I redirect the weak-symbol _vfprint_f (and that means good bye file stream operations on fatfs, thus we re-implement those by hand)
//while allowing to use printf in DS
int _vfprintf_r(struct _reent * reent, FILE *fp,const sint8 *fmt, va_list list){
	//only for debugging. In release code breaks ARM7 if ARM7 fully utilizes IWRAM
	/*
	char * Buf = NULL;
	int BufSize = 0;
	#ifdef ARM7
	Buf = (uint8*)getPrintfBuffer();
	BufSize = (int)sizeof(getsIPCSharedTGDS()->arm7PrintfBuf);
	#endif
	
	#ifdef ARM9
	Buf = (uint8*)&g_printfbuf[0];
	BufSize = (int)sizeof(g_printfbuf);
	#endif
	
	//merge any "..." special arguments where sint8 * ftm requires , store into g_printfbuf
	vsnprintf((sint8*)Buf, BufSize, fmt, list);
	
	#ifdef ARM7
	SendMultipleWordACK(FIFO_PRINTF_7, (uint32)0, (uint32)0, (uint32)0);
	#endif
	
	#ifdef ARM9
	// FIXME
	t_GUIZone zone;
	zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
	zone.font = &trebuchet_9_font;
	GUI_drawText(&zone, 0, GUI.printfy, 255, (sint8*)g_printfbuf);
	GUI.printfy += GUI_getFontHeight(&zone);
	#endif
	
	return (strlen((char*)Buf));
	*/
	
	
	//Release code. Safe
	#ifdef ARM7
	return 0;
	#endif
	
	#ifdef ARM9
	char * Buf = NULL;
	Buf = (uint8*)&g_printfbuf[0];
	
	//merge any "..." special arguments where sint8 * ftm requires , store into g_printfbuf
	vsnprintf ((sint8*)Buf, (int)sizeof(g_printfbuf), fmt, list);
	
	// FIXME
	t_GUIZone zone;
	zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
	zone.font = &trebuchet_9_font;
	GUI_drawText(&zone, 0, GUI.printfy, 255, (sint8*)g_printfbuf);
	GUI.printfy += GUI_getFontHeight(&zone);
	return (strlen((char*)Buf));
	#endif
}



#include "InterruptsARMCores_h.h"

void setupExceptionHandler(){
	
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
	//*(uint32*)0x0380FFDC = (uint32)&sysexit;
	#endif
	
	#ifdef ARM9
	*(uint32*)0x02FFFD9C = (uint32)DebugException;
	#endif
	
	#endif
	
}

//Exception Sources

//data abort



uint32 exceptionArmRegs[0x20];

//crt0 wrong exit
//__attribute__((section(".itcm"))) //cant be at ITCM
void exception_sysexit(){
	#ifdef ARM7
	//SendArm9Command(EXCEPTION_ARM7,unexpectedsysexit_7,0x0,0x0);
	SendMultipleWordACK(EXCEPTION_ARM7, unexpectedsysexit_7, 0, NULL);
	while(1){
		IRQWait(1,IRQ_VBLANK);
	}
	#endif
	
	#ifdef ARM9
	exception_handler((uint32)unexpectedsysexit_9);
	#endif
}

#ifdef ARM9

//data abort
//__attribute__((section(".itcm")))	//cant be at ITCM
void exception_data_abort(){
	exception_handler((uint32)dataabort_9);
}

//__attribute__((section(".itcm"))) //cant be at ITCM
void exception_handler(uint32 arg)
{
	GUI_clear();
	
	if(arg == (uint32)unexpectedsysexit_9){
		printf("sysexit segfault! ARM9: out of NDS main scope...");
	}
	
	else if(arg == (uint32)unexpectedsysexit_7){
		printf("sysexit segfault! ARM7: out of NDS main scope...");
	}
	
	else if(arg == dataabort_9){	
		printf("ARM9: DATA ABORT. ");
	
		uint32 * debugVector = (uint32 *)&exceptionArmRegs[0];
		uint32 pc_abort = (uint32)exceptionArmRegs[0xf];
		
		if((debugVector[0xe] & 0x1f) == 0x17){
			pc_abort = pc_abort - 8;
		}
		
		printf("R0[%x] R1[%X] R2[%X] \n",debugVector[0],debugVector[1],debugVector[2]);
		printf("R3[%x] R4[%X] R5[%X] \n",debugVector[3],debugVector[4],debugVector[5]);
		printf("R6[%x] R7[%X] R8[%X] \n",debugVector[6],debugVector[7],debugVector[8]);
		printf("R9[%x] R10[%X] R11[%X] \n",debugVector[9],debugVector[0xa],debugVector[0xb]);
		printf("R12[%x] R13[%X] R14[%X]  \n",debugVector[0xc],debugVector[0xd],debugVector[0xe]);
		printf("R15[%x] SPSR[%x] CPSR[%X]  \n",pc_abort,debugVector[17],debugVector[16]);
		
		
		//red
		//BG_PALETTE_SUB[0] = RGB15(31,0,0);
		//BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		//green
		//BG_PALETTE_SUB[0] = RGB15(0,31,0);
		//BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		//blue
		BG_PALETTE_SUB[0] = RGB15(0,0,31);
		BG_PALETTE_SUB[255] = RGB15(31,31,31);
	}
	
	while(1){}
}
#endif