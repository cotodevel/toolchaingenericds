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

#ifndef __libUtilsShared_h__
#define __libUtilsShared_h__

#include "typedefsTGDS.h"
#include "dsregs.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void libUtilsFIFONotEmpty(u32 cmd1, u32 cmd2);
extern u8* TGDSARM7Malloc(int size);
extern u8 * TGDSARM7Calloc(int blockCount, int blockSize);
extern void TGDSARM7Free(void *ptr);
extern u8 * TGDSARM7Realloc(void *ptr, int size);
extern u32 TGDSARM7MallocFreeMemory();

#ifdef __cplusplus
}
#endif

