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
#include "powerTGDS.h"
#include "exceptionTGDS.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "global_settings.h"
#include "keypadTGDS.h"
#include "posixHandleTGDS.h"
#include "malloc.h"
#include "utilsTGDS.h"
#include "TGDS_threads.h"

#ifdef ARM7
#include "spifwTGDS.h"
#endif

#ifdef ARM9
#include "nds_cp15_misc.h"
#include "debugNocash.h"
#include "dldi.h"
#include "videoTGDS.h"
#endif

#ifdef ARM7
char * sharedStringExceptionMessageOutput = NULL;
#endif

#ifdef ARM9
char sharedStringExceptionMessage[256];
#endif

//27FFD9Ch - RAM - NDS9 Debug Stacktop / Debug Vector (0=None)
	//380FFDCh - RAM - NDS7 Debug Stacktop / Debug Vector (0=None)
	//These addresses contain a 32bit pointer to the Debug Handler, and, memory below of the addresses is used as Debug Stack. 
	//The debug handler is called on undefined instruction exceptions, on data/prefetch aborts (caused by the protection unit), 
	//on FIQ (possibly caused by hardware debuggers). It is also called by accidental software-jumps to the reset vector, and by unused SWI numbers within range 0..1Fh.
	

void setupDisabledExceptionHandler(){
	#ifdef EXCEPTION_VECTORS_0x00000000
	//todo: replace projects that their own exception vectors @ 0x00000000 methods for raising exceptions
	#endif
	
	#ifdef EXCEPTION_VECTORS_0xffff0000
	
	#ifdef ARM7
	*(uint32*)0x0380FFDC = (uint32)0;
	#endif
	
	#ifdef ARM9
	*(uint32*)0x02FFFD9C = (uint32)0;
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (u32)&exceptionArmRegs[0]);
	setValueSafe(&fifomsg[7], (u32)TGDS_ARM7_SETUPDISABLEDEXCEPTIONHANDLER);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	#endif
	
	#endif
}


u32 sharedBufHandler[2];

void setupDefaultExceptionHandler(){
	
	#ifdef EXCEPTION_VECTORS_0x00000000
	//todo: replace projects that their own exception vectors @ 0x00000000 methods for raising exceptions
	#endif
	
	#ifdef EXCEPTION_VECTORS_0xffff0000
	
	#ifdef ARM7
	*(uint32*)0x0380FFDC = (uint32)DebugException;
	#endif
	
	#ifdef ARM9
	*(uint32*)0x02FFFD9C = (uint32)DebugException;
	setValueSafe(&sharedBufHandler[0], &exceptionArmRegs[0]);
	setValueSafe(&sharedBufHandler[1], &sharedStringExceptionMessage[0]);
	
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (u32)&sharedBufHandler);
	setValueSafe(&fifomsg[7], (u32)TGDS_ARM7_SETUPEXCEPTIONHANDLER);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
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
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (u32)unexpectedsysexit_7);
	setValueSafe(&fifomsg[7], (u32)EXCEPTION_ARM7);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while(1){
		IRQWait(1, IRQ_VBLANK);
	}
	#endif
	
	#ifdef ARM9
	exception_handler((uint32)unexpectedsysexit_9, 0, 0);
	#endif
}

//__attribute__((section(".itcm")))	//cant be at ITCM
void generalARMExceptionHandler(){
	#ifdef ARM7
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (u32)generalARM7Exception);
	setValueSafe(&fifomsg[7], (u32)EXCEPTION_ARM7);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while(1==1){
		IRQVBlankWait();
	}
	#endif
	#ifdef ARM9
	exception_handler((uint32)generalARM9Exception, 0, 0);
	#endif
}

#ifdef ARM9

//Internal callback for TGDS Project IRQ handlers. Required for Exception Handling, so TGDS Project interrupts do not cause issues on Exceptions. 
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TGDSStubbedUserHandlerIRQCallback_Exception(){

}

static bool GDBSession;
char msgDebugException[MAX_TGDSFILENAME_LENGTH];

//__attribute__((section(".itcm"))) //cant be at ITCM
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void exception_handler(uint32 arg, int stage, u32 fwNo){
	bool isWirelessAvailable = isTGDSWirelessServiceAvailable();
	bool isTGDSCustomConsole = false;	//reloading cause issues. Thus this ensures Console to be inited even when reloading
	GUI_init(isTGDSCustomConsole);
	sint32 fwlanguage = (sint32)getLanguage();
	GUI_clear();
	VRAMBLOCK_SETBANK_C(VRAM_C_0x06200000_ENGINE_B_BG);	
	
	//Disable timers, and if we eventually use GDB, re-enable one
	REG_IE &= ~(IRQ_TIMER0|IRQ_TIMER1|IRQ_TIMER2|IRQ_TIMER3);
	
	//Stub out TGDS Project IRQ Handlers
	*(u32*)&Timer0handlerUser = (u32)&TGDSStubbedUserHandlerIRQCallback_Exception;
	*(u32*)&Timer1handlerUser = (u32)&TGDSStubbedUserHandlerIRQCallback_Exception;
	*(u32*)&Timer2handlerUser = (u32)&TGDSStubbedUserHandlerIRQCallback_Exception;
	*(u32*)&Timer3handlerUser = (u32)&TGDSStubbedUserHandlerIRQCallback_Exception;
	*(u32*)&VblankUser = (u32)&TGDSStubbedUserHandlerIRQCallback_Exception;
	*(u32*)&VcounterUser = (u32)&TGDSStubbedUserHandlerIRQCallback_Exception;
	if(isWirelessAvailable == true){
		REG_IME = 1;
	}

	powerOFF3DEngine(); //Power off ARM9 3D Engine to save power
	powerOFF(POWER_2D_A);
	setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
	
	GUI_printf(" ---- ");
	
	if(arg == (uint32)unexpectedsysexit_9){
		GUI_printf("ARM9 Exception:Unexpected main() exit.");
		while(1==1){
			IRQVBlankWait();
		}
	}
	
	else if(arg == (uint32)unexpectedsysexit_7){
		GUI_printf("ARM7 Exception:Unexpected main() exit.");
		while(1==1){
			IRQVBlankWait();
		}
	}
	
	else{
		if(arg == (uint32)generalARM7Exception){
			GUI_printf("ARM7: Hardware Exception. ");
			GUI_printf("ARM7 Debug Vector: ");
			coherent_user_range_by_size((uint32)&exceptionArmRegs[0], sizeof(exceptionArmRegs));
		}
		else if(arg == (uint32)generalARM9Exception){
			GUI_printf("ARM9: Hardware Exception. ");
			GUI_printf("ARM9 Debug Vector: ");
		}
		else if(arg == (uint32)manualexception_9){
			GUI_printf("ARM9: User Exception.");
		}
		else if(arg == (uint32)manualexception_7){
			GUI_printf("ARM7: User Exception.");
		}
		else {
			GUI_printf("ARM9: Unhandled Exception.");
		}
		
		uint32 * debugVector = (uint32 *)&exceptionArmRegs[0]; //Shared buffer ARM7 / ARM9
		uint32 pc_abort = (uint32)exceptionArmRegs[0xf];
		
		if((debugVector[0xe] & 0x1f) == 0x17){
			debugVector[0xf] = pc_abort - 8;
		}
		
		if(
			(arg == (uint32)generalARM7Exception)
			||
			(arg == (uint32)generalARM9Exception)
		){
			GUI_printf("R0[%x] R1[%X] R2[%X] ",debugVector[0],debugVector[1],debugVector[2]);
			GUI_printf("R3[%x] R4[%X] R5[%X] ",debugVector[3],debugVector[4],debugVector[5]);
			GUI_printf("R6[%x] R7[%X] R8[%X] ",debugVector[6],debugVector[7],debugVector[8]);
			GUI_printf("R9[%x] R10[%X] R11[%X] ",debugVector[9],debugVector[0xa],debugVector[0xb]);
			GUI_printf("R12[%x] R13[%X] R14[%X]  ",debugVector[0xc],debugVector[0xd],debugVector[0xe]);
			GUI_printf("R15[%x] SPSR[%x] CPSR[%X]  ",debugVector[0xf],debugVector[17],debugVector[16]);
		}
		else{
			//Stage 0 = TGDS-MB's compatibility on isNDSBinaryV1Slot2 binaries are on the to-do list
			//Stage 1 = Failed initializing ARM7 DLDI.
			//Stage 2 = Failed initializing DSWIFI (ARM9).
			//Stage 3 = Trying to reload ToolchainGenericDS-multiboot, but missing payload (ARM9).
			//Stage 4 = TWL Mode: SCFG_EXT7 locked. ToolchainGenericDS SDK needs it to run from SD in TWL mode.
			//Stage 5 = TWL Mode: SCFG_EXT9 locked. ToolchainGenericDS SDK needs it to run from SD in TWL mode.
			//Stage 6 = TGDS App has quit through exit(int status);
			//Stage 7 = TGDS TWL App trying to be ran in NTR mode. (unused)
			//Stage 8 = TGDS NTR App trying to be ran in TWL mode. (unused)
			//Stage 9 = TGDS ARM7 Payload reloading failed.
			//Stage 10 = Custom manual exception (ARM7)
			//Stage 11 = dlmalloc abort (ARM9). Reasons: mis-aligned buffer freed, fragmented reallocation (buffer was not freed/reallocated correctly), NULL ptr @ free();, EWRAM out of memory, etc
			GUI_printf("TGDS boot fail: Stage [%d], firmware model: [0x%x]", stage, fwNo);
			
			int isNTRTWLBinary = isThisPayloadNTROrTWLMode();
			if (isNTRTWLBinary == isTWLBinary){
				GUI_printf("TGDS ARM9 Payload: [TWL] mode");
			}
			else if (isNTRTWLBinary == isNDSBinaryV3){
				GUI_printf("TGDS ARM9 Payload: [NTR] mode");
			} 
			else{
				GUI_printf("TGDS ARM9 Payload: Failed to detect ");
			}
			if(stage == 0){
				GUI_printf("Unsupported isNDSBinaryV1Slot2 binary format .ds.gba ");
				GUI_printf("for the time being. ");
			}
			else if(stage == 1){
				GUI_printf("ARM7DLDI failed to initialize.");
				GUI_printf("Manually patch the correct DLDI for , ");
				GUI_printf("your card into this TGDS binary ");
				GUI_printf("and try again. ");
				GUI_printf("DLDI: [%s]", (char*)&dldiGet()->friendlyName[0]);
			}
			else if(stage == 2){
				GUI_printf("DSWIFI (ARM9) init fail. Didn't link the dswifi lib in ARM9?");
			}
			else if(stage == 3){
				GUI_printf("ToolchainGenericDS-multiboot: missing [%s] payload.", msgDebugException);
				GUI_printf("Copy [%s] payload in SD root and try again", msgDebugException);
			}
			else if((stage == 4) && (__dsimode == true)){
				GUI_printf("TWL Mode: SCFG_EXT7 locked. Unlaunch and TWiLightMenu++ only supported.");
			}
			else if((stage == 5) && (__dsimode == true)){
				GUI_printf("TWL Mode: SCFG_EXT9 locked. Unlaunch and TWiLightMenu++ only supported.");
			}
			else if(stage == 6){
				if(exitValue != -10000){
					GUI_printf("ToolchainGenericDS App has quit through exit(%d); .", exitValue);
				}
				else{
					GUI_printf("ToolchainGenericDS App: abort(); .");
				}
			}
			else if(stage == 7){
				GUI_printf("Unsupported [TWL] binary running on NTR mode hardware. ");
				GUI_printf("Please run the same TGDS Project,");
				GUI_printf("but using its [NTR] binary counterpart.");
			}
			else if(stage == 8){
				GUI_printf("Unsupported [NTR] binary running on TWL mode hardware. ");
				GUI_printf("Please run the same TGDS Project, ");
				GUI_printf("but using its [TWL] binary counterpart.");
			}
			else if(stage == 9){
				GUI_printf("TGDS ARM7 Payload reloading failed. ");
			}
			else if(stage == 10){
				GUI_printf(sharedStringExceptionMessage);
			}
			else if(stage == 11){
				GUI_printf("dlmalloc abort (ARM9). Reasons: mis-aligned buffer freed, ");
				GUI_printf("fragmented reallocation (buffer was not freed/reallocated correctly), ");
				GUI_printf("NULL ptr @ free();, EWRAM out of memory, etc.");
				GUI_printf("-");
				GUI_printf("Report this issue at:");
				GUI_printf("https://github.com/cotodevel/toolchaingenericds/issues");
			}
			else{
				GUI_printf("handleDSInitError(); Unhandled event. Contact developer.");
				GUI_printf("Halting system. ");
			}
		}
		//red
		//BG_PALETTE_SUB[0] = RGB15(31,0,0);
		//BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		//green
		//BG_PALETTE_SUB[0] = RGB15(0,31,0);
		//BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		//blue
		BG_PALETTE_SUB[0] = RGB15(0,0,31);
		BG_PALETTE_SUB[255] = RGB15(31,31,31);
		
		if( (isWirelessAvailable == true) && (GdbStubUserCodeHandlerLibUtilsCallback != NULL) ){
			GUI_printf("(A): Enable GDB Debugging. ");
			GUI_printf("(check: toolchaingenericds-gdbstub-example project)");
			GUI_printf("(B): Skip GDB Debugging. ");
			
			while(1){
				scanKeys();
				int keysDwn = keysDown();
				if (keysDwn&KEY_A)
				{
					GDBSession =  true;
					ARMEnterSysMode();	//Enter Sys mode: Disable MPU, Allows the GDB process to run.
					if(wifiswitchDsWnifiModeARM9LibUtilsCallback != NULL){
						wifiswitchDsWnifiModeARM9LibUtilsCallback((sint32)9);	//dswifi_gdbstubmode (sint32)(9)	//GDB Stub mode
					}
					break;
				}
				if(keysDwn&KEY_B)
				{
					GDBSession =  false;
					break;
				}
			}

			if(GDBSession == false){
				GUI_printf("(B pressed): GDB Debugging disabled. Halt.");
			}
		}
		else{
			GUI_printf("TGDS Project Missing Wifi: GDB disabled. ");
		}
	}
	
	while(1){
		if( (isWirelessAvailable == true) && (GdbStubUserCodeHandlerLibUtilsCallback != NULL) && (GDBSession == true) ){
			GdbStubUserCodeHandlerLibUtilsCallback();
		}
		HaltUntilIRQ();
	}
}
#endif

int TGDSInitLoopCount=0;


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleDSInitOutputMessage(char * msg){
	#ifdef ARM7
	strcpy(sharedStringExceptionMessageOutput, msg);
	#endif
	#ifdef ARM9
	strcpy(sharedStringExceptionMessage, msg);
	#endif
}

#ifdef ARM7

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleDSInitError7(int stage, u32 fwNo){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (u32)manualexception_7);
	setValueSafe(&fifomsg[1], (u32)stage);
	setValueSafe(&fifomsg[2], (u32)fwNo);
	setValueSafe(&fifomsg[7], (u32)EXCEPTION_ARM7);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while(1==1){
		IRQVBlankWait();
	}
}
#endif

#ifdef ARM7

//////////////////////////////////////////////////////// ARM7 Threading exception handler: TGDS Project template ////////////////////////////////////////////////////////
//User callback when Task Overflows. Intended for debugging purposes only, as normal user code tasks won't overflow if a task is implemented properly.
//	u32 * args = This Task context
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void onThreadOverflowARM7InternalCode(u32 * args){
	struct task_def * thisTask = (struct task_def *)args;
	struct task_Context * parentTaskCtx = thisTask->parentTaskCtx;	//get parent Task Context node 

	char threadStatus[64];
	switch(thisTask->taskStatus){
		case(INVAL_THREAD):{
			strcpy(threadStatus, "INVAL_THREAD");
		}break;
		
		case(THREAD_OVERFLOW):{
			strcpy(threadStatus, "THREAD_OVERFLOW");
		}break;
		
		case(THREAD_EXECUTE_OK_WAIT_FOR_SLEEP):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAIT_FOR_SLEEP");
		}break;
		
		case(THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE");
		}break;
	}
	
	char debOut2[256];
	int TGDSDebuggerStage = 10;
	strcpy(debOut2, "onThreadOverflowARM7InternalCode():[");
	strcat(debOut2, threadStatus);
	strcat(debOut2, "]! Halting. ");
	handleDSInitOutputMessage((char*)&debOut2[0]);
	handleDSInitError7(TGDSDebuggerStage, (u32)savedDSHardware);
	
	while(1==1){
		HaltUntilIRQ();
	}
}

#endif



#ifdef ARM9

char tempBuf[256];

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleDSInitError(int stage, u32 fwNo){
	exception_handler(manualexception_9, stage, fwNo);
}

//newlib-nds's dlmalloc abort handler
void ds_malloc_abort(void){
	//Throw exception always
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	int stage = 11;
	handleDSInitError(stage, (u32)fwNo);
}

//Context: Default callbacks to handle uninitialized, but required functionality at runtime

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleUninitializedTGDSMalloc9(){
	int TGDSDebuggerStage = 10;
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	sprintf((char*)ConsolePrintfBuf, "ARM9: TGDSMalloc9(): NOT initialized. Halt.");
	handleDSInitOutputMessage((char*)ConsolePrintfBuf);
	handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleUninitializedTGDSCalloc9(){
	int TGDSDebuggerStage = 10;
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	sprintf((char*)ConsolePrintfBuf, "ARM9: TGDSCalloc9(): NOT initialized. Halt.");
	handleDSInitOutputMessage((char*)ConsolePrintfBuf);
	handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleUninitializedTGDSFree9(){
	int TGDSDebuggerStage = 10;
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	sprintf((char*)ConsolePrintfBuf, "ARM9: TGDSFree9(): NOT initialized. Halt.");
	handleDSInitOutputMessage((char*)ConsolePrintfBuf);
	handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleUninitializedTGDSMallocFreeMemory9(){
	int TGDSDebuggerStage = 10;
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	sprintf((char*)ConsolePrintfBuf, "ARM9: TGDSMallocFreeMemory9(): NOT initialized. Halt.");
	handleDSInitOutputMessage((char*)ConsolePrintfBuf);
	handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
}

#endif