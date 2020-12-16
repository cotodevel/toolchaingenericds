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

#ifndef __nds_power_h__
#define __nds_power_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"


#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void powerON(uint32 values);
extern void powerOFF(uint32 values);

#ifdef ARM7
extern void SoundPowerON(u8 volume); //aka : enableSound()
#endif


#ifdef __cplusplus
}
#endif