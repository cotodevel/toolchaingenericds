////////////////////////////////////////////////////////////////////
//
// videoGL.h -- Video API vaguely similar to OpenGL
//
// version 0.1, February 14, 2005
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
//   0.2: Added gluFrustrum, gluPerspective, and gluLookAt
//			Converted all floating point math to fixed point
//
//	 0.3: Display lists added thanks to mike260	
//	 0.4: Update GL specs from OpenGL 1.0 to OpenGL 1.1 by implementing standard OpenGL Display Lists support using GX hardware (Coto)
//	 0.5: Extend GL specs on OpenGL 1.1 by implementing Vertex Array Buffers and Vertex Buffer Objects support using GX hardware (Coto)
//////////////////////////////////////////////////////////////////////

#ifndef VIDEOGL_ARM9_INCLUDE
#define VIDEOGL_ARM9_INCLUDE
//////////////////////////////////////////////////////////////////////

//#ifndef ARM9
//#error 3D hardware is only available from the ARM9
//#endif

//////////////////////////////////////////////////////////////////////
#include "arm9math.h"

#ifdef ARM9
#include <typedefsTGDS.h>
#include "videoTGDS.h"
#include "arm9math.h"
#include "dsregs.h"
#include "dsregs.h"
#include "dmaTGDS.h"
#include "nds_cp15_misc.h"
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
#include "TGDSTypes.h"
#include "crc16_win32.h"
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

#define GL_FLAT ((GLenum)0)		// flat shading (primitives are drawn using the same color set before by glColor3b(uint8 red, uint8 green, uint8 blue)) 
#define GL_SMOOTH ((GLenum)1)	//smooth shading (color comes from either Toon Shading or HighLight shading

#endif

//lut resolution for trig functions (must be power of two and must be the same as LUT resolution)
//in other words dont change unless you also change your LUTs
#define LUT_SIZE (512)
#define LUT_MASK (0x1FF)

//////////////////////////////////////////////////////////////////////
// 3D core control
//////////////////////////////////////////////////////////////////////
#define GFX_CONTROL_ADDR           ((vuint32) 0x04000060)

#define GFX_FIFO_ADDR              ((vuint32) 0x04000400)  
#define GFX_STATUS_ADDR            ((vuint32) 0x04000600)
#define GFX_COLOR_ADDR             ((vuint32) 0x04000480)

#define GFX_VERTEX10_ADDR          ((vuint32) 0x04000490)
#define GFX_VERTEX_XY_ADDR          ((vuint32) 0x04000494)
#define GFX_VERTEX_XZ_ADDR          ((vuint32) 0x04000498)
#define GFX_VERTEX_YZ_ADDR          ((vuint32) 0x0400049C)
#define GFX_VERTEX_DIFF_ADDR          ((vuint32) 0x040004A0)

#define GFX_VERTEX16_ADDR          ((vuint32) 0x0400048C)
#define GFX_TEX_COORD_ADDR         ((vuint32) 0x04000488)
#define GFX_TEX_FORMAT_ADDR        ((vuint32) 0x040004A8)

#define GFX_CLEAR_COLOR_ADDR       ((vuint32) 0x04000350)
#define GFX_CLEAR_DEPTH_ADDR       ((vuint32) 0x04000354)

#define GFX_LIGHT_VECTOR_ADDR      ((vuint32) 0x040004C8)
#define GFX_LIGHT_COLOR_ADDR       ((vuint32) 0x040004CC)
#define GFX_NORMAL_ADDR            ((vuint32) 0x04000484)

#define GFX_NOP_ADDR				((vuint32) 0)
#define GFX_MTX_PUSH_ADDR				((vuint32) 0x04000444)
#define GFX_MTX_POP_ADDR				((vuint32) 0x04000448)
#define GFX_MTX_TRANS_ADDR				((vuint32) 0x04000470)
#define GFX_MTX_IDENTITY_ADDR			((vuint32) 0x04000454)
#define GFX_MTX_LOAD_4x4_ADDR			((vuint32) 0x04000458)
#define GFX_MTX_LOAD_4x3_ADDR			((vuint32) 0x0400045C)
#define GFX_MTX_MODE_ADDR				((vuint32) 0x04000440)
#define GFX_VIEWPORT_ADDR				((vuint32) 0x04000580)

#define GFX_DIFFUSE_AMBIENT_ADDR   ((vuint32) 0x040004C0)
#define GFX_SPECULAR_EMISSION_ADDR ((vuint32) 0x040004C4)
#define GFX_SHININESS_ADDR         ((vuint32) 0x040004D0)

#define GFX_POLYGON_ATTR_ADDR       ((vuint32) 0x040004A4)

#define GFX_BEGIN_ADDR             ((vuint32) 0x04000500)
#define GFX_END_ADDR               ((vuint32) 0x04000504)
#define FIFO_SWAP_BUFFERS_ADDR    ((vuint32) 0x04000540)
#define GFX_TOON_TABLE_ADDR		  ((vuint32)  0x04000380)
#define GFX_EDGE_TABLE_ADDR		  ((vuint32)  0x04000330)

#define GFX_POLY_FORMAT       (*(vuint32*) 0x040004A4)
#define GFX_FLUSH             (*(vuint32*) 0x04000540)
//////////////////////////////////////////////////////////////////////
// Matrix processor control
//////////////////////////////////////////////////////////////////////

#define MATRIX_PUSH_ADDR       (0x04000444)
#define MATRIX_POP_ADDR        (0x04000448)
#define MATRIX_SCALE_ADDR      (0x0400046C)
#define MATRIX_TRANSLATE_ADDR  (0x04000470)
#define MATRIX_RESTORE_ADDR    (0x04000450)
#define MATRIX_STORE_ADDR      (0x0400044C)
#define MATRIX_MULT4x4_ADDR    (0x04000460)
#define MATRIX_MULT4x3_ADDR    (0x04000464)
#define MATRIX_MULT3x3_ADDR    (0x04000468)

//////////////////////////////////////////////////////////////////////

#define inttof32(n)          (1 << 12) /*!< \brief convert int to f32 */
#define f32toint(n)          (n >> 12) /*!< \brief convert f32 to int */
#define floattof32(n)        ((int)((n) * (1 << 12))) /*!< \brief convert float to f32 */
#define f32tofloat(n)        (((float)(n)) / (float)(1<<12)) /*!< \brief convert f32 to float */

//newer
typedef uint16 fixed12d3; /*!< \brief Used for depth (glClearDepth, glCutoffDepth) */
#define intto12d3(n)    ((n) << 3) /*!< \brief convert int to fixed12d3 */
#define floatto12d3(n)  ((fixed12d3)((n) * (1 << 3))) /*!< \brief convert float to fixed12d3 */

//////////////////////////////////////////////////////////////////////

typedef short t16;        /*!< \brief text coordinate 12.4 fixed point */
#define f32tot16(n)          ((t16)(n >> 8)) /*!< \brief convert f32 to t16 */
#define inttot16(n)          ((n) << 4) /*!< \brief convert int to t16 */
#define t16toint(n)          ((n) >> 4) /*!< \brief convert t16 to int */
#define floattot16(n)        ((t16)((n) * (1 << 4))) /*!< \brief convert float to t16 */
#define TEXTURE_PACK(u,v)    (((u) & 0xFFFF) | ((v) << 16)) /*!< \brief Pack 2 t16 texture coordinate values into a 32bit value */

typedef short int v16;       /*!< \brief vertex 4.12 fixed format */
#define inttov16(n)          ((n) << 12) /*!< \brief convert int to v16 */
#define f32tov16(n)          (n) /*!< \brief f32 to v16 */
#define v16toint(n)          ((n) >> 12) /*!< \brief convert v16 to int */
#define floattov16(n)        ((v16)((n) * (1 << 12))) /*!< \brief convert float to v16 */
#define VERTEX_PACK(x,y)     (u32)(((x) & 0xFFFF) | ((y) << 16)) /*!< \brief Pack 2x v16 values into one 32bit value */
#define VERTEX_PACKv10(x, y, z)     (u32)(((x) & 0x3FF) | ((y & 0x3FF) << 10) | ((z & 0x3FF) << 20)) /*!< \brief Pack 3x v10 values into one 32bit value */

typedef short int v10;       /*!< \brief normal .10 fixed point, NOT USED FOR 10bit VERTEXES!!!*/
#define inttov10(n)          ((n) << 9) /*!< \brief convert int to v10 */
#define f32tov10(n)          ((v10)(n >> 3)) /*!< \brief convert f32 to v10 */
#define v10toint(n)          ((n) >> 9) /*!< \brief convert v10 to int */
#define floattov10(n)        ((n>.998) ? 0x1FF : ((v10)((n)*(1<<9)))) /*!< \brief convert float to v10 */
#define NORMAL_PACK(x,y,z)   (u32)(((x) & 0x3FF) | (((y) & 0x3FF) << 10) | ((z & 0x3FF) << 20)) /*!< \brief Pack 3 v10 normals into a 32bit value */

typedef unsigned short rgb;

typedef struct {
  f32 m[9];
} m3x3;

typedef struct {
  f32 m[16];
} m4x4;

typedef struct {
  f32 m[12];
} m4x3;

typedef struct {
  f32 x,y,z;
} GLvector;

#ifndef GL_VERSION_1_1
#define GL_VERSION_1_1 1

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

#define GL_FLAT ((GLenum)0)		// flat shading (primitives are drawn using the same color set before by glColor3b(uint8 red, uint8 green, uint8 blue)) 
#define GL_SMOOTH ((GLenum)1)	//smooth shading (color comes from either Toon Shading or HighLight shading

#endif

#define GX_LIGHT0 (1 << 0)
#define GX_LIGHT1 (1 << 1)
#define GX_LIGHT2 (1 << 2)
#define GX_LIGHT3 (1 << 3)

struct GLContext{
	GLenum primitiveShadeModelMode;	//glShadeModel(GLenum mode: [GL_FLAT/GL_SMOOTH]);
	u8 lightsEnabled; //lights enabled are written here
	u32 textureParamsValue;
	u16 diffuseValue;
	u16 ambientValue;
	u16 specularValue;
	u16 emissionValue;

	//latest Viewport
	u32 lastViewport; //(x1) + (y1 << 8) + (x2 << 16) + (y2 << 24) //x1 = x, y1 = y, x2 = 
} 

#ifdef ARM9
__attribute__((aligned (4)));
#endif
#ifdef WIN32
;
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// 3D core control (NDS bits)
//////////////////////////////////////////////////////////////////////

#define GFX_CONTROL           (*(vuint16*) 0x04000060)

#define GFX_FIFO              (*(vuint32*) 0x04000400)  
#define GFX_STATUS            (*(vuint32*) 0x04000600)
#define GFX_COLOR             (*(vuint32*) 0x04000480)

#define GFX_VERTEX10          (*(vuint32*) 0x04000490)
#define GFX_VERTEX_XY          (*(vuint32*) 0x04000494)
#define GFX_VERTEX_XZ          (*(vuint32*) 0x04000498)
#define GFX_VERTEX_YZ          (*(vuint32*) 0x0400049C)
#define GFX_VERTEX_DIFF          (*(vuint32*) 0x040004A0)

#define GFX_VERTEX16          (*(vuint32*) 0x0400048C)
#define GFX_TEX_COORD         (*(vuint32*) 0x04000488)
#define GFX_TEX_FORMAT        (*(vuint32*) 0x040004A8)

#define GFX_CLEAR_COLOR       (*(vuint32*) 0x04000350)
#define GFX_CLEAR_DEPTH       (*(vuint16*) 0x04000354)

#define GFX_LIGHT_VECTOR      (*(vuint32*) 0x040004C8)
#define GFX_LIGHT_COLOR       (*(vuint32*) 0x040004CC)
#define GFX_NORMAL            (*(vuint32*) 0x04000484)

#define GFX_DIFFUSE_AMBIENT   (*(vuint32*) 0x040004C0)
#define GFX_SPECULAR_EMISSION (*(vuint32*) 0x040004C4)
#define GFX_SHININESS         (*(vuint32*) 0x040004D0)

#define GFX_POLYGON_ATTR       (*(vuint32*) 0x040004A4)

#define GFX_BEGIN             (*(vuint32*) 0x04000500)
#define GFX_END               (*(vuint32*) 0x04000504)
#define GFX_SWAP_BUFFERS      (*(vuint32*) 0x04000540)
#define GFX_VIEWPORT          (*(vuint32*) 0x04000580)
#define GFX_TOON_TABLE		  ((vuint16*)  0x04000380)
#define GFX_EDGE_TABLE		  ((vuint16*)  0x04000330)

//////////////////////////////////////////////////////////////////////
// Matrix processor control
//////////////////////////////////////////////////////////////////////

#define MATRIX_CONTROL    (*(vuint32*)0x04000440)
#define MATRIX_PUSH       (*(vuint32*)0x04000444)
#define MATRIX_POP        (*(vuint32*)0x04000448)
#define MATRIX_SCALE      (*(vfixed*) 0x0400046C)
#define MATRIX_TRANSLATE  (*(vfixed*) 0x04000470)
#define MATRIX_RESTORE    (*(vuint32*)0x04000450)
#define MATRIX_STORE      (*(vuint32*)0x0400044C)
#define MATRIX_IDENTITY   (*(vuint32*)0x04000454)
#define MATRIX_LOAD4x4    (*(vfixed*) 0x04000458)
#define MATRIX_LOAD4x3    (*(vfixed*) 0x0400045C)
#define MATRIX_MULT4x4    (*(vfixed*) 0x04000460)
#define MATRIX_MULT4x3    (*(vfixed*) 0x04000464)
#define MATRIX_MULT3x3    (*(vfixed*) 0x04000468)
enum GL_GLBEGIN_ENUM {
	GL_TRIANGLES      = 0, /*!< draw triangles with each 3 vertices defining a triangle */
	GL_QUADS          = 1, /*!< draw quads with each 4 vertices defining a quad */
	GL_TRIANGLE_STRIP = 2, /*!< draw triangles with the first triangle defined by 3 vertices, then each additional triangle being defined by one additional vertex */
	GL_QUAD_STRIP     = 3, /*!< draw quads with the first quad being defined by 4 vertices, then each additional triangle being defined by 2 vertices. */
	GL_TRIANGLE       = 0, /*!< same as GL_TRIANGLES, old non-OpenGL version */
	GL_QUAD           = 1  /*!< same as GL_QUADS, old non-OpenGL version */
} ;

/*! \brief Enums selecting matrix mode<BR>
<A HREF="http://problemkaputt.de/gbatek.htm#ds3dmatrixloadmultiply">GBATEK http://problemkaputt.de/gbatek.htm#ds3dmatrixloadmultiply</A><BR>
related functions: glMatrixMode() */
enum GL_MATRIX_MODE_ENUM {
	GL_PROJECTION     = 0, /*!< used to set the Projection Matrix */
	GL_POSITION       = 1, /*!< used to set the Position Matrix */
	GL_MODELVIEW      = 2, /*!< used to set the Modelview Matrix */
	GL_TEXTURE        = 3  /*!< used to set the Texture Matrix */
} ;

#define GL_AMBIENT              1
#define GL_DIFFUSE              2
#define GL_AMBIENT_AND_DIFFUSE  3
#define GL_SPECULAR             4
#define GL_SHININESS            8
#define GL_EMISSION             0x10

//////////////////////////////////////////////////////////////////////

#define POLY_ALPHA(n)  ((n) << 16)
#define POLY_TOON_SHADING     0x20
#define POLY_CULL_BACK        0x80
#define POLY_CULL_FRONT       0x40
#define POLY_CULL_NONE        0xC0
#define POLY_ID(n)		((n)<<24)

//////////////////////////////////////////////////////////////////////

#define POLY_FORMAT_LIGHT0      0x1
#define POLY_FORMAT_LIGHT1      0x2
#define POLY_FORMAT_LIGHT2      0x4
#define POLY_FORMAT_LIGHT3      0x8

//////////////////////////////////////////////////////////////////////
#define MAX_TEXTURES 2048  //this should be enough ! but feel free to change

#define TEXTURE_SIZE_8     0
#define TEXTURE_SIZE_16    1 
#define TEXTURE_SIZE_32    2
#define TEXTURE_SIZE_64    3
#define TEXTURE_SIZE_128   4
#define TEXTURE_SIZE_256   5
#define TEXTURE_SIZE_512   6
#define TEXTURE_SIZE_1024  7 

//unmodified texcoord
#define TEXGEN_OFF			(0<<30)

//texcoord * texture-matrix
#define TEXGEN_TEXCOORD		(1<<30)			

//normal * texture-matrix
#define TEXGEN_NORMAL		(2<<30)

//vertex * texture-matrix
#define TEXGEN_POSITION		(3<<30)


#define GL_TEXTURE_WRAP_S (1 << 16)
#define GL_TEXTURE_WRAP_T (1 << 17)
#define GL_TEXTURE_FLIP_S (1 << 18)
#define GL_TEXTURE_FLIP_T (1 << 19)

#define GL_TEXTURE_2D		1
#define GL_TOON_HIGHLIGHT	(1<<1)
#define GL_BLEND (1<<3)
#define GL_ANTIALIAS		(1<<4)
#define GL_OUTLINE			(1<<5)

//13    Polygon/Vertex RAM Overflow    (0=None, 1=Overflow/Acknowledge)
#define GL_POLYGON_VERTEX_RAM_OVERFLOW	(1<<13)

//14    Rear-Plane Mode                (0=Blank, 1=Bitmap)
#define REAR_PLANE_MODE_BITMAP			(1<<14)
  
//////////////////////////////////////////////////////////////////////

#define GL_RGB		8
#define GL_RGBA		7	//15 bit color + alpha bit
#define GL_RGB4		2	//4 color palette
#define GL_RGB256	4	//256 color palette
#define GL_RGB16	3	//16 color palette
#define GL_COMPRESSED	5 //compressed texture

//////////////////////////////////////////////////////////////////////
//Fifo commands (NDS bits)

#define FIFO_COMMAND_PACK(c1,c2,c3,c4) (((c4) << 24) | ((c3) << 16) | ((c2) << 8) | (c1)) /*!< \brief packs four packed commands into a 32bit command for sending to the GFX FIFO */
#define REG2ID(r)						(u8)( ( ((u32)(&(r)))-0x04000400 ) >> 2 )
#define REG2IDADDR(r)						(u8)( ( ((u32)((r)))-0x04000400 ) >> 2 )

#define FIFO_NOP				REG2ID(GFX_FIFO)  
#define FIFO_STATUS				REG2ID(GFX_STATUS)            
#define FIFO_COLOR				REG2ID(GFX_COLOR)            

#define FIFO_VERTEX16			REG2ID(GFX_VERTEX16)          
#define FIFO_TEX_COORD			REG2ID(GFX_TEX_COORD)         
#define FIFO_TEX_FORMAT			REG2ID(GFX_TEX_FORMAT)        

#define FIFO_CLEAR_COLOR		REG2ID(GFX_CLEAR_COLOR)       
#define FIFO_CLEAR_DEPTH		REG2ID(GFX_CLEAR_DEPTH)       

#define FIFO_LIGHT_VECTOR		REG2ID(GFX_LIGHT_VECTOR)      
#define FIFO_LIGHT_COLOR		REG2ID(GFX_LIGHT_COLOR)       
#define FIFO_NORMAL				REG2ID(GFX_NORMAL)            

#define FIFO_DIFFUSE_AMBIENT	REG2ID(GFX_DIFFUSE_AMBIENT)   
#define FIFO_SPECULAR_EMISSION	REG2ID(GFX_SPECULAR_EMISSION) 
#define FIFO_SHININESS			REG2ID(GFX_SHININESS)        

#define FIFO_POLY_FORMAT		REG2ID(GFX_POLY_FORMAT)       

#define FIFO_BEGIN				REG2ID(GFX_BEGIN)             
#define FIFO_END				REG2ID(GFX_END)               
#define FIFO_FLUSH				REG2ID(GFX_FLUSH)             
#define FIFO_VIEWPORT			REG2ID(GFX_VIEWPORT)     

//Extended NDS bits
#define MTX_PUSH		REG2ID(MATRIX_PUSH)  
#define MTX_POP			REG2ID(MATRIX_POP)
#define MTX_IDENTITY	REG2ID(MATRIX_IDENTITY)
#define MTX_TRANS		REG2ID(MATRIX_TRANSLATE)
#define MTX_MULT_4x4		REG2ID(MATRIX_MULT4x4)
#define MTX_MULT_4x3		REG2ID(MATRIX_MULT4x3)
#define MTX_MULT_3x3		REG2ID(MATRIX_MULT3x3)

//Custom OpenGL Display List -> GX commands. 
#define MTX_ROTATE_Z	((u8)0x80) //GXFIFO command slot start
#define MTX_ROTATE_Y	((u8)0x81)
#define MTX_ROTATE_X	((u8)0x82)
#define MTX_FRUSTRUM	((u8)0x83)
#define MTX_LOOKAT		((u8)0x84)
#define OPENGL_DL_TO_GX_DL_EXEC_CMD		((u8)0x85) //This command enables OpenGL Display Lists to run directly through GX Display List hardware

//Precalculated argument size per command
#define  MTX_STORE_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_STORE))
#define  MTX_TRANS_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_TRANS))
#define  MTX_IDENTITY_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_IDENTITY))
#define  MTX_MODE_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_MODE))
#define  VIEWPORT_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getVIEWPORT))
#define  FIFO_TEX_COORD_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_TEX_COORD))
#define  FIFO_BEGIN_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_BEGIN))
#define  FIFO_END_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_END))
#define  FIFO_COLOR_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_COLOR))
#define  FIFO_NORMAL_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_NORMAL))
#define  FIFO_VERTEX16_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_VERTEX16))
#define  FIFO_VERTEX10_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_VERTEX10))
#define  FIFO_VTX_XY_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getFIFO_VTX_XY))
#define  MTX_PUSH_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_PUSH))
#define  MTX_POP_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_POP))
#define  MTX_MULT_3x3_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_MULT_3x3))
#define  MTX_MULT_4x4_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_MULT_4x4))
#define  MTX_LOAD_4x4_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_LOAD_4x4))
#define  MTX_LOAD_4x3_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getMTX_LOAD_4x3))
#define  NOP_GXCommandParamsCount ((int)getAGXParamsCountFromCommand(getNOP))

enum {
	/* Boolean values */
	GL_FALSE			= (unsigned int)0,
	GL_TRUE				= (unsigned int)1,

	/* Data types */
	GL_BYTE				= (unsigned int)0x1400,
	GL_UNSIGNED_BYTE		= (unsigned int)0x1401,
	GL_SHORT			= (unsigned int)0x1402,
	GL_UNSIGNED_SHORT		= (unsigned int)0x1403,
	GL_INT				= (unsigned int)0x1404,
	GL_UNSIGNED_INT			= (unsigned int)0x1405,
	GL_FLOAT			= (unsigned int)0x1406,
	GL_DOUBLE			= (unsigned int)0x140A,
	GL_2_BYTES			= (unsigned int)0x1407,
	GL_3_BYTES			= (unsigned int)0x1408,
	GL_4_BYTES			= (unsigned int)0x1409,

	/* Primitives */
	GL_LINES			= (unsigned int)0x0001,
	GL_POINTS			= (unsigned int)0x0000,
	GL_LINE_STRIP			= (unsigned int)0x0003,
	GL_LINE_LOOP			= (unsigned int)0x0002,
	//GL_TRIANGLES			= (unsigned int)0x0004,
	//GL_TRIANGLE_STRIP		= (unsigned int)0x0005,
	GL_TRIANGLE_FAN			= (unsigned int)0x0006,
	//GL_QUADS			= (unsigned int)0x0007,
	//GL_QUAD_STRIP			= (unsigned int)0x0008,
	GL_POLYGON			= (unsigned int)0x0009,
	GL_EDGE_FLAG			= (unsigned int)0x0B43,

	/* Vertex Arrays */
	GL_VERTEX_ARRAY			= (unsigned int)0x8074,
	GL_NORMAL_ARRAY			= (unsigned int)0x8075,
	GL_COLOR_ARRAY			= (unsigned int)0x8076,
	GL_INDEX_ARRAY			= (unsigned int)0x8077,
	GL_TEXTURE_COORD_ARRAY		= (unsigned int)0x8078,
	GL_EDGE_FLAG_ARRAY		= (unsigned int)0x8079,
	GL_VERTEX_ARRAY_SIZE		= (unsigned int)0x807A,
	GL_VERTEX_ARRAY_TYPE		= (unsigned int)0x807B,
	GL_VERTEX_ARRAY_STRIDE		= (unsigned int)0x807C,
	GL_VERTEX_ARRAY_COUNT		= (unsigned int)0x807D,
	GL_NORMAL_ARRAY_TYPE		= (unsigned int)0x807E,
	GL_NORMAL_ARRAY_STRIDE		= (unsigned int)0x807F,
	GL_NORMAL_ARRAY_COUNT		= (unsigned int)0x8080,
	GL_COLOR_ARRAY_SIZE		= (unsigned int)0x8081,
	GL_COLOR_ARRAY_TYPE		= (unsigned int)0x8082,
	GL_COLOR_ARRAY_STRIDE		= (unsigned int)0x8083,
	GL_COLOR_ARRAY_COUNT		= (unsigned int)0x8084,
	GL_INDEX_ARRAY_TYPE		= (unsigned int)0x8085,
	GL_INDEX_ARRAY_STRIDE		= (unsigned int)0x8086,
	GL_INDEX_ARRAY_COUNT		= (unsigned int)0x8087,
	GL_TEXTURE_COORD_ARRAY_SIZE	= (unsigned int)0x8088,
	GL_TEXTURE_COORD_ARRAY_TYPE	= (unsigned int)0x8089,
	GL_TEXTURE_COORD_ARRAY_STRIDE	= (unsigned int)0x808A,
	GL_TEXTURE_COORD_ARRAY_COUNT	= (unsigned int)0x808B,
	GL_EDGE_FLAG_ARRAY_STRIDE	= (unsigned int)0x808C,
	GL_EDGE_FLAG_ARRAY_COUNT	= (unsigned int)0x808D,
	GL_VERTEX_ARRAY_POINTER		= (unsigned int)0x808E,
	GL_NORMAL_ARRAY_POINTER		= (unsigned int)0x808F,
	GL_COLOR_ARRAY_POINTER		= (unsigned int)0x8090,
	GL_INDEX_ARRAY_POINTER		= (unsigned int)0x8091,
	GL_TEXTURE_COORD_ARRAY_POINTER	= (unsigned int)0x8092,
	GL_EDGE_FLAG_ARRAY_POINTER	= (unsigned int)0x8093,
        GL_V2F				= (unsigned int)0x2A20,
	GL_V3F				= (unsigned int)0x2A21,
	GL_C4UB_V2F			= (unsigned int)0x2A22,
	GL_C4UB_V3F			= (unsigned int)0x2A23,
	GL_C3F_V3F			= (unsigned int)0x2A24,
	GL_N3F_V3F			= (unsigned int)0x2A25,
	GL_C4F_N3F_V3F			= (unsigned int)0x2A26,
	GL_T2F_V3F			= (unsigned int)0x2A27,
	GL_T4F_V4F			= (unsigned int)0x2A28,
	GL_T2F_C4UB_V3F			= (unsigned int)0x2A29,
	GL_T2F_C3F_V3F			= (unsigned int)0x2A2A,
	GL_T2F_N3F_V3F			= (unsigned int)0x2A2B,
	GL_T2F_C4F_N3F_V3F		= (unsigned int)0x2A2C,
	GL_T4F_C4F_N3F_V4F		= (unsigned int)0x2A2D,

	/* Matrix Mode */
	GL_MATRIX_MODE			= (unsigned int)0x0BA0,
	//GL_MODELVIEW			= (unsigned int)0x1700,
	//GL_PROJECTION			= (unsigned int)0x1701,
	//GL_TEXTURE			= (unsigned int)0x1702,

	/* Points */
	GL_POINT_SMOOTH			= (unsigned int)0x0B10,
	GL_POINT_SIZE			= (unsigned int)0x0B11,
	GL_POINT_SIZE_GRANULARITY 	= (unsigned int)0x0B13,
	GL_POINT_SIZE_RANGE		= (unsigned int)0x0B12,

	/* Lines */
	GL_LINE_SMOOTH			= (unsigned int)0x0B20,
	GL_LINE_STIPPLE			= (unsigned int)0x0B24,
	GL_LINE_STIPPLE_PATTERN		= (unsigned int)0x0B25,
	GL_LINE_STIPPLE_REPEAT		= (unsigned int)0x0B26,
	GL_LINE_WIDTH			= (unsigned int)0x0B21,
	GL_LINE_WIDTH_GRANULARITY	= (unsigned int)0x0B23,
	GL_LINE_WIDTH_RANGE		= (unsigned int)0x0B22,

	/* Polygons */
	GL_POINT			= (unsigned int)0x1B00,
	GL_LINE				= (unsigned int)0x1B01,
	GL_FILL				= (unsigned int)0x1B02,
	GL_CCW				= (unsigned int)0x0901,
	GL_CW				= (unsigned int)0x0900,
	GL_FRONT			= (unsigned int)0x0404,
	GL_BACK				= (unsigned int)0x0405,
	GL_CULL_FACE			= (unsigned int)0x0B44,
	GL_CULL_FACE_MODE		= (unsigned int)0x0B45,
	GL_POLYGON_SMOOTH		= (unsigned int)0x0B41,
	GL_POLYGON_STIPPLE		= (unsigned int)0x0B42,
	GL_FRONT_FACE			= (unsigned int)0x0B46,
	GL_POLYGON_MODE			= (unsigned int)0x0B40,
	GL_POLYGON_OFFSET_FACTOR	= (unsigned int)0x3038,
	GL_POLYGON_OFFSET_UNITS		= (unsigned int)0x2A00,
	GL_POLYGON_OFFSET_POINT		= (unsigned int)0x2A01,
	GL_POLYGON_OFFSET_LINE		= (unsigned int)0x2A02,
	GL_POLYGON_OFFSET_FILL		= (unsigned int)0x8037,

	/* Display Lists */
	GL_COMPILE			= (unsigned int)0x1300,
	GL_COMPILE_AND_EXECUTE		= (unsigned int)0x1301,
	GL_LIST_BASE			= (unsigned int)0x0B32,
	GL_LIST_INDEX			= (unsigned int)0x0B33,
	GL_LIST_MODE			= (unsigned int)0x0B30,

	/* Depth buffer */
	GL_NEVER			= (unsigned int)0x0200,
	GL_LESS				= (unsigned int)0x0201,
	GL_GEQUAL			= (unsigned int)0x0206,
	GL_LEQUAL			= (unsigned int)0x0203,
	GL_GREATER			= (unsigned int)0x0204,
	GL_NOTEQUAL			= (unsigned int)0x0205,
	GL_EQUAL			= (unsigned int)0x0202,
	GL_ALWAYS			= (unsigned int)0x0207,
	GL_DEPTH_TEST			= (unsigned int)0x0B71,
	GL_DEPTH_BITS			= (unsigned int)0x0D56,
	GL_DEPTH_CLEAR_VALUE		= (unsigned int)0x0B73,
	GL_DEPTH_FUNC			= (unsigned int)0x0B74,
	GL_DEPTH_RANGE			= (unsigned int)0x0B70,
	GL_DEPTH_WRITEMASK		= (unsigned int)0x0B72,
	GL_DEPTH_COMPONENT		= (unsigned int)0x1902,

	/* Lighting */
	GL_LIGHTING			= (unsigned int)0x0B50,
	GL_LIGHT0			= (unsigned int)0x4000,
	GL_LIGHT1			= (unsigned int)0x4001,
	GL_LIGHT2			= (unsigned int)0x4002,
	GL_LIGHT3			= (unsigned int)0x4003,
	GL_LIGHT4			= (unsigned int)0x4004,
	GL_LIGHT5			= (unsigned int)0x4005,
	GL_LIGHT6			= (unsigned int)0x4006,
	GL_LIGHT7			= (unsigned int)0x4007,
	GL_SPOT_EXPONENT		= (unsigned int)0x1205,
	GL_SPOT_CUTOFF			= (unsigned int)0x1206,
	GL_CONSTANT_ATTENUATION		= (unsigned int)0x1207,
	GL_LINEAR_ATTENUATION		= (unsigned int)0x1208,
	GL_QUADRATIC_ATTENUATION	= (unsigned int)0x1209,
	//GL_AMBIENT			= (unsigned int)0x1200,
	//GL_DIFFUSE			= (unsigned int)0x1201,
	//GL_SPECULAR			= (unsigned int)0x1202,
	//GL_SHININESS			= (unsigned int)0x1601,
	//GL_EMISSION			= (unsigned int)0x1600,
	//GL_POSITION			= (unsigned int)0x1203,
	GL_SPOT_DIRECTION		= (unsigned int)0x1204,
	//GL_AMBIENT_AND_DIFFUSE		= (unsigned int)0x1602,
	GL_COLOR_INDEXES		= (unsigned int)0x1603,
	GL_LIGHT_MODEL_TWO_SIDE		= (unsigned int)0x0B52,
	GL_LIGHT_MODEL_LOCAL_VIEWER	= (unsigned int)0x0B51,
	GL_LIGHT_MODEL_AMBIENT		= (unsigned int)0x0B53,
	GL_FRONT_AND_BACK		= (unsigned int)0x0408,
	GL_SHADE_MODEL			= (unsigned int)0x0B54,
	//GL_FLAT				= (unsigned int)0x1D00,
	//GL_SMOOTH			= (unsigned int)0x1D01,
	GL_COLOR_MATERIAL		= (unsigned int)0x0B57,
	GL_COLOR_MATERIAL_FACE		= (unsigned int)0x0B55,
	GL_COLOR_MATERIAL_PARAMETER	= (unsigned int)0x0B56,
	GL_NORMALIZE			= (unsigned int)0x0BA1,

	/* User clipping planes */
	GL_CLIP_PLANE0			= (unsigned int)0x3000,
	GL_CLIP_PLANE1			= (unsigned int)0x3001,
	GL_CLIP_PLANE2			= (unsigned int)0x3002,
	GL_CLIP_PLANE3			= (unsigned int)0x3003,
	GL_CLIP_PLANE4			= (unsigned int)0x3004,
	GL_CLIP_PLANE5			= (unsigned int)0x3005,

	/* Accumulation buffer */
	GL_ACCUM_RED_BITS		= (unsigned int)0x0D58,
	GL_ACCUM_GREEN_BITS		= (unsigned int)0x0D59,
	GL_ACCUM_BLUE_BITS		= (unsigned int)0x0D5A,
	GL_ACCUM_ALPHA_BITS		= (unsigned int)0x0D5B,
	GL_ACCUM_CLEAR_VALUE		= (unsigned int)0x0B80,
	GL_ACCUM			= (unsigned int)0x0100,
	GL_ADD				= (unsigned int)0x0104,
	GL_LOAD				= (unsigned int)0x0101,
	GL_MULT				= (unsigned int)0x0103,
	GL_RETURN			= (unsigned int)0x0102,

	/* Alpha testing */
	GL_ALPHA_TEST			= (unsigned int)0x0BC0,
	GL_ALPHA_TEST_REF		= (unsigned int)0x0BC2,
	GL_ALPHA_TEST_FUNC		= (unsigned int)0x0BC1,

	/* Blending */
	//GL_BLEND			= (unsigned int)0x0BE2,
	GL_BLEND_SRC			= (unsigned int)0x0BE1,
	GL_BLEND_DST			= (unsigned int)0x0BE0,
	GL_ZERO				= (unsigned int)0,
	GL_ONE				= (unsigned int)1,
	GL_SRC_COLOR			= (unsigned int)0x0300,
	GL_ONE_MINUS_SRC_COLOR		= (unsigned int)0x0301,
	GL_DST_COLOR			= (unsigned int)0x0306,
	GL_ONE_MINUS_DST_COLOR		= (unsigned int)0x0307,
	GL_SRC_ALPHA			= (unsigned int)0x0302,
	GL_ONE_MINUS_SRC_ALPHA		= (unsigned int)0x0303,
	GL_DST_ALPHA			= (unsigned int)0x0304,
	GL_ONE_MINUS_DST_ALPHA		= (unsigned int)0x0305,
	GL_SRC_ALPHA_SATURATE		= (unsigned int)0x0308,
	GL_CONSTANT_COLOR		= (unsigned int)0x8001,
	GL_ONE_MINUS_CONSTANT_COLOR	= (unsigned int)0x8002,
	GL_CONSTANT_ALPHA		= (unsigned int)0x8003,
	GL_ONE_MINUS_CONSTANT_ALPHA	= (unsigned int)0x8004,

	/* Render Mode */
	GL_FEEDBACK			= (unsigned int)0x1C01,
	GL_RENDER			= (unsigned int)0x1C00,
	GL_SELECT			= (unsigned int)0x1C02,

	/* Feedback */
	GL_2D				= (unsigned int)0x0600,
	GL_3D				= (unsigned int)0x0601,
	GL_3D_COLOR			= (unsigned int)0x0602,
	GL_3D_COLOR_TEXTURE		= (unsigned int)0x0603,
	GL_4D_COLOR_TEXTURE		= (unsigned int)0x0604,
	GL_POINT_TOKEN			= (unsigned int)0x0701,
	GL_LINE_TOKEN			= (unsigned int)0x0702,
	GL_LINE_RESET_TOKEN		= (unsigned int)0x0707,
	GL_POLYGON_TOKEN		= (unsigned int)0x0703,
	GL_BITMAP_TOKEN			= (unsigned int)0x0704,
	GL_DRAW_PIXEL_TOKEN		= (unsigned int)0x0705,
	GL_COPY_PIXEL_TOKEN		= (unsigned int)0x0706,
	GL_PASS_THROUGH_TOKEN		= (unsigned int)0x0700,

	/* Fog */
	GL_FOG				= (unsigned int)0x0B60,
	GL_FOG_MODE			= (unsigned int)0x0B65,
	GL_FOG_DENSITY			= (unsigned int)0x0B62,
	GL_FOG_COLOR			= (unsigned int)0x0B66,
	GL_FOG_INDEX			= (unsigned int)0x0B61,
	GL_FOG_START			= (unsigned int)0x0B63,
	GL_FOG_END			= (unsigned int)0x0B64,
	GL_LINEAR			= (unsigned int)0x2601,
	GL_EXP				= (unsigned int)0x0800,
	GL_EXP2				= (unsigned int)0x0801,

	/* Logic Ops */
	GL_LOGIC_OP			= (unsigned int)0x0BF1,
	GL_LOGIC_OP_MODE		= (unsigned int)0x0BF0,
	GL_CLEAR			= (unsigned int)0x1500,
	GL_SET				= (unsigned int)0x150F,
	GL_COPY				= (unsigned int)0x1503,
	GL_COPY_INVERTED		= (unsigned int)0x150C,
	GL_NOOP				= (unsigned int)0x1505,
	GL_INVERT			= (unsigned int)0x150A,
	GL_AND				= (unsigned int)0x1501,
	GL_NAND				= (unsigned int)0x150E,
	GL_OR				= (unsigned int)0x1507,
	GL_NOR				= (unsigned int)0x1508,
	GL_XOR				= (unsigned int)0x1506,
	GL_EQUIV			= (unsigned int)0x1509,
	GL_AND_REVERSE			= (unsigned int)0x1502,
	GL_AND_INVERTED			= (unsigned int)0x1504,
	GL_OR_REVERSE			= (unsigned int)0x150B,
	GL_OR_INVERTED			= (unsigned int)0x150D,

	/* Stencil */
	GL_STENCIL_TEST			= (unsigned int)0x0B90,
	GL_STENCIL_WRITEMASK		= (unsigned int)0x0B98,
	GL_STENCIL_BITS			= (unsigned int)0x0D57,
	GL_STENCIL_FUNC			= (unsigned int)0x0B92,
	GL_STENCIL_VALUE_MASK		= (unsigned int)0x0B93,
	GL_STENCIL_REF			= (unsigned int)0x0B97,
	GL_STENCIL_FAIL			= (unsigned int)0x0B94,
	GL_STENCIL_PASS_DEPTH_PASS	= (unsigned int)0x0B96,
	GL_STENCIL_PASS_DEPTH_FAIL	= (unsigned int)0x0B95,
	GL_STENCIL_CLEAR_VALUE		= (unsigned int)0x0B91,
	GL_STENCIL_INDEX		= (unsigned int)0x1901,
	GL_KEEP				= (unsigned int)0x1E00,
	GL_REPLACE			= (unsigned int)0x1E01,
	GL_INCR				= (unsigned int)0x1E02,
	GL_DECR				= (unsigned int)0x1E03,

	/* Buffers, Pixel Drawing/Reading */
	GL_NONE				= (unsigned int)0,
	GL_LEFT				= (unsigned int)0x0406,
	GL_RIGHT			= (unsigned int)0x0407,
	/*GL_FRONT			= (unsigned int)0x0404, */
	/*GL_BACK			= (unsigned int)0x0405, */
	/*GL_FRONT_AND_BACK		= (unsigned int)0x0408, */
	GL_FRONT_LEFT			= (unsigned int)0x0400,
	GL_FRONT_RIGHT			= (unsigned int)0x0401,
	GL_BACK_LEFT			= (unsigned int)0x0402,
	GL_BACK_RIGHT			= (unsigned int)0x0403,
	GL_AUX0				= (unsigned int)0x0409,
	GL_AUX1				= (unsigned int)0x040A,
	GL_AUX2				= (unsigned int)0x040B,
	GL_AUX3				= (unsigned int)0x040C,
	GL_COLOR_INDEX			= (unsigned int)0x1900,
	GL_RED				= (unsigned int)0x1903,
	GL_GREEN			= (unsigned int)0x1904,
	GL_BLUE				= (unsigned int)0x1905,
	GL_ALPHA			= (unsigned int)0x1906,
	GL_LUMINANCE			= (unsigned int)0x1909,
	GL_LUMINANCE_ALPHA		= (unsigned int)0x190A,
	GL_ALPHA_BITS			= (unsigned int)0x0D55,
	GL_RED_BITS			= (unsigned int)0x0D52,
	GL_GREEN_BITS			= (unsigned int)0x0D53,
	GL_BLUE_BITS			= (unsigned int)0x0D54,
	GL_INDEX_BITS			= (unsigned int)0x0D51,
	GL_SUBPIXEL_BITS		= (unsigned int)0x0D50,
	GL_AUX_BUFFERS			= (unsigned int)0x0C00,
	GL_READ_BUFFER			= (unsigned int)0x0C02,
	GL_DRAW_BUFFER			= (unsigned int)0x0C01,
	GL_DOUBLEBUFFER			= (unsigned int)0x0C32,
	GL_STEREO			= (unsigned int)0x0C33,
	GL_BITMAP			= (unsigned int)0x1A00,
	GL_COLOR			= (unsigned int)0x1800,
	GL_DEPTH			= (unsigned int)0x1801,
	GL_STENCIL			= (unsigned int)0x1802,
	GL_DITHER			= (unsigned int)0x0BD0,
	//GL_RGB				= (unsigned int)0x1907,
	//GL_RGBA				= (unsigned int)0x1908,

	/* Implementation limits */
	GL_MAX_LIST_NESTING		= (unsigned int)0x0B31,
	GL_MAX_ATTRIB_STACK_DEPTH	= (unsigned int)0x0D35,
	GL_MAX_MODELVIEW_STACK_DEPTH	= (unsigned int)0x0D36,
	GL_MAX_NAME_STACK_DEPTH		= (unsigned int)0x0D37,
	GL_MAX_PROJECTION_STACK_DEPTH	= (unsigned int)0x0D38,
	GL_MAX_TEXTURE_STACK_DEPTH	= (unsigned int)0x0D39,
	GL_MAX_EVAL_ORDER		= (unsigned int)0x0D30,
	GL_MAX_LIGHTS			= (unsigned int)0x0D31,
	GL_MAX_CLIP_PLANES		= (unsigned int)0x0D32,
	GL_MAX_TEXTURE_SIZE		= (unsigned int)0x0D33,
	GL_MAX_PIXEL_MAP_TABLE		= (unsigned int)0x0D34,
	GL_MAX_VIEWPORT_DIMS		= (unsigned int)0x0D3A,
	GL_MAX_CLIENT_ATTRIB_STACK_DEPTH= (unsigned int)0x0D3B,

	/* Gets */
	GL_ATTRIB_STACK_DEPTH		= (unsigned int)0x0BB0,
	GL_COLOR_CLEAR_VALUE		= (unsigned int)0x0C22,
	GL_COLOR_WRITEMASK		= (unsigned int)0x0C23,
	GL_CURRENT_INDEX		= (unsigned int)0x0B01,
	GL_CURRENT_COLOR		= (unsigned int)0x0B00,
	GL_CURRENT_NORMAL		= (unsigned int)0x0B02,
	GL_CURRENT_RASTER_COLOR		= (unsigned int)0x0B04,
	GL_CURRENT_RASTER_DISTANCE	= (unsigned int)0x0B09,
	GL_CURRENT_RASTER_INDEX		= (unsigned int)0x0B05,
	GL_CURRENT_RASTER_POSITION	= (unsigned int)0x0B07,
	GL_CURRENT_RASTER_TEXTURE_COORDS = (unsigned int)0x0B06,
	GL_CURRENT_RASTER_POSITION_VALID = (unsigned int)0x0B08,
	GL_CURRENT_TEXTURE_COORDS	= (unsigned int)0x0B03,
	GL_INDEX_CLEAR_VALUE		= (unsigned int)0x0C20,
	GL_INDEX_MODE			= (unsigned int)0x0C30,
	GL_INDEX_WRITEMASK		= (unsigned int)0x0C21,
	GL_MODELVIEW_MATRIX		= (unsigned int)0x0BA6,
	GL_MODELVIEW_STACK_DEPTH	= (unsigned int)0x0BA3,
	GL_NAME_STACK_DEPTH		= (unsigned int)0x0D70,
	GL_PROJECTION_MATRIX		= (unsigned int)0x0BA7,
	GL_PROJECTION_STACK_DEPTH	= (unsigned int)0x0BA4,
	GL_RENDER_MODE			= (unsigned int)0x0C40,
	GL_RGBA_MODE			= (unsigned int)0x0C31,
	GL_TEXTURE_MATRIX		= (unsigned int)0x0BA8,
	GL_TEXTURE_STACK_DEPTH		= (unsigned int)0x0BA5,
	GL_VIEWPORT			= (unsigned int)0x0BA2,


	/* Evaluators */
	GL_AUTO_NORMAL			= (unsigned int)0x0D80,
	GL_MAP1_COLOR_4			= (unsigned int)0x0D90,
	GL_MAP1_GRID_DOMAIN		= (unsigned int)0x0DD0,
	GL_MAP1_GRID_SEGMENTS		= (unsigned int)0x0DD1,
	GL_MAP1_INDEX			= (unsigned int)0x0D91,
	GL_MAP1_NORMAL			= (unsigned int)0x0D92,
	GL_MAP1_TEXTURE_COORD_1		= (unsigned int)0x0D93,
	GL_MAP1_TEXTURE_COORD_2		= (unsigned int)0x0D94,
	GL_MAP1_TEXTURE_COORD_3		= (unsigned int)0x0D95,
	GL_MAP1_TEXTURE_COORD_4		= (unsigned int)0x0D96,
	GL_MAP1_VERTEX_3		= (unsigned int)0x0D97,
	GL_MAP1_VERTEX_4		= (unsigned int)0x0D98,
	GL_MAP2_COLOR_4			= (unsigned int)0x0DB0,
	GL_MAP2_GRID_DOMAIN		= (unsigned int)0x0DD2,
	GL_MAP2_GRID_SEGMENTS		= (unsigned int)0x0DD3,
	GL_MAP2_INDEX			= (unsigned int)0x0DB1,
	GL_MAP2_NORMAL			= (unsigned int)0x0DB2,
	GL_MAP2_TEXTURE_COORD_1		= (unsigned int)0x0DB3,
	GL_MAP2_TEXTURE_COORD_2		= (unsigned int)0x0DB4,
	GL_MAP2_TEXTURE_COORD_3		= (unsigned int)0x0DB5,
	GL_MAP2_TEXTURE_COORD_4		= (unsigned int)0x0DB6,
	GL_MAP2_VERTEX_3		= (unsigned int)0x0DB7,
	GL_MAP2_VERTEX_4		= (unsigned int)0x0DB8,
	GL_COEFF			= (unsigned int)0x0A00,
	GL_DOMAIN			= (unsigned int)0x0A02,
	GL_ORDER			= (unsigned int)0x0A01,

	/* Hints */
	GL_FOG_HINT			= (unsigned int)0x0C54,
	GL_LINE_SMOOTH_HINT		= (unsigned int)0x0C52,
	GL_PERSPECTIVE_CORRECTION_HINT	= (unsigned int)0x0C50,
	GL_POINT_SMOOTH_HINT		= (unsigned int)0x0C51,
	GL_POLYGON_SMOOTH_HINT		= (unsigned int)0x0C53,
	GL_DONT_CARE			= (unsigned int)0x1100,
	GL_FASTEST			= (unsigned int)0x1101,
	GL_NICEST			= (unsigned int)0x1102,

	/* Scissor box */
	GL_SCISSOR_TEST			= (unsigned int)0x0C11,
	GL_SCISSOR_BOX			= (unsigned int)0x0C10,

	/* Pixel Mode / Transfer */
	GL_MAP_COLOR			= (unsigned int)0x0D10,
	GL_MAP_STENCIL			= (unsigned int)0x0D11,
	GL_INDEX_SHIFT			= (unsigned int)0x0D12,
	GL_INDEX_OFFSET			= (unsigned int)0x0D13,
	GL_RED_SCALE			= (unsigned int)0x0D14,
	GL_RED_BIAS			= (unsigned int)0x0D15,
	GL_GREEN_SCALE			= (unsigned int)0x0D18,
	GL_GREEN_BIAS			= (unsigned int)0x0D19,
	GL_BLUE_SCALE			= (unsigned int)0x0D1A,
	GL_BLUE_BIAS			= (unsigned int)0x0D1B,
	GL_ALPHA_SCALE			= (unsigned int)0x0D1C,
	GL_ALPHA_BIAS			= (unsigned int)0x0D1D,
	GL_DEPTH_SCALE			= (unsigned int)0x0D1E,
	GL_DEPTH_BIAS			= (unsigned int)0x0D1F,
	GL_PIXEL_MAP_S_TO_S_SIZE	= (unsigned int)0x0CB1,
	GL_PIXEL_MAP_I_TO_I_SIZE	= (unsigned int)0x0CB0,
	GL_PIXEL_MAP_I_TO_R_SIZE	= (unsigned int)0x0CB2,
	GL_PIXEL_MAP_I_TO_G_SIZE	= (unsigned int)0x0CB3,
	GL_PIXEL_MAP_I_TO_B_SIZE	= (unsigned int)0x0CB4,
	GL_PIXEL_MAP_I_TO_A_SIZE	= (unsigned int)0x0CB5,
	GL_PIXEL_MAP_R_TO_R_SIZE	= (unsigned int)0x0CB6,
	GL_PIXEL_MAP_G_TO_G_SIZE	= (unsigned int)0x0CB7,
	GL_PIXEL_MAP_B_TO_B_SIZE	= (unsigned int)0x0CB8,
	GL_PIXEL_MAP_A_TO_A_SIZE	= (unsigned int)0x0CB9,
	GL_PIXEL_MAP_S_TO_S		= (unsigned int)0x0C71,
	GL_PIXEL_MAP_I_TO_I		= (unsigned int)0x0C70,
	GL_PIXEL_MAP_I_TO_R		= (unsigned int)0x0C72,
	GL_PIXEL_MAP_I_TO_G		= (unsigned int)0x0C73,
	GL_PIXEL_MAP_I_TO_B		= (unsigned int)0x0C74,
	GL_PIXEL_MAP_I_TO_A		= (unsigned int)0x0C75,
	GL_PIXEL_MAP_R_TO_R		= (unsigned int)0x0C76,
	GL_PIXEL_MAP_G_TO_G		= (unsigned int)0x0C77,
	GL_PIXEL_MAP_B_TO_B		= (unsigned int)0x0C78,
	GL_PIXEL_MAP_A_TO_A		= (unsigned int)0x0C79,
	GL_PACK_ALIGNMENT		= (unsigned int)0x0D05,
	GL_PACK_LSB_FIRST		= (unsigned int)0x0D01,
	GL_PACK_ROW_LENGTH		= (unsigned int)0x0D02,
	GL_PACK_SKIP_PIXELS		= (unsigned int)0x0D04,
	GL_PACK_SKIP_ROWS		= (unsigned int)0x0D03,
	GL_PACK_SWAP_BYTES		= (unsigned int)0x0D00,
	GL_UNPACK_ALIGNMENT		= (unsigned int)0x0CF5,
	GL_UNPACK_LSB_FIRST		= (unsigned int)0x0CF1,
	GL_UNPACK_ROW_LENGTH		= (unsigned int)0x0CF2,
	GL_UNPACK_SKIP_PIXELS		= (unsigned int)0x0CF4,
	GL_UNPACK_SKIP_ROWS		= (unsigned int)0x0CF3,
	GL_UNPACK_SWAP_BYTES		= (unsigned int)0x0CF0,
	GL_ZOOM_X			= (unsigned int)0x0D16,
	GL_ZOOM_Y			= (unsigned int)0x0D17,

	/* Texture mapping */
	GL_TEXTURE_ENV			= (unsigned int)0x2300,
	GL_TEXTURE_ENV_MODE		= (unsigned int)0x2200,
	GL_TEXTURE_1D			= (unsigned int)0x0DE0,
	//GL_TEXTURE_2D			= (unsigned int)0x0DE1,
	//GL_TEXTURE_WRAP_S		= (unsigned int)0x2802,
	//GL_TEXTURE_WRAP_T		= (unsigned int)0x2803,
	GL_TEXTURE_MAG_FILTER		= (unsigned int)0x2800,
	GL_TEXTURE_MIN_FILTER		= (unsigned int)0x2801,
	GL_TEXTURE_ENV_COLOR		= (unsigned int)0x2201,
	GL_TEXTURE_GEN_S		= (unsigned int)0x0C60,
	GL_TEXTURE_GEN_T		= (unsigned int)0x0C61,
	GL_TEXTURE_GEN_MODE		= (unsigned int)0x2500,
	GL_TEXTURE_BORDER_COLOR		= (unsigned int)0x1004,
	GL_TEXTURE_WIDTH		= (unsigned int)0x1000,
	GL_TEXTURE_HEIGHT		= (unsigned int)0x1001,
	GL_TEXTURE_BORDER		= (unsigned int)0x1005,
	GL_TEXTURE_COMPONENTS		= (unsigned int)0x1003,
	GL_NEAREST_MIPMAP_NEAREST	= (unsigned int)0x2700,
	GL_NEAREST_MIPMAP_LINEAR	= (unsigned int)0x2702,
	GL_LINEAR_MIPMAP_NEAREST	= (unsigned int)0x2701,
	GL_LINEAR_MIPMAP_LINEAR		= (unsigned int)0x2703,
	GL_OBJECT_LINEAR		= (unsigned int)0x2401,
	GL_OBJECT_PLANE			= (unsigned int)0x2501,
	GL_EYE_LINEAR			= (unsigned int)0x2400,
	GL_EYE_PLANE			= (unsigned int)0x2502,
	GL_SPHERE_MAP			= (unsigned int)0x2402,
	GL_DECAL			= (unsigned int)0x2101,
	GL_MODULATE			= (unsigned int)0x2100,
	GL_NEAREST			= (unsigned int)0x2600,
	GL_REPEAT			= (unsigned int)0x2901,
	GL_CLAMP			= (unsigned int)0x2900,
	GL_CLAMP_TO_EDGE 	= (unsigned int)0x812F,
	GL_S				= (unsigned int)0x2000,
	GL_T				= (unsigned int)0x2001,
	GL_R				= (unsigned int)0x2002,
	GL_Q				= (unsigned int)0x2003,
	GL_TEXTURE_GEN_R		= (unsigned int)0x0C62,
	GL_TEXTURE_GEN_Q		= (unsigned int)0x0C63,

	GL_PROXY_TEXTURE_1D		= (unsigned int)0x8063,
	GL_PROXY_TEXTURE_2D		= (unsigned int)0x8064,
	GL_TEXTURE_PRIORITY		= (unsigned int)0x8066,
	GL_TEXTURE_RESIDENT		= (unsigned int)0x8067,
	GL_TEXTURE_1D_BINDING		= (unsigned int)0x8068,
	GL_TEXTURE_2D_BINDING		= (unsigned int)0x8069,

	/* Internal texture formats */
	GL_ALPHA4			= (unsigned int)0x803B,
	GL_ALPHA8			= (unsigned int)0x803C,
	GL_ALPHA12			= (unsigned int)0x803D,
	GL_ALPHA16			= (unsigned int)0x803E,
	GL_LUMINANCE4			= (unsigned int)0x803F,
	GL_LUMINANCE8			= (unsigned int)0x8040,
	GL_LUMINANCE12			= (unsigned int)0x8041,
	GL_LUMINANCE16			= (unsigned int)0x8042,
	GL_LUMINANCE4_ALPHA4		= (unsigned int)0x8043,
	GL_LUMINANCE6_ALPHA2		= (unsigned int)0x8044,
	GL_LUMINANCE8_ALPHA8		= (unsigned int)0x8045,
	GL_LUMINANCE12_ALPHA4		= (unsigned int)0x8046,
	GL_LUMINANCE12_ALPHA12		= (unsigned int)0x8047,
	GL_LUMINANCE16_ALPHA16		= (unsigned int)0x8048,
	GL_INTENSITY			= (unsigned int)0x8049,
	GL_INTENSITY4			= (unsigned int)0x804A,
	GL_INTENSITY8			= (unsigned int)0x804B,
	GL_INTENSITY12			= (unsigned int)0x804C,
	GL_INTENSITY16			= (unsigned int)0x804D,
	GL_R3_G3_B2			= (unsigned int)0x2A10,
	//GL_RGB4				= (unsigned int)0x804F,
	GL_RGB5				= (unsigned int)0x8050,
	GL_RGB8				= (unsigned int)0x8051,
	GL_RGB10			= (unsigned int)0x8052,
	GL_RGB12			= (unsigned int)0x8053,
	//GL_RGB16			= (unsigned int)0x8054,
	GL_RGBA2			= (unsigned int)0x8055,
	GL_RGBA4			= (unsigned int)0x8056,
	GL_RGB5_A1			= (unsigned int)0x8057,
	GL_RGBA8			= (unsigned int)0x8058,
	GL_RGB10_A2			= (unsigned int)0x8059,
	GL_RGBA12			= (unsigned int)0x805A,
	GL_RGBA16			= (unsigned int)0x805B,

	/* Utility */
	GL_VENDOR			= (unsigned int)0x1F00,
	GL_RENDERER			= (unsigned int)0x1F01,
	GL_VERSION			= (unsigned int)0x1F02,
	GL_EXTENSIONS			= (unsigned int)0x1F03,

	/* Errors */
	GL_INVALID_VALUE		= (unsigned int)0x0501,
	GL_INVALID_ENUM			= (unsigned int)0x0500,
	GL_INVALID_OPERATION		= (unsigned int)0x0502,
	GL_STACK_OVERFLOW		= (unsigned int)0x0503,
	GL_STACK_UNDERFLOW		= (unsigned int)0x0504,
	GL_OUT_OF_MEMORY		= (unsigned int)0x0505,

	/*
	 * 1.0 Extensions
	 */
        /* GL_EXT_blend_minmax and GL_EXT_blend_color */
	GL_CONSTANT_COLOR_EXT		= (unsigned int)0x8001,
	GL_ONE_MINUS_CONSTANT_COLOR_EXT	= (unsigned int)0x8002,
	GL_CONSTANT_ALPHA_EXT		= (unsigned int)0x8003,
	GL_ONE_MINUS_CONSTANT_ALPHA_EXT	= (unsigned int)0x8004,
	GL_BLEND_EQUATION_EXT		= (unsigned int)0x8009,
	GL_MIN_EXT			= (unsigned int)0x8007,
	GL_MAX_EXT			= (unsigned int)0x8008,
	GL_FUNC_ADD_EXT			= (unsigned int)0x8006,
	GL_FUNC_SUBTRACT_EXT		= (unsigned int)0x800A,
	GL_FUNC_REVERSE_SUBTRACT_EXT	= (unsigned int)0x800B,
	GL_BLEND_COLOR_EXT		= (unsigned int)0x8005,

	/* GL_EXT_polygon_offset */
        GL_POLYGON_OFFSET_EXT           = (unsigned int)0x8037,
        GL_POLYGON_OFFSET_FACTOR_EXT    = (unsigned int)0x8038,
        GL_POLYGON_OFFSET_BIAS_EXT      = (unsigned int)0x8039,

	/* GL_EXT_vertex_array */
	GL_VERTEX_ARRAY_EXT		= (unsigned int)0x8074,
	GL_NORMAL_ARRAY_EXT		= (unsigned int)0x8075,
	GL_COLOR_ARRAY_EXT		= (unsigned int)0x8076,
	GL_INDEX_ARRAY_EXT		= (unsigned int)0x8077,
	GL_TEXTURE_COORD_ARRAY_EXT	= (unsigned int)0x8078,
	GL_EDGE_FLAG_ARRAY_EXT		= (unsigned int)0x8079,
	GL_VERTEX_ARRAY_SIZE_EXT	= (unsigned int)0x807A,
	GL_VERTEX_ARRAY_TYPE_EXT	= (unsigned int)0x807B,
	GL_VERTEX_ARRAY_STRIDE_EXT	= (unsigned int)0x807C,
	GL_VERTEX_ARRAY_COUNT_EXT	= (unsigned int)0x807D,
	GL_NORMAL_ARRAY_TYPE_EXT	= (unsigned int)0x807E,
	GL_NORMAL_ARRAY_STRIDE_EXT	= (unsigned int)0x807F,
	GL_NORMAL_ARRAY_COUNT_EXT	= (unsigned int)0x8080,
	GL_COLOR_ARRAY_SIZE_EXT		= (unsigned int)0x8081,
	GL_COLOR_ARRAY_TYPE_EXT		= (unsigned int)0x8082,
	GL_COLOR_ARRAY_STRIDE_EXT	= (unsigned int)0x8083,
	GL_COLOR_ARRAY_COUNT_EXT	= (unsigned int)0x8084,
	GL_INDEX_ARRAY_TYPE_EXT		= (unsigned int)0x8085,
	GL_INDEX_ARRAY_STRIDE_EXT	= (unsigned int)0x8086,
	GL_INDEX_ARRAY_COUNT_EXT	= (unsigned int)0x8087,
	GL_TEXTURE_COORD_ARRAY_SIZE_EXT	= (unsigned int)0x8088,
	GL_TEXTURE_COORD_ARRAY_TYPE_EXT	= (unsigned int)0x8089,
	GL_TEXTURE_COORD_ARRAY_STRIDE_EXT= (unsigned int)0x808A,
	GL_TEXTURE_COORD_ARRAY_COUNT_EXT= (unsigned int)0x808B,
	GL_EDGE_FLAG_ARRAY_STRIDE_EXT	= (unsigned int)0x808C,
	GL_EDGE_FLAG_ARRAY_COUNT_EXT	= (unsigned int)0x808D,
	GL_VERTEX_ARRAY_POINTER_EXT	= (unsigned int)0x808E,
	GL_NORMAL_ARRAY_POINTER_EXT	= (unsigned int)0x808F,
	GL_COLOR_ARRAY_POINTER_EXT	= (unsigned int)0x8090,
	GL_INDEX_ARRAY_POINTER_EXT	= (unsigned int)0x8091,
	GL_TEXTURE_COORD_ARRAY_POINTER_EXT= (unsigned int)0x8092,
	GL_EDGE_FLAG_ARRAY_POINTER_EXT	= (unsigned int)0x8093

};

enum {
	GL_CURRENT_BIT		= (unsigned int)0x00000001,
	GL_POINT_BIT		= (unsigned int)0x00000002,
	GL_LINE_BIT		= (unsigned int)0x00000004,
	GL_POLYGON_BIT		= (unsigned int)0x00000008,
	GL_POLYGON_STIPPLE_BIT	= (unsigned int)0x00000010,
	GL_PIXEL_MODE_BIT	= (unsigned int)0x00000020,
	GL_LIGHTING_BIT		= (unsigned int)0x00000040,
	GL_FOG_BIT		= (unsigned int)0x00000080,
	GL_DEPTH_BUFFER_BIT	= (unsigned int)0x00000100,
	GL_ACCUM_BUFFER_BIT	= (unsigned int)0x00000200,
	GL_STENCIL_BUFFER_BIT	= (unsigned int)0x00000400,
	GL_VIEWPORT_BIT		= (unsigned int)0x00000800,
	GL_TRANSFORM_BIT	= (unsigned int)0x00001000,
	GL_ENABLE_BIT		= (unsigned int)0x00002000,
	GL_COLOR_BUFFER_BIT	= (unsigned int)0x00004000,
	GL_HINT_BIT		= (unsigned int)0x00008000,
	GL_EVAL_BIT		= (unsigned int)0x00010000,
	GL_LIST_BIT		= (unsigned int)0x00020000,
	GL_TEXTURE_BIT		= (unsigned int)0x00040000,
	GL_SCISSOR_BIT		= (unsigned int)0x00080000,
	GL_ALL_ATTRIB_BITS	= (unsigned int)0x000fffff
};

#define PHYS_GXFIFO_INTERNAL_SIZE ((int)512)

//Max GL Lists allocated in the OpenGL API
#define InternalUnpackedGX_DL_internalSize ((int)8192) //Max internal DL unpacked GX command/arguments count: 8192*4 = 32768 bytes. first 4K half: Standard OpenGL commands ; Second 4K half: OpenGL Display List commands
#define InternalUnpackedGX_DL_workSize	(InternalUnpackedGX_DL_internalSize/2) 
#define InternalUnpackedGX_DL_StandardOpenGLStartOffset (InternalUnpackedGX_DL_workSize * 0)
#define InternalUnpackedGX_DL_OpenGLDisplayListStartOffset (InternalUnpackedGX_DL_workSize * 1)

//Display List Descriptor
#define DL_INVALID (u32)(-1)
#define DL_TYPE_SIZE (u32)(1)
#define DL_TYPE_FIFO_PACKED_COMMAND_V1 (u32)(DL_TYPE_SIZE+1)	//FIFO_COMMAND_PACK( FIFO_BEGIN , FIFO_COLOR , FIFO_TEX_COORD , FIFO_NORMAL )
#define DL_TYPE_FIFO_PACKED_COMMAND_V2 (u32)(DL_TYPE_FIFO_PACKED_COMMAND_V1+1)	//FIFO_COMMAND_PACK( FIFO_VERTEX16 , FIFO_COLOR , FIFO_TEX_COORD , FIFO_NORMAL )
#define DL_TYPE_FIFO_PACKED_COMMAND_END (u32)(DL_TYPE_FIFO_PACKED_COMMAND_V2+1)	//FIFO_COMMAND_PACK( FIFO_VERTEX16 , FIFO_END , FIFO_NOP , FIFO_NOP )
#define GX_TOP_PARAMS_SIZE (int)(32)	//32  SHININESS - Specular Reflection Shininess Table (W) -- would be the command having the most parameter count
#define DL_DESCRIPTOR_MAX_ITEMS (int)(256) //NDS GX commands descriptor format, which is not compiled. Also OpenGL Display Lists limit size

struct ndsDisplayList {
	int index;
	u32 displayListType; //Display List Descriptor: FIFO_BEGIN, FIFO_COLOR, FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_END, FIFO_NOP, etc... 
	u32 value; //RGB15(31,31,31), TEXTURE_PACK(floattot16(0.000000),floattot16(128.000000)), NORMAL_PACK(floattov10(0.577349),floattov10(0.577349),floattov10(-0.577349)), VERTEX_PACK(floattov16(1.000000),floattov16(1.000000)) , VERTEX_PACK(floattov16(-1.000000),0), FIFO_END (no value after), FIFO_NOP (no value)
} 
#ifdef ARM9
__attribute__((packed)) ;
#endif
#ifdef WIN32
;
#endif

struct ndsDisplayListDescriptor {
	int DisplayListNameAssigned; //Used by the GL List API as a display-list name
	bool isDisplayListAssigned;
	int ndsDisplayListSize;
	struct ndsDisplayList DL[DL_DESCRIPTOR_MAX_ITEMS];
}
#ifdef ARM9
__attribute__((packed)) ;
#endif
#ifdef WIN32
;
#endif

//////////////////////////////////////////////////////////// Extended Display List OpenGL 1.1 start //////////////////////////////////////////

#define MAX_TGDSOGL_DisplayListContexts ((int)2)
#define TGDSOGL_DisplayListContext_Internal ((int)0) //VBO & VBA
#define TGDSOGL_DisplayListContext_External ((int)1) //OpenGL API usermode

struct TGDSOGL_DisplayListContext {
	//Internal Unpacked GX buffer
	GLsizei InternalUnpackedGX_DL_Binary_Enumerator[InternalUnpackedGX_DL_workSize/sizeof(GLsizei)];
	u32 LastGXInternalDisplayListPtr;
	u32 LastActiveOpenGLDisplayList;
	u32 InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_internalSize];
	u32 InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	bool isAnOpenGLExtendedDisplayListCallList;
	u32	mode; //GLenum mode: //Specifies the compilation mode, which can be GL_COMPILE or GL_COMPILE_AND_EXECUTE. Set up by glNewList()
};

#define INTERNAL_TGDS_OGL_DL_POINTER ((struct TGDSOGL_DisplayListContext *)&TGDSOGL_DisplayListContextInst[TGDSOGL_DisplayListContext_Internal])
#define USERSPACE_TGDS_OGL_DL_POINTER ((struct TGDSOGL_DisplayListContext *)&TGDSOGL_DisplayListContextInst[TGDSOGL_DisplayListContext_External])

#ifdef ARM9
#ifdef __cplusplus
extern "C" {
#endif
#endif

extern struct TGDSOGL_DisplayListContext TGDSOGL_DisplayListContextInst[MAX_TGDSOGL_DisplayListContexts];
extern u32 * getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(struct TGDSOGL_DisplayListContext * Inst); 

//Scratchpad GX buffer
extern u32 SingleUnpackedGXCommand_DL_Binary[PHYS_GXFIFO_INTERNAL_SIZE];

//Note: TGDS OpenGL usermode apps use always the identifier:
//struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext = (struct TGDSOGL_DisplayListContext *)&TGDSOGL_DisplayListContextInst[TGDSOGL_DisplayListContext_External];
//.................
//.................
//As opposed to internal VBA & VBO which will use always the identifier:
//struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext = (struct TGDSOGL_DisplayListContext *)&TGDSOGL_DisplayListContextInst[TGDSOGL_DisplayListContext_Internal];
//.................
//.................
extern GLuint glGenLists(GLsizei range, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glListBase(GLuint base, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern GLboolean glIsList(GLuint list, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glNewList(GLuint list, GLenum mode, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glEndList(struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glCallList(GLuint list, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glCallLists(GLsizei n, GLenum type, const void * lists, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glDeleteLists(GLuint list, GLsizei range, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern enum GL_GLBEGIN_ENUM getDisplayListGLType(struct ndsDisplayListDescriptor * dlInst);
extern int CompilePackedNDSGXDisplayListFromObject(u32 * bufOut, struct ndsDisplayListDescriptor * dlInst);
extern void glNormal3dv(const GLdouble *v, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glVertex3dv(const GLdouble *v, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void emitGLShinnyness(float shinyValue, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void  glMultMatrixf(const GLfloat *m, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void  glMultMatrixd(const GLdouble *m, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);
extern void glScalef(
   GLfloat x,
   GLfloat y,
   GLfloat z,
   struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext
);
extern void glScaled(
   GLdouble x,
   GLdouble y,
   GLdouble z,
   struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext
);

extern void glScalev(GLvector* v, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext);

//doesn't rely on struct TGDSOGL_DisplayListContext
extern void glGetFloatv(
   GLenum pname, 
   GLfloat *params
);

//doesn't rely on struct TGDSOGL_DisplayListContext
extern void glGetDoublev(
   GLenum   pname,
   GLdouble *params
);

extern void glCallListGX(const u32* list);

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
inline
#endif
static void handleInmediateGXDisplayList(u32 * sourcePhysDisplayList, u32 * sourcePhysDisplayListPtr, u8 cmdSource, int alternateParamsCount, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	if(TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList == false){ //Only run Standard Open GL calls. Extended OpenGL DL CallLists are ran from standard-specific CallList() opcodes
		//Identify cmdSource, if not exists, use alternateParamsCount instead (cmd(s) + arg(s) count)
		int cmdCount = (alternateParamsCount*4);
		if(cmdCount > 0){
			//substract cmdCount from current sourcePhysDisplayListPtr, get buffer src and copy to target buffer, by cmdCount count, and run
			*sourcePhysDisplayListPtr = (*sourcePhysDisplayListPtr - (cmdCount/4)); //Rewind cmd + arg list to use then restore the original DL offset so it's reentrant in order to prevents overflow.
			if(cmdCount > PHYS_GXFIFO_INTERNAL_SIZE){
				cmdCount = PHYS_GXFIFO_INTERNAL_SIZE;
			}
			memset(SingleUnpackedGXCommand_DL_Binary, 0, PHYS_GXFIFO_INTERNAL_SIZE);
			SingleUnpackedGXCommand_DL_Binary[0] = (u32)cmdCount;
			memcpy((u8*)&SingleUnpackedGXCommand_DL_Binary[1], (u8*)&sourcePhysDisplayList[*sourcePhysDisplayListPtr], cmdCount);

			#ifdef WIN32
			if(cmdSource == MTX_ROTATE_Z){
				printf("\n\n\n\n\n!!!!!![CUSTOM COMMAND: MTX_ROTATE_Z]!!!!!!\n");
			}
			else if(cmdSource == MTX_ROTATE_Y){
				printf("\n\n\n\n\n!!!!!![CUSTOM COMMAND: MTX_ROTATE_Y]!!!!!!\n");
			}
			else if(cmdSource == MTX_ROTATE_X){
				printf("\n\n\n\n\n!!!!!![CUSTOM COMMAND: MTX_ROTATE_X]!!!!!!\n");
			}
			else if(cmdSource == MTX_FRUSTRUM){
				printf("\n\n\n\n\n!!!!!![CUSTOM COMMAND: MTX_FRUSTRUM]!!!!!!\n");
			}
			else if(cmdSource == MTX_LOOKAT){
				printf("\n\n\n\n\n!!!!!![CUSTOM COMMAND: MTX_LOOKAT]!!!!!!\n");
			}
			else if(cmdSource == OPENGL_DL_TO_GX_DL_EXEC_CMD){
				printf("\n\n\n\n\n!!!!!![CUSTOM COMMAND: OPENGL_DL_TO_GX_DL_EXEC_CMD]!!!!!!\n");
			}
			#endif
			//Hardware CallList
			glCallListGX((const u32*)&SingleUnpackedGXCommand_DL_Binary[0]);
			
			//Emulated CallList (slow, debugging purposes)
			/*
			u32 * currCmd = &SingleUnpackedGXCommand_DL_Binary[1];
			int leftArgCnt = (cmdCount/4); // -1 is removed command itself from the arg list count 
			while(leftArgCnt > 0){
				u8 val = (u8)*currCmd;
				if (val == (u32)getMTX_STORE) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_STORE = arg1;
					#endif
					leftArgCnt-= MTX_STORE_GXCommandParamsCount == 0 ? 1 : MTX_STORE_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_TRANS) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					u32 arg2 = *currCmd; currCmd++;
					u32 arg3 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_TRANSLATE = arg1;
					MATRIX_TRANSLATE = arg2;
					MATRIX_TRANSLATE = arg3;
					#endif
					leftArgCnt-= MTX_TRANS_GXCommandParamsCount == 0 ? 1 : MTX_TRANS_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_IDENTITY) {
					//write commands
					currCmd++; 
					#ifdef ARM9
					MATRIX_IDENTITY = 0;
					#endif
					leftArgCnt-= MTX_IDENTITY_GXCommandParamsCount == 0 ? 1 : MTX_IDENTITY_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_MODE) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_CONTROL = arg1;
					#endif
					leftArgCnt-= MTX_MODE_GXCommandParamsCount == 0 ? 1 : MTX_MODE_GXCommandParamsCount;
				}
				else if (val == (u32)getVIEWPORT) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_VIEWPORT = arg1;
					#endif
					leftArgCnt-= VIEWPORT_GXCommandParamsCount == 0 ? 1 : VIEWPORT_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_TEX_COORD) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_TEX_COORD = arg1;
					#endif
					leftArgCnt-= FIFO_TEX_COORD_GXCommandParamsCount == 0 ? 1 : FIFO_TEX_COORD_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_BEGIN) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_BEGIN = arg1;
					#endif
					leftArgCnt-= FIFO_BEGIN_GXCommandParamsCount == 0 ? 1 : FIFO_BEGIN_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_END) {
					//write commands
					currCmd++; 
					#ifdef ARM9
					GFX_END = 0;
					#endif
					leftArgCnt-= FIFO_END_GXCommandParamsCount == 0 ? 1 : FIFO_END_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_COLOR) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_COLOR = arg1;
					#endif
					leftArgCnt-= FIFO_COLOR_GXCommandParamsCount == 0 ? 1 : FIFO_COLOR_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_NORMAL) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_NORMAL = arg1;
					#endif
					leftArgCnt-= FIFO_NORMAL_GXCommandParamsCount == 0 ? 1 : FIFO_NORMAL_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_VERTEX16) { 
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					u32 arg2 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_VERTEX16 = arg1;
					GFX_VERTEX16 = arg2;
					#endif
					leftArgCnt-= FIFO_VERTEX16_GXCommandParamsCount == 0 ? 1 : FIFO_VERTEX16_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_VERTEX10) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_VERTEX10 = arg1;
					#endif
					leftArgCnt-= FIFO_VERTEX10_GXCommandParamsCount == 0 ? 1 : FIFO_VERTEX10_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_VTX_XY()) { 
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_VERTEX_XY = arg1;
					#endif
					leftArgCnt-= FIFO_VTX_XY_GXCommandParamsCount == 0 ? 1 : FIFO_VTX_XY_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_PUSH) { 
					//write commands
					currCmd++; 
					#ifdef ARM9
					MATRIX_PUSH = 0;
					#endif
					leftArgCnt-= MTX_PUSH_GXCommandParamsCount == 0 ? 1 : MTX_PUSH_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_POP) { 
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_POP = arg1;
					#endif
					leftArgCnt-= MTX_POP_GXCommandParamsCount == 0 ? 1 : MTX_POP_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_MULT_3x3) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					u32 arg2 = *currCmd; currCmd++;
					u32 arg3 = *currCmd; currCmd++;
					u32 arg4 = *currCmd; currCmd++;
					u32 arg5 = *currCmd; currCmd++;
					u32 arg6 = *currCmd; currCmd++;
					u32 arg7 = *currCmd; currCmd++;
					u32 arg8 = *currCmd; currCmd++;
					u32 arg9 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_MULT3x3 = arg1;
					MATRIX_MULT3x3 = arg2;
					MATRIX_MULT3x3 = arg3;
					MATRIX_MULT3x3 = arg4;
					MATRIX_MULT3x3 = arg5;
					MATRIX_MULT3x3 = arg6;
					MATRIX_MULT3x3 = arg7;
					MATRIX_MULT3x3 = arg8;
					MATRIX_MULT3x3 = arg9;
					#endif
					leftArgCnt-= MTX_MULT_3x3_GXCommandParamsCount == 0 ? 1 : MTX_MULT_3x3_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_MULT_4x4) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					u32 arg2 = *currCmd; currCmd++;
					u32 arg3 = *currCmd; currCmd++;
					u32 arg4 = *currCmd; currCmd++;
					u32 arg5 = *currCmd; currCmd++;
					u32 arg6 = *currCmd; currCmd++;
					u32 arg7 = *currCmd; currCmd++;
					u32 arg8 = *currCmd; currCmd++;
					u32 arg9 = *currCmd; currCmd++;
					u32 arg10 = *currCmd; currCmd++;
					u32 arg11 = *currCmd; currCmd++;
					u32 arg12 = *currCmd; currCmd++;
					u32 arg13 = *currCmd; currCmd++;
					u32 arg14 = *currCmd; currCmd++;
					u32 arg15 = *currCmd; currCmd++;
					u32 arg16 = *currCmd; currCmd++;
					
					#ifdef ARM9
					MATRIX_MULT4x4 = arg1;
					MATRIX_MULT4x4 = arg2;
					MATRIX_MULT4x4 = arg3;
					MATRIX_MULT4x4 = arg4;
					MATRIX_MULT4x4 = arg5;
					MATRIX_MULT4x4 = arg6;
					MATRIX_MULT4x4 = arg7;
					MATRIX_MULT4x4 = arg8;
					MATRIX_MULT4x4 = arg9;
					MATRIX_MULT4x4 = arg10;
					MATRIX_MULT4x4 = arg11;
					MATRIX_MULT4x4 = arg12;
					MATRIX_MULT4x4 = arg13;
					MATRIX_MULT4x4 = arg14;
					MATRIX_MULT4x4 = arg15;
					MATRIX_MULT4x4 = arg16;
					#endif
					leftArgCnt-= MTX_MULT_4x4_GXCommandParamsCount == 0 ? 1 : MTX_MULT_4x4_GXCommandParamsCount;
				}

				else if (val == (u32)getMTX_LOAD_4x4) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					u32 arg2 = *currCmd; currCmd++;
					u32 arg3 = *currCmd; currCmd++;
					u32 arg4 = *currCmd; currCmd++;
					u32 arg5 = *currCmd; currCmd++;
					u32 arg6 = *currCmd; currCmd++;
					u32 arg7 = *currCmd; currCmd++;
					u32 arg8 = *currCmd; currCmd++;
					u32 arg9 = *currCmd; currCmd++;
					u32 arg10 = *currCmd; currCmd++;
					u32 arg11 = *currCmd; currCmd++;
					u32 arg12 = *currCmd; currCmd++;
					u32 arg13 = *currCmd; currCmd++;
					u32 arg14 = *currCmd; currCmd++;
					u32 arg15 = *currCmd; currCmd++;
					u32 arg16 = *currCmd; currCmd++;
					
					#ifdef ARM9
					MATRIX_LOAD4x4 = arg1;     
					MATRIX_LOAD4x4 = arg2;  
					MATRIX_LOAD4x4 = arg3;      
					MATRIX_LOAD4x4 = arg4;

					MATRIX_LOAD4x4 = arg5;  
					MATRIX_LOAD4x4 = arg6;     
					MATRIX_LOAD4x4 = arg7;      
					MATRIX_LOAD4x4 = arg8;
		
					MATRIX_LOAD4x4 = arg9;  
					MATRIX_LOAD4x4 = arg10;  
					MATRIX_LOAD4x4 = arg11;     
					MATRIX_LOAD4x4 = arg12;
		
					MATRIX_LOAD4x4 = arg13;  
					MATRIX_LOAD4x4 = arg14;  
					MATRIX_LOAD4x4 = arg15;  
					MATRIX_LOAD4x4 = arg16;
					#endif
					leftArgCnt-= MTX_LOAD_4x4_GXCommandParamsCount == 0 ? 1 : MTX_LOAD_4x4_GXCommandParamsCount;
				}

				else if (val == (u32)getMTX_LOAD_4x3) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					u32 arg2 = *currCmd; currCmd++;
					u32 arg3 = *currCmd; currCmd++;
					u32 arg4 = *currCmd; currCmd++;
					u32 arg5 = *currCmd; currCmd++;
					u32 arg6 = *currCmd; currCmd++;
					u32 arg7 = *currCmd; currCmd++;
					u32 arg8 = *currCmd; currCmd++;
					u32 arg9 = *currCmd; currCmd++;
					u32 arg10 = *currCmd; currCmd++;
					u32 arg11 = *currCmd; currCmd++;
					u32 arg12 = *currCmd; currCmd++;
					
					#ifdef ARM9
					MATRIX_LOAD4x3 = arg1;
					MATRIX_LOAD4x3 = arg2;
					MATRIX_LOAD4x3 = arg3;

					MATRIX_LOAD4x3 = arg4;
					MATRIX_LOAD4x3 = arg5;
					MATRIX_LOAD4x3 = arg6;

					MATRIX_LOAD4x3 = arg7;
					MATRIX_LOAD4x3 = arg8;
					MATRIX_LOAD4x3 = arg9;

					MATRIX_LOAD4x3 = arg10;
					MATRIX_LOAD4x3 = arg11;
					MATRIX_LOAD4x3 = arg12;
					#endif
					leftArgCnt-= MTX_LOAD_4x3_GXCommandParamsCount == 0 ? 1 : MTX_LOAD_4x3_GXCommandParamsCount;
				}

				//N/A      00h -  -   NOP - No Operation (for padding packed GXFIFO commands)
				else  {  //if (val == (u32)getNOP())
					//write commands
					currCmd++; 
					leftArgCnt-= (NOP_GXCommandParamsCount == 0 ? 1 : NOP_GXCommandParamsCount);
					//custom command invoked this, so quit
					//break;
				}
			}
			*/
		}
	}
}
//////////////////////////////////////////////////////////// Extended Display List OpenGL 1.1 end //////////////////////////////////////////









//////////////////////////////////////////////////////////// Extended Vertex Array Buffers and Vertex Buffer Objects OpenGL 1.1 start //////////////////////////////////////////
#define MAX_VBO_PER_VBA ((int)6) //Standard defines: vertex, normal, color, index, textureCoords, edgeFlag
#define OBJECT_BUFFER_VERTEX ((int)0) //vertex
#define OBJECT_BUFFER_NORMAL ((int)1) //normal
#define OBJECT_BUFFER_COLOR ((int)2)  //color
#define OBJECT_BUFFER_INDEX ((int)3)  //index
#define OBJECT_BUFFER_TEXCOORD ((int)4)  //texture coordinate
#define OBJECT_BUFFER_EDGEFLAG ((int)5)  //Edge Flag

#define VBO_ARRAY_SIZE ((int)256) //up to 256 elements

// Buffer Objects 
#define GL_ARRAY_BUFFER                   ((GLenum)0x8892)
#define GL_ELEMENT_ARRAY_BUFFER           ((GLenum)0x8893)
#define GL_ARRAY_BUFFER_BINDING           ((GLenum)0x8894)
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   ((GLenum)0x8895)
#define GL_STREAM_DRAW                    ((GLenum)0x88E0)
#define GL_STATIC_DRAW                    ((GLenum)0x88E4)
#define GL_DYNAMIC_DRAW                   ((GLenum)0x88E8)
#define GL_BUFFER_SIZE                    ((GLenum)0x8764)
#define GL_BUFFER_USAGE                   ((GLenum)0x8765)
#define GL_CURRENT_VERTEX_ATTRIB          ((GLenum)0x8626)

#define GL_STATIC_READ                    ((GLenum)0x88E5)
#define GL_STATIC_COPY                    ((GLenum)0x88E6)
#define GL_DYNAMIC_READ                   ((GLenum)0x88E9)
#define GL_DYNAMIC_COPY                   ((GLenum)0x88EA)
#define GL_STREAM_READ                    ((GLenum)0x88E1)
#define GL_STREAM_COPY                    ((GLenum)0x88E2)

//VBO: 
struct vertexBufferObject {
  u32 * vboArrayMemoryStart; //equals to = malloc(VBO_ARRAY_SIZE); / ptr to free later
  bool vboIsDynamicMemoryAllocated;
  int ElementsPerVertexBufferObjectUnit; //unit size per VBO entry in array
  int vertexBufferObjectstrideOffset; //stride / offset per VBO defaults
  int VertexBufferObjectStartOffset; //offset to start buffer from

  //A normal array is enabled when you specify the GL_NORMAL_ARRAY constant with glEnableClientState. When enabled, glDrawArrays, glDrawElements and glArrayElement use the selected array. By default the selected array is disabled. Also multiple arrays can be enabled / disabled at the same time
  bool ClientStateEnabled; //each VBA has an descriptor state, toggled at  glEnableClientState and glDisableClientState, and used by subsequent calls

  //cached last DL precompiled to see if new buffer updates are required
  u16 lastPrebuiltDLCRC16;
};

//VBA
struct vertexBufferArray {
	struct vertexBufferObject vertexBufferObjectInst[MAX_VBO_PER_VBA]; 
};

//pointers of implemented VBOs
extern struct vertexBufferObject * vboVertex;
extern struct vertexBufferObject * vboNormal;
extern struct vertexBufferObject * vboColor;
extern struct vertexBufferObject * vboIndex;
extern struct vertexBufferObject * vboTexCoord;
extern struct vertexBufferObject * vboEdgeFlag;

extern bool InitGLOnlyOnce;
extern GLenum errorStatus;
extern GLenum glGetError();

//VBO
extern void glGenBuffers(GLsizei n, GLuint* ids);
extern void glBindBuffer(GLenum target, GLuint id);
extern void glBufferData(GLenum target, GLsizei size, const void* data, GLenum usage);

extern void glBufferSubData(GLenum target, GLint offset, GLsizei size, void* data);
extern void glDeleteBuffers(GLsizei n, const GLuint* ids);

extern void glEnableClientState(GLenum arr);
extern void glDisableClientState(GLenum arr);


//VBA
extern struct vertexBufferArray TGDSVBAInstance;
extern void glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );
extern void glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr );
extern void glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );
extern void glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr );
extern void glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );
extern void glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr );
extern void glGetPointerv( GLenum pname, GLvoid **params );
extern void glArrayElement( GLint i );
extern void glDrawArrays( GLenum mode, GLint first, GLsizei count );
extern void glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
extern void glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer );

extern int OGL_DL_DRAW_ARRAYS_METHOD;

//////////////////////////////////////////////////////////// Extended Vertex Array Buffers and Vertex Buffer Objects OpenGL 1.1 end //////////////////////////////////////////


extern uint32 diffuse_ambient;
extern uint32 specular_emission;
extern void glEnable(int bits);
extern void glDisable(int bits);
extern void glLoadMatrix4x4(m4x4 * m, struct TGDSOGL_DisplayListContext * Inst);
extern void glLoadMatrix4x3(m4x3 * m, struct TGDSOGL_DisplayListContext * Inst);
extern void glMultMatrix4x4(m4x4 * m, struct TGDSOGL_DisplayListContext * Inst);
extern void glMultMatrix4x3(m4x3 * m, struct TGDSOGL_DisplayListContext * Inst);
extern void glRotateXi(int angle, struct TGDSOGL_DisplayListContext * Inst);
extern void glRotateYi(int angle, struct TGDSOGL_DisplayListContext * Inst);
extern void glRotateZi(int angle, struct TGDSOGL_DisplayListContext * Inst);
extern void glRotateX(float angle, struct TGDSOGL_DisplayListContext * Inst);
extern void glRotateY(float angle, struct TGDSOGL_DisplayListContext * Inst);
extern void glRotateZ(float angle, struct TGDSOGL_DisplayListContext * Inst);
extern void gluLookAtf32(f32 eyex, f32 eyey, f32 eyez, f32 lookAtx, f32 lookAty, f32 lookAtz, f32 upx, f32 upy, f32 upz, struct TGDSOGL_DisplayListContext * Inst);
extern void gluLookAt(float eyex, float eyey, float eyez, float lookAtx, float lookAty, float lookAtz, float upx, float upy, float upz, struct TGDSOGL_DisplayListContext * Inst);
extern void gluFrustumf32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far, struct TGDSOGL_DisplayListContext * Inst);
extern void gluFrustum(float left, float right, float bottom, float top, float near, float far, struct TGDSOGL_DisplayListContext * Inst);
extern void gluPerspectivef32(int fovy, f32 aspect, f32 zNear, f32 zFar, struct TGDSOGL_DisplayListContext * Inst);
extern void gluPerspective(float fovy, float aspect, float zNear, float zFar, struct TGDSOGL_DisplayListContext * Inst);
extern int glTexImage2D(int target, int empty1, int type, int sizeX, int sizeY, int empty2, int param, uint8* texture);
extern void glBindTexture(int target, int name, struct TGDSOGL_DisplayListContext * Inst);
extern int glGenTextures(int n, int *names);
extern void glResetTextures(void);
extern void glMaterialf(int mode, rgb color, struct TGDSOGL_DisplayListContext * Inst);
extern void glResetMatrixStack(struct TGDSOGL_DisplayListContext * Inst);
extern void glSetOutlineColor(int id, rgb color);
extern void glSetToonTable(uint16 *table);
extern void glSetToonTableRange(int start, int end, rgb color);
extern void glReset(struct TGDSOGL_DisplayListContext * Inst);
extern void glBegin(int mode, struct TGDSOGL_DisplayListContext * Inst);
extern void glEnd(struct TGDSOGL_DisplayListContext * Inst);  
extern void glColor3b(uint8 red, uint8 green, uint8 blue, struct TGDSOGL_DisplayListContext * Inst);
extern void glPushMatrix(struct TGDSOGL_DisplayListContext * Inst);
extern void glPopMatrix(sint32 index, struct TGDSOGL_DisplayListContext * Inst);
extern void glRestoreMatrix(sint32 index, struct TGDSOGL_DisplayListContext * Inst);
extern void glStoreMatrix(sint32 index, struct TGDSOGL_DisplayListContext * Inst);
extern void glTranslatev(GLvector* v, struct TGDSOGL_DisplayListContext * Inst);
extern void glTranslate3f32(f32 x, f32 y, f32 z, struct TGDSOGL_DisplayListContext * Inst);
extern void glTranslatef32(int x, int y, int z, struct TGDSOGL_DisplayListContext * Inst);
extern void glLight(int id, rgb color, v10 x, v10 y, v10 z, struct TGDSOGL_DisplayListContext * Inst);
extern void glNormal(uint32 normal, struct TGDSOGL_DisplayListContext * Inst);
extern void glLoadIdentity(struct TGDSOGL_DisplayListContext * Inst);
extern void glMatrixMode(int mode, struct TGDSOGL_DisplayListContext * Inst); 
extern void glMaterialShinnyness(struct TGDSOGL_DisplayListContext * Inst);
extern void glPolyFmt(int alpha, struct TGDSOGL_DisplayListContext * Inst); 
extern void glRotatef(int angle, float x, float y, float z, struct TGDSOGL_DisplayListContext * Inst);
extern void glOrthof32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far, struct TGDSOGL_DisplayListContext * Inst);
extern void glOrtho(float left, float right, float bottom, float top, float near, float far, struct TGDSOGL_DisplayListContext * Inst);
extern void glColor3f(float red, float green, float blue, struct TGDSOGL_DisplayListContext * Inst);
extern void glColor3fv(const GLfloat * v, struct TGDSOGL_DisplayListContext * Inst);
extern struct GLContext globalGLCtx;
extern void glShadeModel(GLenum mode);
extern void glInit();
extern void glVertex2i(int x, int y, struct TGDSOGL_DisplayListContext * Inst); 
extern void glVertex2f(float x, float y, struct TGDSOGL_DisplayListContext * Inst);
extern void glVertex3f(GLfloat x, GLfloat y, GLfloat z, struct TGDSOGL_DisplayListContext * Inst);
extern void glVertex3i(GLint x, GLint y, GLint z, struct TGDSOGL_DisplayListContext * Inst);
extern void glTranslatef(float x, float y, float z, struct TGDSOGL_DisplayListContext * Inst);
extern void glFlush(struct TGDSOGL_DisplayListContext * Inst);
extern void glFinish(struct TGDSOGL_DisplayListContext * Inst);
extern u8 defaultglClearColorR;
extern u8 defaultglClearColorG;
extern u8 defaultglClearColorB;
extern u16 defaultglClearDepth;
extern void glClear( GLbitfield mask );
extern void glClearColor(uint8 red, uint8 green, uint8 blue);
extern void glClearDepth(uint16 depth);
extern void glViewport(uint8 x1, uint8 y1, uint8 x2, uint8 y2, struct TGDSOGL_DisplayListContext * Inst);
extern void glNormal3b(
	GLbyte nx,
 	GLbyte ny,
 	GLbyte nz,
	struct TGDSOGL_DisplayListContext * Inst
);
extern void glNormal3d(
	GLdouble nx,
 	GLdouble ny,
 	GLdouble nz,
	struct TGDSOGL_DisplayListContext * Inst
);
extern void glNormal3f(
	GLfloat nx,
 	GLfloat ny,
 	GLfloat nz,
	struct TGDSOGL_DisplayListContext * Inst
);
extern void glNormal3s(
	GLshort nx,
 	GLshort ny,
 	GLshort nz,
	struct TGDSOGL_DisplayListContext * Inst
);
extern void glNormal3i(
	GLint nx,
 	GLint ny,
 	GLint nz,
	struct TGDSOGL_DisplayListContext * Inst
);
extern void glBegin(int primitiveType, struct TGDSOGL_DisplayListContext * Inst);
extern void glEnd(struct TGDSOGL_DisplayListContext * Inst);
extern void glTexCoord2t16(t16 u, t16 v, struct TGDSOGL_DisplayListContext * Inst);
extern void glTexCoord2f(GLfloat s, GLfloat t, struct TGDSOGL_DisplayListContext * Inst);
extern void glTexCoord2i(GLint s, GLint t, struct TGDSOGL_DisplayListContext * Inst);
extern void glTexCoord1i(uint32 uv, struct TGDSOGL_DisplayListContext * Inst);
extern u16 lastVertexColor;
extern void glColor3b(uint8 red, uint8 green, uint8 blue, struct TGDSOGL_DisplayListContext * Inst);
extern void glVertex3v16(v16 x, v16 y, v16 z, struct TGDSOGL_DisplayListContext * Inst);
extern void glVertex3v10(v10 x, v10 y, v10 z, struct TGDSOGL_DisplayListContext * Inst);
extern void glVertex2v16(v16 x, v16 y, struct TGDSOGL_DisplayListContext * Inst);
extern int getTextureBaseFromTextureSlot(int textureSlot);
extern uint32 textures[MAX_TEXTURES];
extern uint32 activeTexture;
extern uint32* nextBlock;
extern void glLightfv (GLenum light, GLenum pname, const GLfloat *params, struct TGDSOGL_DisplayListContext * Inst);
extern void glMaterialfv (GLenum face, GLenum pname, const GLfloat *params, struct TGDSOGL_DisplayListContext * Inst);
extern void glNormal3fv(const GLfloat *v, struct TGDSOGL_DisplayListContext * Inst);
extern void glVertex3fv(const GLfloat *v, struct TGDSOGL_DisplayListContext * Inst);
extern void glTexParameteri(
	GLenum target,
	GLenum pname,
	GLint  param, 
	struct TGDSOGL_DisplayListContext * Inst
);

#ifdef ARM9
#ifdef __cplusplus
}
#endif
#endif

#endif
