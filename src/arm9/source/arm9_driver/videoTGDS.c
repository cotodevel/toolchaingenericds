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


//Generic console (uses VRAM block C,VRAM block D for ARM7), SUB Engine.

