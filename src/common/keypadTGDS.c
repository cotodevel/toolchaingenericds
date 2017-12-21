
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
#include "ipcfifoTGDS.h"
#include <string.h>
#include "dmaTGDS.h"
#include "biosTGDS.h"

#ifdef ARM9
#include "consoleTGDS.h"
#endif

uint32 KEYCNT_READ(){
	return (uint32)(( ((~KEYINPUT)&0x3ff) | (((~getsIPCSharedTGDS()->buttons7)&3)<<10) | (((~getsIPCSharedTGDS()->buttons7)<<6) & (KEY_TOUCH|KEY_LID) ))^KEY_LID);
}

uint32 global_keys_arm9;
uint32 last_frame_keys_arm9;
//called on vblank
void do_keys(){
	last_frame_keys_arm9 = global_keys_arm9;
	global_keys_arm9 = KEYCNT_READ();
}

uint32 keysPressed(){
	return global_keys_arm9;	//there is no other way. Required by CoreEmu
}

uint32 keysReleased(){
	return (uint32)((~keysPressed()) & last_frame_keys_arm9);
}

uint32 keysHeld(){
	return (uint32)(global_keys_arm9 & last_frame_keys_arm9);
}

uint32 keysRepeated(){
	return (uint32)( keysPressed() | last_frame_keys_arm9);
}
