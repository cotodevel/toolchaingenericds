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

#ifndef __ndsDisplayListUtils_h__
#define __ndsDisplayListUtils_h__

//#define DIRECT_VS2012_NDSDL_EXEC 1 //enabled: VS2012 + NDSDLUtils env (no direct TGDSProject3D)
//#define DIRECT_TGDSPROJECT_WIN32 1 //enabled: TGDSProject3D on VS2012. disabled: TGDSProject3D on TGDS ARM9. 

#if defined(DIRECT_VS2012_NDSDL_EXEC) && defined(DIRECT_TGDSPROJECT_WIN32)
#error "can't choose both direct VS2012 NDSDL and TGDSProject3D environments at the same time, pick either one."
#endif

#if defined(DIRECT_VS2012_NDSDL_EXEC) || defined(DIRECT_TGDSPROJECT_WIN32)
#define TGDSPROJECT_WIN32 //enabled: when ndsDLutils is used in TGDSProject3D directly
#endif

//GX Payload source sample: PackedDisplayListCompiled.bin: 
//Packed Display List commands generated from VS2012 by running unit tests 1,2,3,4 and 5 
//(emited DL unpacked, then packed to a GX binary, whose dump ends up being this file)

#if defined(_MSC_VER)

#if defined(TGDSPROJECT_WIN32) && !defined(DIRECT_VS2012_NDSDL_EXEC)
#include <gl\GLUT.h>
#endif


#if defined(TGDSPROJECT_WIN32) && defined(DIRECT_VS2012_NDSDL_EXEC)
#include "videoGL.h"
#endif


#include "TGDSTypes.h"
#define testSourceFileLocation (char*)"\\cv\\PackedDisplayListCompiled.bin"
#endif

#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
#include "typedefsTGDS.h"
#include "videoGL.h"
//testSourceFileLocation is embedded into ARM9
#endif


//NDS GX C Display List defines
struct unpackedCmd {
	u8 cmd1;
	u8 cmd2;
	u8 cmd3;
	u8 cmd4;
};

#define CRC16_POLYNOMIAL 0xa001
#define CRC32_POLYNOMIAL 0xedb88320

//Inlined commands
#define getFIFO_NOP (u8)REG2IDADDR(GFX_FIFO_ADDR)
#define getFIFO_STATUS (u8)REG2IDADDR(GFX_STATUS_ADDR)
#define getFIFO_COLOR (u8)REG2IDADDR(GFX_COLOR_ADDR)
#define getFIFO_VERTEX16 (u8)REG2IDADDR(GFX_VERTEX16_ADDR)
#define getFIFO_VERTEX10 (u8)REG2IDADDR(GFX_VERTEX10_ADDR)
#define getFIFO_TEX_COORD (u8)REG2IDADDR(GFX_TEX_COORD_ADDR)
#define getFIFO_VTX_XY (u8)REG2IDADDR(GFX_VERTEX_XY_ADDR)
#define getFIFO_TEX_FORMAT (u8)REG2IDADDR(GFX_TEX_FORMAT_ADDR)
#define getFIFO_CLEAR_COLOR (u8)REG2IDADDR(GFX_CLEAR_COLOR_ADDR)
#define getFIFO_CLEAR_DEPTH (u8)REG2IDADDR(GFX_CLEAR_DEPTH_ADDR)
#define getFIFO_LIGHT_VECTOR (u8)REG2IDADDR(GFX_LIGHT_VECTOR_ADDR)
#define getFIFO_LIGHT_COLOR (u8)REG2IDADDR(GFX_LIGHT_COLOR_ADDR)
#define getMTX_MULT_3x3 (u8)REG2IDADDR(MATRIX_MULT3x3_ADDR)
#define getMTX_STORE (u8)REG2IDADDR(MATRIX_STORE_ADDR)
#define getMTX_RESTORE (u8)REG2IDADDR(MATRIX_RESTORE_ADDR)
#define getMTX_MULT_4x3 (u8)REG2IDADDR(MATRIX_MULT4x3_ADDR)
#define getMTX_MULT_4x4 (u8)REG2IDADDR(MATRIX_MULT4x4_ADDR)
#define getMTX_SCALE (u8)REG2IDADDR(MATRIX_SCALE_ADDR)
#define getMTX_PUSH (u8)REG2IDADDR(GFX_MTX_PUSH_ADDR)
#define getNOP (u8)REG2IDADDR(GFX_NOP_ADDR)
#define getMTX_POP (u8)REG2IDADDR(GFX_MTX_POP_ADDR)
#define getMTX_TRANS (u8)REG2IDADDR(GFX_MTX_TRANS_ADDR)
#define getMTX_IDENTITY (u8)REG2IDADDR(GFX_MTX_IDENTITY_ADDR)
#define getMTX_LOAD_4x4 (u8)REG2IDADDR(GFX_MTX_LOAD_4x4_ADDR)
#define getMTX_LOAD_4x3 (u8)REG2IDADDR(GFX_MTX_LOAD_4x3_ADDR)
#define getMTX_MODE (u8)REG2IDADDR(GFX_MTX_MODE_ADDR)
#define getVIEWPORT (u8)REG2IDADDR(GFX_VIEWPORT_ADDR)
#define getFIFO_NORMAL (u8)REG2IDADDR(GFX_NORMAL_ADDR)
#define getFIFO_SHININESS (u8)REG2IDADDR(GFX_SHININESS_ADDR)
#define getFIFO_POLYGON_ATTR (u8)REG2IDADDR(GFX_POLYGON_ATTR_ADDR)
#define getFIFO_DIFFUSE_AMBIENT (u8)REG2IDADDR(GFX_DIFFUSE_AMBIENT_ADDR)
#define getFIFO_SPECULAR_EMISSION (u8)REG2IDADDR(GFX_SPECULAR_EMISSION_ADDR)
#define getFIFO_BEGIN (u8)REG2IDADDR(GFX_BEGIN_ADDR)
#define getFIFO_END (u8)REG2IDADDR(GFX_END_ADDR)
#define getFIFO_SWAP_BUFFERS (u8)REG2IDADDR(FIFO_SWAP_BUFFERS_ADDR) //aka getFIFO_FLUSH
#define getFIFO_VIEWPORT (u8)REG2IDADDR(GFX_VIEWPORT_ADDR)

#endif

#define PI 3.14159
#define TWOPI		    (PI*2)

extern bool isAGXCommand(u32 val);
extern u8 clzero(u32 var);
extern int getAGXParamsCountFromCommand(u32 command);

extern void swap1(char *x, char *y);
extern char* reverse1(char *buffer, int i, int j);
extern char* itoa1(int value, char* buffer, int base);

//crc32 bits
extern void init_crc_table (void *table, unsigned int polynomial);
extern unsigned int *crc32_table;
extern void free_crc32_table (void);
extern unsigned int crc32 (unsigned int *crc, const void *buffer, unsigned int size);
extern int crc32file( FILE *file, unsigned int *outCrc32);

#ifdef __cplusplus
extern "C" {
#endif

extern int TWLPrintf(const char *fmt, ...);

//glut shapes 
extern GLint DLSOLIDCUBE0_06F;
extern GLint DLSPHERE;
extern GLint DLCYLINDER;

extern void drawSphere(float r, int lats, int longs);
extern void glut2SolidCube(float x, float y, float z);
extern void drawCircle(GLfloat x, GLfloat y, GLfloat r, GLfloat BALL_RADIUS);
extern void setupGLUTObjects();
extern void glut2SolidCubeSlow(GLdouble size);
extern void drawCylinder(int numMajor, int numMinor, float height, float radius);

#ifdef __cplusplus
}
#endif
