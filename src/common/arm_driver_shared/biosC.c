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

//Software BIOS functionality replacement for official NDS Bios. C part.

/////////////////////////////////////////////////// Shared BIOS ARM7/9 /////////////////////////////////////////////////////////////////
#include "bios.h"
#include "dma.h"

///////////////////////////////////////////////// swiCRC16 /////////////////////////////////////////////////////////////////////////////
unsigned char mess[12] ={0x15, 0x47, 0xC3, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF}; // dont touch this array, its used by the CRC16

//crc == initial seed 
unsigned short crc16(const unsigned char* data_p, unsigned char length,unsigned short crc){
    unsigned char x;
    //unsigned short crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}

//NDS BIOS Routines.
//SWI 0Eh (NDS7/NDS9) - GetCRC16
//  r0  Initial CRC value (16bit, usually FFFFh)
//  r1  Start Address   (must be aligned by 2)
//  r2  Length in bytes (must be aligned by 2)

//Return:
//  r0  Calculated 16bit CRC Value
/*
uint16 swiCRC16(uint16 crcvalue, uint8 * data,sint32 len){
	return (uint16)crc16((const unsigned char*)data, (unsigned char)len,(unsigned short)crcvalue);
}
*/
///////////////////////////////////////////////// swiCRC16 end //////////////////////////////////////////////////////////////////////////






/////////////////////////////////////////////////////////////// SwiDelay /////////////////////////////////////////////////////////////
//asm
/////////////////////////////////////////////////////////////// SwiDelay end /////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////// swiFastCopy /////////////////////////////////////////////////////////////

//problem kaputt docs say DS uses a rounded 4 byte copy, be it a fillvalue to dest or direct copy from src to dest, by size.
//Dont optimize as vram is 16 or 32bit, optimization can end up in 8bit writes.
//writes either a COPY_MODE_FILL value = [r0], or plain copy from source to destination
__attribute__((optimize("O0")))
void swiFastCopy(uint32 * source, uint32 * dest, int flags){
	int i = 0;
	if(flags & COPY_FIXED_SOURCE){
		uint32 value = *(uint32*)source;
		for(i = 0; i < (int)((flags<<2)&0x1fffff)/4; i++){
			dest[i] = value;
		}
	}
	else{
		for(i = 0; i < (int)((flags<<2)&0x1fffff)/4; i++){
			dest[i] = source[i];
		}
	}
}

/////////////////////////////////////////////////////////////// swiFastCopy end //////////////////////////////////////////////////////////

//dmaFillWords todo:




/////////////////////////////////////////////////// Shared BIOS ARM7/9 end //////////////////////////////////////////////////////////////















