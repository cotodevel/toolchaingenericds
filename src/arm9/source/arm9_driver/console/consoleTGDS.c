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

t_GUI GUI;
volatile sint8	g_printfbuf[consolebuf_size];
ConsoleInstance DefaultConsole = {0};		//generic console
ConsoleInstance CustomConsole = {0};		//project specific console
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
int GUI_drawVChar(t_GUIZone *zone, t_GUIFont *font, uint16 x, uint16 y, int col, uint8 text)
{
	if ((int)text - font->offset < 0 || font->glyphs[text - font->offset] == NULL)
		return 0;
	
    t_GUIGlyph	*glyph = font->glyphs[text - font->offset];

    int	gw = glyph->width + 1;
    
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
            uint16 v = *ptr;

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
int GUI_drawText(t_GUIZone *zone, uint16 x, uint16 y, int col, sint8 *text)
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
    		w += GUI_drawVChar(zone, &katakana_12_font, x+w, y, col, c) + font->space;
    	}
    	else
    		w += GUI_drawVChar(zone, font, x+w, y, col, text[i]) + font->space;
    }
    return w - font->space;
}


//used by gui_printf
int GUI_getFontHeight(t_GUIZone *zone)
{
	return zone->font->height+1;
}

void		GUI_printf(sint8 *fmt, ...)
{	
	va_list args;
	va_start (args, fmt);
	vsnprintf ((sint8*)g_printfbuf, 64, fmt, args);
	va_end (args);
	
    // FIXME
    t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &trebuchet_9_font;
    GUI_drawText(&zone, 0, GUI.printfy, 255, (sint8*)g_printfbuf);
    GUI.printfy += GUI_getFontHeight(&zone);
}

//used by gui_printf
void	GUI_clear()
{
	//flush buffers
	memset ((uint32 *)&g_printfbuf[0], 0, sizeof(g_printfbuf));
	consoleClear(DefaultSessionConsole);
	GUI.printfy = 0;
}

void clrscr(){
	GUI_clear();
}

//project_specific_console == true : you must provide an InitProjectSpecificConsole()
//project_specific_console == false : default console for printf
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


void GUI_setLanguage(int lang)
{
}
