#ifdef _MSC_VER

#ifndef __TGDSTypes_h__
#define __TGDSTypes_h__

#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <math.h>

#define TGDSARM9Malloc malloc
#define TGDSARM9Calloc calloc
#define TGDSARM9Realloc realloc
#define TGDSARM9Free free

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef s8 sint8;
typedef s16	sint16;
typedef s32	sint32;
typedef s64	sint64;

typedef s32	_ssize_t;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef u32	mode_t;

typedef u8	uint8;
typedef u16	uint16;
typedef u32	uint32;

typedef volatile uint8	vuint8;
typedef volatile uint16	vuint16;
typedef volatile uint32	vuint32;

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

#ifndef __cplusplus
typedef unsigned char bool;
static bool false = 0;
static bool true = 1;
#endif

#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))
#define RGB5(r,g,b)  ((r)|((g)<<5)|((b)<<10))
#define RGB8(r,g,b)  (((r)>>3)|(((g)>>3)<<5)|(((b)>>3)<<10))
#define ARGB16(a, r, g, b) ( ((a) << 15) | (r)|((g)<<5)|((b)<<10))

#endif

#endif
