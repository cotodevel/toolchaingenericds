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

.global 	IRQWait
.type   	IRQWait STT_FUNC
IRQWait:

	stmdb sp!, {r0-r7, lr}
	mov r3,r0

	@r0 = IE/dummy / r1 = IRQtowaitfor / r2 = IF /r3 = reentrant / r4 = dummy/BIOSflags / r5 = IME saved / r6 = 0x04000000 /r7 = BIOSflags addr
	mov r6,#0x04000000
	ldr r5,[r6,#0x208]	@IME saved
	mov r0,#1
	str r0,[r6,#0x208]	@force IME = 1
	
waitirq:
	add	r7, r6, #0x210
	ldmia r7, {r0,r2}	@r0 = IE / r2 = IF
	
	@BIOS Flag
	#ifdef ARM7
	ldr	r7, =_arm7_irqcheckbits
	#endif
	#ifdef ARM9
	ldr	r7, =_arm9_irqcheckbits
	#endif
	ldr r4,[r7]
	
	and r2,r2,r0			@IF & IE
	orr r2,r2,r4			@orr swi flags
	
	@sleep if interrupt to wait for (r1) hasnt happened.
	tst r2,r1
	
	#ifdef ARM7
	@4000301h - NDS7 - Halt function (HALTCNT) - Low Power Mode Control (R/W)
	ldrne	r2, =0x04000301
	movne 	r3,#(2<<6)			@2=Halt
	strne 	r3,[r2]
	#endif
	
	#ifdef ARM9
	@NDS9	-	Halt function	CP15
	movne 	r0,#0
	mcrne 	p15,0,r0,c7,c0,4				@low power mode: waitforIrq CP15
	#endif
	
	bne waitirq		@retry until r1 happens
	
	orr r4,r4,r1	@add in bios flags
	str r4,[r7]
	
	str r5,[r6,#0x208]	@restore IME
exitirq:
	ldmia sp!, {r0-r7, lr}
	
bx r14

@----------

.global 	IRQVBlankWait
.type   	IRQVBlankWait STT_FUNC
IRQVBlankWait:
	push {r0-r3,lr}
	mov r0,#1
	mov r1,#(1<<0)
	bl IRQWait
	pop {r0-r3,lr}
bx r14


.global 	InterruptServiceRoutineARMCores, __cpsr_mask
InterruptServiceRoutineARMCores:

push {r0-r12,lr}

@only treat interrupts when IME = 1
mov	r12, #0x4000000		
ldr	r1, [r12, #0x208]	
cmp	r1, #0
popeq {r0-r12,lr} 
bxeq	lr

mov	r0, #0
str	r0, [r12, #0x208]	@REG_IME = 0;

push {r0,r1,r12,lr}
bl NDS_IRQHandler
pop {r0,r1,r12,lr}

str r1,[r12,#0x208]	@REG_IME = 1;

pop {r0-r12,lr}
bx lr



.pool
.end