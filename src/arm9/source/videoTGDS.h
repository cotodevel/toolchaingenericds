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

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdbool.h>

#define VRAM_ENABLE		(1<<7)
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


//VRAM Setup 0: //MST 0
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

//note: ENGINE_A == "MAIN BG ENGINE" , ENGINE_B "SUB BG ENGINE"
//VRAM Setup 1: //MST 1
//ARM9, 2D Graphics Engine A, BG-VRAM (max 512K)
//Vram A (128K)
#define VRAM_A_0x06000000_ENGINE_A_BG 	(1 | (0 << 3))	//OFS 0..3
#define VRAM_A_0x06020000_ENGINE_A_BG 	(1 | (1 << 3))
#define VRAM_A_0x06040000_ENGINE_A_BG 	(1 | (2 << 3))
#define VRAM_A_0x06060000_ENGINE_A_BG 	(1 | (3 << 3))

//Vram B (128K)
#define VRAM_B_0x06000000_ENGINE_A_BG 	(1 | (0 << 3))	//OFS 0..3
#define VRAM_B_0x06020000_ENGINE_A_BG 	(1 | (1 << 3))
#define VRAM_B_0x06040000_ENGINE_A_BG 	(1 | (2 << 3))
#define VRAM_B_0x06060000_ENGINE_A_BG 	(1 | (3 << 3))

//Vram C (128K)
#define VRAM_C_0x06000000_ENGINE_A_BG 	(1 | (0 << 3))	//OFS 0..3
#define VRAM_C_0x06020000_ENGINE_A_BG 	(1 | (1 << 3))
#define VRAM_C_0x06040000_ENGINE_A_BG 	(1 | (2 << 3))
#define VRAM_C_0x06060000_ENGINE_A_BG 	(1 | (3 << 3))

//Vram D (128K)
#define VRAM_D_0x06000000_ENGINE_A_BG 	(1 | (0 << 3))	//OFS 0..3
#define VRAM_D_0x06020000_ENGINE_A_BG 	(1 | (1 << 3))
#define VRAM_D_0x06040000_ENGINE_A_BG 	(1 | (2 << 3))
#define VRAM_D_0x06060000_ENGINE_A_BG 	(1 | (3 << 3))

//Vram E  64K   1    -     6000000h
#define VRAM_E_0x06000000_ENGINE_A_BG 	(1)		//static @ 0x06000000

//Vram F (16K), takes in account OFS0 and OFS1 : 06000000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_F_0x06000000_ENGINE_A_BG  	(1 | (( 0 )<<3))	//OFS 0
#define VRAM_F_0x06004000_ENGINE_A_BG 	(1 | (( 1 )<<3))	//OFS 1
#define VRAM_F_0x06010000_ENGINE_A_BG  	(1 | (( 2 )<<3))	//OFS 2
#define VRAM_F_0x06014000_ENGINE_A_BG  	(1 | (( 3 )<<3))	//OFS 3

//Vram G (16K),	takes in account OFS0 and OFS1 : 06000000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_G_0x060XXXXX_ENGINE_A_BG 	(1)	//OFS 0..3


//VRAM Setup 2: //MST 2
//ARM9, 2D Graphics Engine A, OBJ-VRAM (max 256K) (OFS 0..1)
#define VRAM_A_0x06400000_ENGINE_A_BG 	(2 | (0 << 3))	//0x064(20000*OFS.0) (OFS.1 always zero)
#define VRAM_A_0x06420000_ENGINE_A_BG 	(2 | (1 << 3))	

#define VRAM_B_0x06400000_ENGINE_A_BG 	(2 | (0 << 3))	//0x064(20000*OFS.0) (OFS.1 always zero)
#define VRAM_B_0x06420000_ENGINE_A_BG 	(2 | (1 << 3))	

//both can coexist through OFS 
#define VRAM_E_0x06400000_ENGINE_A_BG 	(2)	//static	: VRAM_E: Main Engine 0x06400000 64K MST 2

//Vram F (16K), takes in account OFS0 and OFS1 : 6400000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_F_0x06400000_ENGINE_A_BG  (2 | (( 0 )<<3))	//OFS 0
#define VRAM_F_0x06404000_ENGINE_A_BG  (2 | (( 1 )<<3))	//OFS 1
#define VRAM_F_0x06410000_ENGINE_A_BG  (2 | (( 2 )<<3))	//OFS 2
#define VRAM_F_0x06414000_ENGINE_A_BG  (2 | (( 3 )<<3))	//OFS 3

//Vram G (16K),	takes in account OFS0 and OFS1 : 6400000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_G_0x06400000_ENGINE_A_BG	(2 | (( 0 )<<3))
#define VRAM_G_0x06404000_ENGINE_A_BG	(2 | (( 1 )<<3))
#define VRAM_G_0x06410000_ENGINE_A_BG	(2 | (( 2 )<<3))
#define VRAM_G_0x06414000_ENGINE_A_BG	(2 | (( 3 )<<3))

//VRAM Setup 3
//2D Graphics Engine A, BG Extended Palette. This mode maps memory to VRAM and not CPU addresses
//Vram E (64K) : Slot 0-3  ;only lower 32K used (No OFS)
#define VRAM_E_SLOT0_ENGINE_A_BG_EXTENDED 	(4 | (( 0 )<<3))	
#define VRAM_E_SLOT1_ENGINE_A_BG_EXTENDED 	(4 | (( 1 )<<3))	
#define VRAM_E_SLOT2_ENGINE_A_BG_EXTENDED 	(4 | (( 2 )<<3))	
#define VRAM_E_SLOT3_ENGINE_A_BG_EXTENDED 	(4 | (( 3 )<<3))

//Vram F (16K): ofst 0..1
//Vram G (16K): ofst 0..1
#define VRAM_F_SLOT01_ENGINE_A_BG_EXTENDED 	(4 | (( 0 )<<3))	  	//Slot 0-1 (OFS=0)
#define VRAM_F_SLOT23_ENGINE_A_BG_EXTENDED 	(4 | (( 1 )<<3))		//Slot 2-3 (OFS=1)
#define VRAM_G_SLOT01_ENGINE_A_BG_EXTENDED 	(4 | (( 0 )<<3))	  	//Slot 0-1 (OFS=0)
#define VRAM_G_SLOT23_ENGINE_A_BG_EXTENDED 	(4 | (( 1 )<<3))		//Slot 2-3 (OFS=1)

//VRAM Setup 5: 3D Engine uses VRAM as Texture memory
#define VRAM_A_ENGINE_A_3DENGINE_TEXTURE 	(3)		//VRAM A: 3d texture slot 0.
#define VRAM_B_ENGINE_A_3DENGINE_TEXTURE 	(3 | (1<<3) )		//VRAM B: 3d texture slot 0.

//VRAM Setup 7
//Vram C (128K)
#define VRAM_C_0x06200000_ENGINE_B_BG 	(4 | (0 << 3))	//static

//Vram H (32K)
#define VRAM_H_0x06200000_ENGINE_B_BG 	(1)	// //VRAM H: 32K 06200000h:  map ENGINE_B BG-VRAM, static.

//Vram I (16K)
#define VRAM_I_0x06208000_ENGINE_B_BG 	(1)	//static. 


//VRAM Setup 8
#define VRAM_D_0x06600000_ENGINE_B_OBJVRAM 	(4)	//VRAM D: 128K 06600000h: map ENGINE_B, OBJ-VRAM. static, no OFFSet.
 

//VRAM Setup 11

//Vram C,D     128K  2    0..1  6000000h+(20000h*OFS.0)  ;OFS.1 must be zero
#define VRAM_C_0x06000000_ARM7 	(2 | (0 << 3))	//static
#define VRAM_C_0x06020000_ARM7 	(2 | (1 << 3))	//static

#define VRAM_D_0x06000000_ARM7 	(2 | (0 << 3))	//static
#define VRAM_D_0x06020000_ARM7 	(2 | (1 << 3))	//static

//todo VRAM setups 4,6,8-10, minus 3

//Screen Rotation registers
#define ORIENTATION_0 (int)(0)
#define ORIENTATION_90 (int)(1)
#define ORIENTATION_180 (int)(2)
#define ORIENTATION_270 (int)(3)

//////////////////////////////////////////////////////////////////////

// macro creates a 15 bit color from 3x5 bit components
#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))


#define SCREEN_HEIGHT 192
#define SCREEN_WIDTH  256
//////////////////////////////////////////////////////////////////////
//	Vram Control
#define VRAM_CR			(*(vuint32*)0x04000240)
#define VRAM_A_CR       (*(vuint8*)0x04000240)
#define VRAM_B_CR       (*(vuint8*)0x04000241)
#define VRAM_C_CR       (*(vuint8*)0x04000242)
#define VRAM_D_CR       (*(vuint8*)0x04000243)
#define VRAM_E_CR       (*(vuint8*)0x04000244)
#define VRAM_F_CR       (*(vuint8*)0x04000245)
#define VRAM_G_CR       (*(vuint8*)0x04000246)

#define VRAM_H_CR       (*(vuint8*)0x04000248)
#define VRAM_I_CR       (*(vuint8*)0x04000249)

#define VRAM_ENABLE   (1<<7)
#define VRAM_OFFSET(n)  ((n)<<3)

typedef enum
{
	VRAM_A_LCD = 0,
	VRAM_A_MAIN_BG  = 1,
	VRAM_A_MAIN_SPRITE = 2,
	VRAM_A_TEXTURE = 3

}VRAM_A_TYPE;

typedef enum
{
	VRAM_B_LCD = 0,
	VRAM_B_MAIN_BG  = 1 | VRAM_OFFSET(1),
	VRAM_B_MAIN_SPRITE = 2,
	VRAM_B_TEXTURE = 3 | VRAM_OFFSET(1)

}VRAM_B_TYPE;	

typedef enum
{
	VRAM_C_LCD = 0,
	VRAM_C_MAIN_BG  = 1 | VRAM_OFFSET(2),
	VRAM_C_MAIN_SPRITE = 2,
	VRAM_C_SUB_BG  = 4,
	VRAM_C_TEXTURE = 3 | VRAM_OFFSET(2)

}VRAM_C_TYPE;

typedef enum
{
	VRAM_D_LCD = 0,
	VRAM_D_MAIN_BG  = 1 | VRAM_OFFSET(3),
	VRAM_D_MAIN_SPRITE = 2,
	VRAM_D_SUB_SPRITE = 4,
	VRAM_D_TEXTURE = 3 | VRAM_OFFSET(3)

}VRAM_D_TYPE;

typedef enum
{
	VRAM_E_LCD			=0,
	VRAM_E_TEX_PALETTE = 3
}VRAM_E_TYPE;

//////////////////////////////////////////////////////////////////////
// Display control registers
//////////////////////////////////////////////////////////////////////

#define DISPLAY_CR       (*(vuint32*)0x04000000)
#define SUB_DISPLAY_CR   (*(vuint32*)0x04001000)

#define MODE_0_2D      0x10000
#define MODE_1_2D      0x10001
#define MODE_2_2D      0x10002
#define MODE_3_2D      0x10003
#define MODE_4_2D      0x10004
#define MODE_5_2D      0x10005

// main display only
#define MODE_6_2D      0x10006

#define ENABLE_3D    (1<<3)
#define DISPLAY_ENABLE_SHIFT 8
#define DISPLAY_BG0_ACTIVE    (1 << 8)
#define DISPLAY_BG1_ACTIVE    (1 << 9)
#define DISPLAY_BG2_ACTIVE    (1 << 10)
#define DISPLAY_BG3_ACTIVE    (1 << 11)
#define DISPLAY_SPR_ACTIVE    (1 << 12)
#define DISPLAY_WIN0_ON       (1 << 13)
#define DISPLAY_WIN1_ON       (1 << 14)
#define DISPLAY_SPR_WIN_ON    (1 << 15)

// Main display only
#define MODE_0_3D    (MODE_0_2D | DISPLAY_BG0_ACTIVE | ENABLE_3D) 
#define MODE_1_3D    (MODE_1_2D | DISPLAY_BG0_ACTIVE | ENABLE_3D)
#define MODE_2_3D    (MODE_2_2D | DISPLAY_BG0_ACTIVE | ENABLE_3D)
#define MODE_3_3D    (MODE_3_2D | DISPLAY_BG0_ACTIVE | ENABLE_3D)
#define MODE_4_3D    (MODE_4_2D | DISPLAY_BG0_ACTIVE | ENABLE_3D)
#define MODE_5_3D    (MODE_5_2D | DISPLAY_BG0_ACTIVE | ENABLE_3D)
#define MODE_6_3D    (MODE_6_2D | DISPLAY_BG0_ACTIVE | ENABLE_3D)

#define MODE_FB0    (0x00020000)
#define MODE_FB1    (0x00060000)
#define MODE_FB2	(0x000A0000)
#define MODE_FB3	(0x000E0000)


#define DISPLAY_OAM_ACCESS    (1 << 5)
#define DISPLAY_SPR_1D_LAYOUT (1 << 4)
#define DISPLAY_SCREEN_OFF    (1 << 7)
#define DISPLAY_BG_EXT_PALETTE	(1 << 30)

#define H_BLANK_OAM    (1<<5)

#define OBJ_MAP_2D    (0<<4)
#define OBJ_MAP_1D    (1<<4)

#define FORCED_BLANK  (1<<7)

#define videoSetMode(mode)  (DISPLAY_CR = (mode))
#define videoSetModeSub(mode)  (SUB_DISPLAY_CR = (mode))
//////////////////////////////////////////////////////////////////////

#define BRIGHTNESS     (*(vuint16*)0x0400006C)
#define SUB_BRIGHTNESS (*(vuint16*)0x0400106C)

//////////////////////////////////////////////////////////////////////

#define BG_CR		    ((vuint16*)0x04000008)
#define BG0_CR         (*(vuint16*)0x04000008)
#define BG1_CR         (*(vuint16*)0x0400000A)
#define BG2_CR         (*(vuint16*)0x0400000C)
#define BG3_CR         (*(vuint16*)0x0400000E)

#define SUB_BG_CR		((vuint16*)0x04001008)
#define SUB_BG0_CR     (*(vuint16*)0x04001008)
#define SUB_BG1_CR     (*(vuint16*)0x0400100A)
#define SUB_BG2_CR     (*(vuint16*)0x0400100C)
#define SUB_BG3_CR     (*(vuint16*)0x0400100E)

#define	BGCTRL			( (vuint16*)0x4000008)
#define	BGCTRL_SUB				( (vuint16*)0x4001008)
#define	REG_BGOFFSETS	( (vuint16*)0x4000010)
#define	REG_BG0VOFS		(*(vuint16*)0x4000012)
#define	REG_BG2PA		(*(vsint16*)0x4000020)
#define	REG_BG2PB		(*(vsint16*)0x4000022)
#define	REG_BG2PC		(*(vsint16*)0x4000024)
#define	REG_BG2PD		(*(vsint16*)0x4000026)
#define	REG_BG2X		(*(vsint32*)0x4000028)
#define	REG_BG2Y		(*(vsint32*)0x400002C)
#define	REG_BG3PA		(*(vsint16*)0x4000030)
#define	REG_BG3PB		(*(vsint16*)0x4000032)
#define	REG_BG3PC		(*(vsint16*)0x4000034)
#define	REG_BG3PD		(*(vsint16*)0x4000036)
#define	REG_BG3X		(*(vsint32*)0x4000038)
#define	REG_BG3Y		(*(vsint32*)0x400003C)

#define BG0_X0         (*(vuint16*)0x04000010)
#define BG0_Y0         (*(vuint16*)0x04000012)
#define BG1_X0         (*(vuint16*)0x04000014)
#define BG1_Y0         (*(vuint16*)0x04000016)
#define BG2_X0         (*(vuint16*)0x04000018)
#define BG2_Y0         (*(vuint16*)0x0400001A)
#define BG3_X0         (*(vuint16*)0x0400001C)
#define BG3_Y0         (*(vuint16*)0x0400001E)

#define BG2_XDX        (*(vuint16*)0x04000020)
#define BG2_XDY        (*(vuint16*)0x04000022)
#define BG2_YDX        (*(vuint16*)0x04000024)
#define BG2_YDY        (*(vuint16*)0x04000026)
#define BG2_CX         (*(vuint32*)0x04000028)
#define BG2_CY         (*(vuint32*)0x0400002C)

#define BG3_XDX        (*(vuint16*)0x04000030)
#define BG3_XDY        (*(vuint16*)0x04000032)
#define BG3_YDX        (*(vuint16*)0x04000034)
#define BG3_YDY        (*(vuint16*)0x04000036)
#define BG3_CX         (*(vuint32*)0x04000038)
#define BG3_CY         (*(vuint32*)0x0400003C)

//////////////////////////////////////////////////////////////////////

#define SUB_BG0_X0     (*(vuint16*)0x04001010)
#define SUB_BG0_Y0     (*(vuint16*)0x04001012)
#define SUB_BG1_X0     (*(vuint16*)0x04001014)
#define SUB_BG1_Y0     (*(vuint16*)0x04001016)
#define SUB_BG2_X0     (*(vuint16*)0x04001018)
#define SUB_BG2_Y0     (*(vuint16*)0x0400101A)
#define SUB_BG3_X0     (*(vuint16*)0x0400101C)
#define SUB_BG3_Y0     (*(vuint16*)0x0400101E)

#define SUB_BG2_XDX    (*(vuint16*)0x04001020)
#define SUB_BG2_XDY    (*(vuint16*)0x04001022)
#define SUB_BG2_YDX    (*(vuint16*)0x04001024)
#define SUB_BG2_YDY    (*(vuint16*)0x04001026)
#define SUB_BG2_CX     (*(vuint32*)0x04001028)
#define SUB_BG2_CY     (*(vuint32*)0x0400102C)

#define SUB_BG3_XDX    (*(vuint16*)0x04001030)
#define SUB_BG3_XDY    (*(vuint16*)0x04001032)
#define SUB_BG3_YDX    (*(vuint16*)0x04001034)
#define SUB_BG3_YDY    (*(vuint16*)0x04001036)
#define SUB_BG3_CX     (*(vuint32*)0x04001038)
#define SUB_BG3_CY     (*(vuint32*)0x0400103C)

//////////////////////////////////////////////////////////////////////

// Window 0
#define WIN0_X0        (*(vuint8*)0x04000041)
#define WIN0_X1        (*(vuint8*)0x04000040)
#define WIN0_Y0        (*(vuint8*)0x04000045)
#define WIN0_Y1        (*(vuint8*)0x04000044)

// Window 1
#define WIN1_X0        (*(vuint8*)0x04000043)
#define WIN1_X1        (*(vuint8*)0x04000042)
#define WIN1_Y0        (*(vuint8*)0x04000047)
#define WIN1_Y1        (*(vuint8*)0x04000046)

#define WIN_IN         (*(vuint16*)0x04000048)
#define WIN_OUT        (*(vuint16*)0x0400004A)

// Window 0
#define SUB_WIN0_X0    (*(vuint8*)0x04001041)
#define SUB_WIN0_X1    (*(vuint8*)0x04001040)
#define SUB_WIN0_Y0    (*(vuint8*)0x04001045)
#define SUB_WIN0_Y1    (*(vuint8*)0x04001044)

// Window 1
#define SUB_WIN1_X0    (*(vuint8*)0x04001043)
#define SUB_WIN1_X1    (*(vuint8*)0x04001042)
#define SUB_WIN1_Y0    (*(vuint8*)0x04001047)
#define SUB_WIN1_Y1    (*(vuint8*)0x04001046)

#define SUB_WIN_IN     (*(vuint16*)0x04001048)
#define SUB_WIN_OUT    (*(vuint16*)0x0400104A)

#define	REG_MOSAIC		(*(vuint16*)0x0400004C)
#define	REG_MOSAIC_SUB	(*(vuint16*)0x0400104C)

#define REG_BLDCNT     (*(vuint16*)0x04000050)
#define REG_BLDY	   (*(vuint16*)0x04000054)
#define REG_BLDALPHA   (*(vuint16*)0x04000052)

//////////////////////////////////////////////////////////////////////

#define MOSAIC_CR      (*(vuint16*)0x0400004C)
#define SUB_MOSAIC_CR  (*(vuint16*)0x0400104C)

//////////////////////////////////////////////////////////////////////

#define BLEND_CR       (*(vuint16*)0x04000050)
#define BLEND_AB       (*(vuint16*)0x04000052)
#define BLEND_Y        (*(vuint16*)0x04000054)

#define SUB_BLEND_CR   (*(vuint16*)0x04001050)
#define SUB_BLEND_AB   (*(vuint16*)0x04001052)
#define SUB_BLEND_Y    (*(vuint16*)0x04001054)

#define BLEND_NONE       (0<<6)
#define BLEND_ALPHA      (1<<6)
#define BLEND_FADE_WHITE (2<<6)
#define BLEND_FADE_BLACK (3<<6)

//////////////////////////////////////////////////////////////////////
// Background control defines
//////////////////////////////////////////////////////////////////////
///
///BGxCNT defines ///
#define BG_MOSAIC_ENABLE    0x40

#define CHAR_SHIFT        2
#define SCREEN_SHIFT      8
#define TEXTBG_SIZE_256x256    0x0
#define TEXTBG_SIZE_256x512    0x8000
#define TEXTBG_SIZE_512x256    0x4000
#define TEXTBG_SIZE_512x512    0xC000

#define ROTBG_SIZE_128x128    0x0
#define ROTBG_SIZE_256x256    0x4000
#define ROTBG_SIZE_512x512    0x8000
#define ROTBG_SIZE_1024x1024  0xC000

#define WRAPAROUND              0x1

//////////////////////////////////////////////////////////////////////
// Sprite control defines
//////////////////////////////////////////////////////////////////////

// Attribute 0 consists of 8 bits of Y plus the following flags:
#define ATTR0_NORMAL          (0<<8)
#define ATTR0_ROTSCALE        (1<<8)
#define ATTR0_DISABLED        (2<<8)
#define ATTR0_ROTSCALE_DOUBLE (3<<8)

#define ATTR0_TYPE_NORMAL     (0<<10)
#define ATTR0_TYPE_BLENDED    (1<<10)
#define ATTR0_TYPE_WINDOWED   (2<<10)

#define ATTR0_MOSAIC          (1<<12)

#define ATTR0_COLOR_16        (0<<13)
#define ATTR0_COLOR_256       (1<<13)

#define ATTR0_SQUARE          (0<<14)
#define ATTR0_WIDE            (1<<14)
#define ATTR0_TALL            (2<<14)
  
// Atribute 1 consists of 9 bits of X plus the following flags:
#define ATTR1_ROTDATA(n)      ((n)<<9)  // note: overlaps with flip flags
#define ATTR1_FLIP_X          (1<<12)
#define ATTR1_FLIP_Y          (1<<13)
#define ATTR1_SIZE_8          (0<<14)
#define ATTR1_SIZE_16         (1<<14)
#define ATTR1_SIZE_32         (2<<14)
#define ATTR1_SIZE_64         (3<<14)

// Atribute 2 consists of the following:
#define ATTR2_PRIORITY(n)     ((n)<<10)
#define ATTR2_PALETTE(n)      ((n)<<12)

#endif

#ifdef __cplusplus
extern "C"{
#endif

//power
extern void SWAP_LCDS();
extern void SET_MAIN_TOP_LCD();
extern void SET_MAIN_BOTTOM_LCD();

//VRAM
extern void SETDISPCNT_MAIN(uint32 mode);	//videoSetMode
extern void SETDISPCNT_SUB(uint32 mode);	//videoSetModeSub

extern void VRAMBLOCK_SETBANK_A(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_B(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_C(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_D(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_E(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_F(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_G(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_H(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_I(uint8 vrambits);

extern void ENABLE_BG_MAIN(int bg);
extern void ENABLE_BG_SUB(int bg);
extern void DISABLE_BG_MAIN(int number);
extern void DISABLE_BG_SUB(int number);

extern vramSetup vramSetupDefaultConsole;
extern vramSetup vramSetupCustomConsole;

extern bool VRAM_SETUP(vramSetup * vramSetupInst);

//weak symbols : the implementation of this is project-defined
extern  vramSetup * getProjectSpecificVRAMSetup();


//Default console VRAM layout setup
//1) VRAM Layout
extern vramSetup * DEFAULT_CONSOLE_VRAMSETUP();
//2) Uses subEngine: VRAM Layout -> Console Setup
extern bool InitDefaultConsole();

extern void initFBModeSubEngine0x06200000();
extern void renderFBMode3Engine(u16 * srcBuf, u16 * targetBuf, int srcWidth, int srcHeight);
extern void setOrientation(int orientation, bool mainEngine);

extern void initFBModeMainEngine0x06000000();	//set FB mode, saves old MainEngine context
extern void restoreFBModeMainEngine();	//restore from old MainEngine context discards such context

#ifdef __cplusplus
}
#endif