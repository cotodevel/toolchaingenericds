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

/* Coto: own linker stuff. Written by me. for NDS hardware (non DSi-3DS)*/
OUTPUT_FORMAT("elf32-littlearm")

/* attributes can actually help in debugging because if you try to move functions to DTCM, for example, compiler will rant since DTCM is DATA only path and not execution*/
MEMORY
{
	IWRAM	(rwx)	: ORIGIN = 0x03800000, LENGTH = 64K
}

_iwram_start = ORIGIN(IWRAM);
_iwram_end = ORIGIN(IWRAM)	+	LENGTH(IWRAM);

/* ARM7 Specific */
_arm7_irqcheckbits = 0x04000000 - 8;	/* __irq_flags */
_arm7_irqhandler = 0x04000000 - 4;	/* __irq_vector */

/* ARM ABI says stacks are full decreasing */
sp_SVC	= _iwram_end - 0x200;
sp_IRQ	= sp_SVC - 0x200;
sp_FIQ	= sp_IRQ - 0x200;
sp_ABT	= sp_FIQ - 0x200;
sp_UND	= sp_ABT - 0x200;
sp_USR	= sp_UND - 0x200;
sp_SYS	= sp_USR - 0x200;

SECTIONS
{
	.init : { __init_start__ = .;
		KEEP(*(.init*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__init_end__ = .;
	__preinit_array_start__ = .;
	.preinit_array : { 
		KEEP (*(.preinit_array*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__preinit_array_end__ = .;
	__init_array_start__ = .;
	.init_array : { 
		KEEP (*(.init_array*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	init_array_end__ = .;
	__fini_array_start__ = .;
	.fini_array : {
		KEEP (*(.fini_array*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__fini_array_end__ = .;
	__text_start__ = .;
	.text : {
		KEEP (*(.text*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__text_end__ = .;
	__bss_vma_start = .;
	.bss : { 
		KEEP (*(.sbss*))
		. = ALIGN(4);
		KEEP (*(.dynbss*))
		. = ALIGN(4);
		KEEP (*(.gnu.linkonce.b*))
		. = ALIGN(4);
		KEEP (*(.bss*))
		. = ALIGN(4);
		KEEP (*(COMMON*))			/* libc requires these for uninitialized objects, include any heap uninit vars*/
		. = ALIGN(4);
	} > IWRAM = 0xff
	__bss_vma_end = .;
	__data_start__ = .;
	.data : {
		KEEP (*(.data*))
		. = ALIGN(4);
		KEEP (*(.gnu.linkonce.d*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__data_end__ = .;
	__rodata_start__ = .;
	.rodata : { 
		KEEP (*(.rodata*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__rodata_end__ = .;
	__eh_frame_start__ = .;
	.eh_frame : { 
		KEEP (*(.eh_frame*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__eh_frame_end__ = .;
	__jcr_start__ = .;
	.jcr : { 
		KEEP (*(.jcr*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__jcr_end__ = .;
	__vfp11_veneer_start__ = .;
	.vfp11_veneer : {
		KEEP (*(.vfp11_veneer*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__vfp11_veneer_end__ = .;
	__exidx_start = .;
    .ARM.exidx   : { 
		*(.ARM.exidx* .gnu.linkonce.armexidx.*) 	/* newlib C++ require these constructors */
	} > IWRAM = 0xff
    __exidx_end = .;
	__v4_bx_start__ = .;
	.v4_bx : {
		KEEP (*(.v4_bx*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__v4_bx_end__ = .;
	__iplt_start__ = .;
	.iplt : { 
		KEEP (*(.iplt*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__iplt_end__ = .;
	__igot_plt_start__ = .;
	.igot.plt : {
		KEEP (*(.igot.plt*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__igot_plt_end__ = .;
	__rel_dyn_start__ = .;
	.rel.dyn : { 
		KEEP (*(.rel.dyn*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__rel_dyn_end__ = .;
	__fini_start__ = .;
	.fini : {
		KEEP (*(.fini*))
		. = ALIGN(4);
	} > IWRAM = 0xff
	__fini_end__ = .;
	__lib__end__ = __fini_end__;
	__vma_stub_end__ = __lib__end__;
}