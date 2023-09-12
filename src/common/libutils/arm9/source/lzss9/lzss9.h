/*----------------------------------------------------------------------------*/
/*--  lzss.c - LZSS coding for Nintendo GBA/DS                              --*/
/*--  Copyright (C) 2011 CUE                                                --*/
/*--                                                                        --*/
/*--  This program is free software: you can redistribute it and/or modify  --*/
/*--  it under the terms of the GNU General Public License as published by  --*/
/*--  the Free Software Foundation, either version 3 of the License, or     --*/
/*--  (at your option) any later version.                                   --*/
/*--                                                                        --*/
/*--  This program is distributed in the hope that it will be useful,       --*/
/*--  but WITHOUT ANY WARRANTY; without even the implied warranty of        --*/
/*--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          --*/
/*--  GNU General Public License for more details.                          --*/
/*--                                                                        --*/
/*--  You should have received a copy of the GNU General Public License     --*/
/*--  along with this program. If not, see <http://www.gnu.org/licenses/>.  --*/
/*----------------------------------------------------------------------------*/


#ifndef __lzss9_h__
#define __lzss9_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "biosTGDS.h"
#include "utilsTGDS.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BREAK(text) { nocashMessage(text); return; }
#define EXIT(text)  { nocashMessage(text); while(1==1); }

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void  Title(void);
extern void  Usage(void);

extern char *Load(char *filenameIn, int *length, int min, int max);
extern void  Save(char *filenameOut, char *buffer, int length);
extern char *Memory(int length, int size);

extern bool LZS_Decode(const char *filenameIn, const char *filenameOut);
extern void LZS_Encode(const char *filenameIn, const char *filenameOut);
extern char *LZS_Code(unsigned char *raw_buffer, int raw_len, int *new_len, int best);

extern char *LZS_Fast(unsigned char *raw_buffer, int raw_len, int *new_len);
extern void  LZS_InitTree(void);
extern void  LZS_InsertNode(int r);
extern void  LZS_DeleteNode(int p);

extern unsigned char ring[LZS_N + LZS_F - 1];
extern int           dad[LZS_N + 1], lson[LZS_N + 1], rson[LZS_N + 1 + 256];
extern int           pos_ring, len_ring, lzs_vram;

#ifdef __cplusplus
}
#endif
