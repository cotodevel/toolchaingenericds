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

#ifndef __nds_video_h__
#define __nds_video_h__

#include "typedefs.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdbool.h>



#define VRAM_ENABLE		(1<<7)
#define VRAM_OFFSET(n)	((n)<<3)




#define VRAM_MAX_LETTER_INDEX	(sint32)(9)

#define VRAM_A_INDEX	(sint32)(0)
#define VRAM_B_INDEX	(sint32)(1)
#define VRAM_C_INDEX	(sint32)(2)
#define VRAM_D_INDEX	(sint32)(3)
#define VRAM_E_INDEX	(sint32)(4)
#define VRAM_F_INDEX	(sint32)(5)
#define VRAM_G_INDEX	(sint32)(6)
#define VRAM_H_INDEX	(sint32)(7)
#define VRAM_I_INDEX	(sint32)(8)

typedef struct vramBankSetup
{
	uint8 vrambankCR;	//a,b,c,d,e,f,g,h,i
	bool enabled;	//true = yes /false = no
	//sint32	setupMode;	//if mapped should setup accordingly by vramsetup(n)
}vramBankSetup;

typedef struct vramSetup
{
	vramBankSetup vramBankSetupInst[VRAM_MAX_LETTER_INDEX];
}vramSetup;


//VRAM Setup 0
/*
 VRAM    SIZE  MST  OFS   ARM9, Plain ARM9-CPU Access (so-called LCDC mode)
  A       128K  0    -     6800000h-681FFFFh
  B       128K  0    -     6820000h-683FFFFh
  C       128K  0    -     6840000h-685FFFFh
  D       128K  0    -     6860000h-687FFFFh
  E       64K   0    -     6880000h-688FFFFh
  F       16K   0    -     6890000h-6893FFFh
  G       16K   0    -     6894000h-6897FFFh
  H       32K   0    -     6898000h-689FFFFh
  I       16K   0    -     68A0000h-68A3FFFh
 */

#define VRAM_A_LCDC_MODE 	(0)
#define VRAM_B_LCDC_MODE 	(0)
#define VRAM_C_LCDC_MODE 	(0)
#define VRAM_D_LCDC_MODE 	(0)
#define VRAM_E_LCDC_MODE 	(0)
#define VRAM_F_LCDC_MODE 	(0)
#define VRAM_G_LCDC_MODE 	(0)
#define VRAM_H_LCDC_MODE 	(0)
#define VRAM_I_LCDC_MODE 	(0)

//VRAM Setup 1
//ARM9, 2D Graphics Engine A, BG-VRAM (max 512K)
//Vram A (128K)
#define VRAM_A_0x06000000_ENGINE_A_BG 	(1 | VRAM_OFFSET(0))	//OFS 0..3
#define VRAM_A_0x06020000_ENGINE_A_BG 	(1 | VRAM_OFFSET(1))
#define VRAM_A_0x06040000_ENGINE_A_BG 	(1 | VRAM_OFFSET(2))
#define VRAM_A_0x06060000_ENGINE_A_BG 	(1 | VRAM_OFFSET(3))

//Vram B (128K)
#define VRAM_B_0x06000000_ENGINE_A_BG 	(1 | VRAM_OFFSET(0))	//OFS 0..3
#define VRAM_B_0x06020000_ENGINE_A_BG 	(1 | VRAM_OFFSET(1))
#define VRAM_B_0x06040000_ENGINE_A_BG 	(1 | VRAM_OFFSET(2))
#define VRAM_B_0x06060000_ENGINE_A_BG 	(1 | VRAM_OFFSET(3))

//Vram C (128K)
#define VRAM_C_0x06000000_ENGINE_A_BG 	(1 | VRAM_OFFSET(0))	//OFS 0..3
#define VRAM_C_0x06020000_ENGINE_A_BG 	(1 | VRAM_OFFSET(1))
#define VRAM_C_0x06040000_ENGINE_A_BG 	(1 | VRAM_OFFSET(2))
#define VRAM_C_0x06060000_ENGINE_A_BG 	(1 | VRAM_OFFSET(3))

//Vram D (128K)
#define VRAM_D_0x06000000_ENGINE_A_BG 	(1 | VRAM_OFFSET(0))	//OFS 0..3
#define VRAM_D_0x06020000_ENGINE_A_BG 	(1 | VRAM_OFFSET(1))
#define VRAM_D_0x06040000_ENGINE_A_BG 	(1 | VRAM_OFFSET(2))
#define VRAM_D_0x06060000_ENGINE_A_BG 	(1 | VRAM_OFFSET(3))

//Vram E (64K)
#define VRAM_E_0x06000000_ENGINE_A_BG 	(1)	//static

//Vram F (16K), takes in account OFS0 and OFS1 : 06000000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_F_0x060XXXXX_ENGINE_A_BG 	(1)	//OFS 0..3

//Vram G (16K),	takes in account OFS0 and OFS1 : 06000000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_G_0x060XXXXX_ENGINE_A_BG 	(1)	//OFS 0..3


//VRAM Setup 2
//ARM9, 2D Graphics Engine A, OBJ-VRAM (max 256K) (OFS 0..1)
#define VRAM_A_0x06400000_ENGINE_A_BG 	(2 | VRAM_OFFSET(0))	//0x064(20000*OFS.0) (OFS.1 always zero)
#define VRAM_A_0x06420000_ENGINE_A_BG 	(2 | VRAM_OFFSET(1))	

#define VRAM_B_0x06400000_ENGINE_A_BG 	(2 | VRAM_OFFSET(0))	//0x064(20000*OFS.0) (OFS.1 always zero)
#define VRAM_B_0x06420000_ENGINE_A_BG 	(2 | VRAM_OFFSET(1))	

//both can coexist through OFS (ofs1 is VRAM start always)

#define VRAM_E_0x06400000_ENGINE_A_BG 	(2)	//static


//Vram F (16K), takes in account OFS0 and OFS1 : 6400000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_F_0x064XXXXX_ENGINE_A_BG 	(2)	//OFS 0..3

//Vram G (16K),	takes in account OFS0 and OFS1 : 6400000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_G_0x064XXXXX_ENGINE_A_BG 	(2)	//OFS 0..3


//VRAM Setup 3
//2D Graphics Engine A, BG Extended Palette
//Vram E (64K)
#define VRAM_E_SLOT_ENGINE_A_BG_EXTENDED 	(4)	//Slot 0-3  ;only lower 32K used (No OFS)

//Vram F (16K)
#define VRAM_F_SLOT_ENGINE_A_BG_EXTENDED 	(4)	//Slot 0-1 (OFS=0), Slot 2-3 (OFS=1) (OFS 0..1)
#define VRAM_G_SLOT_ENGINE_A_BG_EXTENDED 	(4)	//Slot 0-1 (OFS=0), Slot 2-3 (OFS=1) (OFS 0..1)

//VRAM Setup 7
//Vram C (128K)
#define VRAM_C_0x06200000_ENGINE_B_BG 	(4 | VRAM_OFFSET(0))	//static

//Vram H (32K)
#define VRAM_H_0x06200000_ENGINE_B_BG 	(1)	//static. 

//Vram I (16K)
#define VRAM_I_0x06208000_ENGINE_B_BG 	(1)	//static. 


//VRAM Setup 11

//Vram C,D     128K  2    0..1  6000000h+(20000h*OFS.0)  ;OFS.1 must be zero
#define VRAM_C_0x06000000_ARM7 	(2 | VRAM_OFFSET(0))	//static
#define VRAM_C_0x06020000_ARM7 	(2 | VRAM_OFFSET(1))	//static

#define VRAM_D_0x06000000_ARM7 	(2 | VRAM_OFFSET(0))	//static
#define VRAM_D_0x06020000_ARM7 	(2 | VRAM_OFFSET(1))	//static

//todo VRAM setups 3-10, minus 7, minus 11, minus 3


#endif

#ifdef __cplusplus
extern "C"{
#endif

//power
extern void SWAP_LCDS();

//VRAM
extern void SETDISPCNT_MAIN(uint32 mode);
extern void SETDISPCNT_SUB(uint32 mode);
extern vramSetup vramSetupGlobal[1];
extern bool VRAM_SETUP(vramSetup * vramSetupInst);

//project specific
extern vramSetup * SNEMULDS_2DVRAM_SETUP();

#ifdef __cplusplus
}
#endif