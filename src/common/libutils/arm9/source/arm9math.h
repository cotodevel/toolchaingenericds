//////////////////////////////////////////////////////////////////////
//
// math.h -- provides fixed point math functions as well as acces to hardware 
//				divide and square root.
//
// version 0.1, February 17, 2005
//
//  Copyright (C) 2005 Michael Noland (joat) and Jason Rogers (dovoto)
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any
//  purpose, including commercial applications, and to alter it and
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source
//     distribution.
//
// Changelog:
//   0.1: First version
//
//////////////////////////////////////////////////////////////////////

#ifndef MATH_ARM9_INCLUDE
#define MATH_ARM9_INCLUDE

#include "trig_lut.h"

#if defined(_MSC_VER) && defined(ARM9) //ARM9 through NDS DL VS2012
#include "TGDSTypes.h"
#endif

#if defined(_MSC_VER) && !defined(ARM9) //NDS DL VS2012?
#include "TGDSTypes.h"
#endif

typedef int f32;             // 1.19.12 fixed point for matricies

typedef sint64                   int64;
typedef sint32                   int32;
typedef sint16                   int16;
typedef sint8                   int8;

typedef sint32                   fixed;
typedef sint64                   dfixed;
typedef volatile sint32          vfixed;

typedef volatile sint8                   vint8;
typedef volatile sint16                   vint16;
typedef volatile sint32                   vint32;
typedef volatile sint64                   vint64;

/////////////////////////////////////////////////////////////
//  Math coprocessor register definitions

#define DIV_CR				(*(vuint16*)(0x04000280))
#define DIV_NUMERATOR64		(*(vint64*) (0x04000290))
#define DIV_NUMERATOR32		(*(vint32*) (0x04000290))
#define DIV_DENOMINATOR64	(*(vint64*) (0x04000298))
#define DIV_DENOMINATOR32	(*(vint32*) (0x04000298))
#define DIV_RESULT64		(*(vint64*) (0x040002A0))
#define DIV_RESULT32		(*(vint32*) (0x040002A0))
#define DIV_REMANDER64		(*(vint64*) (0x040002A8))
#define DIV_REMANDER32		(*(vint32*) (0x040002A8))

#define SQRT_CR				(*(vuint16*)(0x040002B0))
#define SQRT_PARAM64		(*(vint64*) (0x040002B8))
#define SQRT_RESULT32		(*(vint32*) (0x040002B4))
#define SQRT_PARAM32		(*(vint32*) (0x040002B8))

///////////////////////////////////////////////////////////////
//  Math coprocessor modes

#define DIV_64_64			2
#define DIV_64_32			1
#define DIV_32_32			0
#define DIV_BUSY			(1<<15)

#define SQRT_64				1
#define SQRT_32				0
#define SQRT_BUSY			(1<<15)

///////////////////////////////////////////////////////////////
//  Fixed Point versions

///////////////////////////////////////
//  Fixed point divide
//  Takes 1.19.12 numerator and denominator
//  and returns 1.19.12 result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static f32 divf32(f32 num, f32 den)
{
	#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
	DIV_CR = DIV_64_32;
	
	while(DIV_CR & DIV_BUSY);

	DIV_NUMERATOR64 = ((int64)num) << 12;
	DIV_DENOMINATOR32 = den;

	while(DIV_CR & DIV_BUSY);
	return (DIV_RESULT32);
	#endif
	
	#ifdef WIN32
	return ((num << 8) / den);
	#endif
}

///////////////////////////////////////
//  Fixed point multiply
//	Takes 1.19.12 values and returns
//	1.19.12 result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static f32 mulf32(f32 a, f32 b)
{
	long long result = (long long)a*(long long)b;
	return (f32)(result >> 12);
}

///////////////////////////////////////
//  Fixed point square root
//	Takes 1.19.12 fixed point value and
//	returns the fixed point result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static f32 sqrtf32(f32 a)
{
	#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
	SQRT_CR = SQRT_64;

	while(SQRT_CR & SQRT_BUSY);
	
	SQRT_PARAM64 = ((int64)a) << 12;
	
	while(SQRT_CR & SQRT_BUSY);
	
	return SQRT_RESULT32;
	#endif
	
	#ifdef WIN32
	u32 op  = a;
    u32 res = 0;
    u32 one = 0;
	one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
	#endif
}

///////////////////////////////////////////////////////////////
//  Integer versions

///////////////////////////////////////
//  Integer divide
//  Takes a 32 bit numerator and 32 bit
//	denominator and returns 32 bit result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static int32 div32(int32 num, int32 den)
{
	#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
	DIV_CR = DIV_32_32;
	
	while(DIV_CR & DIV_BUSY);

	DIV_NUMERATOR32 = num;
	DIV_DENOMINATOR32 = den;

	while(DIV_CR & DIV_BUSY);

	return (DIV_RESULT32);
	#endif

	#ifdef WIN32
	return ((int)((int)num/(int)den));
	#endif
}

///////////////////////////////////////
//  Interger divide
//  Takes a 32 bit numerator and 32 bit
//	denominator and returns 32 bit result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static int32 mod32(int32 num, int32 den)
{
	#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
	DIV_CR = DIV_32_32;
	
	while(DIV_CR & DIV_BUSY);

	DIV_NUMERATOR32 = num;
	DIV_DENOMINATOR32 = den;

	while(DIV_CR & DIV_BUSY);

	return (DIV_REMANDER32);
	#endif

	#ifdef WIN32
	return ((int)((int)num%(int)den));
	#endif
}

///////////////////////////////////////
//  Integer divide
//	Takes a 64 bit numerator and 32 bit
//  denominator are returns 32 bit result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static int32 div64(int64 num, int32 den)
{
	#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
	DIV_CR = DIV_32_32;
	
	while(DIV_CR & DIV_BUSY);

	DIV_NUMERATOR64 = num;
	DIV_DENOMINATOR32 = den;

	while(DIV_CR & DIV_BUSY);

	return (DIV_RESULT32);
	#endif

	#ifdef WIN32
	return ((int32)((long int)num/(int)den));
	#endif
}

///////////////////////////////////////
//  Integer divide
//	Takes a 64 bit numerator and 32 bit
//  denominator are returns 32 bit result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static int32 mod64(int64 num, int32 den)
{
	#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
	DIV_CR = DIV_32_32;
	
	while(DIV_CR & DIV_BUSY);

	DIV_NUMERATOR64 = num;
	DIV_DENOMINATOR32 = den;

	while(DIV_CR & DIV_BUSY);

	return (DIV_REMANDER32);
	#endif

	#ifdef WIN32
	return ((int32)((long int)num%(int)den));
	#endif
}
///////////////////////////////////////
//  Integer square root
//  takes a 32 bit integer and returns 
//	32 bit result
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static int32 sqrt32(int a)
{
	#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
	SQRT_CR = SQRT_32;

	while(SQRT_CR & SQRT_BUSY);
	
	SQRT_PARAM32 = a;
	
	while(SQRT_CR & SQRT_BUSY);
	
	return SQRT_RESULT32;
	#endif
	
	#ifdef WIN32
	return ((int32)sqrt(a));
	#endif
}

///////////////////////////////////////////////////////////////
//  Trig Functions  1.19.12 fixed point

///////////////////////////////////////
// Cross product
// x = Ay * Bz - By * Az
// y = Az * Bx - Bz * Ax
// z = Ax * By - Bx * Ay
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static void crossf32(f32 *a, f32 *b, f32 *result)
{
	result[0] = mulf32(a[1], b[2]) - mulf32(b[1], a[2]);
	result[1] = mulf32(a[2], b[0]) - mulf32(b[2], a[0]);
	result[2] = mulf32(a[0], b[1]) - mulf32(b[0], a[1]);
}

///////////////////////////////////////
// Dot Product
// result = Ax * Bx + Ay * By + Az * Bz
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static f32 dotf32(f32 *a, f32 *b)
{
	return mulf32(a[0], b[0]) + mulf32(a[1], b[1]) + mulf32(a[2], b[2]);
}

///////////////////////////////////////
// Normalize 
// Ax = Ax / mag
// Ay = Ay / mag
// Az = Az / mag
#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
inline 
#endif
static void normalizef32(f32* a)
{
	// magnitude = sqrt ( Ax^2 + Ay^2 + Az^2 )
	f32 magnitude = sqrtf32( mulf32(a[0], a[0]) + mulf32(a[1], a[1]) + mulf32(a[2], a[2]) );
	
	a[0] = divf32(a[0], magnitude);
	a[1] = divf32(a[1], magnitude);
	a[2] = divf32(a[2], magnitude);
}
#endif

