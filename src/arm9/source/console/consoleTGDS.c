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

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "consoleTGDS.h"
#include "dmaTGDS.h"
#include "biosTGDS.h"
#include "videoTGDS.h"
#include "ipcfifoTGDS.h"
#include "keypadTGDS.h"

bool globalTGDSCustomConsole=false;
t_GUI GUI;
volatile sint8	ConsolePrintfBuf[256+1];
ConsoleInstance DefaultConsole;		//generic console
ConsoleInstance CustomConsole;		//project specific console
ConsoleInstance * DefaultSessionConsole;	//Default Console Instance Chosen

bool InitializeConsole(ConsoleInstance * ConsoleInst){	
	UpdateConsoleSettings(ConsoleInst);	
	return true;
}

void UpdateConsoleSettings(ConsoleInstance * ConsoleInst){
	//setup DISPCNT
	if(ConsoleInst->ppuMainEngine == mainEngine){
		SETDISPCNT_MAIN((uint32)ConsoleInst->ConsoleEngineStatus.ENGINE_DISPCNT);
	}
	else if(ConsoleInst->ppuMainEngine == subEngine){
		SETDISPCNT_SUB((uint32)ConsoleInst->ConsoleEngineStatus.ENGINE_DISPCNT);
	}
	//setup Backgrounds per NDS Render Engine
	int i = 0;
	for(i = 0; i < (int)backgroundsPerEngine; i++){
	
		if(ConsoleInst->ppuMainEngine == mainEngine){	
			REG_BGXCNT(i) = DefaultSessionConsole->ConsoleEngineStatus.EngineBGS[i].REGBGCNT;
		}
		else if(ConsoleInst->ppuMainEngine == subEngine){
			REG_BGXCNT_SUB(i) = DefaultSessionConsole->ConsoleEngineStatus.EngineBGS[i].REGBGCNT;
		}
	}
}

void SetEngineConsole(PPUEngine engine,ConsoleInstance * ConsoleInst){
	ConsoleInst->ppuMainEngine = engine;
}

void consoleClear(ConsoleInstance * ConsoleInst){
	GUI_clearScreen(0);
}

//used by gui_printf
void		GUI_clearScreen(int color)
{
	uint16		*ptr;
	ptr = GUI.DSFrameBuffer;
	uint32	c = color | (color << 8) | (color << 16) | (color << 24);
	swiFastCopy((uint32*)&c, (uint32*)(uint16*)ptr, ((256*192)/4) | COPY_FIXED_SOURCE);
}

//used by gui_printf
void _glyph_loadline_1(uint8 *dst, uint8 *data, int pos, int size, uint8 *pal)
{
	int	x, x2;
	for (x = 0, x2 = pos; x < size; x++, x2++)
	{
	  //int c = (data[x2 >> 3] >> (7 - (x2 & 7))) & 1;
	  //dst[x] = c ? *pal : c;
	  dst[x] = pal[(data[x2 >> 3] >> (7 - (x2 & 7))) & 1];
	}
		
}

//used by gui_printf
void _glyph_loadline_2(uint8 *dst, uint8 *data, int pos, int size, uint8 *pal)
{
	int	x, x2;	
	for (x = 0, x2 = pos; x < size; x++, x2++)
	{
	  //int c = (data[x2 >> 2] >> ((3 - (x2 & 3)) << 1)) & 3;
	  //dst[x] = c ? pal[c] : c;
	  dst[x] = pal[(data[x2 >> 2] >> ((3 - (x2 & 3)) << 1)) & 3];
	}	
}

//used by gui_printf
void _glyph_loadline_8(uint8 *dst, uint8 *data, int pos, int size, uint8 *pal)
{
	int	x, x2;	
	for (x = 0, x2 = pos; x < size; x++, x2++)
	  dst[x] = data[x2];
		
}

//used by gui_printf
int GUI_drawVChar(t_GUIZone *zone, t_GUIFont *font, uint16 x, uint16 y, int col, uint8 text, bool readAndBlendFromVRAM)
{
	if ((int)text - font->offset < 0 || font->glyphs[text - font->offset] == NULL)
		return 0;
	
    t_GUIGlyph	*glyph = font->glyphs[text - font->offset];

    int	gw = glyph->width + 1;	//glyph width: x1: char start : x2: char end: x: char offset
    
    x += zone->x1;
    y += zone->y1;
    
    if (gw > zone->x2 - x)
    	gw = zone->x2 - x;
    
    void (*glyph_loadline)(uint8 *dst, uint8 *data, int line, int size, uint8 *pal);
    switch (glyph->bpp)
    {
      case 0: return gw;
      case 1: glyph_loadline = _glyph_loadline_1; break;
      case 2: glyph_loadline = _glyph_loadline_2; break;
      case 8: glyph_loadline = _glyph_loadline_8; break;
      default: return gw;    
    }
    
    int wn, xo, w, wo;
    // Number of halft-words to write
    wn = (gw / 2) + ((x & 1) | (gw & 1));
    // If x is odd, we start from x - 1
    xo = x & 1;
    x = (xo) ? (x - 1) : x; 
    // End of the line is odd or not ?
    wo = xo ^ (gw & 1);    

    // Adjust y if glyph is smaller tnat font size
    y += font->height/2 - glyph->height/2;
    
    uint8	*pal = glyph->palette;
   	uint8 _pal[1 << glyph->bpp]; // FIXME
    
   	if (pal == NULL)
   	{
   		pal = &_pal[0];
   		pal[0] = 0;
   		pal[1] = col;
   	}
    
    uint8	glyph_data[glyph->width + 1];
    int		l, p;
    for (l = 0, p = 0; l < glyph->height+1; l++, p += glyph->width + 1) 
    {
    	uint16 *ptr = GUI.DSFrameBuffer + ( ((l+y) * 256 + x) / 2 );
    	uint8	*glyph_ptr = &glyph_data[0];
    	
    	glyph_loadline(glyph_data, glyph->data, p, glyph->width + 1, pal);

    	//  Loop 
        for (w = 0; w < wn; w++) 
        {
            uint16 cl, cr;
            uint16 v = 0;
			if(readAndBlendFromVRAM == true){
				v = *ptr;
			}
            if (w == 0 && xo != 0) 
            {
                // First word
                cl = (v & 0x00ff);
                cr = *glyph_ptr++ << 8;
           		cr = cr ? cr : (v & 0xff00);                
            }
            else if (w == wn-1 && wo != 0) 
            {
            	// Last word
            	cl = *glyph_ptr++;
                cr = (v & 0xff00);
                cl = cl ? cl : (v & 0x00ff);
            }
            else 
            {
                cl = *glyph_ptr++;
                cr = *glyph_ptr++ << 8;
                cl = cl ? cl : (v & 0x00ff);
           		cr = cr ? cr : (v & 0xff00);                
            }
            *ptr++ = cl | cr;
        }
    }

    return glyph->width;
}

// This array convert from jis x201 to the charmap of the katana font
//used by gui_printf
uint8 g_katana_jisx0201_conv[] = 
{ 0x6E,0x59,0x4D,0x7C,0x55,0x4E,0x3C,0x2E,
  0x3F,0x2B,0x5F,0x31,0x32,0x33,0x34,0x35,
  0x71,0x77,0x65,0x72,0x74,0x61,0x73,0x64,
  0x66,0x67,0x7A,0x78,0x63,0x76,0x62,0x36,
  0x37,0x38,0x39,0x30,0x79,0x75,0x69,0x6F,
  0x70,0x68,0x6A,0x6B,0x6C,0x3B,0x2C,0x2E,
  0x2F,0x2D,0x3D,0x5B,0x5D,0x5C,0x27,0x67,
  0x22,0x51 };

//used by gui_printf
int GUI_drawText(t_GUIZone *zone, uint16 x, uint16 y, int col, sint8 *text, bool readAndBlendFromVRAM)
{
	t_GUIFont   *font = zone->font;
	int			in_katakana = 0;
    int 		i, w;

    for (i=0, w=0; i < strlen(text); i++)
    {
    	if (text[i] == 0x0e)
    	{
    		in_katakana = 1;
    		continue;
    	}
    	if (text[i] == 0x0f)
    	{
    		in_katakana = 0;
    		continue;
    	}

    	if (in_katakana)
    	{
    		if (text[i] < 0x26 || text[i] > 0x5f)
    			continue;
    		sint8 c = g_katana_jisx0201_conv[text[i]-0x26];
    		w += GUI_drawVChar(zone, &katakana_12_font, x+w, y, col, c, readAndBlendFromVRAM) + font->space;
    	}
    	else
    		w += GUI_drawVChar(zone, font, x+w, y, col, text[i], readAndBlendFromVRAM) + font->space;
    }
    return w - font->space;
}

//used by gui_printf
int GUI_getFontHeight(t_GUIZone *zone)
{
	return zone->font->height+1;
}

//used by gui_printf
void		GUI_printf(sint8 *fmt, ...)
{	
	va_list args;
	va_start (args, fmt);
	vsnprintf ((sint8*)ConsolePrintfBuf, 64, fmt, args);
	va_end (args);
	
    // FIXME
    bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &smallfont_7_font;
	int color = 0xff;	//white
    GUI_drawText(&zone, 0, GUI.printfy, color, (sint8*)ConsolePrintfBuf, readAndBlendFromVRAM);
    GUI.printfy += GUI_getFontHeight(&zone);
}

void	GUI_clear()
{
	//flush buffers
	memset ((uint32 *)&ConsolePrintfBuf[0], 0, sizeof(ConsolePrintfBuf));
	consoleClear(DefaultSessionConsole);
	GUI.printfy = 0;
}

void clrscr(){
	GUI_clear();
}

int		GUI_getZoneTextHeight(t_GUIZone *zone)
{
	return (zone->y2 - zone->y1) / (GUI_getFontHeight(zone)+1);
}

int GUI_getStrWidth(t_GUIZone *zone, sint8 *text)
{
	t_GUIFont   *font = zone->font;
	int			in_katakana = 0;
    int 		i, w;

    for (i=0, w=0; i < strlen(text); i++)
    {
    	if (text[i] == 0x0e)
    	{
    		in_katakana = 1;
    		continue;
    	}
    	if (text[i] == 0x0f)
    	{
    		in_katakana = 0;
    		continue;
    	}
    	
    	if (in_katakana)
    	{
    		if (text[i] < 0x26 || text[i] > 0x5f)
    			continue;
    		sint8 c = g_katana_jisx0201_conv[text[i]-0x26];
    		w += katakana_12_font.glyphs[c - katakana_12_font.offset]->width + font->space;
    	}
    	else
    	{
    		if (text[i] - font->offset >= 0 && font->glyphs[text[i] - font->offset] != NULL)
    		{
    			w += font->glyphs[text[i] - font->offset]->width + font->space;
    		}
    		else
    			w += font->space;
    	}
    }

    return w - font->space;
}


//project_specific_console == true : you must provide an InitProjectSpecificConsole() (like SnemulDS does)
//project_specific_console == false : default console for GUI_printf
//see gui_console_connector.c (project specific implementation)
void	GUI_init(bool project_specific_console)
{
	if(project_specific_console == true){
		VRAM_SETUP(getProjectSpecificVRAMSetup());
		InitProjectSpecificConsole();
	}
	else{
		VRAM_SETUP(DEFAULT_CONSOLE_VRAMSETUP());
		InitDefaultConsole();
	}
	
	GUI.printfy = 0;
}

//Moves the TGDS Console between Top and Bottom screen and update the console location register 
void TGDSLCDSwap(bool disableTSCWhenTGDSConsoleTop, bool isDirectFramebuffer, bool SaveConsoleContext, u8 * currentVRAMContext){
	if(GUI.consoleAtTopScreen == true){
		GUI.consoleAtTopScreen = false;
		if(disableTSCWhenTGDSConsoleTop == true){
			setTouchScreenEnabled(true);	//Enable TSC
		}
	}
	else{
		GUI.consoleAtTopScreen = true;
		if(disableTSCWhenTGDSConsoleTop == true){
			setTouchScreenEnabled(false);	//Disable TSC
		}
	}
	if(isDirectFramebuffer == true){
		SWAP_LCDS();
	}
	else{
		if(SaveConsoleContext == true){
			swapTGDSConsoleBetweenPPUEngines(currentVRAMContext);	//Proper impl.
			
			//Saving Console Context: Detect if Console is at bottom or top, and move it top always
			if(GUI.consoleAtTopScreen == false){
				GUI.consoleAtTopScreen = true;
				SWAP_LCDS();
			}
		}
		else{
			restoreTGDSConsoleFromSwapEngines(currentVRAMContext);	//Proper impl.
		
			//Restoring Console Context: Detect if Console is at bottom or top, and move it top always
			if(GUI.consoleAtTopScreen == true){
				GUI.consoleAtTopScreen = false;
				SWAP_LCDS();
			}
		}
	}
}


static uint16 * savedDSFrameBuffer = NULL;

//based from video console settings at toolchaingenericds-keyboard-example
void swapTGDSConsoleBetweenPPUEngines(u8 * currentVRAMContext){
	//Only when default console is in use, then use CustomConsole as a current console render
	if(globalTGDSCustomConsole == false){
		//ConsoleInstance * DefaultSessionConsoleInst = (ConsoleInstance *)(&DefaultConsole);
		vramSetup * vramSetupDefault = (vramSetup *)&vramSetupDefaultConsole;
		
		vramSetup * vramSetupCustom = (vramSetup *)&vramSetupCustomConsole;
		memset((u8*)vramSetupCustom, 0, sizeof(vramSetup));
		
		ConsoleInstance * CustomSessionConsoleInst = (ConsoleInstance *)(&CustomConsole);
		memset(CustomSessionConsoleInst, 0, sizeof(sizeof(ConsoleInstance)));
		
		//Save Old Engine context: 0x06000000
		CustomSessionConsoleInst->ConsoleEngineStatus.ENGINE_DISPCNT = REG_DISPCNT;
		
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[0].BGNUM = 0;
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[0].REGBGCNT = REG_BGXCNT(0);
		
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[1].BGNUM = 1;
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[1].REGBGCNT = REG_BGXCNT(1);
		
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[2].BGNUM = 2;
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[2].REGBGCNT = REG_BGXCNT(2);
		
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[3].BGNUM = 3;
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[3].REGBGCNT = REG_BGXCNT(3);
		
		coherent_user_range_by_size((uint32)0x06000000, 128*1024);
		coherent_user_range_by_size((uint32)currentVRAMContext, 128*1024);
		dmaTransferHalfWord(0, (uint32)0x06000000, (uint32)currentVRAMContext, (uint32)(128*1024));
		
		//Perform Console swap
		memcpy ((uint8*)vramSetupCustom, (uint8*)vramSetupDefault, sizeof(vramSetup));
		
		savedDSFrameBuffer = GUI.DSFrameBuffer;
		GUI.DSFrameBuffer = (uint16 *)BG_BMP_RAM(4);	//0x06000000
		
		vramSetupCustom->vramBankSetupInst[VRAM_A_INDEX].vrambankCR = VRAM_A_0x06000000_ENGINE_A_BG;	//console here
		vramSetupCustom->vramBankSetupInst[VRAM_A_INDEX].enabled = true;
		
		vramSetupCustom->vramBankSetupInst[VRAM_C_INDEX].vrambankCR = VRAM_C_0x06200000_ENGINE_B_BG;	//keyboard
		vramSetupCustom->vramBankSetupInst[VRAM_C_INDEX].enabled = true;
		
		//Set mainEngine
		SetEngineConsole(mainEngine, CustomSessionConsoleInst);
		
		//Set mainEngine properties
		CustomSessionConsoleInst->ConsoleEngineStatus.ENGINE_DISPCNT	=	(uint32)(MODE_5_2D | DISPLAY_BG3_ACTIVE );
		
		// BG3: FrameBuffer : 64(TILE:4) - 128 Kb
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[3].BGNUM = 3;
		CustomSessionConsoleInst->ConsoleEngineStatus.EngineBGS[3].REGBGCNT = BG_BMP_BASE(4) | BG_BMP8_256x256 | BG_PRIORITY_1;
		
		bool mainEngine = true;
		setOrientation(ORIENTATION_0, mainEngine);
		
		u16 * newPallete = &GUI.Palette[0];
		BG_PALETTE[0] = 	newPallete[0];			//Back-ground tile color / Black
		BG_PALETTE[1] =		newPallete[1]; 	//White
		BG_PALETTE[2] =  	newPallete[2]; 		//Brown
		BG_PALETTE[3] =  	newPallete[3]; 		//Orange
		BG_PALETTE[4] = 	newPallete[4]; 		//Magenta
		BG_PALETTE[5] = 	newPallete[5]; 		//Cyan
		BG_PALETTE[6] = 	newPallete[6]; 		//Yellow
		BG_PALETTE[7] = 	newPallete[7]; 		//Blue
		BG_PALETTE[8] = 	newPallete[8]; 		//Green
		BG_PALETTE[9] = 	newPallete[9]; 		//Red
		BG_PALETTE[0xa] = 	newPallete[10]; 	//Grey
		BG_PALETTE[0xb] = 	newPallete[11];	//Light-Grey
		
		//Fill the Pallette
		int i = 0;
		for(i=0;i < (256 - 0xb); i++){
			BG_PALETTE[i + 0xc] = newPallete[TGDSPrintfColor_White];
		}
		
		//Change VRAM before working with it
		VRAM_SETUP(vramSetupCustom);
		
		//copy Console
		coherent_user_range_by_size((uint32)0x06200000, 128*1024);
		dmaTransferHalfWord(0, (uint32)0x06200000, (uint32)0x06000000,(uint32)(128*1024));
		
		//clear old console
		dmaFillHalfWord(0, 0, (uint32)0x06200000, (uint32)(128*1024));
		
		UpdateConsoleSettings(CustomSessionConsoleInst);	//Console Top
		
		bool isDirectFramebuffer = true;
		bool disableTSCWhenTGDSConsoleTop = true;
		bool SaveConsoleContext = false;	//no effect because directFB == true
		u8 * FBSaveContext = NULL;			//no effect because directFB == true
		TGDSLCDSwap(disableTSCWhenTGDSConsoleTop, isDirectFramebuffer, SaveConsoleContext, FBSaveContext);
		
	}
	else{	//todo: same swap video logic, but using ConsoleInstance CustomConsole
		
	}	
}

void restoreTGDSConsoleFromSwapEngines(u8 * currentVRAMContext){

	//Only when default console is in use, restore DefaultConsole context
	if(globalTGDSCustomConsole == false){
		
		//ConsoleInstance * OldEngineContextInst = (ConsoleInstance *)(&CustomConsole);
		
		//Restore (new -> current) console
		coherent_user_range_by_size((uint32)0x06000000, 128*1024);
		dmaTransferHalfWord(0, (uint32)0x06000000, (uint32)0x06200000,(uint32)(128*1024));
		
		//Restore TGDS Console's current VRAM Layout
		InitDefaultConsole();
		GUI.DSFrameBuffer = savedDSFrameBuffer;
		
		initFBModeMainEngine0x06000000();
		coherent_user_range_by_size((uint32)currentVRAMContext, 128*1024);
		dmaTransferHalfWord(0, (uint32)currentVRAMContext, (uint32)0x06000000, (uint32)(128*1024));
		
	}
	else{	//todo: same swap video logic, but using ConsoleInstance CustomConsole
		
	}
}