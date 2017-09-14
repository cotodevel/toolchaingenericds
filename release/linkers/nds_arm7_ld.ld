
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
	iwram	(rwx)	: ORIGIN = 0x03800000, LENGTH = 64K	
}


_iwram_start = ORIGIN(iwram);
_iwram_end = ORIGIN(iwram)	+	LENGTH(iwram);

/* ARM7 Specific */
_arm7_irqcheckbits = 0x04000000 - 8;	/* __irq_flags */
_arm7_irqhandler = 0x04000000 - 4;	/* __irq_vector */

/* ARM ABI says stacks are full decreasing */
sp_SVC	= _iwram_end - 0x200	-	0x10;	/* 0x10 because of interrupt handler vector + interrupt check bits (irqs raised by software bios) */
sp_IRQ	= sp_SVC - 0x200;
sp_FIQ	= sp_IRQ - 0x200;
sp_ABT	= sp_FIQ - 0x200;
sp_UND	= sp_ABT - 0x200;
sp_USR	= sp_UND - 0x200;
sp_SYS	= sp_USR - 0x200;
sp_IRQCHANDLER	=	sp_SYS - 0x1000;

SECTIONS
{
	/* coto: Setting the current section cursor (.) takes place in EWRAM for initial section(s) in LMA fashion. */
	__init_vma_start = _iwram_start ;
	
	.init	( __init_vma_start ):
		AT( __init_vma_start )
	{
		KEEP(*(.init))
		. = ALIGN(4);
	}
	__init_vma_end = __init_vma_start + SIZEOF(.init) ;
	
	__preinit_array_start = __init_vma_end ;
	.preinit_array	( __preinit_array_start ):
		AT( __preinit_array_start )
	{
		KEEP (*(.preinit_array*))
		. = ALIGN(4);
	}
	__preinit_array_end	=	__preinit_array_start + SIZEOF(.preinit_array) ;
	
	__init_array_start	=	__preinit_array_end;
	.init_array	( __init_array_start ):
		AT( __init_array_start )
	{
		KEEP (*(SORT(.init_array.*)))
		. = ALIGN(4);
		KEEP (*(.init_array*))
		. = ALIGN(4);
	}
	__init_array_end =	__init_array_start	+ SIZEOF(.init_array) ;
	
	__fini_array_start	=	__init_array_end ;
	.fini_array	( __fini_array_start ):
		AT( __fini_array_start )
	{
		KEEP (*(.fini_array*))
		. = ALIGN(4);
	}
	__fini_array_end	=	__fini_array_start	+ SIZEOF(.fini_array) ;
	
	__text_vma_start = __fini_array_end ;
	.text	( __text_vma_start ):
		AT( __text_vma_start )
	{
		KEEP (*(.text.*))
		. = ALIGN(4);
		KEEP (*(.stub))
		. = ALIGN(4);
		KEEP (*(.gnu.warning))
		. = ALIGN(4);
		KEEP (*(.gnu.linkonce.t*))
		. = ALIGN(4);
		KEEP (*(.glue_7))
		. = ALIGN(4);
		KEEP (*(.glue_7t))
		. = ALIGN(4);
	}
	__text_vma_end = __text_vma_start + SIZEOF(.text) ;
	
	__bss_vma_start = __text_vma_end ;
	.bss ( __bss_vma_start ):
		AT( __bss_vma_start )
	{
		KEEP (*(.bss*))
		. = ALIGN(4);
		KEEP (*(COMMON))		/* libc_nano requires these for uninitialized objects, include any heap uninit vars*/
		. = ALIGN(4);
	}
	__bss_vma_end = __bss_vma_start + SIZEOF(.bss) ;
	
	__data_vma_start = __bss_vma_end;
	.data ( __data_vma_start ):
		AT( __data_vma_start )
	{
		KEEP (*(.data))
		. = ALIGN(4);
		KEEP (*(.data.*))
		. = ALIGN(4);
		KEEP (*(.gnu.linkonce.d*))
		. = ALIGN(4);
		CONSTRUCTORS
		. = ALIGN(4);
	} 
	__data_end = __data_vma_start + SIZEOF(.data) ;
	
	__rodata_vma_start = __data_end ;
	.rodata	( __rodata_vma_start ): 
		AT( __rodata_vma_start )
	{
		KEEP (*(.rodata.*))
		. = ALIGN(4);
	} 
	__rodata_vma_end  = __rodata_vma_start + SIZEOF(.rodata) ;
	
	__eh_frame_vma_start = __rodata_vma_end;
	.eh_frame ( __eh_frame_vma_start ): 
		AT( __eh_frame_vma_start )
	{
		KEEP (*(.eh_frame))
		. = ALIGN(4);
	}
	__eh_frame_vma_end = __eh_frame_vma_start + SIZEOF(.eh_frame) ;
	
	__jcr_vma_start = __eh_frame_vma_end;
	.jcr ( __jcr_vma_start ): 
		AT( __jcr_vma_start )
	{ 
		KEEP (*(.jcr))
		. = ALIGN(4);
	}
	__jcr_vma_end = __jcr_vma_start + SIZEOF(.jcr) ;
	
	
	/* coto: new sections required by libc nano, DS hardware don't use them but kept for compatibility anyway*/
	__vfp11_veneer_vma_start = __jcr_vma_end;
	.vfp11_veneer ( __vfp11_veneer_vma_start ):
		AT( __vfp11_veneer_vma_start )
	{
		KEEP (*(.vfp11_veneer))
		. = ALIGN(4);
	}
	__vfp11_veneer_vma_end = __vfp11_veneer_vma_start + SIZEOF(.vfp11_veneer) ;
	
	__v4_bx_vma_start = __vfp11_veneer_vma_end;
	.v4_bx ( __v4_bx_vma_start ):
		AT( __v4_bx_vma_start )
	{
		KEEP (*(.v4_bx))
		. = ALIGN(4);
	}
	__v4_bx_vma_end = __v4_bx_vma_start + SIZEOF(.v4_bx) ;
	
	__iplt_vma_start = __v4_bx_vma_end;
	.iplt ( __iplt_vma_start ):
		AT( __iplt_vma_start )
	{
		KEEP (*(.iplt))
		. = ALIGN(4);
	}
	__iplt_vma_end = __iplt_vma_start + SIZEOF(.iplt) ;
	
	__igot_plt_vma_start = __iplt_vma_end;
	.igot.plt ( __igot_plt_vma_start ):
		AT( __igot_plt_vma_start )
	{
		KEEP (*(.igot.plt))
		. = ALIGN(4);
	}
	__igot_plt_vma_end = __igot_plt_vma_start + SIZEOF(.igot.plt) ;
	
	__rel_dyn_vma_start = __igot_plt_vma_end;
	.rel.dyn ( __rel_dyn_vma_start ):
		AT( __rel_dyn_vma_start )
	{
		KEEP (*(.rel.dyn))
		. = ALIGN(4);
	}
	__rel_dyn_vma_end = __rel_dyn_vma_start + SIZEOF(.rel.dyn) ;
	
	/* coto: libc nano sections end*/
	
	__fini_vma_start = __rel_dyn_vma_end;
	.fini ( __fini_vma_start ):
		AT( __fini_vma_start )
	{
		KEEP (*(.fini))
		. = ALIGN(4);
	}
	__fini_vma_end = __fini_vma_start + SIZEOF(.fini) ;
	__lib__end__ = __fini_vma_end;					
	__vma_stub_end__ = __lib__end__ ;
	
}