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

#include "keypadTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include <string.h>
#include "dmaTGDS.h"
#include "biosTGDS.h"

#ifdef ARM9
#include "consoleTGDS.h"
#endif

static u16 keys=0;
static u16 keysold=0;

static u16 oldx=0;
static u16 oldy=0;

void keysInit()
{
	keys=0;
	keysold=0;
}

void scanKeys()
{
	keysold=keys;
	keys=KEYS_CUR;
}

u32 keysHeld()
{
	return keys;
}

u32 keysDown()
{
	return (keys^keysold)&keys;
}

u32 keysUp()
{
	return (keys^keysold)&(~keys);
}

void setKeys(u32 newKeys){
	keys |= newKeys;
}