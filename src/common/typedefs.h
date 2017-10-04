
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


//this toolchain will always use typedefs.h as source for typedefs across codebase.

#ifndef __typedefs_common_
#define __typedefs_common_

#include <stdint.h>
#include <stdbool.h>

typedef	unsigned	short	ushort;
typedef	unsigned	char	uchar;

typedef	long long			sint64;
typedef	int					sint32;
typedef	short				sint16;
typedef	char				sint8;
typedef	sint8				bool8;

typedef volatile sint64		vsint64;
typedef volatile sint32		vsint32;
typedef volatile sint16		vsint16;
typedef volatile sint8		vsint8;

typedef unsigned 	long long uint64;
typedef	unsigned	int		uint32;
typedef	unsigned	short	uint16;
typedef	unsigned	char	uint8;

typedef volatile uint64		vuint64;
typedef volatile uint32		vuint32;
typedef volatile uint16		vuint16;
typedef volatile uint8		vuint8;

#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
}
#endif