@
@			Copyright (C) 2017  Coto
@This program is free software; you can redistribute it and/or modify
@it under the terms of the GNU General Public License as published by
@the Free Software Foundation; either version 2 of the License, or
@(at your option) any later version.

@This program is distributed in the hope that it will be useful, but
@WITHOUT ANY WARRANTY; without even the implied warranty of
@MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
@General Public License for more details.

@You should have received a copy of the GNU General Public License
@along with this program; if not, write to the Free Software
@Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
@USA
@

#ifdef ARM7
.text
#endif

#ifdef ARM9
.section .itcm,"ax",%progbits
#endif

.arm
.code 32

#include "dsregs_asm.h"


@void swiDelay(uint32 delayvalue);
.global 	swiDelay
.type   	swiDelay STT_FUNC
swiDelay:
	swi 0x30000
bx r14

@uint16 swiCRC16(uint16 crc, void * data, uint32 size);
.global 	swiCRC16
.type   	swiCRC16 STT_FUNC
swiCRC16:
	swi 0xe0000
bx r14

#ifdef ARM7
@void swiChangeSndBias(int enable, int delayvalue);
.global 	swiChangeSndBias
.type   	swiChangeSndBias STT_FUNC
swiChangeSndBias:
	swi	0x80000
bx	r14
#endif

.pool
.end