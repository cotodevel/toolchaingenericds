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

#ifndef __nds_dma_h__
#define __nds_dma_h__

#include "typedefs.h"
#include "dsregs.h"
#include "dsregs_asm.h"


#define		DMAINCR_DEST	(0<<(16+5))
#define		DMADECR_DEST	(1<<(16+5))
#define		DMAFIXED_DEST	(2<<(16+5))

#define		DMAINCR_SRC		(0<<(16+7))
#define		DMADECR_SRC		(1<<(16+7))
#define		DMAFIXED_SRC	(2<<(16+7))

#define 	DMA16BIT   	(0<<(16+10))
#define 	DMA32BIT   	(1<<(16+10))

#define		DMASTART_INMEDIATE	(0<<(16+12))
#define		DMASTART_VBLANK		(1<<(16+12))
#define		DMASTART_HBLANK		(2<<(16+12))
#define		DMASTART_SPECIAL	(3<<(16+12))

#define		DMAENABLED	(1<<(16+15))	//DMAs disable itself when transfer done

#define DMAXSAD(dmaindx)     (*(vuint32*)(0x040000B0+(dmaindx*0xc)))
#define DMAXDAD(dmaindx)    (*(vuint32*)(0x040000B4+(dmaindx*0xc)))
#define DMAXCNT(dmaindx)      (*(vuint32*)(0x040000B8+(dmaindx*0xc)))	//DMAXCNT_L (0xB8 & 0xB9) DMAXCNT_H (0xBA & 0xBB)

//40000E(X*4)h - NDS9 only - DMAXFILL - DMA X Filldata (R/W)
#ifdef ARM9
#define DMAXFILL(dmaindx)      (*(vuint32*)(0x040000E0+(dmaindx*0x4)))

#endif

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void dmaFill(sint32 dmachannel,uint32 value, uint32 dest, uint32 mode);
extern void dmaFillWord(sint32 dmachannel,uint32 value, uint32 dest, uint32 word_count);
extern void dmaFillHalfWord(sint32 dmachannel,uint32 value, uint32 dest, uint32 word_count);
extern void dmaTransfer(sint32 dmachannel, uint32 source, uint32 dest, uint32 mode);
extern void dmaTransferHalfWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count);
extern void dmaTransferWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count);

#ifdef __cplusplus
}
#endif