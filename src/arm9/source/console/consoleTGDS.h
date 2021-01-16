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

#ifndef __console_toolchain_h__
#define __console_toolchain_h__

#include <stdbool.h>
#include "posixHandleTGDS.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "ipcfifoTGDS.h"
#include "spifwTGDS.h"

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

#define BACKGROUND           (*((bg_attribute *)0x04000008))
#define BG_OFFSET ((bg_scroll *)(0x04000010))

#define BACKGROUND_SUB       (*((bg_attribute *)0x04001008))
#define BG_OFFSET_SUB ((bg_scroll *)(0x04001010))

//main
#define BG_MAP_RAM(base)		((uint16*)(((base)*0x800) + 0x06000000))
#define BG_TILE_RAM(base)		((uint16*)(((base)*0x4000) + 0x06000000))
#define BG_BMP_RAM(base)		((uint16*)(((base)*0x4000) + 0x06000000))
#define CHAR_BASE_BLOCK(n)			(((n)*0x4000)+ 0x06000000)
#define SCREEN_BASE_BLOCK(n)		(((n)*0x800) + 0x06000000)

//sub
#define BG_MAP_RAM_SUB(base)	((uint16*)(((base)*0x800) + 0x06200000))
#define BG_TILE_RAM_SUB(base)	((uint16*)(((base)*0x4000) + 0x06200000))
#define BG_BMP_RAM_SUB(base)	((uint16*)(((base)*0x4000) + 0x06200000))
#define SCREEN_BASE_BLOCK_SUB(n)	(((n)*0x800) + 0x06200000)
#define CHAR_BASE_BLOCK_SUB(n)		(((n)*0x4000)+ 0x06200000)
#define	BGCTRL_SUB				( (vuint16*)0x4001008)
#define	REG_BG0CNT_SUB		(*(vuint16*)0x4001008)
#define	REG_BG1CNT_SUB		(*(vuint16*)0x400100A)
#define	REG_BG2CNT_SUB		(*(vuint16*)0x400100C)
#define	REG_BG3CNT_SUB		(*(vuint16*)0x400100E)

//Recursive IO 
#define	REG_BGXCNT(i)			*(vuint16*)(0x04000008 + ((sint32)i*2))
#define	REG_BGXCNT_SUB(i)		*(vuint16*)(0x04001008 + ((sint32)i*2))

#define	REG_BGOFFSETS_SUB	( (vuint16*)0x4001010)
#define	REG_BG0HOFS_SUB		(*(vuint16*)0x4001010)
#define	REG_BG0VOFS_SUB		(*(vuint16*)0x4001012)
#define	REG_BG1HOFS_SUB		(*(vuint16*)0x4001014)
#define	REG_BG1VOFS_SUB		(*(vuint16*)0x4001016)
#define	REG_BG2HOFS_SUB		(*(vuint16*)0x4001018)
#define	REG_BG2VOFS_SUB		(*(vuint16*)0x400101A)
#define	REG_BG3HOFS_SUB		(*(vuint16*)0x400101C)
#define	REG_BG3VOFS_SUB		(*(vuint16*)0x400101E)

#define	REG_BG2PA_SUB		(*(vsint16*)0x4001020)
#define	REG_BG2PB_SUB		(*(vsint16*)0x4001022)
#define	REG_BG2PC_SUB		(*(vsint16*)0x4001024)
#define	REG_BG2PD_SUB		(*(vsint16*)0x4001026)

#define	REG_BG2X_SUB		(*(vsint32*)0x4001028)
#define	REG_BG2Y_SUB		(*(vsint32*)0x400102C)

#define	REG_BG3PA_SUB		(*(vsint16*)0x4001030)
#define	REG_BG3PB_SUB		(*(vsint16*)0x4001032)
#define	REG_BG3PC_SUB		(*(vsint16*)0x4001034)
#define	REG_BG3PD_SUB		(*(vsint16*)0x4001036)

#define	REG_BG3X_SUB		(*(vsint32*)0x4001038)
#define	REG_BG3Y_SUB		(*(vsint32*)0x400103C)

// TGDS GUI Colors

#define TGDSPrintfColor_PalleteBase			0
#define TGDSPrintfColor_BackGroundColor		(u8)(TGDSPrintfColor_PalleteBase+0)
#define TGDSPrintfColor_Black		(u8)(TGDSPrintfColor_PalleteBase+0)
#define TGDSPrintfColor_White		(u8)(TGDSPrintfColor_PalleteBase+1)
#define TGDSPrintfColor_Brown		(u8)(TGDSPrintfColor_PalleteBase+2)
#define TGDSPrintfColor_Orange		(u8)(TGDSPrintfColor_PalleteBase+3)
#define TGDSPrintfColor_Magenta		(u8)(TGDSPrintfColor_PalleteBase+4)
#define TGDSPrintfColor_Cyan		(u8)(TGDSPrintfColor_PalleteBase+5)
#define TGDSPrintfColor_Yellow		(u8)(TGDSPrintfColor_PalleteBase+6)
#define TGDSPrintfColor_Blue		(u8)(TGDSPrintfColor_PalleteBase+7)
#define TGDSPrintfColor_Green		(u8)(TGDSPrintfColor_PalleteBase+8)
#define TGDSPrintfColor_Red		(u8)(TGDSPrintfColor_PalleteBase+9)
#define TGDSPrintfColor_Grey		(u8)(TGDSPrintfColor_PalleteBase+10)
#define TGDSPrintfColor_LightGrey		(u8)(TGDSPrintfColor_PalleteBase+11)


//Console uses 2D. used by REG_DISPCNT / REG_DISPCNT_SUB
typedef enum {
	MODE_0_2D = 0x10000, 
	MODE_1_2D = 0x10001, 
	MODE_2_2D = 0x10002, 
	MODE_3_2D = 0x10003, 
	MODE_4_2D = 0x10004, 
	MODE_5_2D = 0x10005, 
	MODE_6_2D = 0x10006
	//todo 3D and framebuffer modes
} VideoMode;

#define backgroundsPerEngine 4
typedef struct EngineBGStatus
{
	uint32 tile_base;
	uint32 map_base;
	sint32 BGNUM;
	uint32 REGBGCNT; 	//every backgroundsPerEngine unit
}EngineBGStatus;

typedef struct EngineStatus
{
	uint32 ENGINE_DISPCNT;
	EngineBGStatus EngineBGS[4];
}EngineStatus;

typedef enum {
	mainEngine = (0x1<<0), 
	subEngine = (0x0<<0)
} PPUEngine;

//each is a Console Instance. The idea is to re-use SnemulDS console render so we are not limited to a single background - tile format, but takeover an engine as whole for rendering.
typedef struct ConsoleInstance
{
	PPUEngine ppuMainEngine;
	uint16* gfx;
	EngineStatus ConsoleEngineStatus;
}ConsoleInstance;

//font
typedef struct
{
    int		width;
    int		height;
    int		bpp;
    uint8	*palette;   
    uint8   data[];
} t_GUIGlyph;

typedef struct
{
    int		width;
    int		height;
    int		bpp;
    uint8	*palette;   
    uint16   data[];
} t_GUIGlyph16;

typedef struct
{
    int height;
    int	offset;
    
    int space;
    int interline;

    t_GUIGlyph **glyphs;
} t_GUIFont;

typedef struct
{
	uint16	width;
	uint8	height;
	uint8	flags;
	
	void	*data;
} t_GUIImage;

typedef struct
{
	int			nb;
	int			cnt;
	t_GUIImage	*img[];
} t_GUIImgList;

typedef struct s_GUIZone t_GUIZone;
typedef int (*t_GUIHandler)(t_GUIZone *zone, int message, int param, void *arg);

struct s_GUIZone
{
	int				x1;
	int 			y1;
	int 			x2;
	int				y2;

	uint8			id;	
	uint8			state;	
	uint16			keymask;

	t_GUIHandler	handler;
	void			*data;
	
	t_GUIFont		*font;
};

typedef struct
{
	int			nb_zones;
	int			curs;
	int			stylus_zone;
	int			flags;
	t_GUIHandler	handler;
	t_GUIImgList	*img_list;
	
	uint16		last_focus; // Number of focusable zones 
	uint16		incr_focus; // Increment factor for joypad focus
	
	t_GUIZone	zones[100];
} t_GUIScreen;

typedef struct
{
// TODO : put in one flag field
	int	log;
	int exit;
	int hide;	
	int ScanJoypad; 
	
	uint16	*DSFrameBuffer; // Frame Buffer Layer
	uint16	*DSText; // Text Layer	/ not used
	uint16	*DSBack; // Back Text Layer;	/ not used
	uint16	*DSTileMemory;	// not used
	
	uint16	*Palette;
	
	t_GUIScreen	*screen;
	
	sint8	**string;
	
	t_GUIImgList	*img_list;
	
	uint16	printfy;
	bool consoleAtTopScreen;	//true: Console rendering at top screen / false: Console rendering at bottom screen
	bool consoleBacklightOn;	//true: Backlight is on for console / false: Backlight is off for console
	
} t_GUI;

// GUI Colors

#define GUI_PAL			216
//#define GUI_BLACK		0
#define GUI_BLACK		(GUI_PAL+0)
#define GUI_DARKGREY2	(GUI_PAL+1)
#define GUI_DARKGREY	(GUI_PAL+2)
#define GUI_GREY		(GUI_PAL+3)

#define GUI_DARKRED		(GUI_PAL+4)
#define GUI_RED			(GUI_PAL+5)
#define GUI_LIGHTRED	(GUI_PAL+6)

#define GUI_DARKGREEN	(GUI_PAL+12)
#define GUI_GREEN		(GUI_PAL+13)
#define GUI_LIGHTGREEN	(GUI_PAL+14)

#define GUI_DARKBLUE	(GUI_PAL+8)
#define GUI_BLUE		(GUI_PAL+9)
#define GUI_LIGHTBLUE	(GUI_PAL+10)

#define GUI_DARKYELLOW	(GUI_PAL+17)
#define GUI_YELLOW		(GUI_PAL+18)
#define GUI_LIGHTYELLOW	(GUI_PAL+19)

#define GUI_LIGHTGREY	(GUI_WHITE-2)
#define GUI_LIGHTGREY2	(GUI_WHITE-1)
#define GUI_WHITE		255


#ifdef __cplusplus
extern "C" {
#endif

extern	t_GUI	GUI;

extern ConsoleInstance DefaultConsole;	//default console
extern ConsoleInstance CustomConsole;	//project specific
extern ConsoleInstance * DefaultSessionConsole;

//weak symbols : the implementation of this is project-defined
extern 	bool InitProjectSpecificConsole();

extern bool InitializeConsole(ConsoleInstance * ConsoleInst);
extern void UpdateConsoleSettings(ConsoleInstance * ConsoleInst);
extern void SetEngineConsole(PPUEngine engine,ConsoleInstance * ConsoleInst);
extern void consoleClear(ConsoleInstance * ConsoleInst);
extern t_GUIFont katakana_12_font;

extern void	GUI_console_printf(int cx, int cy, sint8 *fmt, ...);
extern void GUI_printf(sint8 *fmt, ...);
extern void	GUI_clearScreen(int color);
extern void	GUI_clear();
extern volatile sint8 ConsolePrintfBuf[256+1];

//font 
extern t_GUIFont trebuchet_9_font;
extern t_GUIFont smallfont_7_font;

extern void _glyph_loadline_8(uint8 *dst, uint8 *data, int pos, int size, uint8 *pal);
extern void _glyph_loadline_2(uint8 *dst, uint8 *data, int pos, int size, uint8 *pal);
extern void _glyph_loadline_1(uint8 *dst, uint8 *data, int pos, int size, uint8 *pal);

extern int GUI_drawVChar(t_GUIZone *zone, t_GUIFont *font, uint16 x, uint16 y, int col, uint8 text, bool readAndBlendFromVRAM);
extern uint8 g_katana_jisx0201_conv[];
extern int 	GUI_drawText(t_GUIZone *zone, uint16 x, uint16 y, int col, sint8 *text, bool readAndBlendFromVRAM);
extern int 	GUI_getStrWidth(t_GUIZone *zone, sint8 *text);
extern int 	GUI_getFontHeight(t_GUIZone *zone);
extern int	GUI_getZoneTextHeight(t_GUIZone *zone);
extern int 	GUI_drawAlignText(t_GUIZone *zone, int flags, int y, int col, sint8 *text);
extern void clrscr();
extern bool globalTGDSCustomConsole;
extern void	GUI_init(bool project_specific_console);

extern void restoreTGDSConsoleFromSwapEngines(u8 * currentVRAMContext);
extern void swapTGDSConsoleBetweenPPUEngines(u8 * currentVRAMContext);
extern void TGDSLCDSwap(bool disableTSCWhenTGDSConsoleTop, bool isDirectFramebuffer, bool SaveConsoleContext, u8 * currentVRAMContext);


#ifdef __cplusplus
}
#endif

static inline void detectAndTurnOffConsole(){
	//Read the console location register and shut down
	if(GUI.consoleAtTopScreen == true){
		setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
	}
	else{
		setBacklight(POWMAN_BACKLIGHT_TOP_BIT);	
	}
}

static inline void ToggleOnOffConsoleBacklight(){
	if(GUI.consoleBacklightOn == true){
		detectAndTurnOffConsole();
		GUI.consoleBacklightOn = false;
	}
	else{
		setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT | POWMAN_BACKLIGHT_TOP_BIT);
		GUI.consoleBacklightOn = true;
	}
}

//todo: extern void TGDSLCDSwap(bool disableTSCWhenTGDSConsoleTop, bool isDirectFramebuffer, bool SaveConsoleContext, u8 * currentVRAMContext);


#endif

