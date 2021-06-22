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
.align 4

#include "../dsregs_asm.h"
.global 	IRQWait
.type   	IRQWait STT_FUNC
IRQWait:
	swi 0x00040000
bx lr

@----------
.global 	IRQVBlankWait
.type   	IRQVBlankWait STT_FUNC
IRQVBlankWait:
	swi 0x00050000
bx lr

.pool
.end