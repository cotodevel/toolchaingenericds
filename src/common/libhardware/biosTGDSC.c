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
	//printf("- decompressing from buffer... ");

	header = *pak_buffer;
	if (header != CMD_CODE_10) {
		//printf("ERROR: file is not LZSS encoded!");
		LZSSCtx.bufferSource = NULL;
		LZSSCtx.bufferSize = -1;
		return LZSSCtx;
	}

	raw_len = *(unsigned int *)pak_buffer >> 8;
	raw_buffer = (unsigned char *) TGDSARM9Malloc(raw_len * sizeof(char));

	swiDecompressLZSSWram((void *)pak_buffer, (void *)raw_buffer);

	LZSSCtx.bufferSource = raw_buffer;
	LZSSCtx.bufferSize = raw_len;

	//printf("LZS_Decode() end.");
	return LZSSCtx;
}
#endif

//TGDS Services:
#ifdef ARM7

bool isArm7ClosedLid=false;

void handleARM7InitSVC(){
	isArm7ClosedLid=false;
	handleARM7SVC();
}

void handleARM7SVC(){
	//Lid Closing + backlight events (ARM7)
	if( ((REG_KEYXY & KEY_HINGE) == KEY_HINGE) && (isArm7ClosedLid == false)){
		isArm7ClosedLid = true;
		TurnOffScreens();
		screenLidHasClosedhandlerUser();
	}
}
#endif

#ifdef ARM9
void handleARM9InitSVC(){

}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void handleARM9SVC(){
	//handles Local/UDP Nifi / GDB Server
	if(HandledoMULTIDaemonWeakRefLibHardware9Callback != NULL){
		sint32 currentDSWNIFIMode = HandledoMULTIDaemonWeakRefLibHardware9Callback();
	}
	
	//Default TGDS Audio playback: WAV/IMA-ADPCM
	if(SoundStreamUpdateSoundStreamARM9LibUtilsCallback != NULL){
		SoundStreamUpdateSoundStreamARM9LibUtilsCallback();
	}
}
#endif
