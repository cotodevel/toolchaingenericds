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
//	 0.4: Update GL specs from OpenGL 1.0 to OpenGL 1.1 which enables OpenGL standard Display Lists (Coto)
//////////////////////////////////////////////////////////////////////

#include "VideoGL.h"
#include "arm9math.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ndsDisplayListUtils.h"

#ifdef ARM9
#include <typedefsTGDS.h>
#include "dsregs.h"
#include "videoTGDS.h"
#include "dsregs.h"
#endif

#ifdef WIN32
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#include "TGDSTypes.h"
#endif

//////////////////////////////////////////////////////////// Standard OpenGL 1.x start //////////////////////////////////////////
struct GLContext globalGLCtx;
static uint16 enable_bits = GL_TEXTURE_2D | (1<<13) | (1<<14);

static u32 textureParamsValue = 0;
static u16 diffuseValue=0;
static u16 ambientValue=0;
static u16 specularValue=0;
static u16 emissionValue=0;

//Internal Unpacked GX buffer
	#ifdef ARM9
	//__attribute__((section(".dtcm")))
	#endif
	u32 InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_internalSize];
	
	#ifdef ARM9
	__attribute__((section(".dtcm")))
	#endif
	u32 InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;	//2nd half 4K
	u32 SingleUnpackedGXCommand_DL_Binary[PHYS_GXFIFO_INTERNAL_SIZE]; //Unpacked single command GX Buffer
	
//Initializes the NDS OpenGL system
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glInit(){
	//set mode 0, enable BG0 and set it to 3D
	#ifdef ARM9
	SETDISPCNT_MAIN(MODE_0_3D);
	#endif
	memset((u8*)&globalGLCtx, 0, sizeof(struct GLContext));
	glShadeModel(GL_SMOOTH);
	
	InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr=0; //2nd half 4K
	memset(getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(), 0, InternalUnpackedGX_DL_workSize);
	isAnOpenGLExtendedDisplayListCallList = false;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPushMatrix(void){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000444h 11h -  17  MTX_PUSH - Push Current Matrix on Stack (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_PUSH; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_PUSH = 0;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPopMatrix(sint32 index){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000448h 12h 1  36  MTX_POP - Pop Current Matrix from Stack (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_POP; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)index; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_POP = index;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRestoreMatrix(sint32 index){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000450h 14h 1  36  MTX_RESTORE - Restore Current Matrix from Stack (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_RESTORE; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)index; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_RESTORE = index;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glStoreMatrix(sint32 index){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400044Ch - Cmd 13h - MTX_STORE - Store Current Matrix on Stack (W). Sets [N]=C. The stack pointer S is not used, and is left unchanged.
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_STORE; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(index&0x1f); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_STORE = (u32)(index&0x1f);
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glScalev(GLvector* v){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400046Ch 1Bh 3  22  MTX_SCALE - Multiply Current Matrix by Scale Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_SCALE; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)v->x; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)v->y; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)v->z; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_SCALE = v->x;
		MATRIX_SCALE = v->y;
		MATRIX_SCALE = v->z;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslatev(GLvector* v){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000470h 1Ch 3  22* MTX_TRANS - Mult. Curr. Matrix by Translation Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_TRANS; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)v->x; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)v->y; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)v->z; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_TRANSLATE = v->x;
		MATRIX_TRANSLATE = v->y;
		MATRIX_TRANSLATE = v->z;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslatef32(int x, int y, int z) {
	glTranslate3f32(inttof32(x),inttof32(y),inttof32(z));
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslate3f32(f32 x, f32 y, f32 z){
	GLvector vec;
	vec.x = x; vec.y = y; vec.z = z; 
	glTranslatev(&vec);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glScalef32(f32 factor){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400046Ch 1Bh 3  22  MTX_SCALE - Multiply Current Matrix by Scale Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_SCALE; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)factor; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)factor; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)factor; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_SCALE = factor;
		MATRIX_SCALE = factor;
		MATRIX_SCALE = factor;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLight(int id, rgb color, v10 x, v10 y, v10 z){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			id = (id & 3) << 30;
			//40004C8h 32h 1  6   LIGHT_VECTOR - Set Light's Directional Vector (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_LIGHT_VECTOR; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(id | ((z & 0x3FF) << 20) | ((y & 0x3FF) << 10) | (x & 0x3FF)); ptrVal++;
			
			//40004CCh 33h 1  1   LIGHT_COLOR - Set Light Color (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_LIGHT_COLOR; 
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(id | color); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		id = (id & 3) << 30;
		#if defined(ARM9)
		GFX_LIGHT_VECTOR = id | ((z & 0x3FF) << 20) | ((y & 0x3FF) << 10) | (x & 0x3FF);
		GFX_LIGHT_COLOR = id | color;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glNormal(uint32 normal){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_NORMAL; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)normal; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_NORMAL = normal;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadIdentity(void){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000454h 15h -  19  MTX_IDENTITY - Load Unit(Identity) Matrix to Current Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_IDENTITY; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_IDENTITY = 0;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMatrixMode(int mode){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000440h 10h 1  1   MTX_MODE - Set Matrix Mode (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_MODE; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(mode); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_CONTROL = mode;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMaterialShinnyness(void){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		uint32 shiny32[128/4];
		uint8  *shiny8 = (uint8*)shiny32;	
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			int i;
			for (i = 0; i < 128 * 2; i += 2){
				shiny8[i>>1] = i;
			}
			//40004D0h 34h 32 32  SHININESS - Specular Reflection Shininess Table (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_SHININESS; //Unpacked Command format
			ptrVal++;
			for (i = 0; i < 128 / 4; i++){
				InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)shiny32[i]; ptrVal++; //Unpacked Command format
			}
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		uint32 shiny32[128/4];
		uint8  *shiny8 = (uint8*)shiny32;
		int i;
		for (i = 0; i < 128 * 2; i += 2){
			shiny8[i>>1] = i;
		}
		for (i = 0; i < 128 / 4; i++){
			#if defined(ARM9)
			GFX_SHININESS = shiny32[i];
			#endif
		}
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPolyFmt(int alpha){ // obviously more to this
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//40004A4h 29h 1  1   POLYGON_ATTR - Set Polygon Attributes (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_POLYGON_ATTR; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)alpha; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_POLY_FORMAT = alpha;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glViewport(uint8 x1, uint8 y1, uint8 x2, uint8 y2){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000580h 60h 1  1   VIEWPORT - Set Viewport (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getVIEWPORT; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)((x1) + (y1 << 8) + (x2 << 16) + (y2 << 24)); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_VIEWPORT = (x1) + (y1 << 8) + (x2 << 16) + (y2 << 24);
		#endif
	}
}

u8 defaultglClearColorR=0;
u8 defaultglClearColorG=0;
u8 defaultglClearColorB=0;
u16 defaultglClearDepth=0;

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glClearColor(uint8 red, uint8 green, uint8 blue){
	defaultglClearColorR = red;
	defaultglClearColorG = green;
	defaultglClearColorB = blue;
	#ifdef ARM9
	GFX_CLEAR_COLOR = RGB15(red, green, blue);
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glClearDepth(uint16 depth){
	defaultglClearDepth = depth;
	#ifdef ARM9
	GFX_CLEAR_DEPTH = depth;
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslatef(float x, float y, float z){
	glTranslate3f32(floattof32(x), floattof32(y), floattof32(z)); 
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glEnable(int bits){
	enable_bits |= bits & (GL_TEXTURE_2D|GL_TOON_HIGHLIGHT|GL_OUTLINE|GL_ANTIALIAS);
	#if defined(ARM9)
	GFX_CONTROL = enable_bits;
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glDisable(int bits){
	enable_bits &= ~(bits & (GL_TEXTURE_2D|GL_TOON_HIGHLIGHT|GL_OUTLINE|GL_ANTIALIAS));	
	#ifdef ARM9
	GFX_CONTROL = enable_bits;
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glFlush(void){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000540h 50h 1  392 SWAP_BUFFERS - Swap Rendering Engine Buffer (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_SWAP_BUFFERS; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(2); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_FLUSH = 2;
		#endif
	}
}

//OpenGL states this behaves the same as glFlush but also CPU waits for all commands to be executed by the GPU
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glFinish(void){
	glFlush();
	while( (volatile u32)(*((volatile u32*)GFX_STATUS_ADDR)) & (1 << 27) ){ //27    Geometry Engine Busy (0=No, 1=Yes; Busy; Commands are executing)
		
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadMatrix4x4(m4x4 * m){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_LOAD_4x4; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[2]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[3]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[4]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[5]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[6]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[7]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[8]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[9]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[10]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[11]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[12]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[13]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[14]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[15]; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
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
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadMatrix4x3(m4x3* m){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400045Ch 17h 12 30  MTX_LOAD_4x3 - Load 4x3 Matrix to Current Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_LOAD_4x3; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[2]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[3]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[4]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[5]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[6]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[7]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[8]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[9]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[10]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[11]; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
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
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMultMatrix4x4(m4x4* m){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000460h 18h 16 35* MTX_MULT_4x4 - Multiply Current Matrix by 4x4 Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_MULT_4x4; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[2]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[3]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[4]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[5]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[6]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[7]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[8]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[9]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[10]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[11]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[12]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[13]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[14]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[15]; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_MULT4x4 = m->m[0];
		MATRIX_MULT4x4 = m->m[1];
		MATRIX_MULT4x4 = m->m[2];
		MATRIX_MULT4x4 = m->m[3];

		MATRIX_MULT4x4 = m->m[4];
		MATRIX_MULT4x4 = m->m[5];
		MATRIX_MULT4x4 = m->m[6];
		MATRIX_MULT4x4 = m->m[7];

		MATRIX_MULT4x4 = m->m[8];
		MATRIX_MULT4x4 = m->m[9];
		MATRIX_MULT4x4 = m->m[10];
		MATRIX_MULT4x4 = m->m[11];

		MATRIX_MULT4x4 = m->m[12];
		MATRIX_MULT4x4 = m->m[13];
		MATRIX_MULT4x4 = m->m[14];
		MATRIX_MULT4x4 = m->m[15];
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMultMatrix4x3(m4x3* m){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000464h 19h 12 31* MTX_MULT_4x3 - Multiply Current Matrix by 4x3 Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_MULT_4x3; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[2]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[3]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[4]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[5]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[6]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[7]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[8]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[9]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[10]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[11]; ptrVal++;
			
			//4000468h 1Ah 9  28* MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_MULT_3x3; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[2]; ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[3]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[4]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[5]; ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[6]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[7]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)m->m[8]; ptrVal++;
			
			
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
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
		#endif
	}
}

// Integer rotation (not gl standard)
//	based on 512 degree circle
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateZi(int angle){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];	
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
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
		
			//Mul(Rotate)
			//4000468h - Cmd 1Ah - MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
			//Sets C=M*C. Parameters: 9, m[0..8]
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_MULT_3x3;  //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)cosine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)sine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-sine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)cosine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)inttof32(1); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = sine;
		MATRIX_MULT3x3 = 0;

		MATRIX_MULT3x3 = -sine;
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = 0;

		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = inttof32(1);
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateYi(int angle){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
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
		
			//Mul(Rotate)
			//4000468h - Cmd 1Ah - MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
			//Sets C=M*C. Parameters: 9, m[0..8]
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_MULT_3x3;  //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)cosine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-sine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)inttof32(1); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)sine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)cosine; ptrVal++;
			
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = -sine;

		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = inttof32(1);
		MATRIX_MULT3x3 = 0;

		MATRIX_MULT3x3 = sine;
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = cosine;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateXi(int angle){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];	
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
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
		
			//Mul(Rotate)
			//4000468h - Cmd 1Ah - MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
			//Sets C=M*C. Parameters: 9, m[0..8]
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_MULT_3x3;  //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)inttof32(1); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)cosine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)sine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-sine; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)cosine; ptrVal++;
			
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		MATRIX_MULT3x3 = inttof32(1);
		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = 0;

		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = cosine;
		MATRIX_MULT3x3 = sine;

		MATRIX_MULT3x3 = 0;
		MATRIX_MULT3x3 = -sine;
		MATRIX_MULT3x3 = cosine;
		#endif
	}
}

//	rotations wrapped in float...mainly for testing
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateX(float angle){
	glRotateXi((int)(angle * LUT_SIZE / 360.0));
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateY(float angle){
	glRotateYi((int)(angle * LUT_SIZE / 360.0));
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateZ(float angle){
	glRotateZi((int)(angle * LUT_SIZE / 360.0));
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
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

// Fixed point look at function, it appears to work as expected although 
//	testing is recomended
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluLookAtf32(f32 eyex, f32 eyey, f32 eyez, f32 lookAtx, f32 lookAty, f32 lookAtz, f32 upx, f32 upy, f32 upz){
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
	
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			glMatrixMode(GL_MODELVIEW);
			//400045Ch 17h 12 30  MTX_LOAD_4x3 - Load 4x3 Matrix to Current Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_LOAD_4x3; //Unpacked Command format
			ptrVal++;

			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)x[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)x[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)x[2]; ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)y[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)y[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)y[2]; ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)z[0]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)z[1]; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)z[2]; ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)floattof32(-1.0); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			
			glTranslate3f32(-eyex, -eyey, -eyez);			
		}
	}
	else{
		glMatrixMode(GL_MODELVIEW);
		#if defined(ARM9)
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
		#endif
		glTranslate3f32(-eyex, -eyey, -eyez);		
	}
}

//  glu wrapper for standard float call
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluLookAt(float eyex, float eyey, float eyez, float lookAtx, float lookAty, float lookAtz, float upx, float upy, float upz){
	gluLookAtf32(floattof32(eyex), floattof32(eyey), floattof32(eyez), floattof32(lookAtx), floattof32(lookAty), floattof32(lookAtz),
					floattof32(upx), floattof32(upy), floattof32(upz));
}

//	frustrum has only been tested as part of perspective
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluFrustumf32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			glMatrixMode(GL_PROJECTION);
			//4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_LOAD_4x4; //Unpacked Command format
			ptrVal++;

			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)divf32(2*near, right - left); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)divf32(right + left, right - left); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
		
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)divf32(2*near, top - bottom); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)divf32(top + bottom, top - bottom); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;

			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-divf32(far + near, far - near); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)floattof32(-1.0F); ptrVal++;

			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-divf32(2 * mulf32(far, near), far - near); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			glStoreMatrix(0);			
		}
	}
	else{
		glMatrixMode(GL_PROJECTION);
		#if defined(ARM9)
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
		#endif
		glStoreMatrix(0);	   
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glOrtho(float left, float right, float bottom, float top, float near, float far){
	glOrthof32(floattof32(left), floattof32(right), floattof32(bottom), floattof32(top), floattof32(near), floattof32(far));
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glOrthof32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far){	
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			glMatrixMode(GL_PROJECTION);
			//4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getMTX_LOAD_4x4; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)divf32(2, right - left); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-divf32(right + left, right - left); ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)divf32(2, top - bottom); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-divf32(top + bottom, top - bottom); ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)divf32(-2, far - near); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)-divf32(far + near, far - near); ptrVal++;
			
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)0; ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)floattof32(1.0F); ptrVal++;			
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			glStoreMatrix(0);			
		}
	}
	else{
		glMatrixMode(GL_PROJECTION);
		#if defined(ARM9)
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
		#endif
		glStoreMatrix(0);
	}
}

//  Frustrum wrapper
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluFrustum(float left, float right, float bottom, float top, float near, float far){
	gluFrustumf32(floattof32(left), floattof32(right), floattof32(bottom), floattof32(top), floattof32(near), floattof32(far));
}

//	Fixed point perspective setting
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluPerspectivef32(int fovy, f32 aspect, f32 zNear, f32 zFar){
   f32 xmin, xmax, ymin, ymax;

   ymax = mulf32(zNear, TAN[fovy & LUT_MASK]);
   ymin = -ymax;
   xmin = mulf32(ymin, aspect);
   xmax = mulf32(ymax, aspect);

   gluFrustumf32(xmin, xmax, ymin, ymax, zNear, zFar);
}

//  glu wrapper for floating point
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluPerspective(float fovy, float aspect, float zNear, float zFar){
	gluPerspectivef32((int)(fovy * LUT_SIZE / 360.0), floattof32(aspect), floattof32(zNear), floattof32(zFar));    
}


uint32 diffuse_ambient = 0;
uint32 specular_emission = 0;

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMaterialf(int mode, rgb color){ //so far here
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

	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//40004C0h 30h 1  4   DIF_AMB - MaterialColor0 - Diffuse/Ambient Reflect. (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_DIFFUSE_AMBIENT; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(diffuse_ambient); ptrVal++;
			
			//40004C4h 31h 1  4   SPE_EMI - MaterialColor1 - Specular Ref. & Emission (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_SPECULAR_EMISSION; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(specular_emission); ptrVal++;
			
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_DIFFUSE_AMBIENT = diffuse_ambient;
		GFX_SPECULAR_EMISSION = specular_emission;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glResetMatrixStack(void){
  // stack overflow ack ?
  GFX_STATUS |= 1 << 15;

  // pop the stacks to the top...seems projection stack is only 1 deep??
  glMatrixMode(GL_PROJECTION);
  glPopMatrix((GFX_STATUS>>13) & 1);
  
  // 31 deep modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix((GFX_STATUS >> 8) & 0x1F);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glSetOutlineColor(int id, rgb color){
	#if defined(ARM9)
	GFX_EDGE_TABLE[id] = color;
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glSetToonTable(uint16 *table){
	int i;
	for( i = 0; i < 32; i++ ){
		#if defined(ARM9)
		GFX_TOON_TABLE[i] = table[i];
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glSetToonTableRange(int start, int end, rgb color){
	int i;
	for( i = start; i <= end; i++ ){
		#if defined(ARM9)
		GFX_TOON_TABLE[i] = color;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glReset(void){
	#ifdef ARM9
	while (GFX_STATUS & (1<<27)); // wait till gfx engine is not busy
  
	// Clear the FIFO
	GFX_STATUS |= (1<<29);

	// Clear overflows for list memory
	GFX_CONTROL = enable_bits = ((1<<12) | (1<<13)) | GL_TEXTURE_2D;
	glResetMatrixStack();
  
	GFX_TEX_FORMAT = textureParamsValue = 0;
	GFX_POLYGON_ATTR = 0;
  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Texture
/////////////////////////////////////////////////////////////////////
// Texture globals

uint32 textures[MAX_TEXTURES];
uint32 activeTexture = 0;
uint32* nextBlock = (uint32*)0x06800000;

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glResetTextures(void){
	activeTexture = 0;
	nextBlock = (uint32*)0x06800000;
}

// glGenTextures creates intiger names for your table
//	takes n as the number of textures to generate and 
//	a pointer to the names array that it needs to fill.
//  Returns 1 if succesful and 0 if out of texture names
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int glGenTextures(int n, int *names){
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

// glBindTexure sets the current named
//	texture to the active texture.  Target 
//	is ignored as all DS textures are 2D
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glBindTexture(int target, int name){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//40004A8h 2Ah 1  1   TEXIMAGE_PARAM - Set Texture Parameters (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_TEX_FORMAT; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(textures[name]); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_TEX_FORMAT = textureParamsValue = textures[name];
		#endif
	}
	activeTexture = name;
}

// glTexParameter although named the same 
//	as its gl counterpart it is not compatible
//	Effort may be made in the future to make it so.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTexParameter(uint8 sizeX, uint8 sizeY, uint32* addr, uint8 mode, uint32 param){
	textures[activeTexture] = param | (sizeX << 20) | (sizeY << 23) | (((uint32)addr >> 3) & 0xFFFF) | (mode << 26);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
uint16* vramGetBank(uint16 *addr){
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int vramIsTextureBank(uint16 *addr){
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
	#ifdef WIN32
	return 0;
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
uint32* getNextTextureSlot(int size){
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
	#ifdef WIN32
	return 0;
	#endif
}

// Similer to glTextImage2D from gl it takes a pointer to data
//	Empty fields and target are unused but provided for code compatibility.
//	type is simply the texture type (GL_RGB, GL_RGB8 ect...)
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int glTexImage2D(int target, int empty1, int type, int sizeX, int sizeY, int empty2, int param, uint8* texture){
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
	#if defined(ARM9)
	GFX_TEX_FORMAT = textureParamsValue = (sizeX << 20) | (sizeY << 23) | ((type == GL_RGB ? GL_RGBA : type ) << 26);
	#endif
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
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glVertex2i(int x, int y) {
    glVertex2v16(inttov16(x), inttov16(y));
}

//float x , y vertex coords in v16 format
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glVertex2f(float x, float y) {
    glVertex2v16(floattov16(x), floattov16(y));
}

//float x , y , z vertex coords in v16 format
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glVertex3f(GLfloat x, GLfloat y, GLfloat z){
	glVertex3v16(floattov16(x), floattov16(y), floattov16(z));
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glShadeModel(GLenum mode){
	globalGLCtx.primitiveShadeModelMode = mode;
	lastVertexColor = 0;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glColor3f(float red, float green, float blue){
	glColor3b(floattov10(red), floattov10(green), floattov10(blue));

	//Todo: detect light sources and apply colors. Normal GL calls resort to glColor to update material color + light color + texture color
	int id = 0; 
	glLight(id, RGB15(floatto12d3(red)<<1,floatto12d3(green)<<1,floatto12d3(blue)<<1), inttov10(31), inttov10(31), inttov10(31));
}

//Open GL 1.1 Implementation: Texture Objects support
//glTexImage*() == glTexImage3D
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels){

}

//glTexSubImage*() == glTexSubImage3D
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels){
	//The glTexSubImage2D function specifies a portion of an existing one-dimensional texture image. You cannot define a new texture with glTexSubImage2D.
}

//glCopyTexImage*() == glCopyTexImage2D
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border){
	//replaces glTexImage2D() which is hardcoded to a big texture
}

//glCopyTexSubImage*() == glCopyTexSubImage3D
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
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
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities){
	//DS 3D GPU does not support texture priority. There may be a way by sorting them by color but
}

#ifdef ARM9
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glCallListGX(const u32* list) {
	#ifdef ARM9
	u32 count = *list++;

	// flush the area that we are going to DMA
	coherent_user_range_by_size((uint32)list, count);
	
	// send the packed list synchronously via DMA to the FIFO
	DMAXSAD(0) = (u32)list;
	DMAXDAD(0) = 0x4000400;
	DMAXCNT(0) = DMA_FIFO | count;
	while(DMAXCNT(0) & DMAENABLED);
	//printf("(NDS)glCallListGX: Executing DL List. Size: %d", (int)count);
	#endif

	#ifdef WIN32
	char displayListName[256];
	//must be 1:1 from isAGXCommand
	u32 cmd = list[1];
	int cmdParamCount = getAGXParamsCountFromCommand(cmd);
	if (cmd == (u32)getMTX_STORE) {
		strcpy(displayListName, "getMTX_STORE");
	}
	else if (cmd == (u32)getMTX_TRANS) {
		strcpy(displayListName, "getMTX_TRANS");
	}
	else if (cmd == (u32)getMTX_IDENTITY) {
		strcpy(displayListName, "getMTX_IDENTITY");
	}
	else if (cmd == (u32)getMTX_MODE) {
		strcpy(displayListName, "getMTX_MODE");
	}
	else if (cmd == (u32)getVIEWPORT) {
		strcpy(displayListName, "getVIEWPORT");
	}
	else if (cmd == (u32)getFIFO_TEX_COORD) {
		strcpy(displayListName, "getFIFO_TEX_COORD");
	}
	else if (cmd == (u32)getFIFO_BEGIN) {
		strcpy(displayListName, "getFIFO_BEGIN");
	}
	else if (cmd == (u32)getFIFO_END) {
		strcpy(displayListName, "getFIFO_END");
	}
	else if (cmd == (u32)getFIFO_COLOR) {
		strcpy(displayListName, "getFIFO_COLOR");
	}
	else if (cmd == (u32)getFIFO_NORMAL) {
		strcpy(displayListName, "getFIFO_NORMAL");
	}
	else if (cmd == (u32)getFIFO_VERTEX16) {
		strcpy(displayListName, "getFIFO_VERTEX16");
	}
	else if (cmd == (u32)getFIFO_VERTEX10) {
		strcpy(displayListName, "getFIFO_VERTEX10");
	}
	else if (cmd == (u32)getFIFO_VTX_XY) {
		strcpy(displayListName, "getFIFO_VTX_XY");
	}
	else if (cmd == (u32)getMTX_PUSH) {
		strcpy(displayListName, "getMTX_PUSH");
	}
	else if (cmd == (u32)getMTX_POP) {
		strcpy(displayListName, "getMTX_POP");
	}
	else if (cmd == (u32)getMTX_MULT_3x3) {
		strcpy(displayListName, "getMTX_MULT_3x3");
	}
	else if (cmd == (u32)getMTX_MULT_4x4) {
		strcpy(displayListName, "getMTX_MULT_4x4");
	}

	///////////////Custom Display Lists
	else{
		strcpy(displayListName, "CUSTOM DISPLAY LIST OR OTHER CMD");
	}
	printf("\n(WIN32)glCallListGX: Executing DL[%s]; Size: %d\n",displayListName, (int)list[0]);
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glColor3fv(const GLfloat * v){
	if(v != NULL){
		float red = v[0];
		float green = v[1];
		float blue = v[2];
		glColor3f(red, green, blue);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
int getTextureBaseFromTextureSlot(int textureSlot){
	u32 textureDescriptor = textures[textureSlot];
	u8 baseTexSize1 = ((textureDescriptor >> 20) & TEXTURE_SIZE_1024);
	u8 baseTexSize2 = ((textureDescriptor >> 23) & TEXTURE_SIZE_1024);
	int res = (baseTexSize1*baseTexSize2*7);
	return ((res != 0) ? res : 8);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glTexCoord2f(GLfloat s, GLfloat t){
	int texBase = getTextureBaseFromTextureSlot(activeTexture);
	if(s > 0.0){
		s = s + (texBase);
	}
	if(t > 0.0){
		t = t + (texBase);
	}
	glTexCoord2t16(floattot16(t), floattot16(s));
}

//////////////////////////////////////////////////////////////////////
//glTexCoord specifies texture coordinates in one, two, three, or four dimensions. 
//glTexCoord1 sets the current texture coordinates to s 0 0 1 ; 
//glTexCoord2 sets them to s t 0 1 . 
//glTexCoord3 specifies the texture coordinates as s t r 1
//glTexCoord4 defines all four components explicitly as s t r q .

//Note: uv == ((u << 16) | (v & 0xFFFF))
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glTexCoord1i(uint32 uv){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_TEX_COORD; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)uv; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_TEX_COORD = uv;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glTexCoord2t16(t16 u, t16 v){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_TEX_COORD; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)TEXTURE_PACK(u, v); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_TEX_COORD = (u32)TEXTURE_PACK(u, v);
		#endif
	}
}

//Primitive types
//0  Separate Triangle(s)    ;3*N vertices per N triangles
//1  Separate Quadliteral(s) ;4*N vertices per N quads
//2  Triangle Strips         ;3+(N-1) vertices per N triangles
//3  Quadliteral Strips      ;4+(N-1)*2 vertices per N quads
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glBegin(int primitiveType){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000500h 40h 1  1   BEGIN_VTXS - Start of Vertex List (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_BEGIN; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)primitiveType; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_BEGIN = (u32)primitiveType;
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glEnd( void){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000504h 41h -  1   END_VTXS - End of Vertex List (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_END; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_END = 0;
		#endif
	}
}

u16 lastVertexColor = 0;
//set the current color
//4000480h - Cmd 20h - COLOR - Directly Set Vertex Color (W)
//Parameter 1, Bit 0-4    Red
//Parameter 1, Bit 5-9    Green
//Parameter 1, Bit 10-14  Blue

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glColor3b(uint8 red, uint8 green, uint8 blue){
	u16 finalColor = 0;
	switch(globalGLCtx.primitiveShadeModelMode){
		//light vectors are todo
		case(GL_FLAT):{
			//otherwise override all colors to be the same subset of whatever color was passed here
			if(lastVertexColor == 0){
				lastVertexColor = RGB15(red, green, blue);
			}
			finalColor = lastVertexColor;
		}
		break;
		
		case(GL_SMOOTH):{
			//Smooth shading, the default by DS, causes the computed colors of vertices to be interpolated as the primitive is rasterized, 
			//typically assigning different colors to each resulting pixel fragment. 
			finalColor = (vuint32)RGB15(red, green, blue);			
		}
		break;
		
		default:{
			//error! call glInit(); first
			return;
		}
		break;
	}
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000480h 20h 1  1   COLOR - Directly Set Vertex Color (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_COLOR; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)finalColor; ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_COLOR = (u32)finalColor;
		#endif
	}
}

//glNormal: Sets the current normal vector
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3b(
	GLbyte nx,
 	GLbyte ny,
 	GLbyte nz
){
	glNormal3i((GLint)nx, (GLint)ny,(GLint)nz);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3d(
	GLdouble nx,
 	GLdouble ny,
 	GLdouble nz
){
	glNormal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3f(
	GLfloat nx,
 	GLfloat ny,
 	GLfloat nz
){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_NORMAL; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)NORMAL_PACK(floattov10(nx),floattov10(ny),floattov10(nz)); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_NORMAL = (u32)NORMAL_PACK(floattov10(nx),floattov10(ny),floattov10(nz));
		#endif
	}	
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3s(
	GLshort nx,
 	GLshort ny,
 	GLshort nz
){
	glNormal3i((GLint)nx, (GLint)ny,(GLint)nz);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3i(
	GLint nx,
 	GLint ny,
 	GLint nz
){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_NORMAL; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)NORMAL_PACK(inttov10(nx),inttov10(ny),inttov10(nz)); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_NORMAL = (u32)NORMAL_PACK(inttov10(nx),inttov10(ny),inttov10(nz));
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex3v16(v16 x, v16 y, v16 z){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400048Ch 23h 2  9   VTX_16 - Set Vertex XYZ Coordinates (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_VERTEX16; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)(y << 16) | (x & 0xFFFF); ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)((uint32)(uint16)z); ptrVal++;
			
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_VERTEX16 = (y << 16) | (x & 0xFFFF);
		GFX_VERTEX16 = ((uint32)(uint16)z);
		#endif
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex3v10(v10 x, v10 y, v10 z){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000490h 24h 1  8   VTX_10 - Set Vertex XYZ Coordinates (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_VERTEX10; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)VERTEX_PACKv10(x, y, z); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_VERTEX10 = (u32)VERTEX_PACKv10(x, y, z);
		#endif
	}
}

//Parameters. x. Specifies the x-coordinate of a vertex. y. Specifies the y-coordinate of a vertex
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex2v16(v16 x, v16 y){
	if(isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000494h 25h 1  8   VTX_XY - Set Vertex XY Coordinates (W)
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)getFIFO_VTX_XY; //Unpacked Command format
			ptrVal++;
			InternalUnpackedGX_DL_Binary[ptrVal + InternalUnpackedGX_DL_workSize] = (u32)VERTEX_PACK(x, y); ptrVal++;
			InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_VERTEX_XY = (u32)VERTEX_PACK(x, y);
		#endif
	}
}

//Todo
//4000498h 26h 1  8   VTX_XZ - Set Vertex XZ Coordinates (W)
//400049Ch 27h 1  8   VTX_YZ - Set Vertex YZ Coordinates (W)
//40004A0h 28h 1  8   VTX_DIFF - Set Relative Vertex Coordinates (W)
//////////////////////////////////////////////////////////// Standard OpenGL 1.x end //////////////////////////////////////////



//////////////////////////////////////////////////////////// Extended Display List OpenGL 1.x start //////////////////////////////////////////
//DL Notes: Are sent using unpacked format.
//Unpacked Command format:
//Opcodes that write more than one 32bit value (ie. STRD and STM) can be used to send ONE UNPACKED command, 
//plus any parameters which belong to that command. 
//After that, there must be a 1 cycle delay before sending the next command 
//(ie. one cannot sent more than one command at once with a single opcode, each command must be invoked by a new opcode).

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
bool isAnOpenGLExtendedDisplayListCallList; //Toggles between a custom DL constructed on the GX buffer or a direct GX command

//OpenGL DL internal Display Lists enumerator: stores multiple DL pointed by current InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr, starting from 0.
#ifdef ARM9
//__attribute__((section(".dtcm")))
#endif
GLsizei Compiled_DL_Binary_Descriptor[InternalUnpackedGX_DL_workSize];

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 LastGXInternalDisplayListPtr=0; //enumerates last list allocated by glNewList()

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 LastActiveOpenGLDisplayList;

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
u32 * getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(){
	return (u32 *)&InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_OpenGLDisplayListStartOffset];
}

//glGenLists returns the first list name in a range of the length you pass to glGenLists.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
GLuint glGenLists(GLsizei	range){
    if(range < InternalUnpackedGX_DL_workSize){
        int i = 0;
		for(i = 0; i < InternalUnpackedGX_DL_workSize; i++ ){
			Compiled_DL_Binary_Descriptor[i] = DL_INVALID; //Compiled_DL_Binary_Descriptor[i] = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr; //only mapped from glNewList()
		}
		isAnOpenGLExtendedDisplayListCallList = false; //cut-off incoming new OpenGL extended DLs
		LastGXInternalDisplayListPtr=0; //reset to default list start
		InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = 1; //OFFSET 0 IS DL SIZE
        LastActiveOpenGLDisplayList = DL_INVALID;
		return InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr; //starts from range of said length
    }
    return 0;
}

//Specifies the offset that's added to the display-list indices in glCallLists() to obtain the final display-list indices. The default display-list base is 0. The list base has no effect on glCallList(), which executes only one display list or on glNewList().
//Internally, if a new base is set, a pointer to a new section is updated
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glListBase(GLuint base){
	if(base > 0){
		base--;
	}
	if((u32)Compiled_DL_Binary_Descriptor[base] != (u32)DL_INVALID){
		InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = Compiled_DL_Binary_Descriptor[base];
	}
}

//glIsList returns GL_TRUE if list is the name of a display list and returns GL_FALSE if it is not, or if an error occurs.
//A name returned by glGenLists, but not yet associated with a display list by calling glNewList, is not the name of a display list.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
GLboolean glIsList(GLuint list){
	if(list > 0){
		list--;
	}
	if((u32)Compiled_DL_Binary_Descriptor[list] != (u32)DL_INVALID){
		return GL_TRUE;
	}

	return GL_FALSE;
	//Todo: GL_INVALID_OPERATION;
}

//list:Specifies the display-list name.
//mode:Specifies the compilation mode, which can be GL_COMPILE or GL_COMPILE_AND_EXECUTE.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNewList(GLuint list, GLenum mode){
	if(list >= 1){
		list--; //assign current InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr (new) to a List
	}
	InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr++; //OFFSET 0 IS DL SIZE
	Compiled_DL_Binary_Descriptor[list] = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr; 
	globalGLCtx.mode = (u32)mode;
	LastActiveOpenGLDisplayList = (u32)list;
	isAnOpenGLExtendedDisplayListCallList = true;
}

//When glEndList is encountered, the display-list definition is completed 
//by associating the list with the unique name list (specified in the glNewList command). 
//If a display list with name list already exists, it is replaced only when glEndList is called.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glEndList(void){
	int listSize = 0;
	//If LAST display-list name is GL_COMPILE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL. Such binary will be manually executed when glCallList(display-list name) is called 
	//Else If LAST display-list name is GL_COMPILE_AND_EXECUTE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL and executes it inmediately through GX GLCallList()
	
	//define List Size
	listSize = ((InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr - LastGXInternalDisplayListPtr) * 4) + 4;
	InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_OpenGLDisplayListStartOffset + LastGXInternalDisplayListPtr] = (u32)listSize;
	InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr++;
	LastGXInternalDisplayListPtr = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;	

	if(globalGLCtx.mode == GL_COMPILE_AND_EXECUTE){
		glCallList((GLuint)LastActiveOpenGLDisplayList);
	}
	isAnOpenGLExtendedDisplayListCallList = false;
	LastActiveOpenGLDisplayList = DL_INVALID;
	globalGLCtx.mode = GL_COMPILE;
}

/*
OpenGL Display List execution which implements the lower level GX hardware Display Lists execution.
params: 
list
Specifies the integer name of the display list to be executed.
*/
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glCallList(GLuint list){
	if(list != DL_INVALID){
		u32 * InternalDL = getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr();
		int curDLInCompiledDLOffset = 0;
		int singleListSize = 0;
		if(list > 0){
			list--;
		}
		curDLInCompiledDLOffset = Compiled_DL_Binary_Descriptor[list];
		if((u32)curDLInCompiledDLOffset != DL_INVALID){
			u32 * currentPhysicalDisplayListStart = (u32 *)&InternalDL[curDLInCompiledDLOffset];
			if(curDLInCompiledDLOffset == 2){
				currentPhysicalDisplayListStart-=2;
				*(currentPhysicalDisplayListStart+1)=(*currentPhysicalDisplayListStart)-4;
				currentPhysicalDisplayListStart++;
			}
			else{
				currentPhysicalDisplayListStart--;
			}
			singleListSize = *currentPhysicalDisplayListStart;
			if(singleListSize > 0){
				//Run a single GX Display List, having proper DL size
				u32 customsingleOpenGLCompiledDisplayListPtr = (singleListSize/4); //account the internal pointer ahead because DLs executed later are treated as the internal DL GX Binary
				handleInmediateGXDisplayList(currentPhysicalDisplayListStart, (u32*)&customsingleOpenGLCompiledDisplayListPtr, OPENGL_DL_TO_GX_DL_EXEC_CMD, singleListSize/4); 
				#ifdef WIN32
				printf("//////////////////////[OpenGL CallList: %d]//////////////////////////", list);
				#endif
			}
			else{
				//printf("glCallList():This OpenGL list name(%d)'s InternalDL offset points to InternalDL GX end (no more GX DL after this)", (u32)list);
			}
		}
	}
}

/*
 Using display lists
We have seen how to creates a display list.
You should now want to use it :
    gl.glCallList(list)

You probably wants to call multiple lists :
    gl.glListBase(listBase)
    gl.glCallLists(nbLists, type, referenceOffsets)
The first method select the display list refered by listBase as the base list. Indices in the referenceOffsets array are counter by considering that listBase is 0.
nbLists is the number of lists that should be called.
type is the type of information stored in referenceOffsets. It is typically : GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT or GL_UNSIGNED_INT.
indiceOffset is an array that holds offset reference to the desired list to call by comparison to listBase (Ex: if listBase = 100 and you want to draw a list refered by 103, its offset reference is 3).
*/
/*
OpenGL Display Lists collection execution. For each list it'll be executed as a single OpenGL Display List

params:
n
Specifies the number of display lists to be executed.

type
Specifies the type of values in lists. Symbolic constants GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT are accepted.

lists
Specifies the address of an array of name offsets in the display list. The pointer type is void because the offsets can be bytes, shorts, ints, or floats, depending on the value of type.
*/
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glCallLists(GLsizei n, GLenum type, const void * lists){
	int offsetSize = -1;
	GLubyte * u8array = NULL;
	u16 * u16array = NULL;
	u32 * u32array = NULL;

	switch(type){
		//C Type	Bitdepth	Description								Common Enum
		
		//GLbyte	8			Signed, 2's complement binary integer	GL_BYTE
		case(GL_BYTE):{
			offsetSize = 1;
			u8array = (GLubyte *)lists;
		}
		break;

		//GLubyte	8			Unsigned binary integer					GL_UNSIGNED_BYTE
		case(GL_UNSIGNED_BYTE):{
			offsetSize = 1;
			u8array = (GLubyte *)lists;
		}
		break;

		//GLshort	16			Signed, 2's complement binary integer	GL_SHORT
		case(GL_SHORT):{
			offsetSize = 2;
			u16array = (u16 *)lists;
		}
		break;

		//GLushort	16			Unsigned binary integer					GL_UNSIGNED_SHORT
		case(GL_UNSIGNED_SHORT):{
			offsetSize = 2;
			u16array = (u16 *)lists;
		}
		break;

		//GLint		32			Signed, 2's complement binary integer	GL_INT
		case(GL_INT):{
			offsetSize = 4;
			u32array = (u32 *)lists;
		}
		break;

		//GLuint	32			Unsigned binary integer					GL_UNSIGNED_INT
		case(GL_UNSIGNED_INT):{
			offsetSize = 4;
			u32array = (u32 *)lists;
		}
		break;

		//GLfloat	32			An IEEE-754 floating-point value		GL_FLOAT
		case(GL_FLOAT):{
			offsetSize = 4;
			u32array = (u32 *)lists;
		}
		break;
	}

	//Valid? Run each one of them
	if((n > 0) && (offsetSize != -1)){
		int i = 0;
		for(i = 0; i < n; i++){
			GLuint listName = (GLuint)-1;
			
			if(u8array != NULL){
				listName = (GLuint)u8array[i];
			}
			else if(u16array != NULL){
				listName = (GLuint)u16array[i];
			}
			else if(u32array != NULL){
				listName = (GLuint)u32array[i];
			}
			
			if(
				((GLuint)listName != (GLuint)-1)
				&&
				(n > listName) 	//	//
				&&				//	|| Prevent out-of-bound lists
				(listName >= 0)	//
			){
				glCallList(listName);
			}
		}
	}
}

//Note: It's assumed DisplayLists are at the end of the physical DL GX Binary. //todo: maybe have 4K of physical DL for direct OGL cmds, and maybe the other 4K for display list OGL cmds
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glDeleteLists(GLuint list, GLsizei range){
	int lowestCurDLInCompiledDLOffset = 0;
	int i = 0;
	if(list >= 1){
		list--; //assign current InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr (new) to a List
	}
	if (list >= DL_DESCRIPTOR_MAX_ITEMS){
		return;
	}
	for(i = 0; i < range; i++){
		u32 * InternalDL = getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr();
		int curDLInCompiledDLOffset = Compiled_DL_Binary_Descriptor[list + i];
		if((u32)curDLInCompiledDLOffset != DL_INVALID){
			Compiled_DL_Binary_Descriptor[list + i] = DL_INVALID;

			if(lowestCurDLInCompiledDLOffset > curDLInCompiledDLOffset){
				lowestCurDLInCompiledDLOffset = curDLInCompiledDLOffset;
			}
		}
	}
	
	//Find the lowest internal buffer offset assigned, just deleted, and rewind it so it points to unallocated Internal DL memory
	if(lowestCurDLInCompiledDLOffset != -1){
		InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = lowestCurDLInCompiledDLOffset;
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
enum GL_GLBEGIN_ENUM getDisplayListGLType(struct ndsDisplayListDescriptor * dlInst){
	if(dlInst != NULL){
		return (enum GL_GLBEGIN_ENUM)dlInst->DL[1].value;
	}
	return (enum GL_GLBEGIN_ENUM)DL_INVALID;
}

//Compiles a NDS GX Display List / CallList binary using the Command Packed format, from an object one. Understood by the GX hardware.
//Returns: List count (multiplied by 4 is the file size), DL_INVALID if fails.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int CompilePackedNDSGXDisplayListFromObject(u32 * bufOut, struct ndsDisplayListDescriptor * dlInst){
	int i = 0; 
	if( (dlInst != NULL) && (bufOut != NULL)){
		*(bufOut) = dlInst->ndsDisplayListSize;
		bufOut++;
		for(i = 0; i < dlInst->ndsDisplayListSize; i++){
			struct ndsDisplayList * curDL = &dlInst->DL[i];
			*(bufOut) = curDL->value;
			bufOut++;
		}
		return i;
	}

	return DL_INVALID;
}

//////////////////////////////////////////////////////////// Extended Display List OpenGL 1.x end 
void glLightfv (GLenum light, GLenum pname, const GLfloat *params){
	//El parámetro params contiene cuatro valores de punto flotante que especifican la intensidad RGBA ambiente de la luz. Los valores de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante. La intensidad de luz ambiente predeterminada es (0,0, 0,0, 0,0, 1,0).
	if(pname == GL_AMBIENT){
		float rAmbient = params[0];
		float gAmbient = params[1];
		float bAmbient = params[2];
		float aAmbient = params[3];
		ambientValue = ((floattov10(rAmbient) & 0x1F) << 16) | ((floattov10(gAmbient) & 0x1F) << 21) | ((floattov10(bAmbient) & 0x1F) << 26);
		u32 diffuseAmbientWrite = ( ((diffuseValue & 0xFFFF) << 0) | ((ambientValue & 0xFFFF) << 16) );
		GFX_DIFFUSE_AMBIENT = diffuseAmbientWrite;
	}
	//El parámetro params contiene cuatro valores de punto flotante que especifican la intensidad RGBA difusa de la luz. Los valores de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante. La intensidad difusa predeterminada es (0,0, 0,0, 0,0, 1,0) para todas las luces que no sean cero. La intensidad difusa predeterminada de la luz cero es (1,0, 1,0, 1,0, 1,0).
	if(pname == GL_DIFFUSE){
		float rDiffuse = params[0];
		float gDiffuse = params[1];
		float bDiffuse = params[2];
		float aDiffuse = params[3];
		u8 setVtxColor = 1; //15    Set Vertex Color (0=No, 1=Set Diffuse Reflection Color as Vertex Color)
		diffuseValue = ((floattov10(rDiffuse) & 0x1F) << 0) | ((floattov10(gDiffuse) & 0x1F) << 5) | ((floattov10(bDiffuse) & 0x1F) << 10) | ((setVtxColor & 0x1) << 15);
		u32 diffuseAmbientWrite = ( ((diffuseValue & 0xFFFF) << 0) | ((ambientValue & 0xFFFF) << 16) );
		GFX_DIFFUSE_AMBIENT = diffuseAmbientWrite;
	}
	//El parámetro params contiene cuatro valores de punto flotante que especifican la posición de la luz en coordenadas de objeto homogéneas. Los valores enteros y de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante.
	//La posición se transforma mediante la matriz modelview cuando se llama a glLightfv (como si fuera un punto) y se almacena en coordenadas oculares. Si el componente w de la posición es 0,0, la luz se trata como una fuente direccional. Los cálculos de iluminación difusa y especular toman la dirección de la luz, pero no su posición real, en cuenta y la atenuación está deshabilitada. De lo contrario, los cálculos de iluminación difusa y especular se basan en la ubicación real de la luz en coordenadas oculares y se habilita la atenuación. La posición predeterminada es (0,0,1,0); por lo tanto, la fuente de luz predeterminada es direccional, paralela a y en la dirección del eje -z .
	if(pname == GL_POSITION){
		int id = ((((int)light) & 3) << 30);
		float x = params[0];
		float y = params[1];
		float z = params[2];
		GFX_LIGHT_VECTOR = id | ((floattov10(z) & 0x3FF) << 20) | ((floattov10(y) & 0x3FF) << 10) | (floattov10(x) & 0x3FF);
	}
	//El parámetro params contiene cuatro valores de punto flotante que especifican la intensidad RGBA especular de la luz. Los valores de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante. La intensidad especular predeterminada es (0,0, 0,0, 0,0, 1,0) para todas las luces que no sean cero. La intensidad especular predeterminada del cero claro es (1,0, 1,0, 1,0, 1,0).
	if(pname == GL_SPECULAR){
		float rSpecular = params[0];
		float gSpecular = params[1];
		float bSpecular = params[2];
		float aSpecular = params[3];
		u8 useSpecularReflectionShininessTable = 0; //15    Specular Reflection Shininess Table (0=Disable, 1=Enable)
		specularValue = ((floattov10(rSpecular) & 0x1F) << 16) | ((floattov10(gSpecular) & 0x1F) << 21) | ((floattov10(bSpecular) & 0x1F) << 26) | ((useSpecularReflectionShininessTable & 0x1) << 15);
		u32 specularEmissionWrite = ( ((specularValue & 0xFFFF) << 0) | ((emissionValue & 0xFFFF) << 16) );
		GFX_SPECULAR_EMISSION = specularEmissionWrite;
	}
	
	//Unimplemented:
	//GL_SPOT_DIRECTION (GX hardware doesn't support it)
	//GL_SPOT_EXPONENT (GX hardware doesn't support it)
	//GL_SPOT_CUTOFF (GX hardware doesn't support it)
	//GL_CONSTANT_ATTENUATION (GX hardware doesn't support it) 
	//GL_LINEAR_ATTENUATION (GX hardware doesn't support it)
	//GL_QUADRATIC_ATTENUATION (GX hardware doesn't support it)
}

void glMaterialfv (GLenum face, GLenum pname, const GLfloat *params){
	//GX hardware does support "face" attributes:
	//face, specifies whether the GL_FRONT materials, the GL_BACK materials, or both GL_FRONT_AND_BACK materials will be modified. 
	if(face == GL_FRONT){

	}
	else if(face == GL_BACK){
		//GX assumes always this because polygon attribute sets CULL to BACK. todo: inherit to polys
		//glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK );
	}
	else if(face == GL_FRONT_AND_BACK){
		
	}

	//The params parameter contains four floating-point values that specify the RGBA emitted light intensity of the material. Integer values are mapped linearly such that the most positive representable value maps to 1.0, and the most negative representable value maps to -1.0. Floating-point values are mapped directly. Neither integer nor floating-point values are clamped. The default emission intensity for both front-facing and back-facing materials is (0.0, 0.0, 0.0, 1.0).
	if(pname == GL_EMISSION){
		float rEmission = params[0];
		float gEmission = params[1];
		float bEmission = params[2];
		float aEmission = params[3];
		emissionValue = ((floattov10(rEmission) & 0x1F) << 16) | ((floattov10(gEmission) & 0x1F) << 21) | ((floattov10(bEmission) & 0x1F) << 26);
		u32 specularEmissionWrite = ( ((specularValue & 0xFFFF) << 0) | ((emissionValue & 0xFFFF) << 16) );
		GFX_SPECULAR_EMISSION = specularEmissionWrite;
	}

	//	The param parameter is a single integer value that specifies the RGBA specular exponent of the material. Integer values are mapped directly. Only values in the range [0, 128] are accepted. The default specular exponent for both front-facing and back-facing materials is 0.
	if(pname == GL_SHININESS){
		u8 useSpecularReflectionShininessTable = ((specularValue >> 15) & 0x1);
		if(useSpecularReflectionShininessTable == true){
			uint32 shiny32[128/4];
			uint8  *shiny8 = (uint8*)shiny32;
			int i;
			for (i = 0; i < 128 * 2; i += 2){
				shiny8[i>>1] = i;
			}
			for (i = 0; i < 128 / 4; i++){
				GFX_SHININESS = shiny32[i];
			}
		}
		//If the table is disabled (by MaterialColor1.Bit15), then reflection will act as if the table would be filled with linear increasing numbers.
		else{
			float MaterialSpecularComponent = params[0];
			GFX_SHININESS = floattov10(MaterialSpecularComponent);
		}
	}

	//Todo: GL_COLOR_INDEXES

	//Unimplemented:
	//GL_AMBIENT (GX hardware doesn't support it)
	//GL_DIFFUSE (GX hardware doesn't support it)
	//GL_SPECULAR (GX hardware doesn't support it)
}

//v: A pointer to an array of three elements: the x, y, and z coordinates of the new current normal.
void glNormal3fv(const GLfloat *v){
	glNormal3f(v[0], v[1], v[2]);
}

//v: A pointer to an array of three elements. The elements are the x, y, and z coordinates of a vertex.
void glVertex3fv(const GLfloat *v){
	glVertex3f(v[0], v[1], v[2]);
}


//target: The target texture, which must be either GL_TEXTURE_1D or GL_TEXTURE_2D.
//pname: The symbolic name of a single valued texture parameter. The following symbols are accepted in pname.
void glTexParameteri(
   GLenum target,
   GLenum pname,
   GLint  param
){
	target = GL_TEXTURE_2D;

	//16    Repeat in S Direction (0=Clamp Texture, 1=Repeat Texture)
	if((param & GL_REPEAT) == GL_REPEAT){
		if((pname & GL_TEXTURE_WRAP_S) == GL_TEXTURE_WRAP_S){
			textureParamsValue |= GL_TEXTURE_WRAP_S;
		}
		if((pname & GL_TEXTURE_WRAP_T) == GL_TEXTURE_WRAP_T){
			textureParamsValue |= GL_TEXTURE_WRAP_T;
		}

		//18    Flip in S Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
  		if((pname & GL_TEXTURE_FLIP_S) == GL_TEXTURE_FLIP_S){
			textureParamsValue |= GL_TEXTURE_FLIP_S;
		}

		//19    Flip in T Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
		if((pname & GL_TEXTURE_FLIP_T) == GL_TEXTURE_FLIP_T){
			textureParamsValue |= GL_TEXTURE_FLIP_T;
		}
	}
	if(
		( (param & GL_CLAMP) == GL_CLAMP)
		||
		( (param & GL_CLAMP_TO_EDGE) == GL_CLAMP_TO_EDGE)
	){
		if((pname & GL_TEXTURE_WRAP_S) == GL_TEXTURE_WRAP_S){
			textureParamsValue &= ~GL_TEXTURE_WRAP_S;
		}
		
		if((pname & GL_TEXTURE_WRAP_T) == GL_TEXTURE_WRAP_T){
			textureParamsValue &= ~GL_TEXTURE_WRAP_T;
		}

		//18    Flip in S Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
		//19    Flip in T Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
			//no Repeat bit, so we remove Flip in both T and S directions
		textureParamsValue &= ~GL_TEXTURE_FLIP_S;
		textureParamsValue &= ~GL_TEXTURE_FLIP_T;
	}

	GFX_TEX_FORMAT = textureParamsValue;
}