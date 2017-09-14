
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
	DTCM (rw)      : ORIGIN = 0x0b000000, LENGTH = 16K
	ITCM (xrw)		: ORIGIN = 0x01000000, LENGTH = 32K - 4K
	VECTORS (xrw)	: ORIGIN = 0x01000000 + 32K - 4K, LENGTH = 4K
	EWRAM (xrw)		: ORIGIN = 0x02000000, LENGTH = 4M
	GBAROM (xrw)    : ORIGIN = 0x08000000, LENGTH = 32M
}

_itcm_start = ORIGIN(ITCM);
_itcm_end = ORIGIN(ITCM)	+	LENGTH(ITCM);

_vector_start = ORIGIN(VECTORS);
_vector_end = ORIGIN(VECTORS)	+	LENGTH(VECTORS);

_ewram_start = ORIGIN(EWRAM);
_ewram_end = ORIGIN(EWRAM)	+	LENGTH(EWRAM);

_gba_start = ORIGIN(GBAROM);
_gba_end = ORIGIN(GBAROM)	+	LENGTH(GBAROM);

_dtcm_start = ORIGIN(DTCM);
_dtcm_end = ORIGIN(DTCM)	+	LENGTH(DTCM);

/* ARM9 Specific */
_arm9_irqcheckbits = ORIGIN(DTCM) + 0x3FF8;	/* __irq_flags */
_arm9_irqhandler = ORIGIN(DTCM) + 0x3FFC;	/* __irq_vector */


/* ARM ABI says stacks are full decreasing: 16K - 3.5K = 12.5K free dtcm */
sp_SVC	= _dtcm_end - 0x200	-	0x10;	/* 0x10 because of interrupt handler vector + interrupt check bits (irqs raised by software bios) */
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
	__init_vma_start = _ewram_start ;
	
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
		*(EXCLUDE_FILE (*.itcm*) .text)
		. = ALIGN(4);
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
	
	/* anything beyond this is free memory after the crt0 initializes and jumps to main(), thus reusable.*/
	
	/* other sections: EWRAM LMA */
	__dtcm_vma_start = __lib__end__ ;
	.dtcm  ( _dtcm_start ):
		AT( __dtcm_vma_start )
	{
		KEEP (*(.dtcm))
		. = ALIGN(4);
	}
	__dtcm_vma_end = __dtcm_vma_start + SIZEOF(.dtcm) ;	/*coto: honestly this should be inside the LMA section. But we can just DTCM LMA ADDR+ DTCM SIZE for VMA -> DTCM LMA copy*/
									/* as a bonus __dtcm_vma_end - DTCM LMA ADDR = DTCM reserved size :) for validating against assigned stacks earlier or something */
	
	__itcm_vma_start = __dtcm_vma_end ;
	.itcm ( _itcm_start ):
		AT( __itcm_vma_start )
	{
		KEEP (*(.itcm))
		. = ALIGN(4);
	}
	__itcm_vma_end = __itcm_vma_start + SIZEOF(.itcm) ;
	
	__vectors_vma_start = __itcm_vma_end ;
	.vectors ( _vector_start ):
		AT( __vectors_vma_start )
	{
		KEEP (*(.vectors))
		. = ALIGN(4);
	}
	__vectors_vma_end = __vectors_vma_start + SIZEOF(.vectors);
	__vma_stub_end__ = __vectors_vma_end;
	
}