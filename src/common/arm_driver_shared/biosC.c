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

//Software BIOS modules by either replacement or native BIOS support.

/////////////////////////////////////////////////// Shared BIOS ARM7/9 /////////////////////////////////////////////////////////////////
#include "biosTGDS.h"
#include "dmaTGDS.h"

//NDS BIOS Routines.



//extern extern void swiDelay(uint32 delayvalue);


//SWI 0Eh (NDS7/NDS9) - GetCRC16
//  r0  Initial CRC value (16bit, usually FFFFh)
//  r1  Start Address   (must be aligned by 2)
//  r2  Length in bytes (must be aligned by 2)

//Return:
//  r0  Calculated 16bit CRC Value

//extern uint16 swiCRC16(uint16 crcvalue, uint8 * data,sint32 len);




//problem kaputt docs say DS uses a rounded 4 byte copy, be it a fillvalue to dest or direct copy from src to dest, by size.
//Dont optimize as vram is 16 or 32bit, optimization can end up in 8bit writes.
//writes either a COPY_MODE_FILL value = [r0], or plain copy from source to destination
void swiFastCopy(uint32 * source, uint32 * dest, int flags){
	if(flags & COPY_FIXED_SOURCE){
		dmaFillWord(3, (uint32)(*(uint32*)source),(uint32)dest, (uint32)(((flags<<2)&0x1fffff)));
	}
	else{
		dmaTransferWord(3, (uint32)source, (uint32)dest, (uint32) (((flags<<2)&0x1fffff)) );
	}
}




//dmaFillWords todo:



//extern void swiChangeSndBias(int enable, int delayvalue);