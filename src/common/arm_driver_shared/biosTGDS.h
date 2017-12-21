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

#ifndef __nds_bios_h__
#define __nds_bios_h__

#include "typedefsTGDS.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define COPY_FIXED_SOURCE (uint32)(1<<24)
#define swiDelay1ms (sint32)(8388)

#endif

#ifdef __cplusplus
extern "C"{
#endif

//NDS7/9 Bios 
extern void swiDelay(uint32 delayvalue);
extern uint16 swiCRC16(uint16 crc, void * data, uint32 size);
extern void swiFastCopy(uint32 * source, uint32 * dest, int flags);

//NDS7 Bios 
#ifdef ARM7
extern void swiChangeSndBias(int enable, int delayvalue);
#endif


//NDS9 Bios (useful when vectors @ 0x00000000 and manual processor vectors handling is required)

#ifdef __cplusplus
}
#endif
