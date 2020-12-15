
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


#ifndef __nds_keypad_h__
#define __nds_keypad_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"

//TouchScreen 
struct touchScr {
    uint16 buttons7;  			// X, Y, /PENIRQ buttons
	uint16 KEYINPUT7;			//REG_KEYINPUT ARM7
	uint16 touchX,   touchY;   // raw x/y TSC SPI
	sint16 touchXpx, touchYpx; // TFT x/y pixel (converted)
};

#ifdef __cplusplus
extern "C"{
#endif

extern uint32 buffered_keys_arm9;
extern uint32 global_keys_arm9;
extern uint32 last_frame_keys_arm9;	//last frame keys before new frame keys
extern void touchScrRead(struct touchScr * touchScrInst);
#ifdef __cplusplus
}
#endif

static inline void scanKeys(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint16 buttonsARM7 = TGDSIPC->buttons7;
	uint32 readKeys = (uint32)(( ((~KEYINPUT)&0x3ff) | (((~buttonsARM7)&3)<<10) | (((~buttonsARM7)<<6) & (KEY_TOUCH|KEY_LID) ))^KEY_LID);
	last_frame_keys_arm9 = global_keys_arm9;
	global_keys_arm9 = readKeys | buffered_keys_arm9;
	buffered_keys_arm9 = 0;
}

static inline void setKeys(u32 keys){
	buffered_keys_arm9 |= keys;
}

static inline uint32 keysPressed(){
	return global_keys_arm9;	//there is no other way. Required by CoreEmu
}

static inline uint32 keysReleased(){
	return (uint32)((~keysPressed()) & last_frame_keys_arm9);
}

static inline uint32 keysHeld(){
	return (uint32)(global_keys_arm9 & last_frame_keys_arm9);
}

static inline uint32 keysRepeated(){
	return (uint32)( keysPressed() | last_frame_keys_arm9);
}

//Enables / Disables the touchscreen
static inline void setTouchScreenEnabled(bool status){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	TGDSIPC->touchScreenEnabled = status;
}

static inline bool getTouchScreenEnabled(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	return (bool)TGDSIPC->touchScreenEnabled;
}


#endif
