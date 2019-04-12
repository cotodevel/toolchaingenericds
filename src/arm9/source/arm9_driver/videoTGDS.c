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

vramSetup vramSetupGlobal[1] = {0};	//just 1 global vramsetup

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
	vramSetup * vramSetupDefault = (vramSetup *)&vramSetupGlobal[0];
	
	vramSetupDefault->vramBankSetupInst[VRAM_C_INDEX].vrambankCR = VRAM_C_0x06200000_ENGINE_B_BG;
	vramSetupDefault->vramBankSetupInst[VRAM_C_INDEX].enabled = true;
	
	// Some memory for ARM7 (128 Ko!)
	//vramSetBankD(VRAM_D_ARM7_0x06000000);
	vramSetupDefault->vramBankSetupInst[VRAM_D_INDEX].vrambankCR = VRAM_D_0x06000000_ARM7;
	vramSetupDefault->vramBankSetupInst[VRAM_D_INDEX].enabled = true;
	
	return vramSetupDefault;
}

//2) Uses subEngine: VRAM Layout -> Console Setup
bool InitDefaultConsole(){
	DefaultSessionConsole = (ConsoleInstance *)(&DefaultConsole);
	
	//Set subEngine
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
	
	BG_PALETTE_SUB[0] = RGB15(0,0,0);			//back-ground tile color
	BG_PALETTE_SUB[255] = RGB15(31,31,31);		//tile color
	
	InitializeConsole(DefaultSessionConsole);
	
	return true;
}