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
//	 0.4: Update GL specs from OpenGL 1.0 to OpenGL 1.1 which enables Textures Objects (Coto)
//////////////////////////////////////////////////////////////////////


#ifndef VIDEOGL_ARM9_INCLUDE
#define VIDEOGL_ARM9_INCLUDE

#ifdef ARM9
#undef NO_GL_INLINE //NDS
#endif

#ifdef WIN32
#define NO_GL_INLINE //WIN32 debugger
#endif

//////////////////////////////////////////////////////////////////////

//#ifndef ARM9
//#error 3D hardware is only available from the ARM9
//#endif

//////////////////////////////////////////////////////////////////////
#ifdef ARM9
#include <typedefsTGDS.h>
#include "videoTGDS.h"
#include "dmaTGDS.h"
#include "nds_cp15_misc.h"
#endif

#ifdef WIN32
#include "TGDSTypes.h"
#endif

#include "arm9math.h"

//////////////////////////////////////////////////////////////////////
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

#define GFX_MTX_PUSH_ADDR            ((vuint32) 0x04000444)
#define GFX_MTX_POP_ADDR            ((vuint32) 0x04000448)
#define GFX_MTX_TRANS_ADDR            ((vuint32) 0x04000470)

#define GFX_DIFFUSE_AMBIENT_ADDR   ((vuint32) 0x040004C0)
#define GFX_SPECULAR_EMISSION_ADDR ((vuint32) 0x040004C4)
#define GFX_SHININESS_ADDR         ((vuint32) 0x040004D0)

#define GFX_POLY_FORMAT_ADDR       ((vuint32) 0x040004A4)

#define GFX_BEGIN_ADDR             ((vuint32) 0x04000500)
#define GFX_END_ADDR               ((vuint32) 0x04000504)
#define GFX_FLUSH_ADDR             ((vuint32) 0x04000540)
#define GFX_VIEWPORT_ADDR          ((vuint32) 0x04000580)
#define GFX_TOON_TABLE_ADDR		  ((vuint32)  0x04000380)
#define GFX_EDGE_TABLE_ADDR		  ((vuint32)  0x04000330)

//////////////////////////////////////////////////////////////////////
// Matrix processor control
//////////////////////////////////////////////////////////////////////

#define MATRIX_CONTROL_ADDR    (0x04000440)
#define MATRIX_PUSH_ADDR       (0x04000444)
#define MATRIX_POP_ADDR        (0x04000448)
#define MATRIX_SCALE_ADDR      (0x0400046C)
#define MATRIX_TRANSLATE_ADDR  (0x04000470)
#define MATRIX_RESTORE_ADDR    (0x04000450)
#define MATRIX_STORE_ADDR      (0x0400044C)
#define MATRIX_IDENTITY_ADDR   (0x04000454)
#define MATRIX_LOAD4x4_ADDR    (0x04000458)
#define MATRIX_LOAD4x3_ADDR    (0x0400045C)
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

struct GLContext{
	GLenum primitiveShadeModelMode;	//glShadeModel(GLenum mode: [GL_FLAT/GL_SMOOTH]);
	u32	mode; //GLenum mode: //Specifies the compilation mode, which can be GL_COMPILE or GL_COMPILE_AND_EXECUTE. Set up by glNewList()
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

#define GFX_POLY_FORMAT       (*(vuint32*) 0x040004A4)

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
typedef enum {
	GL_TRIANGLES      = 0, /*!< draw triangles with each 3 vertices defining a triangle */
	GL_QUADS          = 1, /*!< draw quads with each 4 vertices defining a quad */
	GL_TRIANGLE_STRIP = 2, /*!< draw triangles with the first triangle defined by 3 vertices, then each additional triangle being defined by one additional vertex */
	GL_QUAD_STRIP     = 3, /*!< draw quads with the first quad being defined by 4 vertices, then each additional triangle being defined by 2 vertices. */
	GL_TRIANGLE       = 0, /*!< same as GL_TRIANGLES, old non-OpenGL version */
	GL_QUAD           = 1  /*!< same as GL_QUADS, old non-OpenGL version */
} GL_GLBEGIN_ENUM;

/*! \brief Enums selecting matrix mode<BR>
<A HREF="http://problemkaputt.de/gbatek.htm#ds3dmatrixloadmultiply">GBATEK http://problemkaputt.de/gbatek.htm#ds3dmatrixloadmultiply</A><BR>
related functions: glMatrixMode() */
typedef enum {
	GL_PROJECTION     = 0, /*!< used to set the Projection Matrix */
	GL_POSITION       = 1, /*!< used to set the Position Matrix */
	GL_MODELVIEW      = 2, /*!< used to set the Modelview Matrix */
	GL_TEXTURE        = 3  /*!< used to set the Texture Matrix */
} GL_MATRIX_MODE_ENUM;

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


#define TEXGEN_OFF			(0<<30)			//unmodified texcoord
#define TEXGEN_TEXCOORD		(1<<30)			//texcoord * texture-matrix
#define TEXGEN_NORMAL		(2<<30)			//normal * texture-matrix
#define TEXGEN_POSITION		(3<<30)			//vertex * texture-matrix
//////////////////////////////////////////////////////////////////////

#define GL_TEXTURE_WRAP_S (1 << 16)
#define GL_TEXTURE_WRAP_T (1 << 17)
#define GL_TEXTURE_FLIP_S (1 << 18)
#define GL_TEXTURE_FLIP_T (1 << 19)

#define GL_TEXTURE_2D		1
#define GL_TOON_HIGHLIGHT	(1<<1)
#define GL_BLEND (1<<3)
#define GL_ANTIALIAS		(1<<4)			//not fully figured out
#define GL_OUTLINE			(1<<5)

//////////////////////////////////////////////////////////////////////

#define GL_RGB		8
#define GL_RGBA		7	//15 bit color + alpha bit
#define GL_RGB4		2	//4 color palette
#define GL_RGB256	4	//256 color palette
#define GL_RGB16	3	//16 color palette
#define GL_COMPRESSED	5 //compressed texture

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//Fifo commands (NDS bits)

#define FIFO_COMMAND_PACK(c1,c2,c3,c4) (((c4) << 24) | ((c3) << 16) | ((c2) << 8) | (c1)) /*!< \brief packs four packed commands into a 32bit command for sending to the GFX FIFO */
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

//Extended NDS bits
#define MTX_PUSH		REG2ID(MATRIX_PUSH)  
#define MTX_POP			REG2ID(MATRIX_POP)
#define MTX_IDENTITY	REG2ID(MATRIX_IDENTITY)
#define MTX_TRANS		REG2ID(MATRIX_TRANSLATE)

#define MTX_MULT_4x4		REG2ID(MATRIX_MULT4x4)
#define MTX_MULT_4x3		REG2ID(MATRIX_MULT4x3)
#define MTX_MULT_3x3		REG2ID(MATRIX_MULT3x3)





#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////

void glEnable(int bits);
void glDisable(int bits);
void glLoadMatrix4x4(m4x4 * m);
void glLoadMatrix4x3(m4x3 * m);
void glMultMatrix4x4(m4x4 * m);
void glMultMatrix4x3(m4x3 * m);
void glMultMatrix3x3(m3x3 * m);
void glRotateXi(int angle);
void glRotateYi(int angle);
void glRotateZi(int angle);
void glRotateX(float angle);
void glRotateY(float angle);
void glRotateZ(float angle);
void gluLookAtf32(f32 eyex, f32 eyey, f32 eyez, f32 lookAtx, f32 lookAty, f32 lookAtz, f32 upx, f32 upy, f32 upz);
void gluLookAt(float eyex, float eyey, float eyez, float lookAtx, float lookAty, float lookAtz, float upx, float upy, float upz);
void gluFrustumf32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
void gluFrustum(float left, float right, float bottom, float top, float near, float far);
void gluPerspectivef32(int fovy, f32 aspect, f32 zNear, f32 zFar);
void gluPerspective(float fovy, float aspect, float zNear, float zFar);
int glTexImage2D(int target, int empty1, int type, int sizeX, int sizeY, int empty2, int param, uint8* texture);
void glBindTexture(int target, int name);
int glGenTextures(int n, int *names);
void glResetTextures(void);
void glMaterialf(int mode, rgb color);
void glResetMatrixStack(void);
void glSetOutlineColor(int id, rgb color);
void glSetToonTable(uint16 *table);
void glSetToonTableRange(int start, int end, rgb color);
void glReset(void);

//////////////////////////////////////////////////////////////////////


#ifdef NO_GL_INLINE
//////////////////////////////////////////////////////////////////////

  void glBegin(int mode);
//////////////////////////////////////////////////////////////////////
  void glEnd( void);  
/////////////////////////////////////////////////////////////////////
  void glColor3b(uint8 red, uint8 green, uint8 blue);
//////////////////////////////////////////////////////////////////////
  void glPushMatrix(void);
//////////////////////////////////////////////////////////////////////
  void glPopMatrix(sint32 index);
//////////////////////////////////////////////////////////////////////
  void glRestoreMatrix(sint32 index);
//////////////////////////////////////////////////////////////////////
 void glStoreMatrix(sint32 index);
//////////////////////////////////////////////////////////////////////
 void glScalev(GLvector* v);
//////////////////////////////////////////////////////////////////////
  void glTranslatev(GLvector* v);
//////////////////////////////////////////////////////////////////////
  void glTranslate3f32(f32 x, f32 y, f32 z);
//////////////////////////////////////////////////////////////////////
  void glScalef32(f32 factor);
//////////////////////////////////////////////////////////////////////
  void glTranslatef32(int x, int y, int z);
//////////////////////////////////////////////////////////////////////
  void glLight(int id, rgb color, v10 x, v10 y, v10 z);
//////////////////////////////////////////////////////////////////////
  void glNormal(uint32 normal);
//////////////////////////////////////////////////////////////////////
  void glLoadIdentity(void);
//////////////////////////////////////////////////////////////////////
  void glMatrixMode(int mode);
//////////////////////////////////////////////////////////////////////
  void glViewPort(uint8 x1, uint8 y1, uint8 x2, uint8 y2);
//////////////////////////////////////////////////////////////////////
void glMaterialShinnyness(void);
//////////////////////////////////////////////////////////////////////
  void glPolyFmt(int alpha); 
//////////////////////////////////////////////////////////////////////
#endif

#ifndef NO_GL_INLINE
//////////////////////////////////////////////////////////////////////
  static inline void glPushMatrix(void)
{
  MATRIX_PUSH = 0;
}

//////////////////////////////////////////////////////////////////////

  static inline void glPopMatrix(sint32 index)
{
  MATRIX_POP = index;
}

//////////////////////////////////////////////////////////////////////

  static inline void glRestoreMatrix(sint32 index)
{
  MATRIX_RESTORE = index;
}

//////////////////////////////////////////////////////////////////////

  static inline void glStoreMatrix(sint32 index)
{
  MATRIX_STORE = index;
}

//////////////////////////////////////////////////////////////////////

  static inline void glScalev(GLvector* v)
{
  MATRIX_SCALE = v->x;
  MATRIX_SCALE = v->y;
  MATRIX_SCALE = v->z;
}

//////////////////////////////////////////////////////////////////////

  static inline void glTranslatev(GLvector* v)
{
  MATRIX_TRANSLATE = v->x;
  MATRIX_TRANSLATE = v->y;
  MATRIX_TRANSLATE = v->z;
}

//////////////////////////////////////////////////////////////////////

  static inline void glTranslate3f32(f32 x, f32 y, f32 z)
{
  MATRIX_TRANSLATE = x;
  MATRIX_TRANSLATE = y;
  MATRIX_TRANSLATE = z;
}

//////////////////////////////////////////////////////////////////////

  static inline void glScalef32(f32 factor)
{
  MATRIX_SCALE = factor;
  MATRIX_SCALE = factor;
  MATRIX_SCALE = factor;
}

//////////////////////////////////////////////////////////////////////

static inline void glTranslatef32(int x, int y, int z) {
	MATRIX_TRANSLATE = x;
	MATRIX_TRANSLATE = y;
	MATRIX_TRANSLATE = z;
}

//////////////////////////////////////////////////////////////////////

  static inline void glLight(int id, rgb color, v10 x, v10 y, v10 z)
{
  id = (id & 3) << 30;
  GFX_LIGHT_VECTOR = id | ((z & 0x3FF) << 20) | ((y & 0x3FF) << 10) | (x & 0x3FF);
  GFX_LIGHT_COLOR = id | color;
}

//////////////////////////////////////////////////////////////////////

  static inline void glNormal(uint32 normal)
{
  GFX_NORMAL = normal;
}

//////////////////////////////////////////////////////////////////////

  static inline void glLoadIdentity(void)
{
  MATRIX_IDENTITY = 0;
}

//////////////////////////////////////////////////////////////////////

  static inline void glMatrixMode(int mode)
{
  MATRIX_CONTROL = mode;
}

//////////////////////////////////////////////////////////////////////

  static inline void glViewPort(uint8 x1, uint8 y1, uint8 x2, uint8 y2)
{
  GFX_VIEWPORT = (x1) + (y1 << 8) + (x2 << 16) + (y2 << 24);
}

//////////////////////////////////////////////////////////////////////

static inline void glMaterialShinnyness(void)

{
	uint32 shiny32[128/4];
	uint8  *shiny8 = (uint8*)shiny32;

	int i;

	for (i = 0; i < 128 * 2; i += 2)
		shiny8[i>>1] = i;

	for (i = 0; i < 128 / 4; i++)
		GFX_SHININESS = shiny32[i];
}

//////////////////////////////////////////////////////////////////////

static inline void glPolyFmt(int alpha) // obviously more to this
{
  GFX_POLY_FORMAT = alpha;
}

#endif  //endif #no inline
extern void glRotate(int angle, float x, float y, float z);

extern void glOrthof32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
extern void glOrtho(float left, float right, float bottom, float top, float near, float far);
extern void glColor3f(float red, float green, float blue);

extern struct GLContext globalGLCtx;
extern void glShadeModel(GLenum mode);
extern void glInit();

extern void glVertex2i(int x, int y); 
extern void glVertex2f(float x, float y);
extern void glVertex3f(GLfloat x, GLfloat y, GLfloat z);

extern void GLInitExt();
extern bool isNdsDisplayListUtilsCallList;
extern void glCallListGX(const u32* list);
extern int float2int(float valor);
extern void glTranslatef(float x, float y, float z);
extern void glFlush(void);
extern void glFinish(void);

extern u8 defaultglClearColorR;
extern u8 defaultglClearColorG;
extern u8 defaultglClearColorB;
extern u16 defaultglClearDepth;
extern void glClear( GLbitfield mask );
extern void glClearColor(uint8 red, uint8 green, uint8 blue);
extern void glClearDepth(uint16 depth);

#ifdef __cplusplus
}
#endif

#endif

//////////////////////////////////////////////////////////////////////
