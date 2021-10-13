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
#include "videoTGDS.h"
#include "consoleTGDS.h"
#include "dmaTGDS.h"
#include "posixHandleTGDS.h"

//power
void SWAP_LCDS(){
	REG_POWERCNT ^= POWER_SWAP_LCDS;
}

void SET_MAIN_TOP_LCD() { 
	REG_POWERCNT |= POWER_SWAP_LCDS; 
}

void SET_MAIN_BOTTOM_LCD() { 
	REG_POWERCNT &= ~POWER_SWAP_LCDS; 
}

//VRAM 
//see console.c for setting up a default console, or use a project specific console

vramSetup vramSetupDefaultConsole;
vramSetup vramSetupCustomConsole;

void SETDISPCNT_MAIN(uint32 mode)  
{ 
	REG_DISPCNT = mode; 
}

void SETDISPCNT_SUB(uint32 mode)  
{ 
	REG_DISPCNT_SUB = mode;
}

void VRAMBLOCK_SETBANK_A(uint8 vrambits)  
{
	VRAM_A_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_B(uint8 vrambits)  
{
	VRAM_B_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_C(uint8 vrambits)  
{
	VRAM_C_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_D(uint8 vrambits)  
{
	VRAM_D_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_E(uint8 vrambits)  
{
	VRAM_E_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_F(uint8 vrambits)  
{
	VRAM_F_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_G(uint8 vrambits)  
{
	VRAM_G_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_H(uint8 vrambits)  
{
	VRAM_H_CR = vrambits | VRAM_ENABLE;
}

void VRAMBLOCK_SETBANK_I(uint8 vrambits)  
{
	VRAM_I_CR = vrambits | VRAM_ENABLE;
}

void ENABLE_BG_MAIN(int bg) {
	REG_DISPCNT |= (1 << (8 + bg));
}

void ENABLE_BG_SUB(int bg) {
	REG_DISPCNT_SUB |= (1 << (8 + bg));
}

void DISABLE_BG_MAIN(int bg) {
	REG_DISPCNT &= ~(1 << (8 + bg));
}

void DISABLE_BG_SUB(int bg) {
	REG_DISPCNT_SUB &= ~(1 << (8 + bg));
}

bool VRAM_SETUP(vramSetup * vramSetupInst){
		
		if(vramSetupInst->vramBankSetupInst[VRAM_A_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_A(vramSetupInst->vramBankSetupInst[VRAM_A_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_B_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_B(vramSetupInst->vramBankSetupInst[VRAM_B_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_C_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_C(vramSetupInst->vramBankSetupInst[VRAM_C_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_D_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_D(vramSetupInst->vramBankSetupInst[VRAM_D_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_E_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_E(vramSetupInst->vramBankSetupInst[VRAM_E_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_F_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_F(vramSetupInst->vramBankSetupInst[VRAM_F_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_G_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_G(vramSetupInst->vramBankSetupInst[VRAM_G_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_H_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_H(vramSetupInst->vramBankSetupInst[VRAM_H_INDEX].vrambankCR);
		}
		if(vramSetupInst->vramBankSetupInst[VRAM_I_INDEX].enabled == true){
			VRAMBLOCK_SETBANK_I(vramSetupInst->vramBankSetupInst[VRAM_I_INDEX].vrambankCR);
		}
		
		return true; 
}

//Generic console (uses VRAM block C,VRAM block D for ARM7), SUB Engine.

//1) VRAM Layout
vramSetup * DEFAULT_CONSOLE_VRAMSETUP(){
	vramSetup * vramSetupDefault = (vramSetup *)&vramSetupDefaultConsole;
	memset((u8*)vramSetupDefault, 0, sizeof(vramSetup));
	
	//VRAM A and B reserved for 2D Textures to-be used with the 3D Engine
	vramSetupDefault->vramBankSetupInst[VRAM_A_INDEX].vrambankCR = VRAM_A_LCDC_MODE;	//6800000h-681FFFFh
	vramSetupDefault->vramBankSetupInst[VRAM_A_INDEX].enabled = true;																		
	vramSetupDefault->vramBankSetupInst[VRAM_B_INDEX].vrambankCR = VRAM_B_LCDC_MODE;	//6820000h-683FFFFh
	vramSetupDefault->vramBankSetupInst[VRAM_B_INDEX].enabled = true;
	
	//VRAM C: WoopsiTGDS Touchscreen UI
	vramSetupDefault->vramBankSetupInst[VRAM_C_INDEX].vrambankCR = VRAM_C_0x06200000_ENGINE_B_BG;
	vramSetupDefault->vramBankSetupInst[VRAM_C_INDEX].enabled = true;
	
	//VRAM D: ARM7 (128 Ko!)
	vramSetupDefault->vramBankSetupInst[VRAM_D_INDEX].vrambankCR = VRAM_D_0x06000000_ARM7;
	vramSetupDefault->vramBankSetupInst[VRAM_D_INDEX].enabled = true;
	
	//144K free decompressor mem
	//E       64K   0    -     6880000h-688FFFFh
	//F       16K   0    -     6890000h-6893FFFh
	//G       16K   0    -     6894000h-6897FFFh
	//H       32K   0    -     6898000h-689FFFFh
	//I       16K   0    -     68A0000h-68A3FFFh
  
	//VRAM E,F,G,H,I: Unused and reserved
	vramSetupDefault->vramBankSetupInst[VRAM_E_INDEX].vrambankCR = VRAM_E_LCDC_MODE;
	vramSetupDefault->vramBankSetupInst[VRAM_E_INDEX].enabled = true;
	
	vramSetupDefault->vramBankSetupInst[VRAM_F_INDEX].vrambankCR = VRAM_F_LCDC_MODE;
	vramSetupDefault->vramBankSetupInst[VRAM_F_INDEX].enabled = true;
	
	vramSetupDefault->vramBankSetupInst[VRAM_G_INDEX].vrambankCR = VRAM_G_LCDC_MODE;
	vramSetupDefault->vramBankSetupInst[VRAM_G_INDEX].enabled = true;
	
	vramSetupDefault->vramBankSetupInst[VRAM_H_INDEX].vrambankCR = VRAM_H_LCDC_MODE;
	vramSetupDefault->vramBankSetupInst[VRAM_H_INDEX].enabled = true;
	
	vramSetupDefault->vramBankSetupInst[VRAM_I_INDEX].vrambankCR = VRAM_I_LCDC_MODE;
	vramSetupDefault->vramBankSetupInst[VRAM_I_INDEX].enabled = true;
	
	return vramSetupDefault;
}

//2) Uses subEngine: VRAM Layout -> Console Setup
bool InitDefaultConsole(){
	DefaultSessionConsole = (ConsoleInstance *)(&DefaultConsole);
	
	//Set subEngine as TGDS Console
	GUI.consoleAtTopScreen = false;
	SetEngineConsole(subEngine,DefaultSessionConsole);
	
	//Set subEngine properties
	DefaultSessionConsole->ConsoleEngineStatus.ENGINE_DISPCNT	=	(uint32)(MODE_5_2D | DISPLAY_BG3_ACTIVE );
	
	// BG3: FrameBuffer : 64(TILE:4) - 128 Kb
	DefaultSessionConsole->ConsoleEngineStatus.EngineBGS[3].BGNUM = 3;
	DefaultSessionConsole->ConsoleEngineStatus.EngineBGS[3].REGBGCNT = BG_BMP_BASE(4) | BG_BMP8_256x256 | BG_PRIORITY_1;
	
	GUI.DSFrameBuffer = (uint16 *)BG_BMP_RAM_SUB(4);
	
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1 << 8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1 << 8;
	
	GUI.Palette = &BG_PALETTE_SUB[0];
	GUI.Palette[0] = 	RGB8(0,0,0);			//Back-ground tile color / Black
	GUI.Palette[1] =	RGB8(255, 255, 255); 	//White
	GUI.Palette[2] =  	RGB8(150, 75, 0); 		//Brown
	GUI.Palette[3] =  	RGB8(255, 127, 0); 		//Orange
	GUI.Palette[4] = 	RGB8(255, 0, 255); 		//Magenta
	GUI.Palette[5] = 	RGB8(0, 255, 255); 		//Cyan
	GUI.Palette[6] = 	RGB8(255, 255, 0); 		//Yellow
	GUI.Palette[7] = 	RGB8(0, 0, 255); 		//Blue
	GUI.Palette[8] = 	RGB8(0, 255, 0); 		//Green
	GUI.Palette[9] = 	RGB8(255, 0, 0); 		//Red
	GUI.Palette[0xa] = 	RGB8(128, 128, 128); 	//Grey
	GUI.Palette[0xb] = 	RGB8(240, 240, 240);	//Light-Grey
	
	//Fill the Pallette
	int i = 0;
	for(i=0;i < (256 - 0xb); i++){
		GUI.Palette[i + 0xc] = GUI.Palette[TGDSPrintfColor_White];
	}
	
	InitializeConsole(DefaultSessionConsole);
	return true;
}

static u32 mainDISPCNTsaved = 0;
static sint16 mainREG_BG3PAsaved = 0;
static sint16 mainREG_BG3PDsaved = 0;
static sint16 mainREG_BG3CNTsaved = 0;
static uint8  mainVRAM_BANK_Asaved = 0;

//Enables the NDS BMP RGB 15bit format for Engine_a at 0x06000000
void initFBModeMainEngine0x06000000(){	
	mainVRAM_BANK_Asaved = VRAM_A_CR;
	VRAMBLOCK_SETBANK_A(VRAM_A_0x06000000_ENGINE_A_BG);
	
	mainDISPCNTsaved = REG_DISPCNT;
	SETDISPCNT_MAIN(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	
	// Don't scale bg3 (set its affine transformation matrix to [[1,0],[0,1]])
	mainREG_BG3PAsaved = REG_BG3PA;
	REG_BG3PA = 1 << 8;
	mainREG_BG3PDsaved = REG_BG3PD;
	REG_BG3PD= 1 << 8;
	
	#define BG_256x256       (1<<14)
	#define BG_15BITCOLOR    (1<<7)
	#define BG_CBB1          (1<<2)
	
	mainREG_BG3CNTsaved = REG_BG3CNT;
	REG_BG3CNT = BG_256x256 | BG_15BITCOLOR | BG_CBB1;	
}

void restoreFBModeMainEngine(){
	dmaFillHalfWord(3, 0, (uint32)0x06000000, (uint32)(128*1024));
	VRAM_A_CR = mainVRAM_BANK_Asaved;
	mainVRAM_BANK_Asaved = 0;
	
	SETDISPCNT_MAIN(mainDISPCNTsaved);
	mainDISPCNTsaved = 0;
	
	REG_BG3PA = mainREG_BG3PAsaved;
	mainREG_BG3PAsaved = 0;
	
	REG_BG3PD = mainREG_BG3PDsaved;
	mainREG_BG3PDsaved = 0;
	
	REG_BG3CNT = mainREG_BG3CNTsaved;
	mainREG_BG3CNTsaved = 0;
}

//Renders a buffer in NDS BMP RGB 15bit format. Note, the buffer must be 256x192
void renderFBMode3Engine(u16 * srcBuf, u16 * targetBuf, int srcWidth, int srcHeight){
	if((srcWidth != 256) || (srcHeight != 192)){
		return;
	}
	dmaTransferHalfWord(3, (uint32)srcBuf, (uint32)(targetBuf), (uint32)srcWidth*srcHeight*(sizeof(u16)));
}

//Screen Rotation registers
void setOrientation(int orientation, bool mainEngine){
	
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 
	switch(orientation){
		case ORIENTATION_0:{
			if(mainEngine == true){
				REG_BG3PA = 1 << 8;
				REG_BG3PB = 0;
				REG_BG3PC = 0;
				REG_BG3PD = 1 << 8;
				REG_BG3X = 0;
				REG_BG3Y = 0;
				TGDSIPC->screenOrientationMainEngine = orientation;
			}
			else{
				REG_BG3PA_SUB = 1 << 8;
				REG_BG3PB_SUB = 0;
				REG_BG3PC_SUB = 0;
				REG_BG3PD_SUB = 1 << 8;
				REG_BG3X_SUB = 0;
				REG_BG3Y_SUB = 0;
				TGDSIPC->screenOrientationSubEngine = orientation;
			}
		}	
		break;
		case ORIENTATION_90:{
			if(mainEngine == true){
				REG_BG3PA = 0;
				REG_BG3PB = ( (sint16)((-1u)<<8) + 1);
				REG_BG3PC = 1 << 8;
				REG_BG3PD = 0;
				REG_BG3X = 191 << 8;
				REG_BG3Y = 0;
				TGDSIPC->screenOrientationMainEngine = orientation;
			}
			else{
				REG_BG3PA_SUB = 0;
				REG_BG3PB_SUB = ( (sint16)((-1u)<<8) + 1);
				REG_BG3PC_SUB = 1 << 8;
				REG_BG3PD_SUB = 0;
				REG_BG3X_SUB = 191 << 8;
				REG_BG3Y_SUB = 0;
				TGDSIPC->screenOrientationSubEngine = orientation;
			}
		}
		break;
		case ORIENTATION_180:{
			if(mainEngine == true){
				REG_BG3PA = ( (sint16)((-1u)<<8) + 1);
				REG_BG3PB = 0;
				REG_BG3PC = 0;
				REG_BG3PD = ( (sint16)((-1u)<<8) + 1);
				REG_BG3X = 255 << 8;
				REG_BG3Y = 191 << 8;
				TGDSIPC->screenOrientationMainEngine = orientation;
			}
			else{
				REG_BG3PA_SUB = ( (sint16)((-1u)<<8) + 1);
				REG_BG3PB_SUB = 0;
				REG_BG3PC_SUB = 0;
				REG_BG3PD_SUB = ( (sint16)((-1u)<<8) + 1);
				REG_BG3X_SUB = 255 << 8;
				REG_BG3Y_SUB = 191 << 8;
				TGDSIPC->screenOrientationSubEngine = orientation;
			}
		}
		break;
		case ORIENTATION_270:{
			if(mainEngine == true){
				REG_BG3PA = 0;
				REG_BG3PB = 1 << 8;
				REG_BG3PC = ( (sint16)((-1u)<<8) + 1);
				REG_BG3PD = 0;
				REG_BG3X = 0;
				REG_BG3Y = 255 << 8;
				TGDSIPC->screenOrientationMainEngine = orientation;
			}
			else{
				REG_BG3PA_SUB = 0;
				REG_BG3PB_SUB = 1 << 8;
				REG_BG3PC_SUB = ( (sint16)((-1u)<<8) + 1);
				REG_BG3PD_SUB = 0;
				REG_BG3X_SUB = 0;
				REG_BG3Y_SUB = 255 << 8;
				TGDSIPC->screenOrientationSubEngine = orientation;
			}
		}
		break;
	}
}