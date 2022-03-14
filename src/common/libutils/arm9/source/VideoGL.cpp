//////////////////////////////////////////////////////////////////////
//
// videoGL.cpp -- Video API vaguely similar to OpenGL
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
//   0.2: Added gluFrustrum, gluPerspective, and gluLookAt
//			Converted all floating point math to fixed point
//	 0.3: Display lists added thanks to mike260	
//	 0.4: Update GL specs from OpenGL 1.0 to OpenGL 1.1 which enables Textures Objects (Coto)
//////////////////////////////////////////////////////////////////////

#include "VideoGL.h"
#include "VideoGLExt.h"
#include "arm9math.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef ARM9
#include <typedefsTGDS.h>
#include "dsregs.h"
#include "videoTGDS.h"
#endif

#ifdef WIN32
#include "TGDSTypes.h"
#endif
//lut resolution for trig functions (must be power of two and must be the same as LUT resolution)
//in other words dont change unless you also change your LUTs
#define LUT_SIZE (512)
#define LUT_MASK (0x1FF)

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
bool isNdsDisplayListUtilsCallList;
struct GLContext globalGLCtx;

#ifdef NO_GL_INLINE
//////////////////////////////////////////////////////////////////////

void glPushMatrix(void){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		Compiled_DL_Binary[interCompiled_DLPtr] = getMTX_PUSH();
		interCompiled_DLPtr++;
	}
	else{
		MATRIX_PUSH = 0;
	}
}

//////////////////////////////////////////////////////////////////////

void glPopMatrix(sint32 index)
{
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		Compiled_DL_Binary[interCompiled_DLPtr] = getMTX_POP();
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)index;
		interCompiled_DLPtr++;

	}
	else{
		MATRIX_POP = index;
	}
}

//////////////////////////////////////////////////////////////////////

  void glRestoreMatrix(sint32 index)
{
  MATRIX_RESTORE = index;
}

//////////////////////////////////////////////////////////////////////

  void glStoreMatrix(sint32 index)
{
  MATRIX_STORE = index;
}

//////////////////////////////////////////////////////////////////////

  void glScalev(GLvector* v)
{
  MATRIX_SCALE = v->x;
  MATRIX_SCALE = v->y;
  MATRIX_SCALE = v->z;
}

//////////////////////////////////////////////////////////////////////

  void glTranslatev(GLvector* v)
{
  MATRIX_TRANSLATE = v->x;
  MATRIX_TRANSLATE = v->y;
  MATRIX_TRANSLATE = v->z;
}

//////////////////////////////////////////////////////////////////////

void glTranslate3f32(f32 x, f32 y, f32 z){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//MTX_TRANS: Sets C=M*C. Parameters: 3, m[0..2] (x,y,z position)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getMTX_TRANS(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)x; interCompiled_DLPtr++; //Unpacked Command format
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)y; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)z; interCompiled_DLPtr++;
	}
	else{
		MATRIX_TRANSLATE = x;
		MATRIX_TRANSLATE = y;
		MATRIX_TRANSLATE = z;
	}
}

//////////////////////////////////////////////////////////////////////

  void glScalef32(f32 factor)
{
  MATRIX_SCALE = factor;
  MATRIX_SCALE = factor;
  MATRIX_SCALE = factor;
}

//////////////////////////////////////////////////////////////////////

void glTranslatef32(int x, int y, int z) {
	MATRIX_TRANSLATE = x;
	MATRIX_TRANSLATE = y;
	MATRIX_TRANSLATE = z;
}

//////////////////////////////////////////////////////////////////////

  void glLight(int id, rgb color, v10 x, v10 y, v10 z)
{
  id = (id & 3) << 30;
  GFX_LIGHT_VECTOR = id | ((z & 0x3FF) << 20) | ((y & 0x3FF) << 10) | (x & 0x3FF);
  GFX_LIGHT_COLOR = id | color;
}

//////////////////////////////////////////////////////////////////////

  void glNormal(uint32 normal)
{
  GFX_NORMAL = normal;
}

//////////////////////////////////////////////////////////////////////

  void glLoadIdentity(void)
{
  MATRIX_IDENTITY = 0;
}

//////////////////////////////////////////////////////////////////////

  void glMatrixMode(int mode)
{
  MATRIX_CONTROL = mode;
}

//////////////////////////////////////////////////////////////////////

void glMaterialShinnyness(void)

{
	uint32 shiny32[128/4];
	uint8  *shiny8 = (uint8*)shiny32;

	int i;

	for (i = 0; i < 128 * 2; i += 2)
		shiny8[i>>1] = i;

	for (i = 0; i < 128 / 4; i++){
		GFX_SHININESS = shiny32[i];
	}
}

//////////////////////////////////////////////////////////////////////

  void glPolyFmt(int alpha) // obviously more to this
{
  GFX_POLY_FORMAT = alpha;
}

#endif  //endif #no inline

//////////////////////////////////////////////////////////////////////

void glViewport(uint8 x1, uint8 y1, uint8 x2, uint8 y2){
	GFX_VIEWPORT = (x1) + (y1 << 8) + (x2 << 16) + (y2 << 24);
}

//////////////////////////////////////////////////////////////////////
u8 defaultglClearColorR=0;
u8 defaultglClearColorG=0;
u8 defaultglClearColorB=0;
u16 defaultglClearDepth=0;

//////////////////////////////////////////////////////////////////////
void glClearColor(uint8 red, uint8 green, uint8 blue){
	defaultglClearColorR = red;
	defaultglClearColorG = green;
	defaultglClearColorB = blue;
	GFX_CLEAR_COLOR = RGB15(red, green, blue);
}

//////////////////////////////////////////////////////////////////////

void glClearDepth(uint16 depth){
	defaultglClearDepth = depth;
	GFX_CLEAR_DEPTH = depth;
}

void glClear( GLbitfield mask ){
	if((mask & GL_COLOR_BUFFER_BIT) == GL_COLOR_BUFFER_BIT){
		glClearColor(defaultglClearColorR, defaultglClearColorG, defaultglClearColorB);
	}
	
	if((mask & GL_DEPTH_BUFFER_BIT) == GL_DEPTH_BUFFER_BIT){
		glClearDepth(defaultglClearDepth);
	}
	
	if((mask & GL_STENCIL_BUFFER_BIT) == GL_STENCIL_BUFFER_BIT){
		//glClearStencil //stencil buffer depends on polygon attributes since DS does not feature a standalone stencilbuffer
	}
	
	//todo: GL_INVALID_VALUE
}

//////////////////////////////////////////////////////////////////////

void glTranslatef(float x, float y, float z){
	glTranslate3f32(floattof32(x), floattof32(y), floattof32(z));
}
//////////////////////////////////////////////////////////////////////

static uint16 enable_bits = GL_TEXTURE_2D | (1<<13) | (1<<14);

//////////////////////////////////////////////////////////////////////

void glEnable(int bits)
{
	enable_bits |= bits & (GL_TEXTURE_2D|GL_TOON_HIGHLIGHT|GL_OUTLINE|GL_ANTIALIAS);
	GFX_CONTROL = enable_bits;
}

//////////////////////////////////////////////////////////////////////

void glDisable(int bits)
{
	enable_bits &= ~(bits & (GL_TEXTURE_2D|GL_TOON_HIGHLIGHT|GL_OUTLINE|GL_ANTIALIAS));
	GFX_CONTROL = enable_bits;
}

//////////////////////////////////////////////////////////////////////

void glFlush(void){
	GFX_FLUSH = 2;
}

//////////////////////////////////////////////////////////////////////
//OpenGL states this behaves the same as glFlush but also CPU waits for all commands to be executed by the GPU
void glFinish(void){
	glFlush();
	while( (vu32)(*((vu32*)GFX_STATUS_ADDR)) & (1 << 27) ){ //27    Geometry Engine Busy (0=No, 1=Yes; Busy; Commands are executing)
		
	}
}

//////////////////////////////////////////////////////////////////////

void glLoadMatrix4x4(m4x4 * m)
{
  MATRIX_LOAD4x4 = m->m[0];
  MATRIX_LOAD4x4 = m->m[1];
  MATRIX_LOAD4x4 = m->m[2];
  MATRIX_LOAD4x4 = m->m[3];

  MATRIX_LOAD4x4 = m->m[4];
  MATRIX_LOAD4x4 = m->m[5];
  MATRIX_LOAD4x4 = m->m[6];
  MATRIX_LOAD4x4 = m->m[7];

  MATRIX_LOAD4x4 = m->m[8];
  MATRIX_LOAD4x4 = m->m[9];
  MATRIX_LOAD4x4 = m->m[10];
  MATRIX_LOAD4x4 = m->m[11];

  MATRIX_LOAD4x4 = m->m[12];
  MATRIX_LOAD4x4 = m->m[13];
  MATRIX_LOAD4x4 = m->m[14];
  MATRIX_LOAD4x4 = m->m[15];
}

//////////////////////////////////////////////////////////////////////

void glLoadMatrix4x3(m4x3* m)
{
  MATRIX_LOAD4x3 = m->m[0];
  MATRIX_LOAD4x3 = m->m[1];
  MATRIX_LOAD4x3 = m->m[2];
  MATRIX_LOAD4x3 = m->m[3];

  MATRIX_LOAD4x3 = m->m[4];
  MATRIX_LOAD4x3 = m->m[5];
  MATRIX_LOAD4x3 = m->m[6];
  MATRIX_LOAD4x3 = m->m[7];

  MATRIX_LOAD4x3 = m->m[8];
  MATRIX_LOAD4x3 = m->m[9];
  MATRIX_LOAD4x3 = m->m[10];
  MATRIX_LOAD4x3 = m->m[11];
}

//////////////////////////////////////////////////////////////////////

void glMultMatrix4x4(m4x4* m)
{
  MATRIX_LOAD4x4 = m->m[0];
  MATRIX_LOAD4x4 = m->m[1];
  MATRIX_LOAD4x4 = m->m[2];
  MATRIX_LOAD4x4 = m->m[3];

  MATRIX_LOAD4x4 = m->m[4];
  MATRIX_LOAD4x4 = m->m[5];
  MATRIX_LOAD4x4 = m->m[6];
  MATRIX_LOAD4x4 = m->m[7];

  MATRIX_LOAD4x4 = m->m[8];
  MATRIX_LOAD4x4 = m->m[9];
  MATRIX_LOAD4x4 = m->m[10];
  MATRIX_LOAD4x4 = m->m[11];

  MATRIX_LOAD4x4 = m->m[12];
  MATRIX_LOAD4x4 = m->m[13];
  MATRIX_LOAD4x4 = m->m[14];
  MATRIX_LOAD4x4 = m->m[15];
}

//////////////////////////////////////////////////////////////////////

void glMultMatrix4x3(m4x3* m)
{
  MATRIX_MULT4x3 = m->m[0];
  MATRIX_MULT4x3 = m->m[1];
  MATRIX_MULT4x3 = m->m[2];
  MATRIX_MULT4x3 = m->m[3];

  MATRIX_MULT4x3 = m->m[4];
  MATRIX_MULT4x3 = m->m[5];
  MATRIX_MULT4x3 = m->m[6];
  MATRIX_MULT4x3 = m->m[7];

  MATRIX_MULT4x3 = m->m[8];
  MATRIX_MULT4x3 = m->m[9];
  MATRIX_MULT4x3 = m->m[10];
  MATRIX_MULT4x3 = m->m[11];

  MATRIX_MULT3x3 = m->m[0];
  MATRIX_MULT3x3 = m->m[1];
  MATRIX_MULT3x3 = m->m[2];
  
  MATRIX_MULT3x3 = m->m[3];
  MATRIX_MULT3x3 = m->m[4];
  MATRIX_MULT3x3 = m->m[5];
  
  MATRIX_MULT3x3 = m->m[6];
  MATRIX_MULT3x3 = m->m[7];
  MATRIX_MULT3x3 = m->m[8];
}

//////////////////////////////////////////////////////////////////////
// Integer rotation (not gl standard)
//	based on 512 degree circle
void glRotateZi(int angle){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//Rotate Z DL:
		//Identity Matrix
		//The MTX_IDENTITY command can be used to initialize the Position Matrix before doing any Translation/Scaling/Rotation, for example:
		//  Load(Identity), Mul(Rotate), Mul(Scale)  ;rotation/scaling (not so efficient)
		//Rotation Matrices
		//Rotation can be performed with MTX_MULT_3x3 command, simple examples are:
		//Around Z-Axis
		//| cos   sin   0   |
		//| -sin  cos   0   |
		//| 0     0     1.0 |

		//Load(Identity)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getMTX_IDENTITY(); //Unpacked Command format
		interCompiled_DLPtr++;
		
		//Mul(Rotate)
		//4000468h - Cmd 1Ah - MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
		//Sets C=M*C. Parameters: 9, m[0..8]
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getMTX_MULT_3x3(); interCompiled_DLPtr++; //Unpacked Command format
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)cosine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)sine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)-sine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)cosine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)inttof32(1); interCompiled_DLPtr++;
	}
	else{
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = sine;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = - sine;
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = inttof32(1);
	}
}

//////////////////////////////////////////////////////////////////////

void glRotateYi(int angle)
{
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//Rotate Y DL:
		//Identity Matrix
		//The MTX_IDENTITY command can be used to initialize the Position Matrix before doing any Translation/Scaling/Rotation, for example:
		//  Load(Identity), Mul(Rotate), Mul(Scale)  ;rotation/scaling (not so efficient)
		//Rotation Matrices
		//Rotation can be performed with MTX_MULT_3x3 command, simple examples are:
		//Around Y-Axis
		//| cos   0    sin |
		//| 0     1.0  0   |
		//| -sin  0    cos |

		//Load(Identity)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getMTX_IDENTITY(); //Unpacked Command format
		interCompiled_DLPtr++;
		
		//Mul(Rotate)
		//4000468h - Cmd 1Ah - MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
		//Sets C=M*C. Parameters: 9, m[0..8]
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getMTX_MULT_3x3(); interCompiled_DLPtr++; //Unpacked Command format
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)cosine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)-sine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)inttof32(1); interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)sine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)cosine; interCompiled_DLPtr++;
	}
	else{
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = -sine;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = inttof32(1);
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = sine;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = cosine;
	}
}

//////////////////////////////////////////////////////////////////////

void glRotateXi(int angle)
{
  f32 sine = SIN[angle &  LUT_MASK];
  f32 cosine = COS[angle & LUT_MASK];

  if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//Rotate X DL:
		//Identity Matrix
		//The MTX_IDENTITY command can be used to initialize the Position Matrix before doing any Translation/Scaling/Rotation, for example:
		//  Load(Identity), Mul(Rotate), Mul(Scale)  ;rotation/scaling (not so efficient)
		//Rotation Matrices
		//Rotation can be performed with MTX_MULT_3x3 command, simple examples are:
		//Around X-Axis          
		//| 1.0  0     0   |
		//| 0    cos   sin |
		//| 0    -sin  cos |

		//Load(Identity)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getMTX_IDENTITY(); //Unpacked Command format
		interCompiled_DLPtr++;
		
		//Mul(Rotate)
		//4000468h - Cmd 1Ah - MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
		//Sets C=M*C. Parameters: 9, m[0..8]
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getMTX_MULT_3x3(); interCompiled_DLPtr++; //Unpacked Command format
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)inttof32(1); interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)cosine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)sine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)0; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)-sine; interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)cosine; interCompiled_DLPtr++;
	}
	else{
		MATRIX_MULT3x3 = inttof32(1);
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = sine;  
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = -sine;
		MATRIX_MULT3x3 = cosine;
	}
}
//////////////////////////////////////////////////////////////////////
//	rotations wrapped in float...mainly for testing
void glRotateX(float angle)
{
	glRotateXi((int)(angle * LUT_SIZE / 360.0));
}
//////////////////////////////////////////////////////////////////////

void glRotateY(float angle)
{
	glRotateYi((int)(angle * LUT_SIZE / 360.0));
}
///////////////////////////////////////////////////////////////////////
void glRotateZ(float angle)
{
	glRotateZi((int)(angle * LUT_SIZE / 360.0));
}

void glRotatef(int angle, float x, float y, float z){
	if(x > 0){
		glRotateX(angle); 
	}
	if(y > 0){
		glRotateY(angle); 
	}
	if(z > 0){
		glRotateZ(angle);
	}
}

//////////////////////////////////////////////////////////////////////
// Fixed point look at function, it appears to work as expected although 
//	testing is recomended
void gluLookAtf32(f32 eyex, f32 eyey, f32 eyez, f32 lookAtx, f32 lookAty, f32 lookAtz, f32 upx, f32 upy, f32 upz)
{
	f32 x[3], y[3], z[3], up[3];

	z[0] = eyex - lookAtx;
	z[1] = eyey - lookAty;
	z[2] = eyez - lookAtz;

	up[0] = upx;
	up[1] = upy;
	up[2] = upz;

	normalizef32(z);

	crossf32(up, z, x);
	crossf32(z, x, y);

	normalizef32(x);
	normalizef32(y);
	
	glMatrixMode(GL_MODELVIEW);

	MATRIX_LOAD4x3 = x[0];
	MATRIX_LOAD4x3 = x[1];
	MATRIX_LOAD4x3 = x[2];

	MATRIX_LOAD4x3 = y[0];
	MATRIX_LOAD4x3 = y[1];
	MATRIX_LOAD4x3 = y[2];

	MATRIX_LOAD4x3 = z[0];
	MATRIX_LOAD4x3 = z[1];
	MATRIX_LOAD4x3 = z[2];

	MATRIX_LOAD4x3 = 0;
	MATRIX_LOAD4x3 = 0;
	MATRIX_LOAD4x3 = floattof32(-1.0);

	glTranslate3f32(-eyex, -eyey, -eyez);
}
///////////////////////////////////////
//  glu wrapper for standard float call
void gluLookAt(float eyex, float eyey, float eyez, float lookAtx, float lookAty, float lookAtz, float upx, float upy, float upz)
{
	gluLookAtf32(floattof32(eyex), floattof32(eyey), floattof32(eyez), floattof32(lookAtx), floattof32(lookAty), floattof32(lookAtz),
					floattof32(upx), floattof32(upy), floattof32(upz));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//	frustrum has only been tested as part of perspective
void gluFrustumf32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far){
   glMatrixMode(GL_PROJECTION);

   MATRIX_LOAD4x4 = divf32(2*near, right - left);     
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = divf32(right + left, right - left);      
   MATRIX_LOAD4x4 = 0;

   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = divf32(2*near, top - bottom);     
   MATRIX_LOAD4x4 = divf32(top + bottom, top - bottom);      
   MATRIX_LOAD4x4 = 0;
   
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = -divf32(far + near, far - near);     
   MATRIX_LOAD4x4 = floattof32(-1.0F);
   
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = -divf32(2 * mulf32(far, near), far - near);  
   MATRIX_LOAD4x4 = 0;
	
   glStoreMatrix(0);
}

void glOrtho(float left, float right, float bottom, float top, float near, float far){
	glOrthof32(floattof32(left), floattof32(right), floattof32(bottom), floattof32(top), floattof32(near), floattof32(far));
}

///////////////////////////////////////
void glOrthof32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far){
   glMatrixMode(GL_PROJECTION);

   MATRIX_LOAD4x4 = divf32(2, right - left);     
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = 0;      
   MATRIX_LOAD4x4 = -divf32(right + left, right - left);

   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = divf32(2, top - bottom);
   MATRIX_LOAD4x4 = 0;
   MATRIX_LOAD4x4 = -divf32(top + bottom, top - bottom);;
   
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = divf32(-2, far - near);
   MATRIX_LOAD4x4 = -divf32(far + near, far - near);
   
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = 0;  
   MATRIX_LOAD4x4 = floattof32(1.0F);
	
   glStoreMatrix(0);
}
///////////////////////////////////////

//  Frustrum wrapper
void gluFrustum(float left, float right, float bottom, float top, float near, float far){
	gluFrustumf32(floattof32(left), floattof32(right), floattof32(bottom), floattof32(top), floattof32(near), floattof32(far));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Fixed point perspective setting
void gluPerspectivef32(int fovy, f32 aspect, f32 zNear, f32 zFar)
{
   f32 xmin, xmax, ymin, ymax;

   ymax = mulf32(zNear, TAN[fovy & LUT_MASK]);
   ymin = -ymax;
   xmin = mulf32(ymin, aspect);
   xmax = mulf32(ymax, aspect);

   gluFrustumf32(xmin, xmax, ymin, ymax, zNear, zFar);
}

///////////////////////////////////////
//  glu wrapper for floating point
void gluPerspective(float fovy, float aspect, float zNear, float zFar){
	gluPerspectivef32((int)(fovy * LUT_SIZE / 360.0), floattof32(aspect), floattof32(zNear), floattof32(zFar));    
}


void glMaterialf(int mode, rgb color)
{
  static uint32 diffuse_ambient = 0;
  static uint32 specular_emission = 0;

  switch(mode) {
    case GL_AMBIENT:
      diffuse_ambient = (color << 16) | (diffuse_ambient & 0xFFFF);
      break;
    case GL_DIFFUSE:
      diffuse_ambient = color | (diffuse_ambient & 0xFFFF0000);
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      diffuse_ambient= color + (color << 16);
      break;
    case GL_SPECULAR:
      specular_emission = color | (specular_emission & 0xFFFF0000);
      break;
    case GL_SHININESS:
      break;
    case GL_EMISSION:
      specular_emission = (color << 16) | (specular_emission & 0xFFFF);
      break;
  }
  
  GFX_DIFFUSE_AMBIENT = diffuse_ambient;
  GFX_SPECULAR_EMISSION = specular_emission;
}

//////////////////////////////////////////////////////////////////////

void glResetMatrixStack(void)
{
  // stack overflow ack ?
  GFX_STATUS |= 1 << 15;

  // pop the stacks to the top...seems projection stack is only 1 deep??
  glMatrixMode(GL_PROJECTION);
  glPopMatrix((GFX_STATUS>>13) & 1);
  
  // 31 deep modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix((GFX_STATUS >> 8) & 0x1F);
}

//////////////////////////////////////////////////////////////////////

void glSetOutlineColor(int id, rgb color)
{
	GFX_EDGE_TABLE[id] = color;
}

//////////////////////////////////////////////////////////////////////

void glSetToonTable(uint16 *table)
{
	int i;
	for( i = 0; i < 32; i++ ){
		GFX_TOON_TABLE[i] = table[i];
	}
}

//////////////////////////////////////////////////////////////////////

void glSetToonTableRange(int start, int end, rgb color)
{
	int i;
	for( i = start; i <= end; i++ ){
		GFX_TOON_TABLE[i] = color;
	}
}

//////////////////////////////////////////////////////////////////////

void glReset(void){
  while (GFX_STATUS & (1<<27)); // wait till gfx engine is not busy
  
  // Clear the FIFO
  GFX_STATUS |= (1<<29);

  // Clear overflows for list memory
  GFX_CONTROL = enable_bits = ((1<<12) | (1<<13)) | GL_TEXTURE_2D;
  glResetMatrixStack();
  
  glInit(); //Initializes a new videoGL context
  
  GFX_TEX_FORMAT = 0;
  GFX_POLY_FORMAT = 0;
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Texture
/////////////////////////////////////////////////////////////////////
// Texture globals

uint32 textures[MAX_TEXTURES];

uint32 activeTexture = 0;


uint32* nextBlock = (uint32*)0x06800000;
////////////////////////////////////
void glResetTextures(void)
{
	activeTexture = 0;
	nextBlock = (uint32*)0x06800000;
}
///////////////////////////////////////
// glGenTextures creates intiger names for your table
//	takes n as the number of textures to generate and 
//	a pointer to the names array that it needs to fill.
//  Returns 1 if succesful and 0 if out of texture names
int glGenTextures(int n, int *names)
{
	static int name = 0;

	int index = 0;

	for(index = 0; index < n; index++)
	{
		if(name >= MAX_TEXTURES)
			return 0;
		else
			names[index] = name++;
	}

	return 1;
}

///////////////////////////////////////
// glBindTexure sets the current named
//	texture to the active texture.  Target 
//	is ignored as all DS textures are 2D
void glBindTexture(int target, int name)
{
	GFX_TEX_FORMAT = textures[name];
	
	activeTexture = name;
}

///////////////////////////////////////
// glTexParameter although named the same 
//	as its gl counterpart it is not compatible
//	Effort may be made in the future to make it so.
void glTexParameter(uint8 sizeX, uint8 sizeY, uint32* addr, uint8 mode, uint32 param)
{
	textures[activeTexture] = param | (sizeX << 20) | (sizeY << 23) | (((uint32)addr >> 3) & 0xFFFF) | (mode << 26);
}


uint16* vramGetBank(uint16 *addr)
{
	#ifdef ARM9
	if(addr >= VRAM_A && addr < VRAM_B)
		return VRAM_A;
	else if(addr >= VRAM_B && addr < VRAM_C)
		return VRAM_B;
	else if(addr >= VRAM_C && addr < VRAM_D)
		return VRAM_C;
	else if(addr >= VRAM_D && addr < VRAM_E)
		return VRAM_D;
	else if(addr >= VRAM_E && addr < VRAM_F)
		return VRAM_E;
	else if(addr >= VRAM_F && addr < VRAM_G)
		return VRAM_F;
	else if(addr >= VRAM_G && addr < VRAM_H)
		return VRAM_H;
	else if(addr >= VRAM_H && addr < VRAM_I)
		return VRAM_H;
	else return VRAM_I;
	#endif
	return NULL;
}

int vramIsTextureBank(uint16 *addr)
{
	uint16* vram = vramGetBank(addr);
	#ifdef ARM9
	if(vram == VRAM_A)
	{
		if((VRAM_A_CR & 3) == VRAM_A_TEXTURE)
			return 1;
		else return 0;
	}
	else if(vram == VRAM_B)
	{
		if((VRAM_B_CR & 3) == VRAM_B_TEXTURE)
			return 1;
		else return 0;
	}
	else if(vram == VRAM_C)
	{
		if((VRAM_C_CR & 3) == VRAM_C_TEXTURE)
			return 1;
		else return 0;
	}
	else if(vram == VRAM_D)
	{
		if((VRAM_D_CR & 3) == VRAM_D_TEXTURE)
			return 1;
		else return 0;
	}
	#endif
	return 0;
}

uint32* getNextTextureSlot(int size)
{
	#ifdef ARM9
	uint32* result = nextBlock;
	nextBlock += size >> 2;

	//uh-oh...out of texture memory in this bank...find next one assigned to textures
	while(!vramIsTextureBank((uint16*)nextBlock) && nextBlock < (uint32*)VRAM_E)
	{
		nextBlock = (uint32*)vramGetBank((uint16*)nextBlock) + (0x20000 >> 2); //next bank
		result = nextBlock;	  	
		nextBlock += size >> 2;
	}

	if(nextBlock >= (uint32*)VRAM_E)
		return 0;

	else return result;	
	#endif
	return 0;
}

///////////////////////////////////////
// Similer to glTextImage2D from gl it takes a pointer to data
//	Empty fields and target are unused but provided for code compatibility.
//	type is simply the texture type (GL_RGB, GL_RGB8 ect...)
int glTexImage2D(int target, int empty1, int type, int sizeX, int sizeY, int empty2, int param, uint8* texture)
{
	uint16 alpha = 0;
	uint32 size = 0;
	uint16 palette = 0;
	uint32* addr;
	uint32 vramTemp;

	size = 1 << (sizeX + sizeY + 6) ;
	
	if(type == GL_RGB)
	{
		alpha = (1 << 15);
		type--;
	}
	
	switch (type)
	{
	case GL_RGB:
	case GL_RGBA:
		size = size << 1;
		break;
	case GL_RGB4:
		size = size >> 2;
		palette = 4 * 2;
		break;
	case GL_RGB16:
		size = size >> 1;
		palette = 16 * 2;
		break;
	case GL_RGB256:
		palette = 256 * 2;
		break;
	default:
		break;
	}
	
	addr = getNextTextureSlot(size);
	
	if(!addr)
		return 0;

	glTexParameter(sizeX, sizeY, addr, type, param);
	GFX_TEX_FORMAT = (sizeX << 20) | (sizeY << 23) | ((type == GL_RGB ? GL_RGBA : type ) << 26);
	
	//unlock texture memory
	#ifdef ARM9
	vramTemp = VRAM_CR; //vramTemp = vramSetMainBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);
	VRAMBLOCK_SETBANK_A(VRAM_A_LCDC_MODE);	
	VRAMBLOCK_SETBANK_B(VRAM_B_LCDC_MODE);	
	
	//Make RGB visible
	uint16 *src = (uint16*)texture;
	uint16 *dest = (uint16*)addr;
	size >>= 1;
	while (size--) {
		*dest++ = (*(src)) | 0x8000;
		src++;
	}
	
	VRAM_CR = vramTemp;	//vramRestorMainBanks(vramTemp);
	/*
	if(palette)
	{
		vramSetBankE(VRAM_E_LCD);

		dmaTransferWord(3, (uint32)(texture+size), (uint32)VRAM_E, (uint32)palette);  //dmaCopyWords(3, (uint32*)(texture+size), (uint32*)(VRAM_E), palette);

		vramSetBankE(VRAM_E_TEX_PALETTE);
	}
	*/
	#endif
	
	return 1;
}

//integer x , y vertex coords in v16 format
void glVertex2i(int x, int y) {
    glVertex2v16((v16)x, (v16)y);
}

//float x , y vertex coords in v16 format
void glVertex2f(float x, float y) {
    glVertex2v16(floattov16(x), floattov16(y));
}

//float x , y , z vertex coords in v16 format
void glVertex3f(GLfloat x, GLfloat y, GLfloat z){
	glVertex3v16(floattov16(x), floattov16(y), floattov16(z));
}


void glShadeModel(GLenum mode){
	globalGLCtx.primitiveShadeModelMode = mode;
	lastVertexColor = 0;
}

void glColor3f(float red, float green, float blue){
	glColor3b(f32toint(floattof32(red)), f32toint(floattof32(green)), f32toint(floattof32(blue)));
}

//Must be called everytime a new videoGL context starts
void glInit(){
	memset((u8*)&globalGLCtx, 0, sizeof(struct GLContext));
	glShadeModel(GL_SMOOTH);
	GLInitExt();
}

//Open GL 1.1 Implementation: Texture Objects support
//glTexImage*() == glTexImage3D
void glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels){

}

//glTexSubImage*() == glTexSubImage3D
void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels){
	//The glTexSubImage2D function specifies a portion of an existing one-dimensional texture image. You cannot define a new texture with glTexSubImage2D.
}

//glCopyTexImage*() == glCopyTexImage2D
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border){
	//replaces glTexImage2D() which is hardcoded to a big texture
}

//glCopyTexSubImage*() == glCopyTexSubImage3D
void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height){

}

//glTexParameter*(),  
/*  Example: Create a texture object with linear mipmaps and edge clamping.
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// texture_data is the source data of your texture, in this case
	// its size is sizeof(unsigned char) * texture_width * texture_height * 4
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	glGenerateMipmap(GL_TEXTURE_2D); // Unavailable in OpenGL 2.1, use gluBuild2DMipmaps() insteads.
*/
//requires to update glTexParameter

//glPrioritizeTextures()
void glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities){
	//DS 3D GPU does not support texture priority. There may be a way by sorting them by color but
}

////////////////////////////////////////////////////////////
void glCallListGX(const u32* list) {
	#ifdef ARM9
	u32 count = *list++;

	// flush the area that we are going to DMA
	coherent_user_range_by_size((uint32)list, count*4);
	
	// don't start DMAing while anything else is being DMAed because FIFO DMA is touchy as hell
	//    If anyone can explain this better that would be great. -- gabebear
	while((DMAXCNT(0) & DMAENABLED)||(DMAXCNT(1) & DMAENABLED)||(DMAXCNT(2) & DMAENABLED)||(DMAXCNT(3) & DMAENABLED));

	// send the packed list asynchronously via DMA to the FIFO
	DMAXSAD(0) = (u32)list;
	DMAXDAD(0) = 0x4000400;
	DMAXCNT(0) = DMA_FIFO | count;
	while(DMAXCNT(0) & DMAENABLED);
	//printf("(NDS)glCallListGX: Executing DL List. Size: %d", (int)count);
	#endif

	#ifdef WIN32
	printf("\n(WIN32)glCallListGX: Executing DL List. Size: %d\n", (int)list[0]);
	#endif
}

void glColor3fv(const GLfloat * v){
	if(v != NULL){
		float red = v[0];
		float green = v[1];
		float blue = v[2];
		glColor3f(red, green, blue);
	}
}