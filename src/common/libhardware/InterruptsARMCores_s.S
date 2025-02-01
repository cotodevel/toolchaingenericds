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

@Jan 31, 2025(Coto):Implement proper ARM7(i) and ARM9(i) IntrWait/VBlankIntrWait bios calls replacement.

#ifdef ARM7
.text
#endif
#ifdef ARM9
.section .itcm,"ax",%progbits
#endif
.arm
.align 4

#include "../dsregs_asm.h"
@CPU enters low power mode until any IRQ happens, the rest of the circuitry is kept working regardless.
@ARM7(i) as well ARM9(i) bios is properly implemented.
.global 	HaltUntilIRQ
.type   	HaltUntilIRQ STT_FUNC
HaltUntilIRQ:
	swi 0x00060000
bx lr

@CPU enters low power mode until a specified interrupt happens.
@ARM7(i) bios is properly implemented.
@ARM9i bios is broken. Because the bios is hardcoded to ARM7 IRQWait routine which doesn't work. 
@Needs proper implementation on ARM9 side.
.global 	IRQWait
.type   	IRQWait STT_FUNC
IRQWait:
	#ifdef ARM7
	swi 0x00040000
	#endif
	
	#ifdef ARM9
	tst r1, #IRQ_HBLANK
	bne IRQWaitDirect
	tst r1, #IRQ_VBLANK
	bne IRQWaitDirect
	tst r1, #IRQ_VCOUNT
	bne IRQWaitDirect
	b IRQWaitSWI

IRQWaitDirect:
	@r0 = IRQtowaitfor / r1,r2,r3 = irq bits / r4-r5 = temp
	push {r0-r6, lr}
	mov r3,#0x04000000
	add	r3, r3, #0x210

	#ifdef ARM7
	ldr	r4, =_arm7_irqcheckbits
	#endif
	#ifdef ARM9
	ldr	r4, =_arm9_irqcheckbits
	#endif

	@clean @ BIOS flags
	mov r5,#0
	str r5,[r4]

waitirq:
	ldr r2, [r3]		
	ldr r5, [r4]
	and r2,r5,r2		@IE & IRQtowaitfor(s) & @BIOS
	and r2,r2,r1
	
	@CPU goes to sleep until IRQtowaitfor(s) have been served.
	cmp r2, #0
	
	#ifdef ARM7
		@4000301h - NDS7 - Halt function (HALTCNT) - Low Power Mode Control (R/W)
		ldreq	r6, =0x04000301
		moveq 	r2,#(2<<6)			@2=Halt
		streq 	r2,[r6]
	#endif
	#ifdef ARM9
		@NDS9	-	Halt function	CP15
		moveq 	r6,#0
		mcreq 	p15,0,r6,c7,c0,4				@low power mode: waitforIrq CP15
	#endif

	beq waitirq		@not the irqs (r0) we want? Retry.
	pop {r0-r6, lr}
	#endif
bx lr

IRQWaitSWI:
	swi 0x00040000
bx lr


@CPU enters low power mode until a Vblank interrupt happens.
@ARM7(i) bios is properly implemented.
@ARM9i bios is broken. Because the bios is hardcoded to ARM7 IRQWait routine which doesn't work. 
@Needs proper implementation on ARM9 side.
.global 	IRQVBlankWait
.type   	IRQVBlankWait STT_FUNC
IRQVBlankWait:
#ifdef ARM7
swi 0x00050000
#endif

#ifdef ARM9
	push {r0-r3, lr}
	mov r0, #1
	mov r1, #IRQ_VBLANK
	bl IRQWait
	pop {r0-r3, lr}
#endif

bx lr


.pool
.end