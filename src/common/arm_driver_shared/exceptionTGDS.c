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
#include "keypadTGDS.h"

#ifdef ARM9
#include "dswnifi_lib.h"
#endif

//File IO is stubbed even in buffered writes, so as a workaround I redirect the weak-symbol _vfprint_f (and that means good bye file stream operations on fatfs, thus we re-implement those by hand)
//while allowing to use printf in DS
int _vfprintf_r(struct _reent * reent, FILE *fp,const sint8 *fmt, va_list list){
	#ifdef ARM7
	return 0;
	#endif
	
	#ifdef ARM9
	char * stringBuf = (char *)&g_printfbuf[0];
	//merge any "..." special arguments where sint8 * ftm requires , store into g_printfbuf
	vsnprintf((sint8*)stringBuf, (int)sizeof(g_printfbuf), fmt, list);
	int stringSize = (int)strlen(stringBuf);
	t_GUIZone * zoneInst = getDefaultZoneConsole();
	bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	int color = 0xff;	//white
	GUI_drawText(zoneInst, 0, GUI.printfy, color, stringBuf, readAndBlendFromVRAM);
	GUI.printfy += getFontHeightFromZone(zoneInst);	//skip to next line
	return stringSize;
	#endif
}


#ifdef ARM9
//this needs a rework
void printfCoords(int x, int y, const char *format, ...){
	va_list args;
    va_start(args, format);
	char * stringBuf = (char *)&g_printfbuf[0];
	//merge any "..." special arguments where sint8 * ftm requires , store into g_printfbuf
	vsnprintf ((sint8*)stringBuf, (int)sizeof(g_printfbuf), format, args);
	va_end(args);
	int stringSize = (int)strlen(stringBuf);
	t_GUIZone * zoneInst = getDefaultZoneConsole();
	GUI.printfy = y * getFontHeightFromZone(zoneInst);
	bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	int color = 0xff;	//white
	GUI_drawText(zoneInst, x, GUI.printfy, color, stringBuf, readAndBlendFromVRAM);
	GUI.printfy += getFontHeightFromZone(zoneInst);	//skip to next line
	return stringSize;
}
#endif

#include "InterruptsARMCores_h.h"

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
	//*(uint32*)0x0380FFDC = (uint32)&sysexit;
	#endif
	
	#ifdef ARM9
	*(uint32*)0x02FFFD9C = (uint32)DebugException;
	#endif
	
	#endif
	
}

//setupCustomExceptionHandler(&HandlerFunction);
uint32 CustomHandler = 0;
void setupCustomExceptionHandler(uint32 * Handler){
	
	#ifdef ARM9
	CustomHandler = (uint32)(uint32 *)Handler;
	*(uint32*)0x02FFFD9C = (uint32)CustomDebugException;
	#endif
}

//Exception Sources

//data abort



uint32 exceptionArmRegs[0x20];

//crt0 wrong exit
//__attribute__((section(".itcm"))) //cant be at ITCM
void exception_sysexit(){
	#ifdef ARM7
	SendFIFOWords(EXCEPTION_ARM7, unexpectedsysexit_7, 0, NULL);
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

static bool GDBSession;

//__attribute__((section(".itcm"))) //cant be at ITCM
void exception_handler(uint32 arg){
	GUI_clear();
	
	if(arg == (uint32)unexpectedsysexit_9){
		printf("sysexit segfault! ARM9: out of NDS main scope...");
	}
	
	else if(arg == (uint32)unexpectedsysexit_7){
		printf("sysexit segfault! ARM7: out of NDS main scope...");
	}
	
	else if(arg == dataabort_9){
		printf("          ");
		printf("ARM9: DATA ABORT. ");
		uint32 * debugVector = (uint32 *)&exceptionArmRegs[0];
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
			int isdaas = keysPressed();
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
		
		if(GDBSession == true){
			//GDB Stub Process must run here
			int retGDBVal = remoteStubMain();
			if(retGDBVal == remoteStubMainWIFINotConnected){
				if (switch_dswnifi_mode(dswifi_gdbstubmode) == true){
					//clrscr();
					//Show IP and port here
					printf("[Connect to GDB]:");
					printf("Port:%d GDB IP:%s",remotePort,(char*)print_ip((uint32)Wifi_GetIP()));
					remoteInit();
				}
				else{
					//GDB Client Reconnect:ERROR
				}
			}
			else if(retGDBVal == remoteStubMainWIFIConnectedGDBDisconnected){
				setWIFISetup(false);
				if (switch_dswnifi_mode(dswifi_gdbstubmode) == true){ // gdbNdsStart() called
					reconnectCount++;
					//clrscr();
					//Show IP and port here
					printf("[Re-Connect to GDB]:So far:%d time(s)",reconnectCount);
					printf("Port:%d GDB IP:%s",remotePort,(char*)print_ip((uint32)Wifi_GetIP()));
					remoteInit();
				}
			}
		}
		IRQVBlankWait();
	}
}
#endif