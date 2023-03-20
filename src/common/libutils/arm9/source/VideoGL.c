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
//	 0.4: Update GL specs from OpenGL 1.0 to OpenGL 1.1 by implementing standard OpenGL Display Lists support using GX hardware (Coto)
//	 0.5: Extend GL specs on OpenGL 1.1 by implementing Vertex Array Buffers and Vertex Buffer Objects support using GX hardware (Coto)
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
#include "winDir.h"
#endif

//////////////////////////////////////////////////////////// Standard OpenGL 1.0 start //////////////////////////////////////////
#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct GLContext globalGLCtx;

static uint16 enable_bits = GL_TEXTURE_2D | GL_POLYGON_VERTEX_RAM_OVERFLOW | REAR_PLANE_MODE_BITMAP;
	
//Initializes the NDS OpenGL system
#ifdef ARM9
bool InitGLOnlyOnce = false;
#endif

#ifdef WIN32
bool InitGLOnlyOnce;
#endif

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glInit(){
	int i = 0;
	//set mode 0, enable BG0 and set it to 3D
	#ifdef ARM9
	SETDISPCNT_MAIN(MODE_0_3D);
	#endif
	memset((u8*)&globalGLCtx, 0, sizeof(struct GLContext));
	glShadeModel(GL_SMOOTH);

	globalGLCtx.polyAttributes = POLY_ALPHA(31);
	glCullFace(GL_FRONT); 
	glDisable(GL_CULL_FACE);

	globalGLCtx.textureParamsValue = 0;
	globalGLCtx.diffuseValue=0;
	globalGLCtx.ambientValue=0;
	globalGLCtx.specularValue=0;
	globalGLCtx.emissionValue=0;

	{
		//Internal
		struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContextThis = (struct TGDSOGL_DisplayListContext *)&TGDSOGL_DisplayListContextInst[TGDSOGL_DisplayListContext_Internal];
		TGDSOGL_DisplayListContextThis->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr=0;
		memset(getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(TGDSOGL_DisplayListContextThis), 0, InternalUnpackedGX_DL_workSize);
		TGDSOGL_DisplayListContextThis->isAnOpenGLExtendedDisplayListCallList = false;
	
		//External
		TGDSOGL_DisplayListContextThis = (struct TGDSOGL_DisplayListContext *)&TGDSOGL_DisplayListContextInst[TGDSOGL_DisplayListContext_External];
		TGDSOGL_DisplayListContextThis->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr=0; 
		memset(getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(TGDSOGL_DisplayListContextThis), 0, InternalUnpackedGX_DL_workSize);
		TGDSOGL_DisplayListContextThis->isAnOpenGLExtendedDisplayListCallList = false;
	}
	//////////////////////////////////////////////////////VBO & VBA init//////////////////////////////////////////////////////

	//Start clean once. Because subsequent re-init calls will require array memory to be freed/reallocated
	if(InitGLOnlyOnce == false){
		memset(&TGDSVBAInstance, 0, sizeof(TGDSVBAInstance));
		vboVertex	= &TGDSVBAInstance.vertexBufferObjectInst[OBJECT_BUFFER_VERTEX];
		vboNormal	= &TGDSVBAInstance.vertexBufferObjectInst[OBJECT_BUFFER_NORMAL];
		vboColor	= &TGDSVBAInstance.vertexBufferObjectInst[OBJECT_BUFFER_COLOR];
		vboIndex	= &TGDSVBAInstance.vertexBufferObjectInst[OBJECT_BUFFER_INDEX];
		vboTexCoord	= &TGDSVBAInstance.vertexBufferObjectInst[OBJECT_BUFFER_TEXCOORD];
		vboEdgeFlag	= &TGDSVBAInstance.vertexBufferObjectInst[OBJECT_BUFFER_EDGEFLAG];
		vboVertex->vboArrayMemoryStart = NULL;
		vboVertex->vboIsDynamicMemoryAllocated = false;
		vboNormal->vboArrayMemoryStart = NULL;
		vboNormal->vboIsDynamicMemoryAllocated = false;
		vboColor->vboArrayMemoryStart = NULL;
		vboColor->vboIsDynamicMemoryAllocated = false;
		vboIndex->vboArrayMemoryStart = NULL;
		vboIndex->vboIsDynamicMemoryAllocated = false;
		vboTexCoord->vboArrayMemoryStart = NULL;
		vboTexCoord->vboIsDynamicMemoryAllocated = false;
		vboEdgeFlag->vboArrayMemoryStart = NULL;
		vboEdgeFlag->vboIsDynamicMemoryAllocated = false;
		InitGLOnlyOnce = true;
	}
	for(i=0; i < MAX_VBO_HANDLES_GL; i++){
		TGDSVBAInstance.vertexBufferObjectReferences[i] = NULL;
		TGDSVBAInstance.vboName[i] = (GLint)VBO_DESCRIPTOR_INVALID;
	}

	//vertex
	vboVertex->vertexBufferObjectstrideOffset = -1;
	vboVertex->ElementsPerVertexBufferObjectUnit = -1;
	vboVertex->VertexBufferObjectStartOffset = -1;
	vboVertex->lastPrebuiltDLCRC16 = 0;
	vboVertex->ClientStateEnabled = false;
	//normal
	vboNormal->vertexBufferObjectstrideOffset = -1;
	vboNormal->ElementsPerVertexBufferObjectUnit = -1;
	vboNormal->VertexBufferObjectStartOffset = -1;
	vboNormal->lastPrebuiltDLCRC16 = 0;
	vboNormal->ClientStateEnabled = false;
	//color
	vboColor->vertexBufferObjectstrideOffset = -1;
	vboColor->ElementsPerVertexBufferObjectUnit = -1;
	vboColor->VertexBufferObjectStartOffset = -1;
	vboColor->lastPrebuiltDLCRC16 = 0;
	vboColor->ClientStateEnabled = false;
	//index
	vboIndex->vertexBufferObjectstrideOffset = -1;
	vboIndex->ElementsPerVertexBufferObjectUnit = -1;
	vboIndex->VertexBufferObjectStartOffset = -1;
	vboIndex->lastPrebuiltDLCRC16 = 0;
	vboIndex->ClientStateEnabled = false;
	//Texture coordinates
	vboTexCoord->vertexBufferObjectstrideOffset = -1;
	vboTexCoord->ElementsPerVertexBufferObjectUnit = -1;
	vboTexCoord->VertexBufferObjectStartOffset = -1;
	vboTexCoord->lastPrebuiltDLCRC16 = 0;
	vboTexCoord->ClientStateEnabled = false;
	//Edge flag
	vboEdgeFlag->vertexBufferObjectstrideOffset = -1;
	vboEdgeFlag->ElementsPerVertexBufferObjectUnit = -1;
	vboEdgeFlag->VertexBufferObjectStartOffset = -1;
	vboEdgeFlag->lastPrebuiltDLCRC16 = 0;
	vboEdgeFlag->ClientStateEnabled = false;
	{
		struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext = (struct TGDSOGL_DisplayListContext *)&TGDSOGL_DisplayListContextInst[TGDSOGL_DisplayListContext_Internal];
		OGL_DL_DRAW_ARRAYS_METHOD = glGenLists(1, TGDSOGL_DisplayListContext);
	}
	glDisable(GL_LIGHT0|GL_LIGHT1|GL_LIGHT2|GL_LIGHT3); //No lights enabled as default.
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPushMatrix(struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000444h 11h -  17  MTX_PUSH - Push Current Matrix on Stack (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_PUSH; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glPopMatrix(sint32 index, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000448h 12h 1  36  MTX_POP - Pop Current Matrix from Stack (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_POP; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)index; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glRestoreMatrix(sint32 index, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000450h 14h 1  36  MTX_RESTORE - Restore Current Matrix from Stack (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_RESTORE; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)index; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glStoreMatrix(sint32 index, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400044Ch - Cmd 13h - MTX_STORE - Store Current Matrix on Stack (W). Sets [N]=C. The stack pointer S is not used, and is left unchanged.
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_STORE; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(index&0x1f); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glScalev(GLvector* v, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	if(TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400046Ch 1Bh 3  22  MTX_SCALE - Multiply Current Matrix by Scale Matrix (W)
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_SCALE; //Unpacked Command format
			ptrVal++;
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)v->x; ptrVal++;
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)v->y; ptrVal++;
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)v->z; ptrVal++;
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glTranslatev(GLvector* v, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000470h 1Ch 3  22* MTX_TRANS - Mult. Curr. Matrix by Translation Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_TRANS; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)v->x; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)v->y; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)v->z; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glTranslatef32(int x, int y, int z, struct TGDSOGL_DisplayListContext * Inst) {
	glTranslate3f32(inttof32(x),inttof32(y),inttof32(z), Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslate3f32(f32 x, f32 y, f32 z, struct TGDSOGL_DisplayListContext * Inst){
	GLvector vec;
	vec.x = x; vec.y = y; vec.z = z; 
	glTranslatev(&vec, Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLight(int id, rgb color, v10 x, v10 y, v10 z, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			id = (id & 3) << 30;
			//40004C8h 32h 1  6   LIGHT_VECTOR - Set Light's Directional Vector (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_LIGHT_VECTOR; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(id | ((z & 0x3FF) << 20) | ((y & 0x3FF) << 10) | (x & 0x3FF)); ptrVal++;
			
			//40004CCh 33h 1  1   LIGHT_COLOR - Set Light Color (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_LIGHT_COLOR; 
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(id | color); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glNormal(uint32 normal, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_NORMAL; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)normal; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glLoadIdentity(struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000454h 15h -  19  MTX_IDENTITY - Load Unit(Identity) Matrix to Current Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_IDENTITY; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glMatrixMode(int mode, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000440h 10h 1  1   MTX_MODE - Set Matrix Mode (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_MODE; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(mode); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void emitGLShinnyness(float shinyValue, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	float shinyFragment = (shinyValue/64.0f);
	float shinyFragmentCount = 0.0f;
	globalGLCtx.shininessValue = shinyValue;
	if(TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList == true){
		uint32 shiny32[128/4];
		uint8  *shiny8 = (uint8*)shiny32;	
		u32 ptrVal = TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			int i;
			for (i = 0; i < 128 * 2; i += 2){
				shiny8[i>>1] = floatto12d3(shinyFragmentCount);
				shinyFragmentCount+=shinyFragment;
			}
			//40004D0h 34h 32 32  SHININESS - Specular Reflection Shininess Table (W)
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_SHININESS; //Unpacked Command format
			ptrVal++;
			for (i = 0; i < 128 / 4; i++){
				TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)shiny32[i]; ptrVal++; //Unpacked Command format
			}
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		uint32 shiny32[128/4];
		uint8  *shiny8 = (uint8*)shiny32;
		int i;
		for (i = 0; i < 128 * 2; i += 2){
			shiny8[i>>1] = floatto12d3(shinyFragmentCount);
			shinyFragmentCount+=shinyFragment;
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
void glMaterialShinnyness(struct TGDSOGL_DisplayListContext * Inst){
	emitGLShinnyness(128.0f, Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPolyFmt(int alpha, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//40004A4h 29h 1  1   POLYGON_ATTR - Set Polygon Attributes (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_POLYGON_ATTR; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)alpha; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glViewport(uint8 x1, uint8 y1, uint8 x2, uint8 y2, struct TGDSOGL_DisplayListContext * Inst){
	u32 viewPortWrite = (u32)((x1) + (y1 << 8) + (x2 << 16) + (y2 << 24));
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000580h 60h 1  1   VIEWPORT - Set Viewport (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getVIEWPORT; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = viewPortWrite; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_VIEWPORT = viewPortWrite;
		#endif
	}
	globalGLCtx.lastViewport = viewPortWrite;
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
void glTranslatef(float x, float y, float z, struct TGDSOGL_DisplayListContext * Inst){
	glTranslate3f32(floattof32(x), floattof32(y), floattof32(z), Inst); 
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
	if((bits&GL_CULL_FACE) == GL_CULL_FACE){
		//faces are enabled through glCullFace() because culling occurs per polygon on GX
	}

	//Lights enable
	if((bits&GL_LIGHT0) == GL_LIGHT0){
		globalGLCtx.lightsEnabled |= (u32)GL_LIGHT0;
	}

	if((bits&GL_LIGHT1) == GL_LIGHT1){
		globalGLCtx.lightsEnabled |= (u32)GL_LIGHT1;
	}

	if((bits&GL_LIGHT2) == GL_LIGHT2){
		globalGLCtx.lightsEnabled |= (u32)GL_LIGHT2;
	}

	if((bits&GL_LIGHT3) == GL_LIGHT3){
		globalGLCtx.lightsEnabled |= (u32)GL_LIGHT3;
	}

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
	if((bits&GL_CULL_FACE) == GL_CULL_FACE){
		u32 polyAttr = (globalGLCtx.polyAttributes & ~(POLY_CULL_BACK | POLY_CULL_FRONT | POLY_CULL_NONE));
		globalGLCtx.polyAttributes = polyAttr | POLY_CULL_NONE;
	}
	
	//Lights disable
	if((bits&GL_LIGHT0) == GL_LIGHT0){
		globalGLCtx.lightsEnabled &= ~((u32)GL_LIGHT0);
	}

	if((bits&GL_LIGHT1) == GL_LIGHT1){
		globalGLCtx.lightsEnabled &= ~((u32)GL_LIGHT1);
	}

	if((bits&GL_LIGHT2) == GL_LIGHT2){
		globalGLCtx.lightsEnabled &= ~((u32)GL_LIGHT2);
	}

	if((bits&GL_LIGHT3) == GL_LIGHT3){
		globalGLCtx.lightsEnabled &= ~((u32)GL_LIGHT3);
	}
	
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
void glFlush(struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000540h 50h 1  392 SWAP_BUFFERS - Swap Rendering Engine Buffer (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_SWAP_BUFFERS; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(2); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glFinish(struct TGDSOGL_DisplayListContext * Inst){
	glFlush(Inst);
	while( (volatile u32)(*((volatile u32*)GFX_STATUS_ADDR)) & (1 << 27) ){ //27    Geometry Engine Busy (0=No, 1=Yes; Busy; Commands are executing)
		
	}
}

/*
Parameters
m
A pointer to a 4x4 matrix stored in column-major order as 16 consecutive values.
*/
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadMatrixf(const GLfloat *m, struct TGDSOGL_DisplayListContext * Inst){
	m4x4 inMtx; //Float -> fixed point conversion
	inMtx.m[0] = floattof32(m[0]); //0
	inMtx.m[1] = floattof32(m[1]); //1
	inMtx.m[2] = floattof32(m[2]); //2
	inMtx.m[3] = floattof32(m[3]); //3
	inMtx.m[4] = floattof32(m[4]); //4
	inMtx.m[5] = floattof32(m[5]); //5
	inMtx.m[6] = floattof32(m[6]); //6
	inMtx.m[7] = floattof32(m[7]); //7
	inMtx.m[8] = floattof32(m[8]); //8
	inMtx.m[9] = floattof32(m[9]); //9
	inMtx.m[10] = floattof32(m[10]); //10
	inMtx.m[11] = floattof32(m[11]); //11
	inMtx.m[12] = floattof32(m[12]); //12
	inMtx.m[13] = floattof32(m[13]); //13
	inMtx.m[14] = floattof32(m[14]); //14
	inMtx.m[15] = floattof32(m[15]); //15
	glLoadMatrix4x4(&inMtx, Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadMatrix4x4(m4x4 * m, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_LOAD_4x4; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[2]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[3]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[4]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[5]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[6]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[7]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[8]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[9]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[10]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[11]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[12]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[13]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[14]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[15]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glLoadMatrix4x3(m4x3* m, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400045Ch 17h 12 30  MTX_LOAD_4x3 - Load 4x3 Matrix to Current Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_LOAD_4x3; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[2]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[3]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[4]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[5]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[6]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[7]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[8]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[9]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[10]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[11]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glMultMatrix4x4(m4x4* m, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000460h 18h 16 35* MTX_MULT_4x4 - Multiply Current Matrix by 4x4 Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_MULT_4x4; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[2]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[3]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[4]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[5]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[6]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[7]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[8]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[9]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[10]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[11]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[12]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[13]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[14]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[15]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glMultMatrix4x3(m4x3* m, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000464h 19h 12 31* MTX_MULT_4x3 - Multiply Current Matrix by 4x3 Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_MULT_4x3; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[2]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[3]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[4]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[5]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[6]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[7]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[8]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[9]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[10]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[11]; ptrVal++;
			
			//4000468h 1Ah 9  28* MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_MULT_3x3; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[2]; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[3]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[4]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[5]; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[6]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[7]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)m->m[8]; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glRotateZi(int angle, struct TGDSOGL_DisplayListContext * Inst){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];	
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
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
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_MULT_3x3;  //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)cosine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)sine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-sine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)cosine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)inttof32(1); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glRotateYi(int angle, struct TGDSOGL_DisplayListContext * Inst){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
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
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_MULT_3x3;  //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)cosine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-sine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)inttof32(1); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)sine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)cosine; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glRotateXi(int angle, struct TGDSOGL_DisplayListContext * Inst){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];	
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
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
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_MULT_3x3;  //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)inttof32(1); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)cosine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)sine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-sine; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)cosine; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glRotateX(float angle, struct TGDSOGL_DisplayListContext * Inst){
	glRotateXi((int)(angle * LUT_SIZE / 360.0), Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateY(float angle, struct TGDSOGL_DisplayListContext * Inst){
	glRotateYi((int)(angle * LUT_SIZE / 360.0), Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateZ(float angle, struct TGDSOGL_DisplayListContext * Inst){
	glRotateZi((int)(angle * LUT_SIZE / 360.0), Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotatef(int angle, float x, float y, float z, struct TGDSOGL_DisplayListContext * Inst){
	if(x > 0){
		glRotateX(angle, Inst); 
	}
	if(y > 0){
		glRotateY(angle, Inst); 
	}
	if(z > 0){
		glRotateZ(angle, Inst);
	}
}

// Fixed point look at function, it appears to work as expected although testing is recomended
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluLookAtf32(f32 eyex, f32 eyey, f32 eyez, f32 lookAtx, f32 lookAty, f32 lookAtz, f32 upx, f32 upy, f32 upz, struct TGDSOGL_DisplayListContext * Inst){
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
	
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			glMatrixMode(GL_MODELVIEW, Inst);
			//400045Ch 17h 12 30  MTX_LOAD_4x3 - Load 4x3 Matrix to Current Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_LOAD_4x3; //Unpacked Command format
			ptrVal++;

			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)x[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)x[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)x[2]; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)y[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)y[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)y[2]; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)z[0]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)z[1]; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)z[2]; ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)floattof32(-1.0); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			
			glTranslate3f32(-eyex, -eyey, -eyez, Inst);			
		}
	}
	else{
		glMatrixMode(GL_MODELVIEW, Inst);
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
		glTranslate3f32(-eyex, -eyey, -eyez, Inst);		
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
void gluLookAt(float eyex, float eyey, float eyez, float lookAtx, float lookAty, float lookAtz, float upx, float upy, float upz, struct TGDSOGL_DisplayListContext * Inst){
	gluLookAtf32(
		floattof32(eyex), floattof32(eyey), floattof32(eyez), 
		floattof32(lookAtx), floattof32(lookAty), floattof32(lookAtz), 
		floattof32(upx), floattof32(upy), floattof32(upz),
		Inst
	);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluFrustumf32(f32 left, f32 right, f32 bottom, f32 top, f32 nearVal, f32 farVal, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			glMatrixMode(GL_PROJECTION, Inst);
			//4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_LOAD_4x4; //Unpacked Command format
			ptrVal++;

			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)divf32(2*nearVal, right - left); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)divf32(right + left, right - left); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
		
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)divf32(2*nearVal, top - bottom); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)divf32(top + bottom, top - bottom); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;

			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-divf32(farVal + nearVal, farVal - nearVal); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)floattof32(-1.0F); ptrVal++;

			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-divf32(2 * mulf32(farVal, nearVal), farVal - nearVal); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			glStoreMatrix(0, Inst);			
		}
	}
	else{
		glMatrixMode(GL_PROJECTION, Inst);
		#if defined(ARM9)
		MATRIX_LOAD4x4 = divf32(2*nearVal, right - left);     
		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = divf32(right + left, right - left);      
		MATRIX_LOAD4x4 = 0;

		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = divf32(2*nearVal, top - bottom);     
		MATRIX_LOAD4x4 = divf32(top + bottom, top - bottom);      
		MATRIX_LOAD4x4 = 0;

		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = -divf32(farVal + nearVal, farVal - nearVal);     
		MATRIX_LOAD4x4 = floattof32(-1.0F);

		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = -divf32(2 * mulf32(farVal, nearVal), farVal - nearVal);  
		MATRIX_LOAD4x4 = 0;
		#endif
		glStoreMatrix(0, Inst);	   
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
void glOrtho(float left, float right, float bottom, float top, float nearVal, float farVal, struct TGDSOGL_DisplayListContext * Inst){
	glOrthof32(floattof32(left), floattof32(right), floattof32(bottom), floattof32(top), floattof32(nearVal), floattof32(farVal), Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glOrthof32(f32 left, f32 right, f32 bottom, f32 top, f32 nearVal, f32 farVal, struct TGDSOGL_DisplayListContext * Inst){	
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			glMatrixMode(GL_PROJECTION, Inst);
			//4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getMTX_LOAD_4x4; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)divf32(2, right - left); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-divf32(right + left, right - left); ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)divf32(2, top - bottom); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-divf32(top + bottom, top - bottom); ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)divf32(-2, farVal - nearVal); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)-divf32(farVal + nearVal, farVal - nearVal); ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)floattof32(1.0F); ptrVal++;			
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			glStoreMatrix(0, Inst);			
		}
	}
	else{
		glMatrixMode(GL_PROJECTION, Inst);
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
		MATRIX_LOAD4x4 = divf32(-2, farVal - nearVal);
		MATRIX_LOAD4x4 = -divf32(farVal + nearVal, farVal - nearVal);

		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = 0;  
		MATRIX_LOAD4x4 = floattof32(1.0F);
		#endif
		glStoreMatrix(0, Inst);
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
void gluFrustum(float left, float right, float bottom, float top, float nearVal, float farVal, struct TGDSOGL_DisplayListContext * Inst){
	gluFrustumf32(floattof32(left), floattof32(right), floattof32(bottom), floattof32(top), floattof32(nearVal), floattof32(farVal), Inst);
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
void gluPerspectivef32(int fovy, f32 aspect, f32 zNear, f32 zFar, struct TGDSOGL_DisplayListContext * Inst){
   f32 xmin, xmax, ymin, ymax;

   ymax = mulf32(zNear, TAN[fovy & LUT_MASK]);
   ymin = -ymax;
   xmin = mulf32(ymin, aspect);
   xmax = mulf32(ymax, aspect);

   gluFrustumf32(xmin, xmax, ymin, ymax, zNear, zFar, Inst);
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
void gluPerspective(float fovy, float aspect, float zNear, float zFar, struct TGDSOGL_DisplayListContext * Inst){
	gluPerspectivef32((int)(fovy * LUT_SIZE / 360.0), floattof32(aspect), floattof32(zNear), floattof32(zFar), Inst);    
}

//Default OpenGL 1.0 implementation:
//The param parameter is a single floating-point value that specifies the RGBA specular exponent of the material. 
//Integer values are mapped directly. Only values in the range [0, 128] are accepted. 
//The default specular exponent for both front-facing and back-facing materials is 0.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMaterialf(
	GLenum  face,
	GLenum  pname,
	GLfloat param, 
	struct TGDSOGL_DisplayListContext * Inst
   ){
	GLfloat params[4];
	if((GLenum)pname != (GLenum)GL_SHININESS){ //only this command is supported, reject everything else.
		errorStatus = GL_INVALID_ENUM;
		return;
	}
	params[0] = param; 
	glMaterialfv (face, pname, (const GLfloat *)&params, Inst);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glResetMatrixStack(struct TGDSOGL_DisplayListContext * Inst){
  // stack overflow ack ?
  GFX_STATUS |= 1 << 15;

  // pop the stacks to the top...seems projection stack is only 1 deep??
  glMatrixMode(GL_PROJECTION, Inst);
  glPopMatrix((GFX_STATUS>>13) & 1, Inst);
  
  // 31 deep modelview matrix
  glMatrixMode(GL_MODELVIEW, Inst);
  glPopMatrix((GFX_STATUS >> 8) & 0x1F, Inst);
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
void glReset(struct TGDSOGL_DisplayListContext * Inst){
	#ifdef ARM9
	while (GFX_STATUS & (1<<27)); // wait till gfx engine is not busy
  
	// Clear the FIFO
	GFX_STATUS |= (1<<29);

	// Clear overflows for list memory
	GFX_CONTROL = enable_bits = ((1<<12) | (1<<13)) | GL_TEXTURE_2D;
	glResetMatrixStack(Inst);
  
	GFX_TEX_FORMAT = globalGLCtx.textureParamsValue = 0;
	GFX_POLYGON_ATTR = 0;
  
	glMatrixMode(GL_PROJECTION, Inst);
	glLoadIdentity(Inst);

	glMatrixMode(GL_MODELVIEW, Inst);
	glLoadIdentity(Inst);
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
void glBindTexture(int target, int name, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//40004A8h 2Ah 1  1   TEXIMAGE_PARAM - Set Texture Parameters (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_TEX_FORMAT; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(textures[name]); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_TEX_FORMAT = globalGLCtx.textureParamsValue = textures[name];
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
	return 0;
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
	GFX_TEX_FORMAT = globalGLCtx.textureParamsValue = (sizeX << 20) | (sizeY << 23) | ((type == GL_RGB ? GL_RGBA : type ) << 26);
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
void glVertex2i(int x, int y, struct TGDSOGL_DisplayListContext * Inst) {
    glVertex2v16(inttov16(x), inttov16(y), Inst);
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
void glVertex2f(float x, float y, struct TGDSOGL_DisplayListContext * Inst) {
    glVertex2v16(floattov16(x), floattov16(y), Inst);
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
void glVertex3f(GLfloat x, GLfloat y, GLfloat z, struct TGDSOGL_DisplayListContext * Inst){
	glVertex3v16(floattov16(x), floattov16(y), floattov16(z), Inst);
}

//int x , y , z vertex coords in v16 format
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glVertex3i(GLint x, GLint y, GLint z, struct TGDSOGL_DisplayListContext * Inst){
	glVertex3v16(inttov16(x), inttov16(y), inttov16(z), Inst);
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
void glColor3f(float red, float green, float blue, struct TGDSOGL_DisplayListContext * Inst){
	//Detect light sources and apply colors. Normal GL calls resort to glColor to update material color + light color + texture color
	glColor3b(floattov10(red), floattov10(green), floattov10(blue), Inst);
	
	//Handle light vectors
	//Note: Light depth is 10bit. Which means only glColor3f(); can colour normals on polygons. glColor3b(); can't. Also don't forget to enable at least one light per scene or colour over normals won't reflect in the light vector.
	{
		u32 lightsEnabled = globalGLCtx.lightsEnabled;
		if((lightsEnabled&GL_LIGHT0) == GL_LIGHT0){
			glLight(0, RGB15(floatto12d3(red)<<1,floatto12d3(green)<<1,floatto12d3(blue)<<1), inttov10(31), inttov10(31), inttov10(31), Inst);
		}
		if((lightsEnabled&GL_LIGHT1) == GL_LIGHT1){
			glLight(1, RGB15(floatto12d3(red)<<1,floatto12d3(green)<<1,floatto12d3(blue)<<1), inttov10(31), inttov10(31), inttov10(31), Inst);
		}
		if((lightsEnabled&GL_LIGHT2) == GL_LIGHT2){
			glLight(2, RGB15(floatto12d3(red)<<1,floatto12d3(green)<<1,floatto12d3(blue)<<1), inttov10(31), inttov10(31), inttov10(31), Inst);
		}
		if((lightsEnabled&GL_LIGHT3) == GL_LIGHT3){
			glLight(3, RGB15(floatto12d3(red)<<1,floatto12d3(green)<<1,floatto12d3(blue)<<1), inttov10(31), inttov10(31), inttov10(31), Inst);
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
void glColor3fv(const GLfloat * v, struct TGDSOGL_DisplayListContext * Inst){
	if(v != NULL){
		float red = v[0];
		float green = v[1];
		float blue = v[2];
		glColor3f(red, green, blue, Inst);
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
void glTexCoord2f(GLfloat s, GLfloat t, struct TGDSOGL_DisplayListContext * Inst){
	int texBase = getTextureBaseFromTextureSlot(activeTexture);
	if(s > 0.0){
		s = s + (texBase);
	}
	if(t > 0.0){
		t = t + (texBase);
	}
	glTexCoord2t16(floattot16(t), floattot16(s), Inst);
}

void glTexCoord2i(GLint s, GLint t, struct TGDSOGL_DisplayListContext * Inst){
	int texBase = getTextureBaseFromTextureSlot(activeTexture);
	if(s > 0.0){
		s = s + (texBase);
	}
	if(t > 0.0){
		t = t + (texBase);
	}
	glTexCoord2t16(inttot16(t), inttot16(s), Inst);
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
void glTexCoord1i(uint32 uv, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_TEX_COORD; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)uv; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glTexCoord2t16(t16 u, t16 v, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_TEX_COORD; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)TEXTURE_PACK(u, v); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glBegin(int primitiveType, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000500h 40h 1  1   BEGIN_VTXS - Start of Vertex List (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_BEGIN; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)primitiveType; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glEnd(struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000504h 41h -  1   END_VTXS - End of Vertex List (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_END; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)0; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_END = 0;
		#endif
	}
}

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
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
void glColor3b(uint8 red, uint8 green, uint8 blue, struct TGDSOGL_DisplayListContext * Inst){
	u16 finalColor = 0;
	switch(globalGLCtx.primitiveShadeModelMode){
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
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000480h 20h 1  1   COLOR - Directly Set Vertex Color (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_COLOR; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)finalColor; ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_COLOR = (u32)finalColor;
		#endif
	}

	//Light vectors are not possible using glColor8b();. Use glColor3f(); instead.
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
 	GLbyte nz,
	struct TGDSOGL_DisplayListContext * Inst
){
	glNormal3i((GLint)nx, (GLint)ny,(GLint)nz, Inst);
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
 	GLdouble nz,
	struct TGDSOGL_DisplayListContext * Inst
){
	glNormal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz, Inst);
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
 	GLfloat nz,
	struct TGDSOGL_DisplayListContext * Inst
){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_NORMAL; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)NORMAL_PACK(floattov10(nx),floattov10(ny),floattov10(nz)); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glNormal3v10(
	v10 nx,
 	v10 ny,
 	v10 nz,
	struct TGDSOGL_DisplayListContext * Inst
){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_NORMAL; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)NORMAL_PACK(nx, ny, nz); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
 	GLshort nz,
	struct TGDSOGL_DisplayListContext * Inst
){
	glNormal3i((GLint)nx, (GLint)ny,(GLint)nz, Inst);
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
 	GLint nz,
	struct TGDSOGL_DisplayListContext * Inst
){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_NORMAL; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)NORMAL_PACK(inttov10(nx),inttov10(ny),inttov10(nz)); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glVertex3v16(v16 x, v16 y, v16 z, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//400048Ch 23h 2  9   VTX_16 - Set Vertex XYZ Coordinates (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_VERTEX16; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)((y << 16) | (x & 0xFFFF)); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(z & 0xFFFF); ptrVal++;
			
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glVertex3v10(v10 x, v10 y, v10 z, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000490h 24h 1  8   VTX_10 - Set Vertex XYZ Coordinates (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_VERTEX10; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)VERTEX_PACKv10(x, y, z); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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
void glVertex2v16(v16 x, v16 y, struct TGDSOGL_DisplayListContext * Inst){
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//4000494h 25h 1  8   VTX_XY - Set Vertex XY Coordinates (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_VTX_XY; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)VERTEX_PACK(x, y); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
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


//-

//To be called right before vertices, normals, texCoords and colors are rendered, but right after the polygon scene has been prepared.
//Usage: https://bitbucket.org/Coto88/toolchaingenericds-unittest/src example
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void updateGXLights(struct TGDSOGL_DisplayListContext * Inst){
	u32 activeLights = globalGLCtx.lightsEnabled;
	if((activeLights & GL_LIGHT0) == GL_LIGHT0){
		#define GX_LIGHT0 (1 << 0)
		glPolyFmt(GX_LIGHT0 | POLY_ALPHA(31) | POLY_CULL_NONE, Inst);
	}
	if((activeLights & GL_LIGHT1) == GL_LIGHT1){
		#define GX_LIGHT1 (1 << 1)
		glPolyFmt(GX_LIGHT1 | POLY_ALPHA(31) | POLY_CULL_NONE, Inst);
	}
	if((activeLights & GL_LIGHT2) == GL_LIGHT2){
		#define GX_LIGHT2 (1 << 2)
		glPolyFmt(GX_LIGHT2 | POLY_ALPHA(31) | POLY_CULL_NONE, Inst);
	}
	if((activeLights & GL_LIGHT3) == GL_LIGHT3){
		#define GX_LIGHT3 (1 << 3)
		glPolyFmt(GX_LIGHT3 | POLY_ALPHA(31) | POLY_CULL_NONE, Inst);
	}
}

//////////////////////////////////////////////////////////// Standard OpenGL 1.0 end //////////////////////////////////////////



//////////////////////////////////////////////////////////// Extended Display List OpenGL 1.1 start //////////////////////////////////////////
//DL Notes: Are sent using unpacked format.
//Unpacked Command format:
//Opcodes that write more than one 32bit value (ie. STRD and STM) can be used to send ONE UNPACKED command, 
//plus any parameters which belong to that command. 
//After that, there must be a 1 cycle delay before sending the next command 
//(ie. one cannot sent more than one command at once with a single opcode, each command must be invoked by a new opcode).

//Scratchpad GX buffer
u32 SingleUnpackedGXCommand_DL_Binary[PHYS_GXFIFO_INTERNAL_SIZE]; //Unpacked single command GX Buffer

struct TGDSOGL_DisplayListContext TGDSOGL_DisplayListContextInst[MAX_TGDSOGL_DisplayListContexts];

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
u32 * getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(struct TGDSOGL_DisplayListContext * Inst){
	return (u32 *)&Inst->InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_OpenGLDisplayListStartOffset];
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
GLuint glGenLists(GLsizei	range, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
    if(range < InternalUnpackedGX_DL_workSize){
        int i = 0;
		for(i = 0; i < (InternalUnpackedGX_DL_workSize/sizeof(GLsizei)); i++ ){
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[i] = DL_INVALID; //Compiled_DL_Binary_Descriptor[i] = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr; //only mapped from glNewList()
		}
		TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList = false; //cut-off incoming new OpenGL extended DLs
		TGDSOGL_DisplayListContext->LastGXInternalDisplayListPtr=0; //reset to default list start
		TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = 1; //OFFSET 0 IS DL SIZE
        TGDSOGL_DisplayListContext->LastActiveOpenGLDisplayList = DL_INVALID;
		return TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr; //starts from range of said length
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
void glListBase(GLuint base, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	if(base > 0){
		base--;
	}
	if((u32)TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[base] != (u32)DL_INVALID){
		TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[base];
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
GLboolean glIsList(GLuint list, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	if(list > 0){
		list--;
	}
	if((u32)TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[list] != (u32)DL_INVALID){
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
void glNewList(GLuint list, GLenum mode, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	if(list >= 1){
		list--; //assign current InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr (new) to a List
	}
	TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr++; //OFFSET 0 IS DL SIZE

	//Rewind list if out of memory
	if(TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr > InternalUnpackedGX_DL_workSize){
		TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = 0 + 1; //OFFSET 0 IS DL SIZE
	}

	TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[list] = TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr; 
	TGDSOGL_DisplayListContext->mode = (u32)mode;
	TGDSOGL_DisplayListContext->LastActiveOpenGLDisplayList = (u32)list;
	TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList = true;
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
void glEndList(struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	int listSize = 0;
	//If LAST display-list name is GL_COMPILE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL. Such binary will be manually executed when glCallList(display-list name) is called 
	//Else If LAST display-list name is GL_COMPILE_AND_EXECUTE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL and executes it inmediately through GX GLCallList()
	
	//define List Size
	listSize = ((TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr - TGDSOGL_DisplayListContext->LastGXInternalDisplayListPtr) * 4) + 4;
	TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_OpenGLDisplayListStartOffset + TGDSOGL_DisplayListContext->LastGXInternalDisplayListPtr] = (u32)listSize;
	TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr++;
	TGDSOGL_DisplayListContext->LastGXInternalDisplayListPtr = TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;	

	if(TGDSOGL_DisplayListContext->mode == GL_COMPILE_AND_EXECUTE){
		TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList = false; //Standard OpenGL DisplayList marked as executable now. Run it through the GX hardware from indirect glCallList(); call
		glCallList((GLuint)TGDSOGL_DisplayListContext->LastActiveOpenGLDisplayList, TGDSOGL_DisplayListContext);
	}
	TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList = false;
	TGDSOGL_DisplayListContext->LastActiveOpenGLDisplayList = DL_INVALID;
	TGDSOGL_DisplayListContext->mode = GL_COMPILE; //Even if Standard OpenGL DisplayList was marked earlier as executable. Disable further calls because it's slow, and precompiled GX Display List can be ran directly later through standard glCallList()
	/*//disable
	//fifo overflow/underflow? acknowledge error
	u32 gxbits = *(u32*)0x04000600;
	if( (gxbits & GX_ERROR_BIT) == GX_ERROR_BIT){
		*(u32*)0x04000600 |= GX_ERROR_BIT;
	}
	*/
}

/*
Standard OpenGL Display List function implementing native GX Display Lists execution.
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
void glCallList(GLuint list, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	if(list != DL_INVALID){
		u32 * InternalDL = getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(TGDSOGL_DisplayListContext);
		int curDLInCompiledDLOffset = 0;
		int singleGXDisplayListSize = 0;
		if(list > 0){
			list--;
		}
		curDLInCompiledDLOffset = TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[list];
		if((u32)curDLInCompiledDLOffset != DL_INVALID){
			u32 * currentPhysicalDisplayListStart = (u32 *)&InternalDL[curDLInCompiledDLOffset];
			currentPhysicalDisplayListStart--;
			singleGXDisplayListSize = (*currentPhysicalDisplayListStart);
			if(singleGXDisplayListSize > 0){
				//Run a single GX Display List, having proper DL size
				if(TGDSOGL_DisplayListContext->isAnOpenGLExtendedDisplayListCallList == false){ 
					if(singleGXDisplayListSize > PHYS_GXFIFO_INTERNAL_SIZE){
						singleGXDisplayListSize = PHYS_GXFIFO_INTERNAL_SIZE;
					}
					memset(SingleUnpackedGXCommand_DL_Binary, 0, PHYS_GXFIFO_INTERNAL_SIZE);
					SingleUnpackedGXCommand_DL_Binary[0] = (u32)singleGXDisplayListSize;
					memcpy((u8*)&SingleUnpackedGXCommand_DL_Binary[1], (u8*)&currentPhysicalDisplayListStart[0], singleGXDisplayListSize);

					//Hardware CallList
					glCallListGX((const u32*)&SingleUnpackedGXCommand_DL_Binary[0]);
					
					//Emulated CallList (slow, debugging purposes)
					/*
					u32 * currCmd = &SingleUnpackedGXCommand_DL_Binary[1];
					int leftArgCnt = (singleListSize/4); // -1 is removed command itself from the arg list count 
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
void glCallLists(GLsizei n, GLenum type, const void * lists, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
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
				glCallList(listName, TGDSOGL_DisplayListContext);
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
void glDeleteLists(GLuint list, GLsizei range, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	int lowestCurDLInCompiledDLOffset = 0;
	int i = 0;
	if(list >= 1){
		list--; //assign current InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr (new) to a List
	}
	if (list >= DL_DESCRIPTOR_MAX_ITEMS){
		return;
	}
	for(i = 0; i < range; i++){
		u32 * InternalDL = getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(TGDSOGL_DisplayListContext);
		int curDLInCompiledDLOffset = TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[list + i];
		if((u32)curDLInCompiledDLOffset != DL_INVALID){
			TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_Enumerator[list + i] = DL_INVALID;

			if(lowestCurDLInCompiledDLOffset > curDLInCompiledDLOffset){
				lowestCurDLInCompiledDLOffset = curDLInCompiledDLOffset;
			}
		}
	}
	
	//Find the lowest internal buffer offset assigned, just deleted, and rewind it so it points to unallocated Internal DL memory
	if(lowestCurDLInCompiledDLOffset != -1){
		TGDSOGL_DisplayListContext->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = lowestCurDLInCompiledDLOffset;
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
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glLightfv (GLenum light, GLenum pname, const GLfloat *params, struct TGDSOGL_DisplayListContext * Inst){
	//El parámetro params contiene cuatro valores de punto flotante que especifican la intensidad RGBA ambiente de la luz. Los valores de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante. La intensidad de luz ambiente predeterminada es (0,0, 0,0, 0,0, 1,0).
	if(pname == GL_AMBIENT){
		float rAmbient = params[0];
		float gAmbient = params[1];
		float bAmbient = params[2];
		//float aAmbient = params[3];
		globalGLCtx.ambientValue = ((floattov10(rAmbient) & 0x1F) << 16) | ((floattov10(gAmbient) & 0x1F) << 21) | ((floattov10(bAmbient) & 0x1F) << 26);
		if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
			u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
			if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
				//40004C0h 30h 1  4   DIF_AMB - MaterialColor0 - Diffuse/Ambient Reflect. (W)
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_DIFFUSE_AMBIENT; //Unpacked Command format
				ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = ((u32)( ((globalGLCtx.diffuseValue & 0xFFFF) << 0) | ((globalGLCtx.ambientValue & 0xFFFF) << 16) )); ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			}
		}
		else{
			#if defined(ARM9)
			GFX_DIFFUSE_AMBIENT = (u32)( ((globalGLCtx.diffuseValue & 0xFFFF) << 0) | ((globalGLCtx.ambientValue & 0xFFFF) << 16) );
			#endif
		}
	}
	//El parámetro params contiene cuatro valores de punto flotante que especifican la intensidad RGBA difusa de la luz. Los valores de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante. La intensidad difusa predeterminada es (0,0, 0,0, 0,0, 1,0) para todas las luces que no sean cero. La intensidad difusa predeterminada de la luz cero es (1,0, 1,0, 1,0, 1,0).
	if(pname == GL_DIFFUSE){
		float rDiffuse = params[0];
		float gDiffuse = params[1];
		float bDiffuse = params[2];
		//float aDiffuse = params[3];
		u8 setVtxColor = 1; //15    Set Vertex Color (0=No, 1=Set Diffuse Reflection Color as Vertex Color)
		globalGLCtx.diffuseValue = ((floattov10(rDiffuse) & 0x1F) << 0) | ((floattov10(gDiffuse) & 0x1F) << 5) | ((floattov10(bDiffuse) & 0x1F) << 10) | ((setVtxColor & 0x1) << 15);
		if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
			u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
			if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
				//40004C0h 30h 1  4   DIF_AMB - MaterialColor0 - Diffuse/Ambient Reflect. (W)
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_DIFFUSE_AMBIENT; //Unpacked Command format
				ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(( ((globalGLCtx.diffuseValue & 0xFFFF) << 0) | ((globalGLCtx.ambientValue & 0xFFFF) << 16) )); ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			}
		}
		else{
			#if defined(ARM9)
			GFX_DIFFUSE_AMBIENT = (u32)( ((globalGLCtx.diffuseValue & 0xFFFF) << 0) | ((globalGLCtx.ambientValue & 0xFFFF) << 16) );
			#endif
		}
	}
	//El parámetro params contiene cuatro valores de punto flotante que especifican la posición de la luz en coordenadas de objeto homogéneas. Los valores enteros y de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante.
	//La posición se transforma mediante la matriz modelview cuando se llama a glLightfv (como si fuera un punto) y se almacena en coordenadas oculares. Si el componente w de la posición es 0,0, la luz se trata como una fuente direccional. Los cálculos de iluminación difusa y especular toman la dirección de la luz, pero no su posición real, en cuenta y la atenuación está deshabilitada. De lo contrario, los cálculos de iluminación difusa y especular se basan en la ubicación real de la luz en coordenadas oculares y se habilita la atenuación. La posición predeterminada es (0,0,1,0); por lo tanto, la fuente de luz predeterminada es direccional, paralela a y en la dirección del eje -z .
	if(pname == GL_POSITION){
		int id = ((((int)light) & 3) << 30);
		float x = params[0];
		float y = params[1];
		float z = params[2];
		u32 writeVal = id | ((floattov10(z) & 0x3FF) << 20) | ((floattov10(y) & 0x3FF) << 10) | (floattov10(x) & 0x3FF);
		if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
			u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
			if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
				id = (id & 3) << 30;
				//40004C8h 32h 1  6   LIGHT_VECTOR - Set Light's Directional Vector (W)
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_LIGHT_VECTOR; //Unpacked Command format
				ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = writeVal; ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			}
		}
		else{
			#if defined(ARM9)
			GFX_LIGHT_VECTOR = writeVal;
			#endif
		}
	}
	//El parámetro params contiene cuatro valores de punto flotante que especifican la intensidad RGBA especular de la luz. Los valores de punto flotante se asignan directamente. No se fijan valores enteros ni de punto flotante. La intensidad especular predeterminada es (0,0, 0,0, 0,0, 1,0) para todas las luces que no sean cero. La intensidad especular predeterminada del cero claro es (1,0, 1,0, 1,0, 1,0).
	if(pname == GL_SPECULAR){
		float rSpecular = params[0];
		float gSpecular = params[1];
		float bSpecular = params[2];
		//float aSpecular = params[3];
		u8 useSpecularReflectionShininessTable = 0; //15    Specular Reflection Shininess Table (0=Disable, 1=Enable)
		globalGLCtx.specularValue = ((floattov10(rSpecular) & 0x1F) << 16) | ((floattov10(gSpecular) & 0x1F) << 21) | ((floattov10(bSpecular) & 0x1F) << 26) | ((useSpecularReflectionShininessTable & 0x1) << 15);
		if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
			u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
			if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
				//40004C4h 31h 1  4   SPE_EMI - MaterialColor1 - Specular Ref. & Emission (W)
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_SPECULAR_EMISSION; //Unpacked Command format
				ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)((u32 )( ((globalGLCtx.specularValue & 0xFFFF) << 0) | ((globalGLCtx.emissionValue & 0xFFFF) << 16) )); ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			}
		}
		else{
			#if defined(ARM9)
			GFX_SPECULAR_EMISSION = (u32 )( ((globalGLCtx.specularValue & 0xFFFF) << 0) | ((globalGLCtx.emissionValue & 0xFFFF) << 16) );
			#endif
		}
	}
	
	//Unimplemented:
	//GL_SPOT_DIRECTION (GX hardware doesn't support it)
	//GL_SPOT_EXPONENT (GX hardware doesn't support it)
	//GL_SPOT_CUTOFF (GX hardware doesn't support it)
	//GL_CONSTANT_ATTENUATION (GX hardware doesn't support it) 
	//GL_LINEAR_ATTENUATION (GX hardware doesn't support it)
	//GL_QUADRATIC_ATTENUATION (GX hardware doesn't support it)
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glMaterialfv (GLenum face, GLenum pname, const GLfloat *params, struct TGDSOGL_DisplayListContext * Inst){
	unsigned short emissionValueOut = 0;
	
	//GL_FRONT, GL_BACK and GL_FRONT_AND_BACK are ignored in GX because material face attributes aren't supported by hardware.
	//Just slip through and execute all other supported commands
	switch(face){
		case GL_FRONT:
		case GL_BACK:
		case GL_FRONT_AND_BACK:{
			
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}
	switch(pname){
		//	The param parameter is a single integer value that specifies the RGBA specular exponent of the material. Integer values are mapped directly. Only values in the range [0, 128] are accepted. The default specular exponent for both front-facing and back-facing materials is 0.
		case GL_SHININESS:{
			emitGLShinnyness((float)params[0], Inst);
			return;
		}break;

		//The params parameter contains four floating-point values that specify the RGBA emitted light intensity of the material. Integer values are mapped linearly such that the most positive representable value maps to 1.0, and the most negative representable value maps to -1.0. Floating-point values are mapped directly. Neither integer nor floating-point values are clamped. The default emission intensity for both front-facing and back-facing materials is (0.0, 0.0, 0.0, 1.0).
		case GL_AMBIENT: 
		case GL_DIFFUSE: 
		case GL_AMBIENT_AND_DIFFUSE:
		case GL_SPECULAR:
		case GL_EMISSION:{ //special case: convert float -> rgb color
			float rEmission = params[0];
			float gEmission = params[1];
			float bEmission = params[2];
			//float aEmission = params[3]; //GX can't do alpha emission through GX cmds
			emissionValueOut = RGB15(((int)rEmission),((int)gEmission),((int)bEmission));
			//glMaterialGX((int)pname, colOut, Inst);
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}
	{
		u32 diffuse_ambient = ((globalGLCtx.ambientValue << 16) | globalGLCtx.diffuseValue);
		u32 specular_emission = ((globalGLCtx.emissionValue << 16) | globalGLCtx.specularValue);
		switch(pname){
			case GL_AMBIENT:{
				diffuse_ambient = (emissionValueOut << 16) | (diffuse_ambient & 0xFFFF); //high part = ambient
			}break;
			case GL_DIFFUSE:{
				diffuse_ambient = emissionValueOut | (diffuse_ambient & 0xFFFF0000); //low part = diffuse
			}break;
			case GL_AMBIENT_AND_DIFFUSE:{
				diffuse_ambient= emissionValueOut + (emissionValueOut << 16); //repeat same values for both low and high part = ambient_and_diffuse
			}break;
			case GL_SPECULAR:{
				specular_emission = emissionValueOut | (specular_emission & 0xFFFF0000); //low part = specular
			}break;
			case GL_EMISSION:{
				specular_emission = (emissionValueOut << 16) | (specular_emission & 0xFFFF); //high part = emission
			}break;
			case GL_COLOR_INDEXES:{
				//Todo
			}break;
		}

		//Update GX properties
		globalGLCtx.ambientValue = (diffuse_ambient >> 16);
		globalGLCtx.diffuseValue = (diffuse_ambient&0xFFFF);
		globalGLCtx.emissionValue = (specular_emission >> 16);
		globalGLCtx.specularValue = (specular_emission&0xFFFF);

		if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
			u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
			if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
				//40004C0h 30h 1  4   DIF_AMB - MaterialColor0 - Diffuse/Ambient Reflect. (W)
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_DIFFUSE_AMBIENT; //Unpacked Command format
				ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(diffuse_ambient); ptrVal++;
				
				//40004C4h 31h 1  4   SPE_EMI - MaterialColor1 - Specular Ref. & Emission (W)
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_SPECULAR_EMISSION; //Unpacked Command format
				ptrVal++;
				Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(specular_emission); ptrVal++;
				
				Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
			}
		}
		else{
			#if defined(ARM9)
			GFX_DIFFUSE_AMBIENT = diffuse_ambient;
			GFX_SPECULAR_EMISSION = specular_emission;
			#endif
		}
	}
}

//glNormal(v): A pointer to an array of three elements: the x, y, and z coordinates of the new current normal.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glNormal3dv(const GLdouble *v, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	glNormal3d(v[0], v[1], v[2], TGDSOGL_DisplayListContext);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glNormal3fv(const GLfloat *v, struct TGDSOGL_DisplayListContext * Inst){
	glNormal3f(v[0], v[1], v[2], Inst);
}

//v: A pointer to an array of three elements. The elements are the x, y, and z coordinates of a vertex.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glVertex3fv(const GLfloat *v, struct TGDSOGL_DisplayListContext * Inst){
	glVertex3f(v[0], v[1], v[2], Inst);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glVertex3dv(const GLdouble *v, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	glVertex3f((float)v[0], (float)v[1], (float)v[2], TGDSOGL_DisplayListContext);
}


//target: The target texture, which must be either GL_TEXTURE_1D or GL_TEXTURE_2D.
//pname: The symbolic name of a single valued texture parameter. The following symbols are accepted in pname.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glTexParameteri(
	GLenum target,
	GLenum pname,
	GLint  param, 
	struct TGDSOGL_DisplayListContext * Inst
){
	target = GL_TEXTURE_2D;

	//16    Repeat in S Direction (0=Clamp Texture, 1=Repeat Texture)
	if((param & GL_REPEAT) == GL_REPEAT){
		if((pname & GL_TEXTURE_WRAP_S) == GL_TEXTURE_WRAP_S){
			globalGLCtx.textureParamsValue |= GL_TEXTURE_WRAP_S;
		}
		if((pname & GL_TEXTURE_WRAP_T) == GL_TEXTURE_WRAP_T){
			globalGLCtx.textureParamsValue |= GL_TEXTURE_WRAP_T;
		}

		//18    Flip in S Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
  		if((pname & GL_TEXTURE_FLIP_S) == GL_TEXTURE_FLIP_S){
			globalGLCtx.textureParamsValue |= GL_TEXTURE_FLIP_S;
		}

		//19    Flip in T Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
		if((pname & GL_TEXTURE_FLIP_T) == GL_TEXTURE_FLIP_T){
			globalGLCtx.textureParamsValue |= GL_TEXTURE_FLIP_T;
		}
	}
	if(
		( (param & GL_CLAMP) == GL_CLAMP)
		||
		( (param & GL_CLAMP_TO_EDGE) == GL_CLAMP_TO_EDGE)
	){
		if((pname & GL_TEXTURE_WRAP_S) == GL_TEXTURE_WRAP_S){
			globalGLCtx.textureParamsValue &= ~GL_TEXTURE_WRAP_S;
		}
		
		if((pname & GL_TEXTURE_WRAP_T) == GL_TEXTURE_WRAP_T){
			globalGLCtx.textureParamsValue &= ~GL_TEXTURE_WRAP_T;
		}

		//18    Flip in S Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
		//19    Flip in T Direction   (0=No, 1=Flip each 2nd Texture) (requires Repeat)
			//no Repeat bit, so we remove Flip in both T and S directions
		globalGLCtx.textureParamsValue &= ~GL_TEXTURE_FLIP_S;
		globalGLCtx.textureParamsValue &= ~GL_TEXTURE_FLIP_T;
	}
	
	if(Inst->isAnOpenGLExtendedDisplayListCallList == true){
		u32 ptrVal = Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
		if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
			//40004A8h 2Ah 1  1   TEXIMAGE_PARAM - Set Texture Parameters (W)
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)getFIFO_TEX_FORMAT; //Unpacked Command format
			ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary[ptrVal] = (u32)(globalGLCtx.textureParamsValue); ptrVal++;
			Inst->InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = ptrVal;
		}
	}
	else{
		#if defined(ARM9)
		GFX_TEX_FORMAT = globalGLCtx.textureParamsValue;
		#endif
	}
}

//4000640h..67Fh - CLIPMTX_RESULT - Read Current Clip Coordinates Matrix (R)
//This 64-byte region (16 words) contains the m[0..15] values of the Current Clip Coordinates Matrix, arranged in 4x4 Matrix format. Make sure that the Geometry Engine is stopped (GXSTAT.27) before reading from these registers.
//The Clip Matrix is internally used to convert vertices to screen coordinates, and is internally re-calculated anytime when changing the Position or Projection matrices:
//ClipMatrix = PositionMatrix * ProjectionMatrix
//To read only the Position Matrix, or only the Projection Matrix: Use Load Identity on the OTHER matrix, so the ClipMatrix becomes equal to the DESIRED matrix (multiplied by the Identity Matrix, which has no effect on the result).

//pname: The symbolic name of a single valued texture parameter. The following symbols are accepted in pname.
//params: misc
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glGetFloatv(
   GLenum pname, 
   GLfloat *params
){
	//The params parameter returns four values: the x and y window coordinates of the viewport, followed by its width and height.
	if((pname & GL_VIEWPORT) == GL_VIEWPORT){
		u32 readLastViewport = globalGLCtx.lastViewport;
		u32 * paramOut = (u32*)params;
		*paramOut = (u32)(readLastViewport&0xFF); paramOut++; //x1
		*paramOut = (u32)((readLastViewport>>8)&0xFF); paramOut++; //y1
		*paramOut = (u32)((readLastViewport>>16)&0xFF); paramOut++; //width
		*paramOut = (u32)((readLastViewport>>24)&0xFF); paramOut++; //height
	}

	//The params parameter returns a single Boolean value indicating whether the vertex array is enabled.
	else if((pname & GL_VERTEX_ARRAY) == GL_VERTEX_ARRAY){
		*params = (u32)false; //Won't be implemented: It's slower than DisplayLists because of the NDS CPU, no server-side only rendering (DL -> VRAM -> GX), let alone programmable arrays
	}
	
	//The params parameter returns four values: the red, green, blue, and alpha components 
	//of the ambient intensity of the entire scene. Integer values, if requested, are linearly mapped from 
	//the internal floating-point representation such that 1.0 returns the most positive representable integer value, 
	//and -1.0 returns the most negative representable integer value.
	else if((pname & GL_LIGHT_MODEL_AMBIENT) == GL_LIGHT_MODEL_AMBIENT){
		u32 specVal = globalGLCtx.specularValue;
		u32 * paramOut = (u32*)params;
		//globalGLCtx.specularValue = ((floattov10(rSpecular) & 0x1F) << 16) | ((floattov10(gSpecular) & 0x1F) << 21) | ((floattov10(bSpecular) & 0x1F) << 26);
		int rIntAmb = v10toint((specVal >> 16) & 0x1F); 
		int gIntAmb = v10toint((specVal >> 21) & 0x1F); 
		int bIntAmb = v10toint((specVal >> 26) & 0x1F); 
		*paramOut = (u32)rIntAmb; paramOut++;
		*paramOut = (u32)gIntAmb; paramOut++;
		*paramOut = (u32)bIntAmb; paramOut++;
		*paramOut = (u32)0; paramOut++;
	}
	
	//The params parameter returns a single Boolean value indicating whether lighting is enabled.
	else if((pname & GL_LIGHTING) == GL_LIGHTING){
		u8 activeLights = globalGLCtx.lightsEnabled;
		if(activeLights > 0){
			*params = (u32)true;
		}
		else{
			*params = (u32)false;
		}
	}
	
	//The params parameter returns one value: the maximum number of lights.
	else if((pname & GL_MAX_LIGHTS) == GL_MAX_LIGHTS){
		//GX: Max lights
		int activeLightCount = 4;
		*params = (u32)activeLightCount;
	}
	
	//The params parameter returns one value: the maximum recursion depth allowed during display-list traversal
	else if((pname & GL_MAX_LIST_NESTING) == GL_MAX_LIST_NESTING){
		*params = (u32)(InternalUnpackedGX_DL_workSize/sizeof(int));
	}
	
	//The params parameter returns 16 values: the modelview matrix on the top of the modelview matrix stack.
	else if((pname & GL_MODELVIEW_MATRIX) == GL_MODELVIEW_MATRIX){
		u32 * curModelViewMatrixPtr = (u32*)0x04000640;
		u32 * targetModelViewMatrixPtr = (u32 *)params;
		while (GFX_STATUS & (1<<27)); // wait till gfx engine is not busy
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //0
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //1
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //2
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //3
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //4
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //5
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //6
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //7
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //8
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //9
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //10
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //11
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //12
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //13
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //14
		*targetModelViewMatrixPtr = (u32)*curModelViewMatrixPtr; curModelViewMatrixPtr++; targetModelViewMatrixPtr++; //15
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glGetDoublev(
   GLenum   pname,
   GLdouble *params
   ){
	   glGetFloatv(pname, (GLfloat*)params);
}

/*
The glMultMatrixd and glMultMatrixf functions multiply the current matrix by an arbitrary matrix.

Parameters:
m 
A pointer to a 4x4 matrix stored in column-major order as 16 consecutive values.

Remarks:
The glMultMatrix function multiplies the current matrix by the one specified in m. That is, 
if M is the current matrix and T is the matrix passed to glMultMatrix, then M is replaced with M T.
The current matrix is the projection matrix, modelview matrix, or texture matrix, determined by
 the current matrix mode (see glMatrixMode).

The m parameter points to a 4x4 matrix of single-precision or double-precision floating-point values 
stored in column-major order.
*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void  glMultMatrixd(const GLdouble *m, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	glMultMatrixf((GLfloat*)m, TGDSOGL_DisplayListContext);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void  glMultMatrixf(const GLfloat *m, struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext){
	m4x4 inMtx;
	inMtx.m[0] = floattof32(m[0]); //0
	inMtx.m[1] = floattof32(m[1]); //1
	inMtx.m[2] = floattof32(m[2]); //2
	inMtx.m[3] = floattof32(m[3]); //3
	inMtx.m[4] = floattof32(m[4]); //4
	inMtx.m[5] = floattof32(m[5]); //5
	inMtx.m[6] = floattof32(m[6]); //6
	inMtx.m[7] = floattof32(m[7]); //7
	inMtx.m[8] = floattof32(m[8]); //8
	inMtx.m[9] = floattof32(m[9]); //9
	inMtx.m[10] = floattof32(m[10]); //10
	inMtx.m[11] = floattof32(m[11]); //11
	inMtx.m[12] = floattof32(m[12]); //12
	inMtx.m[13] = floattof32(m[13]); //13
	inMtx.m[14] = floattof32(m[14]); //14
	inMtx.m[15] = floattof32(m[15]); //15
	glMultMatrix4x4(&inMtx, TGDSOGL_DisplayListContext);	
}

//The glScaled and glScalef functions multiply the current matrix by a general scaling matrix.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glScalef(
   GLfloat x,
   GLfloat y,
   GLfloat z,
   struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext
){
	GLvector scaleVector;
	scaleVector.x = floattof32(x);
	scaleVector.y = floattof32(y);
	scaleVector.z = floattof32(z);
	glScalev(&scaleVector, TGDSOGL_DisplayListContext);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glScaled(
   GLdouble x,
   GLdouble y,
   GLdouble z,
   struct TGDSOGL_DisplayListContext * TGDSOGL_DisplayListContext
){
	glScalef((GLfloat)x, (GLfloat)y, (GLfloat)z, TGDSOGL_DisplayListContext);
}

void glCullFace(GLenum mode){
	switch(mode){
		case GL_FRONT:{
			u32 polyAttr = (globalGLCtx.polyAttributes & ~(POLY_CULL_BACK | POLY_CULL_FRONT | POLY_CULL_NONE));
			globalGLCtx.polyAttributes = polyAttr | POLY_CULL_FRONT;
		}break;
		case GL_BACK:{
			u32 polyAttr = (globalGLCtx.polyAttributes & ~(POLY_CULL_BACK | POLY_CULL_FRONT | POLY_CULL_NONE));
			globalGLCtx.polyAttributes = polyAttr | POLY_CULL_BACK;
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
		}break;
	}
}

//////////////////////////////////////////////////////////// Extended Display List OpenGL 1.1 end //////////////////////////////////////////

//////////////////////////////////////////////////////////// Extended Vertex Array Buffers and Vertex Buffer Objects OpenGL 1.1 start //////////////////////////////////////////

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
GLenum errorStatus;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct vertexBufferObject * vboVertex = NULL;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct vertexBufferObject * vboNormal = NULL;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct vertexBufferObject * vboColor = NULL;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct vertexBufferObject * vboIndex = NULL;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct vertexBufferObject * vboTexCoord = NULL;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct vertexBufferObject * vboEdgeFlag = NULL;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
GLenum glGetError(){
	return errorStatus;
}

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct vertexBufferArray TGDSVBAInstance; //Client side (NintendoDS) implements a single one

//VBO

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glGenBuffers(GLsizei n, GLuint* ids){
	int i = 0;
	if(n < MAX_VBO_PER_VBA){
		if(n > 0){
			n--;
		}
		for(i = 0; i < MAX_VBO_HANDLES_GL; i++){
			if( (i <= n) && ((GLuint)TGDSVBAInstance.vboName[i] == (GLuint)VBO_DESCRIPTOR_INVALID)){
				TGDSVBAInstance.vboName[i] = (GLuint)*(ids + i);
				TGDSVBAInstance.vertexBufferObjectReferences[i] = NULL;
			}
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glDeleteBuffers(GLsizei n, const GLuint* ids){
	int i = 0;
	if(n < MAX_VBO_PER_VBA){
		if(n > 0){
			n--;
		}
		for(i = 0; i < MAX_VBO_PER_VBA; i++){
			if( (ids[i] != NULL) && ((GLint)TGDSVBAInstance.vboName[i] == (GLint)ids[i]) ){
				GLuint * ptr = (GLuint * )(ids + i);
				*ptr = (GLuint)VBO_DESCRIPTOR_INVALID;
				TGDSVBAInstance.vboName[i] = (GLint)VBO_DESCRIPTOR_INVALID;
			}
		}
	}
}

//Note: Method adapted to work around OpenGL1.5 
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glBindBuffer(GLenum target, GLuint id){
	switch(target){
		case GL_ARRAY_BUFFER:{ //OpenGL 1.5+: allocate vertex buffer
			int i = 0;
			//OpenGL spec says it's Dynamic object allocation or using a referenced array
			if(vboVertex->vboIsDynamicMemoryAllocated == true){
				TGDSARM9Free(vboVertex->vboArrayMemoryStart);
			}
			vboVertex->vboArrayMemoryStart = (u32*)TGDSARM9Malloc(vboVertex->ElementsPerVertexBufferObjectUnit*VBO_ARRAY_SIZE);
			vboVertex->vboIsDynamicMemoryAllocated = true;
			//Lookaround the id and bind it to vertex buffer
			for(i = 0; i < MAX_VBO_HANDLES_GL; i++){
				if((GLuint)TGDSVBAInstance.vboName[i] == (GLuint)id){
					TGDSVBAInstance.vertexBufferObjectReferences[i] = vboVertex;
					break;
				}
			}
		}break;
		case GL_ELEMENT_ARRAY_BUFFER:{ //OpenGL 1.5+: allocate indices buffer
			int i = 0;
			//OpenGL spec says it's Dynamic object allocation or using a referenced array
			if(vboIndex->vboIsDynamicMemoryAllocated == true){
				TGDSARM9Free(vboIndex->vboArrayMemoryStart);
			}
			vboIndex->vboArrayMemoryStart = (u32*)TGDSARM9Malloc(vboIndex->ElementsPerVertexBufferObjectUnit*VBO_ARRAY_SIZE);
			vboIndex->vboIsDynamicMemoryAllocated = true;
			//Lookaround the id and bind it to index buffer
			for(i = 0; i < MAX_VBO_HANDLES_GL; i++){
				if((GLuint)TGDSVBAInstance.vboName[i] == (GLuint)id){
					TGDSVBAInstance.vertexBufferObjectReferences[i] = vboIndex;
					break;
				}
			}
		}break;

		//Unsupported
		/*
		case GL_PIXEL_PACK_BUFFER:{
		}break;
		
		case GL_PIXEL_UNPACK_BUFFER:{
		}break;
		*/

		default:{
			errorStatus = GL_INVALID_ENUM;
		}break;
	}
}

/*
Parameters
target
Specifies the target buffer object. The symbolic constant must be GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER, or GL_PIXEL_UNPACK_BUFFER.

size
Specifies the size in bytes of the buffer object's new data store.

data
Specifies a pointer to data that will be copied into the data store for initialization, or NULL if no data is to be copied.

usage
Specifies the expected usage pattern of the data store. The symbolic constant must be GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, or GL_DYNAMIC_COPY.
*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glBufferData(GLenum target, GLsizei size, const void* data, GLenum usage){
	u32 * targetBuffer = NULL;
	switch(target){
		case GL_ARRAY_BUFFER:{ //OpenGL 1.5+: allocate vertex buffer
			targetBuffer = vboVertex->vboArrayMemoryStart;
		}break;
		case GL_ELEMENT_ARRAY_BUFFER:{ //OpenGL 1.5+: allocate indices buffer
			targetBuffer = vboIndex->vboArrayMemoryStart;
		}break;
		//Unsupported
		/*
		case GL_PIXEL_PACK_BUFFER:{
		}break;
		
		case GL_PIXEL_UNPACK_BUFFER:{
		}break;
		*/
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}

	switch(usage){
		//The data store contents are modified by the application, and used as the source for GL drawing and image specification commands.
		case	GL_STATIC_DRAW:
		case	GL_DYNAMIC_DRAW:
		case	GL_STREAM_DRAW:
		{
			memcpy(targetBuffer, (void*)data, (int)size);
		}break;

		//The data store contents are modified by reading data from the GL, and used to return that data when queried by the application.
		case	GL_DYNAMIC_READ:
		case	GL_STATIC_READ:
		case	GL_STREAM_READ:
		{
			memcpy((void*)data, targetBuffer, (int)size);
		}break;
		
		//The data store contents are modified by reading data from the GL, and used as the source for GL drawing and image specification commands.
		case	GL_STATIC_COPY:{
		case	GL_DYNAMIC_COPY:
		case	GL_STREAM_COPY:
			data = targetBuffer;
		}break;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glBufferSubData(GLenum target, GLint offset, GLsizei size, void* data){
	//OpenGL 1.1 supported only (and some of OpenGL 1.5)
	errorStatus = GL_INVALID_ENUM;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glEnableClientState(GLenum arr){
	switch(arr){
		case GL_COLOR_ARRAY:{
			vboColor->ClientStateEnabled = true;
		}break;
		case GL_EDGE_FLAG_ARRAY:{
			vboEdgeFlag->ClientStateEnabled = true;
		}break;
		case GL_INDEX_ARRAY:{
			vboIndex->ClientStateEnabled = true;
		}break;
		case GL_NORMAL_ARRAY:{
			vboNormal->ClientStateEnabled = true;
		}break;
		case GL_TEXTURE_COORD_ARRAY:{
			vboTexCoord->ClientStateEnabled = true;
		}break;
		case GL_VERTEX_ARRAY:{
			vboVertex->ClientStateEnabled = true;
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
		}break;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glDisableClientState(GLenum arr){
	switch(arr){
		case GL_COLOR_ARRAY:{
			vboColor->ClientStateEnabled = false;
		}break;
		case GL_EDGE_FLAG_ARRAY:{
			vboEdgeFlag->ClientStateEnabled = false;
		}break;
		case GL_INDEX_ARRAY:{
			vboIndex->ClientStateEnabled = false;
		}break;
		case GL_NORMAL_ARRAY:{
			vboNormal->ClientStateEnabled = false;
		}break;
		case GL_TEXTURE_COORD_ARRAY:{
			vboTexCoord->ClientStateEnabled = false;
		}break;
		case GL_VERTEX_ARRAY:{
			vboVertex->ClientStateEnabled = false;
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
		}break;
	}
}

//VBA

/*
glVertexPointer: function defines an array of vertex data.

size: The number of coordinates per vertex. The value of size must be 2, 3, or 4.

type: The data type of each coordinate in the array using the following symbolic constants: GL_SHORT, GL_INT, GL_FLOAT, and GL_DOUBLE.

stride: The byte offset between consecutive vertices. When stride is zero, the vertices are tightly packed in the array.

pointer: A pointer to the first coordinate of the first vertex in the array.
*/
//Note: Method adapted to work around OpenGL1.5
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr){
	int typeSizeOf = 0;
	switch(type){
		case GL_SHORT:{
			typeSizeOf = sizeof(GLshort);
		}break;
		case GL_INT:{
			typeSizeOf = sizeof(GLint);
		}break;
		case GL_FLOAT:{
			typeSizeOf = sizeof(GLfloat);
		}break;
		case GL_DOUBLE:{
			typeSizeOf = sizeof(GLdouble);
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}
	if((size != 2) && (size != 3) && (size != 4)){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	if((stride < 0) || (size < 0)){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	if(ptr != NULL){
		vboVertex->vboArrayMemoryStart = (u32*)ptr;
		vboVertex->vboIsDynamicMemoryAllocated = false;
	}
	vboVertex->vertexBufferObjectstrideOffset = stride;
	vboVertex->ElementsPerVertexBufferObjectUnit = typeSizeOf; //defines pointer length
	vboVertex->VertexBufferObjectStartOffset = (int)0; //default to 0
	vboVertex->argCount = size;
}

/*
glNormalPointer: function defines an array of normals.
type: The data type of each coordinate in the array using the following symbolic constants: GL_BYTE, GL_SHORT, GL_INT, GL_FLOAT, and GL_DOUBLE.
stride: The byte offset between consecutive normals. When stride is zero, the normals are tightly packed in the array.
pointer: A pointer to the first normal in the array.

NOTE: DS floats ARE TREATED AS v10 in all cases for max GX normal precision
*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr ){
	int typeSizeOf = 0;
	switch(type){
		case GL_BYTE:{
			typeSizeOf = sizeof(GLbyte); //treat DS floats as v10 for max GX normal precision
		}break;
		case GL_SHORT:{
			typeSizeOf = sizeof(GLshort); //treat DS floats as v10 for max GX normal precision
		}break;
		case GL_INT:{
			typeSizeOf = sizeof(GLint); //treat DS floats as v10 for max GX normal precision
		}break;
		case GL_FLOAT:{
			typeSizeOf = sizeof(GLfloat); //treat DS floats as v10 for max GX normal precision
		}break;
		case GL_DOUBLE:{
			typeSizeOf = sizeof(GLdouble); //treat DS floats as v10 for max GX normal precision
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}
	if(stride < 0){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	//OpenGL spec says it's Dynamic object allocation or using a referenced array
	if(vboNormal->vboIsDynamicMemoryAllocated == true){
		TGDSARM9Free(vboNormal->vboArrayMemoryStart);
	}
	if(ptr == NULL){
		vboNormal->vboArrayMemoryStart = (u32*)TGDSARM9Malloc(typeSizeOf*VBO_ARRAY_SIZE);
		vboNormal->vboIsDynamicMemoryAllocated = true;
	}
	else{
		vboNormal->vboArrayMemoryStart = (u32*)ptr;
		vboNormal->vboIsDynamicMemoryAllocated = false;
	}
	vboNormal->vertexBufferObjectstrideOffset = stride;
	vboNormal->ElementsPerVertexBufferObjectUnit = typeSizeOf; //defines pointer length
	vboNormal->VertexBufferObjectStartOffset = (int)0; //default to 0
	vboNormal->argCount = 3; //GX uses always three normals
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){
	int typeSizeOf = 0;
	switch(type){
		case GL_BYTE:{
			typeSizeOf = sizeof(GLbyte);
		}break;
		case GL_UNSIGNED_BYTE:{
			typeSizeOf = sizeof(GLubyte);
		}break;
		case GL_SHORT:{
			typeSizeOf = sizeof(GLshort);	//GX supports packed color as 8bit only anyway
		}break;
		case GL_UNSIGNED_SHORT:{
			typeSizeOf = sizeof(GLushort);	//GX supports packed color as 8bit only anyway
		}break;
		case GL_INT:{
			typeSizeOf = sizeof(GLint);		//GX supports packed color as 8bit only anyway
		}break;
		case GL_UNSIGNED_INT:{
			typeSizeOf = sizeof(GLuint);	//GX supports packed color as 8bit only anyway
		}break;
		case GL_FLOAT:{
			typeSizeOf = sizeof(GLfloat);	//GX supports packed color as 8bit only anyway
		}break;
		case GL_DOUBLE:{
			typeSizeOf = sizeof(GLdouble);	//GX supports packed color as 8bit only anyway
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;	
			return;
		}break;
	}
	if((size != 1) && (size != 2) && (size != 3) && (size != 4)){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	if((stride < 0) || (size < 0)){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	//OpenGL spec says it's Dynamic object allocation or using a referenced array
	if(vboColor->vboIsDynamicMemoryAllocated == true){
		TGDSARM9Free(vboColor->vboArrayMemoryStart);
	}
	if(ptr == NULL){
		vboColor->vboArrayMemoryStart = (u32*)TGDSARM9Malloc(size*typeSizeOf*VBO_ARRAY_SIZE);
		vboColor->vboIsDynamicMemoryAllocated = true;
	}
	else{
		vboColor->vboArrayMemoryStart = (u32*)ptr;
		vboColor->vboIsDynamicMemoryAllocated = false;
	}
	vboColor->vertexBufferObjectstrideOffset = stride;
	vboColor->ElementsPerVertexBufferObjectUnit = typeSizeOf; //defines pointer length
	vboColor->VertexBufferObjectStartOffset = (int)0; //default to 0
	vboColor->argCount = size;
}

//Note: Method adapted to work around OpenGL1.5
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr ){
	int typeSizeOf = 0;
	switch(type){
		case GL_SHORT:{
			typeSizeOf = sizeof(GLshort);
		}break;
		case GL_INT:{
			typeSizeOf = sizeof(GLint);
		}break;
		case GL_FLOAT:{
			typeSizeOf = sizeof(GLfloat);
		}break;
		case GL_DOUBLE:{
			typeSizeOf = sizeof(GLdouble);
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}
	if(stride < 0){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	if(ptr != NULL){
		vboIndex->vboArrayMemoryStart = (u32*)ptr;
		vboIndex->vboIsDynamicMemoryAllocated = false;
	}
	vboIndex->vertexBufferObjectstrideOffset = stride;
	vboIndex->ElementsPerVertexBufferObjectUnit = typeSizeOf; //defines pointer length
	vboIndex->VertexBufferObjectStartOffset = (int)0; //default to 0
	vboIndex->argCount = 0; //todo
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){
	int typeSizeOf = 0;
	switch(type){
		case GL_SHORT:{
			typeSizeOf = sizeof(GLshort);
		}break;
		case GL_INT:{
			typeSizeOf = sizeof(GLint);
		}break;
		case GL_FLOAT:{
			typeSizeOf = sizeof(GLfloat);
		}break;
		case GL_DOUBLE:{
			typeSizeOf = sizeof(GLdouble);
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}
	if((size != 1) && (size != 2) && (size != 3) && (size != 4)){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	if((stride < 0) || (size < 0)){
		errorStatus = GL_INVALID_VALUE;
		return;
	}
	//OpenGL spec says it's Dynamic object allocation or using a referenced array
	if(vboTexCoord->vboIsDynamicMemoryAllocated == true){
		TGDSARM9Free(vboTexCoord->vboArrayMemoryStart);
	}
	if(ptr == NULL){
		vboTexCoord->vboArrayMemoryStart = (u32*)TGDSARM9Malloc(size*typeSizeOf*VBO_ARRAY_SIZE);
		vboTexCoord->vboIsDynamicMemoryAllocated = true;
	}
	else{
		vboTexCoord->vboArrayMemoryStart = (u32*)ptr;
		vboTexCoord->vboIsDynamicMemoryAllocated = false;
	}
	vboTexCoord->vertexBufferObjectstrideOffset = stride;
	vboTexCoord->ElementsPerVertexBufferObjectUnit = typeSizeOf; //defines pointer length
	vboTexCoord->VertexBufferObjectStartOffset = (int)0; //default to 0
	vboTexCoord->argCount = size;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr ){
	//unimplemented
	errorStatus = GL_INVALID_ENUM;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glGetPointerv( GLenum pname, GLvoid **params ){
	switch(pname){
		case GL_EDGE_FLAG_ARRAY_POINTER:{
			*params=vboEdgeFlag->vboArrayMemoryStart;
		}break;
		case GL_INDEX_ARRAY_POINTER:{
			*params=vboIndex->vboArrayMemoryStart;
		}break;
		case GL_NORMAL_ARRAY_POINTER:{
			*params=vboNormal->vboArrayMemoryStart;
		}break;
		case GL_TEXTURE_COORD_ARRAY_POINTER:{
			*params=vboTexCoord->vboArrayMemoryStart;
		}break;
		case GL_VERTEX_ARRAY_POINTER:{
			*params=vboVertex->vboArrayMemoryStart;
		}break;
		case GL_COLOR_ARRAY_POINTER:{
			*params=vboColor->vboArrayMemoryStart;
		}break;
		/*GL_FOG_COORD_ARRAY_POINTER, GL_FEEDBACK_BUFFER_POINTER, GL_SECONDARY_COLOR_ARRAY_POINTER, GL_SELECTION_BUFFER_POINTER, */ //unimplemented
		default:{
			errorStatus = GL_INVALID_ENUM;
		}break;
	}
}





/*
Example:
void		display_rect(int x, int y, int lettre)
{
	static GLfloat	vertices[] = {

	0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0,
	0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0};
	glPushMatrix();
	glLoadIdentity();
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, vertices + 12);
	glBindTexture(GL_TEXTURE_2D, g_env->textures[lettre]);
	glScalef(0.03, 0.03, 0);
	glTranslatef(x, y, 0);
	glBegin(GL_QUADS);
	{
		glArrayElement(0);
		glArrayElement(1);
		glArrayElement(2);
		glArrayElement(3);
	}
	glEnd();
	glPopMatrix();
}
*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glArrayElement( GLint index ){
	bool vboVertexEnabled = vboVertex->ClientStateEnabled;
	bool vboNormalEnabled = vboNormal->ClientStateEnabled;
	bool vboColorEnabled = vboColor->ClientStateEnabled;
	bool vboIndexEnabled = vboIndex->ClientStateEnabled;
	bool vboTexCoordEnabled = vboTexCoord->ClientStateEnabled;
	bool vboEdgeFlagEnabled = vboEdgeFlag->ClientStateEnabled;
	if(vboColorEnabled == true){
		switch(vboColor->ElementsPerVertexBufferObjectUnit){
			//unsigned char
			case 1:{
				unsigned char * colorOffset = (unsigned char*)( ((int)vboColor->vboArrayMemoryStart) +  (index * sizeof(unsigned char) + ((vboColor->vertexBufferObjectstrideOffset*sizeof(unsigned char)*vboColor->argCount))));
				unsigned char arg0 = (unsigned char)*(colorOffset+0);
				unsigned char arg1 = (unsigned char)*(colorOffset+1);
				unsigned char arg2 = (unsigned char)*(colorOffset+2);
				glColor3b(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
						
			//unsigned short
			/*
			case 2:{
			//no GX methods for 2 byte colors
			}break;
			*/
			
			//unsigned int (packed as Float -> u8)
			case 4:{
				GLfloat * colorOffset = (GLfloat*)( ((int)vboColor->vboArrayMemoryStart) +  (index * sizeof(GLfloat) + ((vboColor->vertexBufferObjectstrideOffset*sizeof(GLfloat)*vboColor->argCount))));
				u8 arg0 = (u8)*(colorOffset+0);
				u8 arg1 = (u8)*(colorOffset+1);
				u8 arg2 = (u8)*(colorOffset+2);
				glColor3b((u8)arg0, (u8)arg1, (u8)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
		}
	}

	if(vboNormalEnabled == true){
		switch(vboNormal->ElementsPerVertexBufferObjectUnit){
			/*
			//unsigned char
			case 1:{
				//Normals are 10bit at least
			}break;
			*/
			//unsigned short
			case 2:{
				unsigned short * normalOffset = (unsigned short*)( ((int)vboNormal->vboArrayMemoryStart) +  (index * sizeof(unsigned short) + ((vboNormal->vertexBufferObjectstrideOffset*sizeof(unsigned short)*vboNormal->argCount))));
				unsigned short arg0 = (unsigned short)*(normalOffset+0);
				unsigned short arg1 = (unsigned short)*(normalOffset+1);
				unsigned short arg2 = (unsigned short)*(normalOffset+2);
				glNormal3v10((v10)arg0, (v10)arg1, (v10)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
			
			//unsigned int (packed as Float -> v10)
			case 4:{
				GLfloat * normalOffset = (GLfloat*)( ((int)vboNormal->vboArrayMemoryStart) +  (index * sizeof(GLfloat) + ((vboNormal->vertexBufferObjectstrideOffset*sizeof(GLfloat)*vboNormal->argCount))));
				v10 arg0 = (v10)*(normalOffset+0);
				v10 arg1 = (v10)*(normalOffset+1);
				v10 arg2 = (v10)*(normalOffset+2);
				glNormal3v10((v10)arg0, (v10)arg1, (v10)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
		}
	}

	if(vboTexCoordEnabled == true){
		switch(vboTexCoord->ElementsPerVertexBufferObjectUnit){
			/*
			//unsigned char
			case 1:{
				//GX texture coordinates are at least 12.4bit
			}break;
			*/
			//unsigned short
			case 2:{
				unsigned short * TexCoordOffset = (unsigned short*)( ((int)vboTexCoord->vboArrayMemoryStart) +  (index * sizeof(unsigned short) + ((vboTexCoord->vertexBufferObjectstrideOffset*sizeof(unsigned short)*vboTexCoord->argCount))));
				unsigned short arg0 = (unsigned short)*(TexCoordOffset+0);
				unsigned short arg1 = (unsigned short)*(TexCoordOffset+1);
				glTexCoord2t16((t16)arg0, (t16)arg1, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
			
			//unsigned int (packed as Float -> t16)
			case 4:{
				GLfloat * TexCoordOffset = (GLfloat*)( ((int)vboTexCoord->vboArrayMemoryStart) +  (index * sizeof(GLfloat) + ((vboTexCoord->vertexBufferObjectstrideOffset*sizeof(GLfloat)*vboTexCoord->argCount))));
				t16 arg0 = (t16)*(TexCoordOffset+0);
				t16 arg1 = (t16)*(TexCoordOffset+1);
				glTexCoord2t16((t16)arg0, (t16)arg1, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
		}
	}

	if(vboVertexEnabled == true){
		switch(vboVertex->ElementsPerVertexBufferObjectUnit){
			/*
			//unsigned char
			case 1:{
				//GX vertex are at least 10bit
			}break;
			*/
			//unsigned short
			case 2:{
				u16 * vertexOffset = (u16*)( ((int)vboVertex->vboArrayMemoryStart) +  (index * sizeof(v16) + ((vboVertex->vertexBufferObjectstrideOffset*sizeof(v16)*vboVertex->argCount))));
				v16 arg0 = (v16)*(vertexOffset+0);
				v16 arg1 = (v16)*(vertexOffset+1);
				v16 arg2 = (v16)*(vertexOffset+2);
				glVertex3v16(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
			
			//unsigned int (packed as Float -> v16)
			case 4:{
				GLfloat * vertexOffset = (GLfloat*)( ((int)vboVertex->vboArrayMemoryStart) +  (index * sizeof(GLfloat) + ((vboVertex->vertexBufferObjectstrideOffset*sizeof(GLfloat)*vboVertex->argCount))));
				v16 arg0 = (v16)*(vertexOffset+0);
				v16 arg1 = (v16)*(vertexOffset+1);
				v16 arg2 = (v16)*(vertexOffset+2);
				glVertex3v16(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
			}break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
glDrawArrays(): render primitives from array data
C Specification
void glDrawArrays(	GLenum	mode,
GLint	first,
GLsizei	count);
Parameters
mode
Specifies what kind of primitives to render. Symbolic constants GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY are accepted.

first
Specifies the starting index in the enabled arrays.

count
Specifies the number of indices to be rendered.

Description
glDrawArrays specifies multiple geometric primitives with very few subroutine calls. Instead of calling a GL procedure to pass each individual vertex, normal, texture coordinate, edge flag, or color, you can prespecify separate arrays of vertices, normals, and colors and use them to construct a sequence of primitives with a single call to glDrawArrays.

When glDrawArrays is called, it uses count sequential elements from each enabled array to construct a sequence of geometric primitives, beginning with element first. mode specifies what kind of primitives are constructed and how the array elements construct those primitives.

Vertex attributes that are modified by glDrawArrays have an unspecified value after glDrawArrays returns. Attributes that aren't modified remain well defined.
*/

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
int OGL_DL_DRAW_ARRAYS_METHOD = -1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glDrawArrays( GLenum mode, GLint first, GLsizei count ){
	bool requiresRecompileGXDisplayList = false;
	bool vboVertexEnabled = vboVertex->ClientStateEnabled;
	bool vboNormalEnabled = vboNormal->ClientStateEnabled;
	bool vboColorEnabled = vboColor->ClientStateEnabled;
	bool vboIndexEnabled = vboIndex->ClientStateEnabled;
	bool vboTexCoordEnabled = vboTexCoord->ClientStateEnabled;
	bool vboEdgeFlagEnabled = vboEdgeFlag->ClientStateEnabled;
	int argsTotal = 0;
	if(count < 0){
		errorStatus = GL_INVALID_ENUM;
		return;
	}
	if((count > 6144)){
		count = 6144; //DS can only render up to 6144 vertices / 2048 polys 
	}
	switch(mode){
		//unsupported by DS hardware
		case	GL_POINTS:
				GL_LINE_STRIP:
				GL_LINE_LOOP:
				GL_LINES:
				GL_LINE_STRIP_ADJACENCY:
				GL_LINES_ADJACENCY:
				GL_TRIANGLE_FAN:
				GL_TRIANGLE_STRIP_ADJACENCY:
				GL_TRIANGLES_ADJACENCY:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
		
		//Otherwise, supported modes natively by DS hardware: GL_TRIANGLE_STRIP, GL_TRIANGLES, 
	}

	//////////!!!!!!!!!!!!!!!!!!ORDER IS IMPORTANT DUE TO HOW GX DISPLAYLISTS ARE BUILT ON NDS HARDWARE!!!!!!!!!!!!!!!!!!
	//////////////////////////Build DisplayLists and CRC all buffers generated from each VBO buffer//////////////////////////
	//With glDrawArrays, you can specify multiple geometric primitives to render. Instead of calling separate OpenGL functions to pass each individual vertex, normal, or color, you can specify separate arrays of vertices, normals, and colors to define a sequence of primitives (all the same kind) with a single call to glDrawArrays.
	//When you call glDrawArrays, count sequential elements from each enabled array are used to construct a sequence of geometric primitives, beginning with the first element. The mode parameter specifies what kind of primitive to construct and how to use the array elements to construct the primitives.
	//1
	if(vboColorEnabled == true){
		u16 colorArraycrc16 = swiCRC16(0xffff, (void*)((int)vboColor->vboArrayMemoryStart + first), (uint32)((int)count*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboColor->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != colorArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = colorArraycrc16;
		}
		argsTotal++;
	}
	
	//2
	if(vboNormalEnabled == true){
		u16 normalArraycrc16 = swiCRC16(0xffff, (void*)((int)vboNormal->vboArrayMemoryStart + first), (uint32)((int)count*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboNormal->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != normalArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = normalArraycrc16;
		}
		argsTotal++;
	}

	//3
	if(vboTexCoordEnabled == true){
		u16 texcoordArraycrc16 = swiCRC16(0xffff, (void*)((int)vboTexCoord->vboArrayMemoryStart + first), (uint32)((int)count*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboTexCoord->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != texcoordArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = texcoordArraycrc16;
		}
		argsTotal++;
	}

	//4
	if(vboVertexEnabled == true){
		u16 vertexArraycrc16 = swiCRC16(0xffff, (void*)((int)vboVertex->vboArrayMemoryStart + first), (uint32)((int)count*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboVertex->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != vertexArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = vertexArraycrc16;
		}
		argsTotal++;
	}

	//todo
	/*
	if(vboIndexEnabled == true){

	}
	*/
	
	//unused
	/*
	if(vboEdgeFlagEnabled == true){

	}
	*/

	//Emit DL and execute it inmediately if arrays are dirty, otherwise run the DL directly
	if(requiresRecompileGXDisplayList == true){
		int i = 0;
		int argCount = 3;
		if( (mode == GL_TRIANGLE_STRIP) || (mode == GL_QUAD_STRIP) || (mode == GL_QUADS) ){
			argCount = 4;
		}
		glNewList(OGL_DL_DRAW_ARRAYS_METHOD, GL_COMPILE_AND_EXECUTE, INTERNAL_TGDS_OGL_DL_POINTER);
			glBegin(mode, INTERNAL_TGDS_OGL_DL_POINTER);
			for(i = 0; i < count; i+=(argsTotal*argCount)){
				if(vboColorEnabled == true){
					switch(vboColor->ElementsPerVertexBufferObjectUnit){
						//unsigned char
						case 1:{
							unsigned char * colorOffset = (unsigned char*)( ((int)vboColor->vboArrayMemoryStart) +  (i * sizeof(unsigned char) + ((vboColor->vertexBufferObjectstrideOffset*sizeof(unsigned char)*argCount))));
							unsigned char arg0 = (unsigned char)*(colorOffset+0);
							unsigned char arg1 = (unsigned char)*(colorOffset+1);
							unsigned char arg2 = (unsigned char)*(colorOffset+2);
							glColor3b(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
						
						//unsigned short
						/*
						case 2:{
						//no GX methods for 2 byte colors
						}break;
						*/
						
						//unsigned int (packed as Float -> u8)
						case 4:{
							GLfloat * colorOffset = (GLfloat*)( ((int)vboColor->vboArrayMemoryStart) +  (i * sizeof(GLfloat) + ((vboColor->vertexBufferObjectstrideOffset*sizeof(GLfloat)*argCount))));
							u8 arg0 = (u8)*(colorOffset+0);
							u8 arg1 = (u8)*(colorOffset+1);
							u8 arg2 = (u8)*(colorOffset+2);
							glColor3b((u8)arg0, (u8)arg1, (u8)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}

				if(vboNormalEnabled == true){
					switch(vboNormal->ElementsPerVertexBufferObjectUnit){
						/*
						//unsigned char
						case 1:{
							//Normals are 10bit at least
						}break;
						*/
						//unsigned short
						case 2:{
							unsigned short * normalOffset = (unsigned short*)( ((int)vboNormal->vboArrayMemoryStart) +  (i * sizeof(unsigned short) + ((vboNormal->vertexBufferObjectstrideOffset*sizeof(unsigned short)*argCount))));
							unsigned short arg0 = (unsigned short)*(normalOffset+0);
							unsigned short arg1 = (unsigned short)*(normalOffset+1);
							unsigned short arg2 = (unsigned short)*(normalOffset+2);
							glNormal3v10((v10)arg0, (v10)arg1, (v10)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
						
						//unsigned int (packed as Float -> v10)
						case 4:{
							GLfloat * normalOffset = (GLfloat*)( ((int)vboNormal->vboArrayMemoryStart) +  (i * sizeof(GLfloat) + ((vboNormal->vertexBufferObjectstrideOffset*sizeof(GLfloat)*argCount))));
							v10 arg0 = (v10)floattov10(*(normalOffset+0));
							v10 arg1 = (v10)floattov10(*(normalOffset+1));
							v10 arg2 = (v10)floattov10(*(normalOffset+2));
							glNormal3v10((v10)arg0, (v10)arg1, (v10)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}

				if(vboTexCoordEnabled == true){
					switch(vboTexCoord->ElementsPerVertexBufferObjectUnit){
						/*
						//unsigned char
						case 1:{
							//GX texture coordinates are at least 12.4bit
						}break;
						*/
						//unsigned short
						case 2:{
							unsigned short * TexCoordOffset = (unsigned short*)( ((int)vboTexCoord->vboArrayMemoryStart) +  (i * sizeof(unsigned short) + ((vboTexCoord->vertexBufferObjectstrideOffset*sizeof(unsigned short)*argCount))));
							unsigned short arg0 = (unsigned short)*(TexCoordOffset+0);
							unsigned short arg1 = (unsigned short)*(TexCoordOffset+1);
							glTexCoord2t16((t16)arg0, (t16)arg1, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;

						//unsigned int (packed as Float -> t16)
						case 4:{
							GLfloat * TexCoordOffset = (GLfloat*)( ((int)vboTexCoord->vboArrayMemoryStart) +  (i * sizeof(unsigned int) + ((vboTexCoord->vertexBufferObjectstrideOffset*sizeof(unsigned int)*argCount))));
							t16 arg0 = (t16)floattot16(*(TexCoordOffset+0));
							t16 arg1 = (t16)floattot16(*(TexCoordOffset+1));
							glTexCoord2t16((t16)arg0, (t16)arg1, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}

				if(vboVertexEnabled == true){
					switch(vboVertex->ElementsPerVertexBufferObjectUnit){
						/*
						//unsigned char
						case 1:{
							//GX vertex are at least 10bit
						}break;
						*/
						//unsigned short
						case 2:{
							u16 * vertexOffset = (u16*)( ((int)vboVertex->vboArrayMemoryStart) +  (i * sizeof(v16) + ((vboVertex->vertexBufferObjectstrideOffset*sizeof(v16)*argCount))));
							v16 arg0 = (v16)*(vertexOffset+0);
							v16 arg1 = (v16)*(vertexOffset+1);
							v16 arg2 = (v16)*(vertexOffset+2);
							glVertex3v16(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;

						//unsigned int (packed as Float -> v16)
						case 4:{
							GLfloat * vertexOffset = (GLfloat*)( ((int)vboVertex->vboArrayMemoryStart) +  (i * sizeof(unsigned int) + ((vboVertex->vertexBufferObjectstrideOffset*sizeof(unsigned int)*argCount))));
							v16 arg0 = (v16)floattov16(*(vertexOffset+0));
							v16 arg1 = (v16)floattov16(*(vertexOffset+1));
							v16 arg2 = (v16)floattov16(*(vertexOffset+2));
							glVertex3v16(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}
			}
			glEnd(INTERNAL_TGDS_OGL_DL_POINTER);
		glEndList(INTERNAL_TGDS_OGL_DL_POINTER); 
		//TWLPrintf("glDrawArrays() Count: %d \n", count);
	}
	else{
		glCallList(OGL_DL_DRAW_ARRAYS_METHOD, INTERNAL_TGDS_OGL_DL_POINTER);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ){
	//OpenGL 1.1 supported only (and some of OpenGL 1.5)
	errorStatus = GL_INVALID_ENUM;
}

/* glInterleavedArrays() 
NOTE: You need to make sure that you set up the data in your structure (or class) 
// in the same order as you tell OpenGL it is.  By passing in GL_T2F_V3F we told OpenGL that 
// our texture data comes before the vertex data.  In our class/structure make sure that
// you define your data as such (see CVertex).  If you don't it won't work.  The data will
// be confused.
*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ){
	bool requiresRecompileGXDisplayList = false;
	bool vboVertexEnabled = true; //Vertex coordinates are always extracted.
	bool vboNormalEnabled = false;
	bool vboColorEnabled = false;
	bool vboIndexEnabled = false;
	bool vboTexCoordEnabled = false;
	bool vboEdgeFlagEnabled = false;
	int argsTotal = 0;
	int countVertex = 0;
	int countNormal = 0;
	int countColor = 0;
	int countTexCoord = 0;
	int mode = 0;
	u32* targetVertexOffset = NULL;
	u32* targetNormalOffset = NULL;
	u32* targetColorOffset = NULL;
	u32* targetTexCoordOffset = NULL;

	switch(format){
		//format: If format contains a T, then texture coordinates are extracted from the interleaved array.
		//If C is present, color values are extracted.
		//If N is present, normal coordinates are extracted.
		//Vertex coordinates are always extracted.
		//The digits 2, 3, and 4 denote how many values are extracted.
		//F indicates that values are extracted as floating point values.
		//If 4UB follows the C, colors may also be extracted as 4 unsigned bytes. If a color is extracted as 4 unsigned bytes, the vertex array element that follows is located at the first possible floating-point aligned address.

		case GL_V2F:{
			mode = GL_TRIANGLES;
			countVertex = 2 * sizeof(GLfloat);
			targetVertexOffset = (u32*)( ((int)pointer) +  (0 * countVertex));
		}break;

		case GL_V3F:{
			mode = GL_TRIANGLES;
			countVertex = 3 * sizeof(GLfloat);
			targetVertexOffset = (u32*)( ((int)pointer) +  (0 * countVertex));
		}break;
		
		//unsupported by NDS hardware
		/*
		case GL_C4UB_V2F:{
			
		}break;
		
		case GL_C4UB_V3F:{
		}break;
		*/

		case GL_C3F_V3F:{
			mode = GL_TRIANGLES;
			countColor = 3 * sizeof(GLfloat);
			vboColorEnabled = true;
			countVertex = 3 * sizeof(GLfloat);
			vboVertexEnabled = true;
			targetColorOffset = (u32*)( ((int)pointer) +  (0 * countColor));
			targetVertexOffset = (u32*)( ((int)pointer) +  (1 * countVertex));
		}break;
		
		case GL_N3F_V3F:{
			mode = GL_TRIANGLES;
			countNormal = 3 * sizeof(GLfloat);
			vboNormalEnabled = true;
			countVertex = 3 * sizeof(GLfloat);
			vboVertexEnabled = true;
			targetNormalOffset = (u32*)( ((int)pointer) +  (0 * countNormal));
			targetVertexOffset = (u32*)( ((int)pointer) +  (1 * countVertex));
		}break;
		
		//unsupported by NDS hardware
		/*
		case GL_C4F_N3F_V3F:{
		}break;
		*/

		case GL_T2F_V3F:{
			mode = GL_TRIANGLES;
			countTexCoord = 2 * sizeof(GLfloat);
			vboTexCoordEnabled = true;
			countVertex = 3 * sizeof(GLfloat);
			vboVertexEnabled = true;
			targetTexCoordOffset = (u32*)( ((int)pointer) +  (0 * countTexCoord));
			targetVertexOffset = (u32*)( ((int)pointer) +  (1 * countVertex));
		}break;
		
		//unsupported by NDS hardware
		/*
		case GL_T4F_V4F:{
		}break;
		
		case GL_T2F_C4UB_V3F:{
		}break;
		*/

		case GL_T2F_C3F_V3F:{
			mode = GL_TRIANGLES;
			countTexCoord = 2 * sizeof(GLfloat);
			vboTexCoordEnabled = true;
			countColor = 3 * sizeof(GLfloat);
			vboColorEnabled = true;
			countVertex = 3 * sizeof(GLfloat);
			vboVertexEnabled = true;
			targetTexCoordOffset = (u32*)( ((int)pointer) +  (0 * countTexCoord));
			targetColorOffset = (u32*)( ((int)pointer) +  (1 * countColor));
			targetVertexOffset = (u32*)( ((int)pointer) +  (2 * countVertex));
		}break;
		
		case GL_T2F_N3F_V3F:{
			mode = GL_TRIANGLES;
			countTexCoord = 2 * sizeof(GLfloat);
			vboTexCoordEnabled = true;
			countNormal = 3 * sizeof(GLfloat);
			vboNormalEnabled = true;
			countVertex = 3 * sizeof(GLfloat);
			vboVertexEnabled = true;
			targetTexCoordOffset = (u32*)( ((int)pointer) +  (0 * countTexCoord));
			targetNormalOffset = (u32*)( ((int)pointer) +  (1 * countNormal));
			targetVertexOffset = (u32*)( ((int)pointer) +  (2 * countVertex));
		}break;

		case GL_T2F_C4F_N3F_V3F:{
			mode = GL_TRIANGLES;
			countTexCoord = 2 * sizeof(GLfloat);
			vboTexCoordEnabled = true;
			countColor = 3 * sizeof(GLfloat); //max 3 colors
			vboColorEnabled = true;
			countNormal = 3 * sizeof(GLfloat);
			vboNormalEnabled = true;
			countVertex = 3 * sizeof(GLfloat);
			vboVertexEnabled = true;
			targetTexCoordOffset = (u32*)( ((int)pointer) +  (0 * countTexCoord));
			targetColorOffset = (u32*)( ((int)pointer) +  (1 * countColor));
			targetNormalOffset = (u32*)( ((int)pointer) +  (2 * countNormal));
			targetVertexOffset = (u32*)( ((int)pointer) +  (3 * countVertex));
		}break;
		
		//Unsupported by NDS hardware
		/*
		case GL_T4F_C4F_N3F_V4F:{
		}break;
		*/

		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}


	//////////!!!!!!!!!!!!!!!!!!ORDER IS IMPORTANT DUE TO HOW GX DISPLAYLISTS ARE BUILT ON NDS HARDWARE!!!!!!!!!!!!!!!!!!
	//////////////////////////Build DisplayLists and CRC all buffers generated from each VBO buffer//////////////////////////
	//With glDrawArrays, you can specify multiple geometric primitives to render. Instead of calling separate OpenGL functions to pass each individual vertex, normal, or color, you can specify separate arrays of vertices, normals, and colors to define a sequence of primitives (all the same kind) with a single call to glDrawArrays.
	//When you call glDrawArrays, count sequential elements from each enabled array are used to construct a sequence of geometric primitives, beginning with the first element. The mode parameter specifies what kind of primitive to construct and how to use the array elements to construct the primitives.
	//1
	if(vboColorEnabled == true){
		u16 colorArraycrc16 = swiCRC16(0xffff, (void*)((int)targetColorOffset), (uint32)((int)countColor*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboColor->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != colorArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = colorArraycrc16;
		}
		argsTotal++;
	}
	
	//2
	if(vboNormalEnabled == true){
		u16 normalArraycrc16 = swiCRC16(0xffff, (void*)((int)targetNormalOffset), (uint32)((int)countNormal*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboNormal->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != normalArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = normalArraycrc16;
		}
		argsTotal++;
	}

	//3
	if(vboTexCoordEnabled == true){
		u16 texcoordArraycrc16 = swiCRC16(0xffff, (void*)((int)targetTexCoordOffset), (uint32)((int)countTexCoord*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboTexCoord->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != texcoordArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = texcoordArraycrc16;
		}
		argsTotal++;
	}

	//4
	if(vboVertexEnabled == true){
		u16 vertexArraycrc16 = swiCRC16(0xffff, (void*)((int)targetVertexOffset), (uint32)((int)countVertex*sizeof(GLfloat)));
		u16 * crc16Ptr = &vboVertex->lastPrebuiltDLCRC16;
		if( ((u16)*(crc16Ptr)) != vertexArraycrc16){
			requiresRecompileGXDisplayList = true;
			*crc16Ptr = vertexArraycrc16;
		}
		argsTotal++;
	}

	//todo
	/*
	if(vboIndexEnabled == true){

	}
	*/
	
	//unused
	/*
	if(vboEdgeFlagEnabled == true){

	}
	*/

	//Emit DL and execute it inmediately if arrays are dirty, otherwise run the DL directly
	if(requiresRecompileGXDisplayList == true){
		int i = 0;
		bool isTriangleStrip = false; //false = GL_TRIANGLES (3 args) / true = GL_TRIANGLE_STRIP (4 args)
		int argCount = 3;
		if(mode == GL_TRIANGLE_STRIP){
			isTriangleStrip = true;
			argCount = 4;
		}
		glNewList(OGL_DL_DRAW_ARRAYS_METHOD, GL_COMPILE_AND_EXECUTE, INTERNAL_TGDS_OGL_DL_POINTER);
			glBegin(mode, INTERNAL_TGDS_OGL_DL_POINTER);
			for(i = 0; i < (countColor+countNormal+countTexCoord+countVertex); i+=(argsTotal*argCount)){
				if(vboColorEnabled == true){
					switch(vboColor->ElementsPerVertexBufferObjectUnit){
						//unsigned char
						case 1:{
							unsigned char * colorOffset = (unsigned char*)( ((int)targetColorOffset) +  (i * sizeof(unsigned char) + ((stride*sizeof(unsigned char)*argCount))));
							unsigned char arg0 = (unsigned char)*(colorOffset+0);
							unsigned char arg1 = (unsigned char)*(colorOffset+1);
							unsigned char arg2 = (unsigned char)*(colorOffset+2);
							glColor3b(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
						
						//unsigned short
						/*
						case 2:{
						//no GX methods for 2 byte colors
						}break;
						*/
						
						//unsigned int (packed as Float -> u8)
						case 4:{
							GLfloat * colorOffset = (GLfloat*)( ((int)targetColorOffset) +  (i * sizeof(GLfloat) + ((stride*sizeof(GLfloat)*argCount))));
							u8 arg0 = (u8)*(colorOffset+0);
							u8 arg1 = (u8)*(colorOffset+1);
							u8 arg2 = (u8)*(colorOffset+2);
							glColor3b((u8)arg0, (u8)arg1, (u8)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}

				if(vboNormalEnabled == true){
					switch(vboNormal->ElementsPerVertexBufferObjectUnit){
						/*
						//unsigned char
						case 1:{
							//Normals are 10bit at least
						}break;
						*/
						//unsigned short
						case 2:{
							unsigned short * normalOffset = (unsigned short*)( ((int)targetNormalOffset) +  (i * sizeof(unsigned short) + ((stride*sizeof(unsigned short)*argCount))));
							unsigned short arg0 = (unsigned short)*(normalOffset+0);
							unsigned short arg1 = (unsigned short)*(normalOffset+1);
							unsigned short arg2 = (unsigned short)*(normalOffset+2);
							glNormal3v10((v10)arg0, (v10)arg1, (v10)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
						
						//unsigned int (packed as Float -> v10)
						case 4:{
							GLfloat * normalOffset = (GLfloat*)( ((int)targetNormalOffset) +  (i * sizeof(GLfloat) + ((stride*sizeof(GLfloat)*argCount))));
							v10 arg0 = (v10)floattov10(*(normalOffset+0));
							v10 arg1 = (v10)floattov10(*(normalOffset+1));
							v10 arg2 = (v10)floattov10(*(normalOffset+2));
							glNormal3v10((v10)arg0, (v10)arg1, (v10)arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}

				if(vboTexCoordEnabled == true){
					switch(vboTexCoord->ElementsPerVertexBufferObjectUnit){
						/*
						//unsigned char
						case 1:{
							//GX texture coordinates are at least 12.4bit
						}break;
						*/
						//unsigned short
						case 2:{
							unsigned short * TexCoordOffset = (unsigned short*)( ((int)targetTexCoordOffset) +  (i * sizeof(unsigned short) + ((stride*sizeof(unsigned short)*argCount))));
							unsigned short arg0 = (unsigned short)*(TexCoordOffset+0);
							unsigned short arg1 = (unsigned short)*(TexCoordOffset+1);
							glTexCoord2t16((t16)arg0, (t16)arg1, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
						
						//unsigned int (packed as Float -> t16)
						case 4:{
							GLfloat * TexCoordOffset = (GLfloat*)( ((int)targetTexCoordOffset) +  (i * sizeof(GLfloat) + ((stride*sizeof(GLfloat)*argCount))));
							t16 arg0 = (t16)floattot16(*(TexCoordOffset+0));
							t16 arg1 = (t16)floattot16(*(TexCoordOffset+1));
							glTexCoord2t16((t16)arg0, (t16)arg1, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}

				if(vboVertexEnabled == true){
					switch(vboVertex->ElementsPerVertexBufferObjectUnit){
						/*
						//unsigned char
						case 1:{
							//GX vertex are at least 10bit
						}break;
						*/
						//unsigned short
						case 2:{
							u16 * vertexOffset = (u16*)( ((int)targetVertexOffset) +  (i * sizeof(v16) + ((stride*sizeof(v16)*argCount))));
							v16 arg0 = (v16)*(vertexOffset+0);
							v16 arg1 = (v16)*(vertexOffset+1);
							v16 arg2 = (v16)*(vertexOffset+2);
							glVertex3v16(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
						
						//unsigned int (packed as Float -> v16)
						case 4:{
							GLfloat * vertexOffset = (GLfloat*)( ((int)targetVertexOffset) +  (i * sizeof(GLfloat) + ((stride*sizeof(GLfloat)*argCount))));
							v16 arg0 = (v16)floattov16(*(vertexOffset+0));
							v16 arg1 = (v16)floattov16(*(vertexOffset+1));
							v16 arg2 = (v16)floattov16(*(vertexOffset+2));
							glVertex3v16(arg0, arg1, arg2, INTERNAL_TGDS_OGL_DL_POINTER);
						}break;
					}
				}
			}
			glEnd(INTERNAL_TGDS_OGL_DL_POINTER);
		glEndList(INTERNAL_TGDS_OGL_DL_POINTER); 
	}
	else{
		glCallList(OGL_DL_DRAW_ARRAYS_METHOD, INTERNAL_TGDS_OGL_DL_POINTER);
	}
}

//The glGetMaterialfv and glGetMaterialiv functions return material parameters.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glGetMaterialfv(
   GLenum  face,
   GLenum  pname,
   GLfloat *params
   ){
	//GL_FRONT or GL_BACK are allowed and ignored in GX
	switch(face){
		case GL_FRONT:
		case GL_BACK:{
				
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
		return;
		}break;
	}
	switch(pname){
		/*40004C0h - Cmd 30h - DIF_AMB - MaterialColor0 - Diffuse/Ambient Reflect. (W)
		0-4   Diffuse Reflection Red     ;\light(s) that directly hits the polygon,
		5-9   Diffuse Reflection Green   ; ie. max when NormalVector has opposite
		10-14 Diffuse Reflection Blue    ;/direction of LightVector
		15    Set Vertex Color (0=No, 1=Set Diffuse Reflection Color as Vertex Color)
		16-20 Ambient Reflection Red     ;\light(s) that indirectly hits the polygon,
		21-25 Ambient Reflection Green   ; ie. assuming that light is reflected by
		26-30 Ambient Reflection Blue    ;/walls/floor, regardless of LightVector
		31    Not used*/
		case GL_AMBIENT:{
			//The params parameter returns four integer or floating-point values representing the ambient reflectance of the material. 
			//Integer values, when requested, are linearly mapped from the internal floating-point representation such that 1.0 maps to 
			//the most positive representable integer value, and -1.0 maps to the most negative representable integer value. 
			//If the internal value is outside the range [-1,1], the corresponding integer return value is undefined.
			*(params+0) = ((float)((globalGLCtx.ambientValue>>0)&0x1f)); //r
			*(params+1) = ((float)((globalGLCtx.ambientValue>>5)&0x1f)); //g
			*(params+2) = ((float)((globalGLCtx.ambientValue>>10)&0x1f)); //b
			*(params+3) = 0;
		}break;
		case GL_DIFFUSE:{
			//The params parameter returns four integer or floating-point values representing the diffuse reflectance of the material. 
			//Integer values, when requested, are linearly mapped from the internal floating-point representation such that 1.0 maps to 
			//the most positive representable integer value, and -1.0 maps to the most negative representable integer value. 
			//If the internal value is outside the range [-1,1], the corresponding integer return value is undefined.
			*(params+0) = ((float)((globalGLCtx.diffuseValue>>0)&0x1f)); //r
			*(params+1) = ((float)((globalGLCtx.diffuseValue>>5)&0x1f)); //g
			*(params+2) = ((float)((globalGLCtx.diffuseValue>>10)&0x1f)); //b
			*(params+3) = 0;
		}break;
		case GL_SPECULAR:{
			//The params parameter returns four integer or floating-point values representing the specular reflectance of the material. 
			//Integer values, when requested, are linearly mapped from the internal floating-point representation such that 1.0 maps to 
			//the most positive representable integer value, and -1.0 maps to the most negative representable integer value. 
			//If the internal value is outside the range [-1,1], the corresponding integer return value is undefined.
			*(params+0) = ((float)((globalGLCtx.specularValue>>0)&0x1f)); //r
			*(params+1) = ((float)((globalGLCtx.specularValue>>5)&0x1f)); //g
			*(params+2) = ((float)((globalGLCtx.specularValue>>10)&0x1f)); //b
			*(params+3) = 0;
		}break;
		case GL_SHININESS:{
			//The params parameter returns one integer or floating-point value representing the specular exponent of the material. 
			//Integer values, when requested, are computed by rounding the internal floating-point value to the nearest integer value.
			*(params+0) = globalGLCtx.shininessValue;
		}break;
		case GL_EMISSION:{
			//The params parameter returns four integer or floating-point values representing the emitted light intensity of the material. 
			//Integer values, when requested, are linearly mapped from the internal floating-point representation such that 1.0 maps to 
			//the most positive representable integer value, and -1.0 maps to the most negative representable integer value. 
			//If the internal value is outside the range [-1,1], the corresponding integer return value is undefined.
			*(params+0) = ((float)((globalGLCtx.emissionValue>>0)&0x1f)); //r
			*(params+1) = ((float)((globalGLCtx.emissionValue>>5)&0x1f)); //g
			*(params+2) = ((float)((globalGLCtx.emissionValue>>10)&0x1f)); //b
			*(params+3) = 0;
		}break;
		default:{
			errorStatus = GL_INVALID_ENUM;
			return;
		}break;
	}
}

//See glGetFloatv(); for implementation. 
//Note: It's the same because both int and float datatypes are 4-byte on ARM v5t CPU platforms.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void glGetIntegerv(
   GLenum pname,
   GLint  *params
   ){
	   glGetFloatv(pname, (GLfloat*)params);
}

//////////////////////////////////////////////////////////// Extended Vertex Array Buffers and Vertex Buffer Objects OpenGL 1.1 end //////////////////////////////////////////
