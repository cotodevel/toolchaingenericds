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

#ifndef __powerTGDS_h__
#define __powerTGDS_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define LED_LONGBLINK	1
#define LED_SHORTBLINK	3
#define LED_ON			0

#ifdef ARM9
#undef POWER_LCD
#define POWER_LCD			(1 << 0)

#undef POWER_2D_A
#define POWER_2D_A			(1 << 1)

#undef POWER_MATRIX
#define POWER_MATRIX		(1 << 2)

#undef POWER_3D_CORE
#define POWER_3D_CORE		(1 << 3)

#undef POWER_2D_B
#define POWER_2D_B			(1 << 9)

#undef POWER_SWAP_LCDS
#define POWER_SWAP_LCDS		(1 << 15)

#undef POWER_ALL_2D
#define POWER_ALL_2D     (POWER_LCD | POWER_2D_A | POWER_2D_B)

#undef POWER_ALL
#define POWER_ALL		 (POWER_ALL_2D | POWER_3D_CORE | POWER_MATRIX)
#endif

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void powerON(uint32 values);
extern void powerOFF(uint32 values);

#ifdef ARM7
extern void SoundPowerON(u8 volume); //aka : enableSound()
#endif

#ifdef ARM9
extern void powerOFF3DEngine();
extern void powerON3DEngine();
#endif

#ifdef __cplusplus
}
#endif