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

//Software BIOS modules by either replacement (bios logic replacement) or native BIOS support (use of the hardware bios rom vector, provided by ninty)

/////////////////////////////////////////////////// Shared BIOS ARM7/9 /////////////////////////////////////////////////////////////////
#include "biosTGDS.h"
#include "global_settings.h"
#include "dmaTGDS.h"
#include "InterruptsARMCores_h.h"
#include "posixHandleTGDS.h"
#include "TGDS_threads.h"

#ifdef ARM7
#include "spitscTGDS.h"
#endif

#ifdef ARM9
#include "dldi.h"
#include "dswnifi_lib.h"
#endif

//NDS BIOS Routines C code

//problem kaputt docs say DS uses a rounded 4 byte copy, be it a fillvalue to dest or direct copy from src to dest, by size.
//Dont optimize as vram is 16 or 32bit, optimization can end up in 8bit writes.
//writes either a COPY_FIXED_SOURCE value = [r0], or plain copy from source to destination
__attribute__((optimize("O0")))
void swiFastCopy(uint32 * source, uint32 * dest, int flags){
	int i = 0;
	if(flags & COPY_FIXED_SOURCE){
		uint32 value = *(uint32*)source;
		for(i = 0; i < (int)((flags<<2)&0x1fffff)/4; i++){
			dest[i] = value;
		}
	}
	else{
		for(i = 0; i < (int)((flags<<2)&0x1fffff)/4; i++){
			dest[i] = source[i];
		}
	}
}

#ifdef ARM9
HandledoMULTIDaemonWeakRefLibHardware9_fn HandledoMULTIDaemonWeakRefLibHardware9Callback = NULL;

//Once called, you consume the struct LZSSContext and then call free(struct LZSSContext.bufferSource)
struct LZSSContext LZS_DecodeFromBuffer(unsigned char *pak_buffer, unsigned int   pak_len){
	struct LZSSContext LZSSCtx;
	unsigned char *raw_buffer;
	unsigned int   raw_len, header;
	
	header = *pak_buffer;
	if (header != CMD_CODE_10) {
		LZSSCtx.bufferSource = NULL;
		LZSSCtx.bufferSize = -1;
		return LZSSCtx;
	}

	raw_len = *(unsigned int *)pak_buffer >> 8;
	raw_buffer = (unsigned char *) TGDSARM9Malloc(raw_len * sizeof(char));

	swiDecompressLZSSWram((void *)pak_buffer, (void *)raw_buffer);

	LZSSCtx.bufferSource = raw_buffer;
	LZSSCtx.bufferSize = raw_len;
	return LZSSCtx;
}
#endif

//TGDS Services:
#ifdef ARM7
bool isArm7ClosedLid=false;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleARM7InitSVC(){
	isArm7ClosedLid=false;
	
	//Thread context initialized here. Do not initialize it later or it'll get destroyed.
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	initThreadSystem(TGDSThreads);
	
	//ARM7 Lid Services
	int taskARM7SVCTimeMS = 2; //Task execution requires at least 2ms
	if(registerThread(TGDSThreads, (TaskFn)&taskARM7SVC, (u32*)NULL, taskARM7SVCTimeMS, (TaskFn)&onThreadOverflowARM7InternalCode, tUnitsMilliseconds) != THREAD_OVERFLOW){
        
    }

	//ARM7 Touchscreen. can't be enabled here because it's blocking. Needs to be ran on vcount interrupts.
	//int taskARM7TouchScreenTimeMS = 10; //Task execution requires at least 10ms
	//if(registerThread(TGDSThreads, (TaskFn)&taskARM7TouchScreen, (u32*)NULL, taskARM7TouchScreenTimeMS, (TaskFn)&onThreadOverflowARM7InternalCode, tUnitsMicroseconds) != THREAD_OVERFLOW){
			
	//}
	enableARM7TouchScreen();

}


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void taskARM7SVC(u32 * args){
	//Lid Closing + backlight events (ARM7)
	if( ((REG_KEYXY & KEY_HINGE) == KEY_HINGE) && (isArm7ClosedLid == false)){
		isArm7ClosedLid = true;
		TurnOffScreens();
		screenLidHasClosedhandlerUser();
	}
}
#endif

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleARM9InitSVC(){
	//Thread context initialized here. Do not initialize it later or it'll get destroyed.
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	initThreadSystem(TGDSThreads);
	
	int taskARM9SVCTimeMS = 5; //Task execution requires at least 5ms
	if(registerThread(TGDSThreads, (TaskFn)&taskARM9SVC, (u32*)NULL, taskARM9SVCTimeMS, (TaskFn)&onThreadOverflowUserCode, tUnitsMilliseconds) != THREAD_OVERFLOW){
        
    }
}

__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void taskARM9SVC(u32 * args){
	//handles Local/UDP Nifi / GDB Server
	if(HandledoMULTIDaemonWeakRefLibHardware9Callback != NULL){
		HandledoMULTIDaemonWeakRefLibHardware9Callback(); //returns: currentDSWNIFIMode
	}
	
	//Default TGDS Audio playback: WAV/IMA-ADPCM
	if(SoundStreamUpdateSoundStreamARM9LibUtilsCallback != NULL){
		SoundStreamUpdateSoundStreamARM9LibUtilsCallback();
	}
}
#endif
