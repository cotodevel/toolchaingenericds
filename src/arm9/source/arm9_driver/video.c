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
#include "video.h"

//power
void SWAP_LCDS(){
	REG_POWERCNT ^= POWER_SWAP_LCDS;
}

//VRAM 
//TODO: create default setups for every DS BG Video mode. 
//I just offer a custom setup + Validate custom setup

vramSetup vramSetupGlobal[1];	//just 1 global vramsetup

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

//project specific
vramSetup * SNEMULDS_2DVRAM_SETUP(){
	
	vramSetup * vramSetupDefault = (vramSetup *)&vramSetupGlobal[0];
	
	//vramSetBankA(VRAM_A_MAIN_BG_0x06020000);
	vramSetupDefault->vramBankSetupInst[VRAM_A_INDEX].vrambankCR = VRAM_A_0x06020000_ENGINE_A_BG;
	vramSetupDefault->vramBankSetupInst[VRAM_A_INDEX].enabled = true;
	
	//vramSetBankB(VRAM_B_MAIN_BG_0x06040000);
	vramSetupDefault->vramBankSetupInst[VRAM_B_INDEX].vrambankCR = VRAM_B_0x06040000_ENGINE_A_BG;
	vramSetupDefault->vramBankSetupInst[VRAM_B_INDEX].enabled = true;
	
	// 128Ko (+48kb) for sub screen / GUI 
	//vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetupDefault->vramBankSetupInst[VRAM_C_INDEX].vrambankCR = VRAM_C_0x06200000_ENGINE_B_BG;
	vramSetupDefault->vramBankSetupInst[VRAM_C_INDEX].enabled = true;
	
	// Some memory for ARM7 (128 Ko!)
	//vramSetBankD(VRAM_D_ARM7_0x06000000);
	vramSetupDefault->vramBankSetupInst[VRAM_D_INDEX].vrambankCR = VRAM_D_0x06000000_ARM7;
	vramSetupDefault->vramBankSetupInst[VRAM_D_INDEX].enabled = true;
	
	// 80Ko for Sprites (SNES : 32-64Ko)
	//vramSetBankE(VRAM_E_MAIN_SPRITE); // 0x6400000
	vramSetupDefault->vramBankSetupInst[VRAM_E_INDEX].vrambankCR = VRAM_E_0x06400000_ENGINE_A_BG;
	vramSetupDefault->vramBankSetupInst[VRAM_E_INDEX].enabled = true;
	
	//vramSetBankF(VRAM_F_MAIN_SPRITE);
	vramSetupDefault->vramBankSetupInst[VRAM_F_INDEX].vrambankCR = VRAM_F_0x064XXXXX_ENGINE_A_BG;
	vramSetupDefault->vramBankSetupInst[VRAM_F_INDEX].enabled = true;
	
	//vramSetBankG(VRAM_G_BG_EXT_PALETTE);
	vramSetupDefault->vramBankSetupInst[VRAM_G_INDEX].vrambankCR = VRAM_G_SLOT_ENGINE_A_BG_EXTENDED;
	vramSetupDefault->vramBankSetupInst[VRAM_G_INDEX].enabled = true;
	
	// 48ko For CPU 
	//vramSetBankH(VRAM_H_LCD);
	vramSetupDefault->vramBankSetupInst[VRAM_H_INDEX].vrambankCR = VRAM_H_LCDC_MODE;
	vramSetupDefault->vramBankSetupInst[VRAM_H_INDEX].enabled = true;
	
	//vramSetBankI(VRAM_I_LCD);
	vramSetupDefault->vramBankSetupInst[VRAM_I_INDEX].vrambankCR = VRAM_I_LCDC_MODE;
	vramSetupDefault->vramBankSetupInst[VRAM_I_INDEX].enabled = true;
	
	
	return vramSetupDefault;
}