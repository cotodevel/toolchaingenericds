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

void scanKeys()
{
	keysold=keys;
	keys=KEYS_CUR;
}

uint32 keysHeld()
{
	return keys;
}

uint32 keysDown()
{
	return (keys&~keysold);
}

uint32 keysUp()
{
	return (keys^keysold)&(~keys);
}

void setKeys(u32 newKeys){
	keys |= newKeys;
}