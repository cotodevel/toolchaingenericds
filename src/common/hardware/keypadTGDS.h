
//////////////////////////////////////////////////////////////////////
//
// keys.h -- provides slightly higher level input forming
//
//  Contributed by DesktopMA
//
// version 0.1, February 14, 2005
//
//  Copyright (C) 2005 Michael Noland (joat) and Jason Rogers (dovoto)
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any
//  purpose, including commercial applications, and to alter it and
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source
//     distribution.
//
// Changelog:
//   0.1: First version
//	
//
//////////////////////////////////////////////////////////////////////


#ifndef __nds_keypad_h__
#define __nds_keypad_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"

#define KEYS_CUR ( ((~REG_KEYINPUT)&0x3ff) | (((~((struct sIPCSharedTGDS *)TGDSIPCStartAddress)->ARM7REG_KEYXY)&3)<<10) | (((~((struct sIPCSharedTGDS *)TGDSIPCStartAddress)->ARM7REG_KEYXY)<<6) & KEY_TOUCH ))

#ifdef __cplusplus
extern "C"{
#endif

extern void keysInit();
extern void scanKeys();
extern u32 keysHeld();
extern u32 keysDown();
extern u32 keysUp();
extern void setKeys(u32 newKeys);

#ifdef __cplusplus
}
#endif

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
