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

#define swiDelay1ms (sint32)(8388)


struct DecompressionStream {
	int (*getSize)(uint8 * source, uint16 * dest, uint32 r2);
	int (*getResult)(uint8 * source); // can be NULL
	uint8 (*readByte)(uint8 * source);
};

#endif

#ifdef __cplusplus
extern "C"{
#endif

//NDS7/9 Bios 
//ARM ASM NDS9 Bios (useful when vectors @ 0xffff0000 and manual processor vectors handling is required)

extern void swiDelay(uint32 delayvalue);
extern uint16 swiCRC16(uint16 crc, void * data, uint32 size);
extern void swiSleep(uint32 delayvalue);
extern int swiIsDebugger(void);
extern int swiDivide(int numerator, int divisor);
extern int swiRemainder(int numerator, int divisor);
extern void swiDivMod(int numerator, int divisor, int * result, int * remainder);
extern void swiCopy(const void * source, void * dest, int flags);
extern int swiSqrt(int value);
extern void swiSoftReset(void);

//NDS7 Bios 
#ifdef ARM7
extern void swiHalt(void);
extern uint16 swiGetSineTable(int index);
extern uint16 swiGetPitchTable(int index);
extern uint8 swiGetVolumeTable(int index);
extern void swiSetHaltCR(uint32 data);
extern void swiChangeSoundBias(int enabled, int delay);
#endif

extern void swiDecompressLZSSWram(void * source, void * destination);
extern int swiDecompressLZSSVram(void * source, void * destination, uint32 toGetSize, struct DecompressionStream * stream);

//C
extern void swiFastCopy(uint32 * source, uint32 * dest, int flags);


#ifdef __cplusplus
}
#endif
