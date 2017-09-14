
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

typedef	unsigned	int		uint32;	//reenable when using this lib
typedef	unsigned	short	uint16;
typedef	uchar				uint8;

typedef	int					sint32;
typedef volatile sint32		vsint32;
typedef	short				sint16;
typedef volatile sint16		vsint16;
typedef	char				sint8;
typedef volatile sint8		vsint8;
typedef	char				bool8;

typedef int64_t 			uint64;

typedef volatile uint32		vuint32;
typedef volatile uint16		vuint16;
typedef volatile uint8		vuint8;


//deprecated: let stdbool.h handle these defs because toolchain itself provides these variables.
/*
#if (TRUE != 1)
#undef TRUE
#define TRUE    1
#endif

#if (FALSE != 0)
#undef FALSE
#define FALSE   0
#endif
*/

//snemul specific end
//misc (libraries)
//typedef	uint32				u32;		//DSWIFI compatibility
typedef	uchar				u8;		
typedef	uint16				u16;		
typedef	sint16				int16;
//typedef uchar 				bool;
typedef volatile uint32		vu32;
typedef volatile uint16		vu16;
typedef volatile uchar		vu8;



#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
}
#endif
