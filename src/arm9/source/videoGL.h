/*---------------------------------------------------------------------------------
	videoGL.h -- Video API vaguely similar to OpenGL

	Copyright (C) 2005
		Michael Noland (joat)
		Jason Rogers (dovoto)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

	$Log: not supported by cvs2svn $
	Revision 1.7  2005/07
	Updated GL with float wrappers for NeHe
	

---------------------------------------------------------------------------------*/

#ifndef VIDEOGL_ARM9_INCLUDE
#define VIDEOGL_ARM9_INCLUDE

#undef NO_GL_INLINE
//---------------------------------------------------------------------------------

#ifndef ARM9
#error 3D hardware is only available from the ARM9
#endif

//---------------------------------------------------------------------------------
//#include <nds/jtypes.h>
//#include <nds/arm9/video.h>
//#include <nds/dma.h>

#include "typedefsTGDS.h"
#include "dmaTGDS.h"

typedef sint64                   int64;
typedef sint32                   int32;
typedef sint16                   int16;
typedef sint8                   int8;

typedef int32                   fixed;
typedef int64                   dfixed;
typedef volatile int32          vfixed;

/*---------------------------------------------------------------------------------
	lut resolution for trig functions
	(must be power of two and must be the same as LUT resolution)
	in other words dont change unless you also change your LUTs
---------------------------------------------------------------------------------*/

#define VRAM_0        ((uint16*)0x6000000)
#define VRAM          ((uint16*)0x6800000)
#define VRAM_A        ((uint16*)0x6800000)
#define VRAM_B        ((uint16*)0x6820000)
#define VRAM_C        ((uint16*)0x6840000)
#define VRAM_D        ((uint16*)0x6860000)
#define VRAM_E        ((uint16*)0x6880000)
#define VRAM_F        ((uint16*)0x6890000)
#define VRAM_G        ((uint16*)0x6894000)
#define VRAM_H        ((uint16*)0x6898000)
#define VRAM_I        ((uint16*)0x68A0000)

#define OAM           ((uint16*)0x07000000)
#define OAM_SUB       ((uint16*)0x07000400)

//////////////////////////////////////////////////////////////////////

// macro creates a 15 bit color from 3x5 bit components
#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))


#define SCREEN_HEIGHT 192
#define SCREEN_WIDTH  256
//////////////////////////////////////////////////////////////////////
//	Vram Control
#define VRAM_CR			(*(vuint32*)0x04000240)
#define VRAM_A_CR       (*(vuint8*)0x04000240)
#define VRAM_B_CR       (*(vuint8*)0x04000241)
#define VRAM_C_CR       (*(vuint8*)0x04000242)
#define VRAM_D_CR       (*(vuint8*)0x04000243)
#define VRAM_E_CR       (*(vuint8*)0x04000244)
#define VRAM_F_CR       (*(vuint8*)0x04000245)
#define VRAM_G_CR       (*(vuint8*)0x04000246)
#define WRAM_CR			(*(vuint8*)0x04000247)
#define VRAM_H_CR       (*(vuint8*)0x04000248)
#define VRAM_I_CR       (*(vuint8*)0x04000249)

#define VRAM_ENABLE   (1<<7)


#define VRAM_OFFSET(n)  ((n)<<3)

typedef enum
{
	VRAM_A_LCD = 0,
	VRAM_A_MAIN_BG  = 1,
	VRAM_A_MAIN_BG_0x6000000  = 1 | VRAM_OFFSET(0),
	VRAM_A_MAIN_BG_0x6020000  = 1 | VRAM_OFFSET(1),
	VRAM_A_MAIN_BG_0x6040000  = 1 | VRAM_OFFSET(2),
	VRAM_A_MAIN_BG_0x6060000  = 1 | VRAM_OFFSET(3),
	VRAM_A_MAIN_SPRITE = 2,
	VRAM_A_TEXTURE = 3,
	VRAM_A_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_A_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_A_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_A_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

}VRAM_A_TYPE;

typedef enum
{
	VRAM_B_LCD = 0,
	VRAM_B_MAIN_BG  = 1 | VRAM_OFFSET(1),
	VRAM_B_MAIN_BG_0x6000000  = 1 | VRAM_OFFSET(0),
	VRAM_B_MAIN_BG_0x6020000  = 1 | VRAM_OFFSET(1),
	VRAM_B_MAIN_BG_0x6040000  = 1 | VRAM_OFFSET(2),
	VRAM_B_MAIN_BG_0x6060000  = 1 | VRAM_OFFSET(3),
	VRAM_B_MAIN_SPRITE = 2,
	VRAM_B_TEXTURE = 3 | VRAM_OFFSET(1),
	VRAM_B_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_B_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_B_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_B_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

}VRAM_B_TYPE;	

typedef enum
{
	VRAM_C_LCD = 0,
	VRAM_C_MAIN_BG  = 1 | VRAM_OFFSET(2),
	VRAM_C_MAIN_BG_0x6000000  = 1 | VRAM_OFFSET(0),
	VRAM_C_MAIN_BG_0x6020000  = 1 | VRAM_OFFSET(1),
	VRAM_C_MAIN_BG_0x6040000  = 1 | VRAM_OFFSET(2),
	VRAM_C_MAIN_BG_0x6060000  = 1 | VRAM_OFFSET(3),
	VRAM_C_ARM7 = 2,
	VRAM_C_SUB_BG  = 4,
	VRAM_C_SUB_BG_0x6200000  = 4 | VRAM_OFFSET(0),
	VRAM_C_SUB_BG_0x6220000  = 4 | VRAM_OFFSET(1),
	VRAM_C_SUB_BG_0x6240000  = 4 | VRAM_OFFSET(2),
	VRAM_C_SUB_BG_0x6260000  = 4 | VRAM_OFFSET(3),
	VRAM_C_TEXTURE = 3 | VRAM_OFFSET(2),
	VRAM_C_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_C_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_C_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_C_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

}VRAM_C_TYPE;

typedef enum
{	
	VRAM_D_LCD = 0,
	VRAM_D_MAIN_BG  = 1 | VRAM_OFFSET(3),
	VRAM_D_MAIN_BG_0x6000000  = 1 | VRAM_OFFSET(0),
	VRAM_D_MAIN_BG_0x6020000  = 1 | VRAM_OFFSET(1),
	VRAM_D_MAIN_BG_0x6040000  = 1 | VRAM_OFFSET(2),
	VRAM_D_MAIN_BG_0x6060000  = 1 | VRAM_OFFSET(3),
	VRAM_D_ARM7 = 2,
	VRAM_D_SUB_SPRITE  = 4,
	VRAM_D_TEXTURE = 3 | VRAM_OFFSET(3),
	VRAM_D_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_D_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_D_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_D_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)
}VRAM_D_TYPE;

typedef enum
{
	VRAM_E_LCD			=0,
	VRAM_E_MAIN_BG  = 1,
	VRAM_E_MAIN_SPRITE = 2,
	VRAM_E_TEX_PALETTE = 3,
	VRAM_E_BG_EXT_PALETTE = 4,
	VRAM_E_OBJ_EXT_PALETTE = 5,

}VRAM_E_TYPE;

typedef enum
{
	VRAM_F_LCD			=0,
	VRAM_F_MAIN_BG  = 1,
	VRAM_F_MAIN_SPRITE = 2,
	VRAM_F_TEX_PALETTE = 3,
	VRAM_F_BG_EXT_PALETTE = 4,
	VRAM_F_OBJ_EXT_PALETTE = 5,

}VRAM_F_TYPE;

typedef enum
{
	VRAM_G_LCD			=0,
	VRAM_G_MAIN_BG  = 1,
	VRAM_G_MAIN_SPRITE = 2,
	VRAM_G_TEX_PALETTE = 3,
	VRAM_G_BG_EXT_PALETTE = 4,
	VRAM_G_OBJ_EXT_PALETTE = 5,

}VRAM_G_TYPE;

typedef enum
{
	VRAM_H_LCD			=0,
	VRAM_H_SUB_BG = 1,
	VRAM_H_SUB_BG_EXT_PALETTE = 2,

}VRAM_H_TYPE;

typedef enum
{
	VRAM_I_LCD			=0,
	VRAM_I_SUB_BG	= 1,
	VRAM_I_SUB_SPRITE = 2,
	VRAM_I_SUB_SPRITE_EXT_PALETTE = 3,

}VRAM_I_TYPE;
//////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// 3D core control
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

#define GFX_POLY_FORMAT       (*(vuint32*) 0x040004A4)
#define GFX_ALPHA             (*(vuint16*) 0x04000340)

#define GFX_BEGIN             (*(vuint32*) 0x04000500)
#define GFX_END               (*(vuint32*) 0x04000504)
#define GFX_FLUSH             (*(vuint32*) 0x04000540)
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

//////////////////////////////////////////////////////////////////////


#define LUT_SIZE (512)
#define LUT_MASK (0x1FF)

#define GLuint u32
#define GLfloat float


typedef int f32;             // 1.19.12 fixed point for matricies
#define intof32(n)           ((n) << 12)
#define f32toint(n)          ((n) >> 12)
#define floatof32(n)         ((f32)((n) * (1 << 12)))
#define f32tofloat(n)        (((float)(n)) / (float)(1<<12))

typedef short int t16;       // text coordinate 1.11.4 fixed point

#define f32tot16(n)             ((t16)(n >> 8))
#define intot16(n)           ((n) << 4)
#define t16toint(n)          ((n) >> 4)
#define floatot16(n)         ((t16)((n) * (1 << 4)))
#define TEXTURE_PACK(u,v)    (((u) << 16) | (v & 0xFFFF))

typedef short int v16;       // vertex 1.3.12 fixed format
#define intov16(n)           ((n) << 12)
#define f32tov16(n)             (n)
#define v16toint(n)          ((n) >> 12)
#define floatov16(n)         ((v16)((n) * (1 << 12)))
#define VERTEX_PACK(x,y)		(((y) << 16) | ((x) & 0xFFFF))


typedef short int v10;       // vertex 1.0.9 fixed point
#define intov10(n)           ((n) << 9)
#define f32tov10(n)          ((v10)(n >> 3))
#define v10toint(n)          ((n) >> 9)
#define floatov10(n)         ((v10)((n) * (1 << 9)))
#define NORMAL_PACK(x,y,z)   (((x) & 0x3FF) | (((y) & 0x3FF) << 10) | ((z) << 20))

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
} vector;



#define GL_FALSE				0
#define GL_TRUE					1

#define GL_TRIANGLE        0
#define GL_QUAD            1
#define GL_TRIANGLES        0
#define GL_QUADS            1
#define GL_TRIANGLE_STRIP  2
#define GL_QUAD_STRIP      3

#define GL_MODELVIEW       2
#define GL_PROJECTION      0
#define GL_TEXTURE         3

#define GL_AMBIENT              1
#define GL_DIFFUSE              2
#define GL_AMBIENT_AND_DIFFUSE  3
#define GL_SPECULAR             4
#define GL_SHININESS            8
#define GL_EMISSION             0x10

#define GL_LIGHTING				1

#define POLY_ALPHA(n)  ((n) << 16)
#define POLY_TOON_SHADING     0x20
#define POLY_CULL_BACK        0x80
#define POLY_CULL_FRONT       0x40
#define POLY_CULL_NONE        0xC0
#define POLY_ID(n)		((n)<<24)


#define POLY_FORMAT_LIGHT0      0x1
#define POLY_FORMAT_LIGHT1      0x2
#define POLY_FORMAT_LIGHT2      0x4
#define POLY_FORMAT_LIGHT3      0x8

#define MAX_TEXTURES 2048  //this should be enough ! but feel free to change

#define TEXTURE_SIZE_8     0
#define TEXTURE_SIZE_16    1 
#define TEXTURE_SIZE_32    2
#define TEXTURE_SIZE_64    3
#define TEXTURE_SIZE_128   4
#define TEXTURE_SIZE_256   5
#define TEXTURE_SIZE_512   6
#define TEXTURE_SIZE_1024  7 


#define TEXGEN_OFF			(0<<30)			//unmodified texcoord
#define TEXGEN_TEXCOORD		(1<<30)			//texcoord * texture-matrix
#define TEXGEN_NORMAL		(2<<30)			//normal * texture-matrix
#define TEXGEN_POSITION		(3<<30)			//vertex * texture-matrix

#define GL_TEXTURE_WRAP_S (1 << 16)
#define GL_TEXTURE_WRAP_T (1 << 17)
#define GL_TEXTURE_FLIP_S (1 << 18)
#define GL_TEXTURE_FLIP_T (1 << 19)

#define GL_TEXTURE_2D		1

#define GL_TOON_HIGHLIGHT	(1<<1)
#define GL_ANTIALIAS		(1<<4)			//not fully figured out
#define GL_OUTLINE			(1<<5)
#define GL_BLEND			(1<<3)
#define GL_ALPHA_TEST		(1<<2)
#define GL_TEXTURE_ALPHA_MASK (1 << 29)

#define GL_RGB		8
#define GL_RGBA		7	//15 bit color + alpha bit
#define GL_RGB4		2	//4 color palette
#define GL_RGB256	4	//256 color palette
#define GL_RGB16	3	//16 color palette
#define GL_COMPRESSED	5 //compressed texture

//---------------------------------------------------------------------------------
//Fifo commands
//---------------------------------------------------------------------------------
#define FIFO_COMMAND_PACK(c1,c2,c3,c4) (((c4) << 24) | ((c3) << 16) | ((c2) << 8) | (c1))

#define REG2ID(r)						(u8)( ( ((u32)(&(r)))-0x04000400 ) >> 2 )

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

#ifdef __cplusplus
extern "C" {
#endif



extern void glEnable(int bits);
extern void glDisable(int bits);
extern void glLoadMatrix4x4(m4x4 * m);
extern void glLoadMatrix4x3(m4x3 * m);
extern void glMultMatrix4x4(m4x4 * m);
extern void glMultMatrix4x3(m4x3 * m);
extern void glMultMatrix3x3(m3x3 * m);
extern void glRotateXi(int angle);
extern void glRotateYi(int angle);
extern void glRotateZi(int angle);
extern void glRotateX(float angle);
extern void glRotateY(float angle);
extern void glRotateZ(float angle);
extern void glRotatef32i(int angle, f32 x, f32 y, f32 z);


extern void gluLookAtf32(f32 eyex, f32 eyey, f32 eyez, f32 lookAtx, f32 lookAty, f32 lookAtz, f32 upx, f32 upy, f32 upz);
extern void gluLookAt(float eyex, float eyey, float eyez, float lookAtx, float lookAty, float lookAtz, float upx, float upy, float upz);
extern void gluFrustumf32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
extern void gluFrustum(float left, float right, float bottom, float top, float near, float far);
extern void gluPerspectivef32(int fovy, f32 aspect, f32 zNear, f32 zFar);
extern void gluPerspective(float fovy, float aspect, float zNear, float zFar);

extern int glTexImage2D(int target, int empty1, int type, int sizeX, int sizeY, int empty2, int param, uint8* texture);
extern void glTexLoadPal(u16* pal, u8 count);
extern void glBindTexture(int target, int name);
extern int glGenTextures(int n, int *names);
extern void glResetTextures(void);
extern void glTexCoord2f32(f32 u, f32 v);

extern void glMaterialf(int mode, rgb color);
extern void glResetMatrixStack(void);
extern void glSetOutlineColor(int id, rgb color);
extern void glSetToonTable(uint16 *table);
extern void glSetToonTableRange(int start, int end, rgb color);
extern void glReset(void);

//---------------------------------------------------------------------------------
//float wrappers for porting
//---------------------------------------------------------------------------------
extern void glRotatef(float angle, float x, float y, float z);
extern void glVertex3f(float x, float y, float z);
extern void glTexCoord2f(float s, float t);
extern void glColor3f(float r, float g, float b);
extern void glScalef(float x, float y, float z);
extern void glTranslatef(float x, float y, float z);
extern void glNormal3f(float x, float y, float z);

extern void glClearColor(uint8 red, uint8 green, uint8 blue, uint8 alpha);
extern void glClearPolyID(uint8 ID);
	
#ifdef NO_GL_INLINE

	void glBegin(int mode);
	void glEnd( void);
	void glClearDepth(uint16 depth);
	void glColor3b(uint8 red, uint8 green, uint8 blue);
	void glColor(rgb color);
	void glVertex3v16(v16 x, v16 y, v16 z);
	void glVertex2v16(int yx, v16 z);
	void glTexCoord2t16(t16 u, t16 v);
	void glTexCoord1i(uint32 uv);
	void glPushMatrix(void);
	void glPopMatrix(int32 index);
	void glRestoreMatrix(int32 index);
	void glStoreMatrix(int32 index);
	void glScalev(vector* v);
	void glTranslatev(vector* v);
	void glTranslate3f32(f32 x, f32 y, f32 z);
	void glScalef32(f32 factor);
	void glTranslatef32(f32 delta);
	void glLight(int id, rgb color, v10 x, v10 y, v10 z);
	void glNormal(uint32 normal);
	void glMatrixMode(int mode);
	void glViewPort(uint8 x1, uint8 y1, uint8 x2, uint8 y2);
	void glFlush(void);
	void glMaterialShinnyness(void);
	void glPolyFmt(int alpha); 
#endif

#ifndef NO_GL_INLINE

static inline void glSetAlpha(int alpha) { GFX_ALPHA = alpha; }

static inline void glBegin(int mode) { GFX_BEGIN = mode; }

static inline void glEnd( void) { GFX_END = 0; }

static inline void glClearDepth(uint16 depth) { GFX_CLEAR_DEPTH = depth; }

static inline void glColor3b(uint8 red, uint8 green, uint8 blue) { GFX_COLOR = (vuint32)RGB15(red>>3, green>>3, blue>>3); }


static inline void glColor(rgb color) { GFX_COLOR = (vuint32)color; }

//---------------------------------------------------------------------------------
static inline void glVertex3v16(v16 x, v16 y, v16 z) {
//---------------------------------------------------------------------------------
	GFX_VERTEX16 = (y << 16) | (x & 0xFFFF);
	GFX_VERTEX16 = ((uint32)(uint16)z);
}


//---------------------------------------------------------------------------------
static inline void glVertex2v16(int yx, v16 z) {
//---------------------------------------------------------------------------------
	GFX_VERTEX16 = yx;
	GFX_VERTEX16 = (z);
}

static inline void glTexCoord2t16(t16 u, t16 v) { GFX_TEX_COORD = TEXTURE_PACK(u,v); }

static inline void glTexCoord1i(uint32 uv) { GFX_TEX_COORD = uv; }

static inline void glPushMatrix(void) { MATRIX_PUSH = 0; }

static inline void glPopMatrix(int32 index) { MATRIX_POP = index; }

static inline void glRestoreMatrix(int32 index) { MATRIX_RESTORE = index; }


static inline void glStoreMatrix(int32 index) { MATRIX_STORE = index; }

//---------------------------------------------------------------------------------
static inline void glScalev(vector* v) { 
//---------------------------------------------------------------------------------
	MATRIX_SCALE = v->x;
	MATRIX_SCALE = v->y;
	MATRIX_SCALE = v->z;
}


//---------------------------------------------------------------------------------
static inline void glTranslatev(vector* v) {
//---------------------------------------------------------------------------------
	MATRIX_TRANSLATE = v->x;
	MATRIX_TRANSLATE = v->y;
	MATRIX_TRANSLATE = v->z;
}

//---------------------------------------------------------------------------------
static inline void glTranslate3f32(int x, int y, int z) {
//---------------------------------------------------------------------------------
	MATRIX_TRANSLATE = x;
	MATRIX_TRANSLATE = y;
	MATRIX_TRANSLATE = z;
}

//---------------------------------------------------------------------------------
static inline void glScalef32(f32 factor) {
//---------------------------------------------------------------------------------
	MATRIX_SCALE = factor;
	MATRIX_SCALE = factor;
	MATRIX_SCALE = factor;
}

//---------------------------------------------------------------------------------
static inline void glTranslatef32(f32 delta) {
//---------------------------------------------------------------------------------
	MATRIX_TRANSLATE = delta;
	MATRIX_TRANSLATE = delta;
	MATRIX_TRANSLATE = delta;
}

//---------------------------------------------------------------------------------
static inline void glLight(int id, rgb color, v10 x, v10 y, v10 z) {
//---------------------------------------------------------------------------------
	id = (id & 3) << 30;
	GFX_LIGHT_VECTOR = id | ((z & 0x3FF) << 20) | ((y & 0x3FF) << 10) | (x & 0x3FF);
	GFX_LIGHT_COLOR = id | color;
}

static inline void glNormal(uint32 normal) { GFX_NORMAL = normal; }

static inline void glLoadIdentity(void) { MATRIX_IDENTITY = 0; }
static inline void glMatrixMode(int mode) { MATRIX_CONTROL = mode; }

static inline void glViewPort(uint8 x1, uint8 y1, uint8 x2, uint8 y2) { GFX_VIEWPORT = (x1) + (y1 << 8) + (x2 << 16) + (y2 << 24); }

static inline void glFlush(void) { GFX_FLUSH = 2; }

//---------------------------------------------------------------------------------
static inline void glMaterialShinyness(void) {
//---------------------------------------------------------------------------------
	uint32 shiny32[128/4];
	uint8  *shiny8 = (uint8*)shiny32;

	int i;

	for (i = 0; i < 128 * 2; i += 2)
		shiny8[i>>1] = i;

	for (i = 0; i < 128 / 4; i++)
		GFX_SHININESS = shiny32[i];
}

static inline void glPolyFmt(int alpha) // obviously more to this
{
  GFX_POLY_FORMAT = alpha;
}

//---------------------------------------------------------------------------------
//Display lists have issues that need to resolving.
//There seems to be some issues with list size.
//---------------------------------------------------------------------------------
static inline void glCallList(u32* list)
//---------------------------------------------------------------------------------
{
	u32 count = *list++;

	while(count--)
		GFX_FIFO = *list++;

	//this works sometimes??
//	DMA_SRC(0) = (uint32)list;
//	DMA_DEST(0) = 0x4000400;
//	DMA_CR(0) = DMA_FIFO | count;
//
//	while(DMA_CR(0) & DMA_BUSY);

}

#endif  //endif #no inline


extern void initGL();

#ifdef __cplusplus
}
#endif

#endif

//////////////////////////////////////////////////////////////////////
