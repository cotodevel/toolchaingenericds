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

#ifdef ARM9
.text	@cant be at itcm
.arm
.code 32

#include "dsregs_asm.h"

@only in 0x00000000 vectors mode
#ifdef EXCEPTION_VECTORS_0x00000000
@todo
#endif

@only in 0xffff0000 vectors mode
#ifdef EXCEPTION_VECTORS_0xffff0000
.global DebugException
DebugException:
	
	@exception stack
	ldr sp,=exceptionArmRegs
	stmia sp, {r0-r12}	@r0 -- r12 
	
	@disable MPU
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#1
	mcr	p15,0,r0,c1,c0,0
	
	mov r8,sp
	mrs r10,cpsr
	
	@save processor regs according to spsr
	mrs	r9, spsr
	msr cpsr,r9
	
	str r0, [r8, #(4*0)]
	str sp, [r8, #(4*13)]
	str lr, [r8, #(4*14)]
	
	mrs r0,cpsr
	mrs r1,spsr
	
	str r0, [r8, #(4*16)]	@cpsr
	str r1, [r8, #(4*17)]	@spsr
	
	msr	cpsr,r10
	
	@get PC from source abort
	ldr r1, =(0x02FFFD98)
	ldr	r2,[r1]
	str r2, [sp, #(4*15)]
	
	@new stack
	ldr	r13,=sp_ABT
	push {r0-r12,lr}
	ldr	r12,=exception_data_abort
	blx	r12
	pop {r0-r12,lr}
	
	bx lr
#endif

#endif
