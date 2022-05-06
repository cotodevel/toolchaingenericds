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

//Internal Unpacked GX buffer
	u32 InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_internalSize];
	u32 InternalUnpackedGX_DL_Binary_StandardOGLPtr;			//First 4K
	u32 InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;	//Second 4K
	
	u32 SingleUnpackedGXCommand_DL_Binary[PHYS_GXFIFO_INTERNAL_SIZE]; //Unpacked single command GX Buffer
//Initializes the NDS OpenGL system
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
	InternalUnpackedGX_DL_Binary_StandardOGLPtr=0; //1st 4K
	InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr=0; //2nd 4K
	memset(getInternalUnpackedDisplayListBuffer_StandardOGLCurOffset(), 0, InternalUnpackedGX_DL_workSize);
	memset(getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(), 0, InternalUnpackedGX_DL_workSize);
	globalGLCtx.mode = GL_COMPILE;
    isCustomDisplayList = false;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPushMatrix(void){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = getMTX_PUSH();
		(*savedDLBufferOffsetPtr)++;
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPopMatrix(sint32 index){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = getMTX_POP();
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)index;
		(*savedDLBufferOffsetPtr)++;
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void handleInmediateGXDisplayList(u32 * sourcePhysDisplayList, u32 * sourcePhysDisplayListPtr, u8 cmdSource, int alternateParamsCount){
	if((isCustomDisplayList == false) && (isAnOpenGLExtendedDisplayListCallList == false)){ //Only run Standard Open GL calls. Extended OpenGL DL CallLists are ran from standard-specific CallList() opcodes
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
				if (val == (u32)getMTX_STORE()) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_STORE = arg1;
					#endif
					leftArgCnt-= MTX_STORE_GXCommandParamsCount == 0 ? 1 : MTX_STORE_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_TRANS()) {
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
				else if (val == (u32)getMTX_IDENTITY()) {
					//write commands
					currCmd++; 
					#ifdef ARM9
					MATRIX_IDENTITY = 0;
					#endif
					leftArgCnt-= MTX_IDENTITY_GXCommandParamsCount == 0 ? 1 : MTX_IDENTITY_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_MODE()) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_CONTROL = arg1;
					#endif
					leftArgCnt-= MTX_MODE_GXCommandParamsCount == 0 ? 1 : MTX_MODE_GXCommandParamsCount;
				}
				else if (val == (u32)getVIEWPORT()) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_VIEWPORT = arg1;
					#endif
					leftArgCnt-= VIEWPORT_GXCommandParamsCount == 0 ? 1 : VIEWPORT_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_TEX_COORD()) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_TEX_COORD = arg1;
					#endif
					leftArgCnt-= FIFO_TEX_COORD_GXCommandParamsCount == 0 ? 1 : FIFO_TEX_COORD_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_BEGIN()) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_BEGIN = arg1;
					#endif
					leftArgCnt-= FIFO_BEGIN_GXCommandParamsCount == 0 ? 1 : FIFO_BEGIN_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_END()) {
					//write commands
					currCmd++; 
					#ifdef ARM9
					GFX_END = 0;
					#endif
					leftArgCnt-= FIFO_END_GXCommandParamsCount == 0 ? 1 : FIFO_END_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_COLOR()) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_COLOR = arg1;
					#endif
					leftArgCnt-= FIFO_COLOR_GXCommandParamsCount == 0 ? 1 : FIFO_COLOR_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_NORMAL()) {
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					GFX_NORMAL = arg1;
					#endif
					leftArgCnt-= FIFO_NORMAL_GXCommandParamsCount == 0 ? 1 : FIFO_NORMAL_GXCommandParamsCount;
				}
				else if (val == (u32)getFIFO_VERTEX16()) { 
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
				else if (val == (u32)getFIFO_VERTEX10()) {
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
				else if (val == (u32)getMTX_PUSH()) { 
					//write commands
					currCmd++; 
					#ifdef ARM9
					MATRIX_PUSH = 0;
					#endif
					leftArgCnt-= MTX_PUSH_GXCommandParamsCount == 0 ? 1 : MTX_PUSH_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_POP()) { 
					//write commands
					currCmd++; 
					u32 arg1 = *currCmd; currCmd++;
					#ifdef ARM9
					MATRIX_POP = arg1;
					#endif
					leftArgCnt-= MTX_POP_GXCommandParamsCount == 0 ? 1 : MTX_POP_GXCommandParamsCount;
				}
				else if (val == (u32)getMTX_MULT_3x3()) {
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
				else if (val == (u32)getMTX_MULT_4x4()) {
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

				else if (val == (u32)getMTX_LOAD_4x4()) {
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

				else if (val == (u32)getMTX_LOAD_4x3()) {
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRestoreMatrix(sint32 index){
	MATRIX_RESTORE = index;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glStoreMatrix(sint32 index){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//400044Ch - Cmd 13h - MTX_STORE - Store Current Matrix on Stack (W). Sets [N]=C. The stack pointer S is not used, and is left unchanged.
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_STORE(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)(index&0x1f); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glScalev(GLvector* v){
	MATRIX_SCALE = v->x;
	MATRIX_SCALE = v->y;
	MATRIX_SCALE = v->z;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslatev(GLvector* v){
	MATRIX_TRANSLATE = v->x;
	MATRIX_TRANSLATE = v->y;
	MATRIX_TRANSLATE = v->z;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslate3f32(f32 x, f32 y, f32 z){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//MTX_TRANS: Sets C=M*C. Parameters: 3, m[0..2] (x,y,z position)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] =(getMTX_TRANS()); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)(x); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)(y); (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)(z); (*savedDLBufferOffsetPtr)++;
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], (u32*)savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glScalef32(f32 factor){
	MATRIX_SCALE = factor;
	MATRIX_SCALE = factor;
	MATRIX_SCALE = factor;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glTranslatef32(int x, int y, int z) {
	MATRIX_TRANSLATE = x;
	MATRIX_TRANSLATE = y;
	MATRIX_TRANSLATE = z;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLight(int id, rgb color, v10 x, v10 y, v10 z){
	id = (id & 3) << 30;
	GFX_LIGHT_VECTOR = id | ((z & 0x3FF) << 20) | ((y & 0x3FF) << 10) | (x & 0x3FF);
	GFX_LIGHT_COLOR = id | color;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glNormal(uint32 normal){
	GFX_NORMAL = normal;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadIdentity(void){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000454h 15h -  19  MTX_IDENTITY - Load Unit(Identity) Matrix to Current Matrix (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_IDENTITY(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMatrixMode(int mode){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000440h 10h 1  1   MTX_MODE - Set Matrix Mode (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_MODE(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)(mode); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMaterialShinnyness(void){
	uint32 shiny32[128/4];
	uint8  *shiny8 = (uint8*)shiny32;

	int i;

	for (i = 0; i < 128 * 2; i += 2)
		shiny8[i>>1] = i;

	for (i = 0; i < 128 / 4; i++){
		GFX_SHININESS = shiny32[i];
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPolyFmt(int alpha){ // obviously more to this
	#ifdef ARM9
	GFX_POLY_FORMAT = alpha;
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glViewport(uint8 x1, uint8 y1, uint8 x2, uint8 y2){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000580h 60h 1  1   VIEWPORT - Set Viewport (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getVIEWPORT(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)((x1) + (y1 << 8) + (x2 << 16) + (y2 << 24)); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

u8 defaultglClearColorR=0;
u8 defaultglClearColorG=0;
u8 defaultglClearColorB=0;
u16 defaultglClearDepth=0;

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glEnable(int bits){
	enable_bits |= bits & (GL_TEXTURE_2D|GL_TOON_HIGHLIGHT|GL_OUTLINE|GL_ANTIALIAS);
	#ifdef ARM9
	GFX_CONTROL = enable_bits;
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glDisable(int bits){
	enable_bits &= ~(bits & (GL_TEXTURE_2D|GL_TOON_HIGHLIGHT|GL_OUTLINE|GL_ANTIALIAS));
	GFX_CONTROL = enable_bits;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glFlush(void){
	#ifdef ARM9
	GFX_FLUSH = 2;
	#endif
}

//OpenGL states this behaves the same as glFlush but also CPU waits for all commands to be executed by the GPU
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadMatrix4x4(m4x4 * m){
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glLoadMatrix4x3(m4x3* m){
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMultMatrix4x4(m4x4* m){
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMultMatrix4x3(m4x3* m){
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

// Integer rotation (not gl standard)
//	based on 512 degree circle
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateZi(int angle){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	if(((int)((*savedDLBufferOffsetPtr)+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		isCustomDisplayList = true; //Begin manual GX Display List
		u32 argCnt = (*savedDLBufferOffsetPtr);
		{
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
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_MULT_3x3(); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)cosine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)sine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)-sine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)cosine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)inttof32(1); (*savedDLBufferOffsetPtr)++;
		}
		isCustomDisplayList = false; //End manual GX Display List
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, MTX_ROTATE_Z, (*savedDLBufferOffsetPtr) - argCnt);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateYi(int angle){
	f32 sine = SIN[angle &  LUT_MASK];
	f32 cosine = COS[angle & LUT_MASK];
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	if(((int)((*savedDLBufferOffsetPtr)+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		isCustomDisplayList = true; //Begin manual GX Display List
		u32 argCnt = (*savedDLBufferOffsetPtr);
		{
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
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_MULT_3x3(); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)cosine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)-sine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)inttof32(1); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)sine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)cosine; (*savedDLBufferOffsetPtr)++;
		}
		isCustomDisplayList = false; //End manual GX Display List
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, MTX_ROTATE_Y, (*savedDLBufferOffsetPtr) - argCnt);	
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glRotateXi(int angle){
  f32 sine = SIN[angle &  LUT_MASK];
  f32 cosine = COS[angle & LUT_MASK];
  u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
  u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
  if(((int)((*savedDLBufferOffsetPtr)+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
	  	isCustomDisplayList = true; //Begin manual GX Display List
		u32 argCnt = (*savedDLBufferOffsetPtr);
		{
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
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_MULT_3x3(); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)inttof32(1); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)cosine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)sine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)-sine; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)cosine; (*savedDLBufferOffsetPtr)++;
		}
		isCustomDisplayList = false; //End manual GX Display List
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, MTX_ROTATE_X, (*savedDLBufferOffsetPtr) - argCnt);
	}
}

//	rotations wrapped in float...mainly for testing
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
	
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	isCustomDisplayList = true; //Begin manual GX Display List
	u32 argCnt = (*savedDLBufferOffsetPtr);
	{
		glMatrixMode(GL_MODELVIEW);
		//400045Ch 17h 12 30  MTX_LOAD_4x3 - Load 4x3 Matrix to Current Matrix (W)
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_LOAD_4x3(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;

		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)x[0]; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)x[1]; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)x[2]; (*savedDLBufferOffsetPtr)++;
		
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)y[0]; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)y[1]; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)y[2]; (*savedDLBufferOffsetPtr)++;
		
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)z[0]; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)z[1]; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)z[2]; (*savedDLBufferOffsetPtr)++;
		
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)floattof32(-1.0); (*savedDLBufferOffsetPtr)++;
		glTranslate3f32(-eyex, -eyey, -eyez);
	}
	isCustomDisplayList = false; //End manual GX Display List
	handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, MTX_LOOKAT, (*savedDLBufferOffsetPtr) - argCnt);
}

//  glu wrapper for standard float call
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluFrustumf32(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	if(((int)((*savedDLBufferOffsetPtr)+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		isCustomDisplayList = true; //Begin manual GX Display List
		u32 argCnt = (*savedDLBufferOffsetPtr);
		{
			glMatrixMode(GL_PROJECTION);
			//4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getMTX_LOAD_4x4(); //Unpacked Command format
			(*savedDLBufferOffsetPtr)++;

			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)divf32(2*near, right - left); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)divf32(right + left, right - left); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
		
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)divf32(2*near, top - bottom); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)divf32(top + bottom, top - bottom); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;

			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)-divf32(far + near, far - near); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)floattof32(-1.0F); (*savedDLBufferOffsetPtr)++;

			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)-divf32(2 * mulf32(far, near), far - near); (*savedDLBufferOffsetPtr)++;
			InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)0; (*savedDLBufferOffsetPtr)++;
			glStoreMatrix(0);
		}
		isCustomDisplayList = false; //End manual GX Display List
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, MTX_FRUSTRUM, (*savedDLBufferOffsetPtr) - argCnt);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
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

//  Frustrum wrapper
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void gluPerspective(float fovy, float aspect, float zNear, float zFar){
	gluPerspectivef32((int)(fovy * LUT_SIZE / 360.0), floattof32(aspect), floattof32(zNear), floattof32(zFar));    
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glMaterialf(int mode, rgb color){
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glSetOutlineColor(int id, rgb color){
	GFX_EDGE_TABLE[id] = color;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glSetToonTable(uint16 *table){
	int i;
	for( i = 0; i < 32; i++ ){
		GFX_TOON_TABLE[i] = table[i];
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glSetToonTableRange(int start, int end, rgb color){
	int i;
	for( i = start; i <= end; i++ ){
		GFX_TOON_TABLE[i] = color;
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
  
	GFX_TEX_FORMAT = 0;
	GFX_POLY_FORMAT = 0;
  
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glBindTexture(int target, int name){
	GFX_TEX_FORMAT = textures[name];
	
	activeTexture = name;
}

// glTexParameter although named the same 
//	as its gl counterpart it is not compatible
//	Effort may be made in the future to make it so.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glColor3f(float red, float green, float blue){
	glColor3b(f32toint(floattof32(red)), f32toint(floattof32(green)), f32toint(floattof32(blue)));
}

//Open GL 1.1 Implementation: Texture Objects support
//glTexImage*() == glTexImage3D
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities){
	//DS 3D GPU does not support texture priority. There may be a way by sorting them by color but
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
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
	char displayListName[256];
	//must be 1:1 from isAGXCommand
	u32 cmd = list[1];
	int cmdParamCount = getAGXParamsCountFromCommand(cmd);
	if (cmd == (u32)getMTX_STORE()) {
		strcpy(displayListName, "getMTX_STORE()");
	}
	else if (cmd == (u32)getMTX_TRANS()) {
		strcpy(displayListName, "getMTX_TRANS()");
	}
	else if (cmd == (u32)getMTX_IDENTITY()) {
		strcpy(displayListName, "getMTX_IDENTITY()");
	}
	else if (cmd == (u32)getMTX_MODE()) {
		strcpy(displayListName, "getMTX_MODE()");
	}
	else if (cmd == (u32)getVIEWPORT()) {
		strcpy(displayListName, "getVIEWPORT()");
	}
	else if (cmd == (u32)getFIFO_TEX_COORD()) {
		strcpy(displayListName, "getFIFO_TEX_COORD()");
	}
	else if (cmd == (u32)getFIFO_BEGIN()) {
		strcpy(displayListName, "getFIFO_BEGIN()");
	}
	else if (cmd == (u32)getFIFO_END()) {
		strcpy(displayListName, "getFIFO_END()");
	}
	else if (cmd == (u32)getFIFO_COLOR()) {
		strcpy(displayListName, "getFIFO_COLOR()");
	}
	else if (cmd == (u32)getFIFO_NORMAL()) {
		strcpy(displayListName, "getFIFO_NORMAL()");
	}
	else if (cmd == (u32)getFIFO_VERTEX16()) {
		strcpy(displayListName, "getFIFO_VERTEX16()");
	}
	else if (cmd == (u32)getFIFO_VERTEX10()) {
		strcpy(displayListName, "getFIFO_VERTEX10()");
	}
	else if (cmd == (u32)getFIFO_VTX_XY()) {
		strcpy(displayListName, "getFIFO_VTX_XY()");
	}
	else if (cmd == (u32)getMTX_PUSH()) {
		strcpy(displayListName, "getMTX_PUSH()");
	}
	else if (cmd == (u32)getMTX_POP()) {
		strcpy(displayListName, "getMTX_POP()");
	}
	else if (cmd == (u32)getMTX_MULT_3x3()) {
		strcpy(displayListName, "getMTX_MULT_3x3()");
	}
	else if (cmd == (u32)getMTX_MULT_4x4()) {
		strcpy(displayListName, "getMTX_MULT_4x4()");
	}

	///////////////Custom Display Lists
	else{
		strcpy(displayListName, "CUSTOM DISPLAY LIST");
	}
	printf("\n(WIN32)glCallListGX: Executing DL[%s]; Size: %d\n",displayListName, (int)list[0]);
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glTexCoord2f(GLfloat s, GLfloat t){
	glTexCoord2t16(floattot16(s), floattot16(t));
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glTexCoord1i(uint32 uv){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_TEX_COORD(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)uv; (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glTexCoord2t16(t16 u, t16 v){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_TEX_COORD(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)(u << 16) + v; (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

//Primitive types
//0  Separate Triangle(s)    ;3*N vertices per N triangles
//1  Separate Quadliteral(s) ;4*N vertices per N quads
//2  Triangle Strips         ;3+(N-1) vertices per N triangles
//3  Quadliteral Strips      ;4+(N-1)*2 vertices per N quads
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glBegin(int primitiveType){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000500h 40h 1  1   BEGIN_VTXS - Start of Vertex List (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_BEGIN(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)primitiveType; (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glEnd( void){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000504h 41h -  1   END_VTXS - End of Vertex List (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_END(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		//no args used by this GX command
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glColor3b(uint8 red, uint8 green, uint8 blue){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
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

		//4000480h 20h 1  1   COLOR - Directly Set Vertex Color (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_COLOR(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)finalColor; (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

//glNormal: Sets the current normal vector
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_NORMAL(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)NORMAL_PACK(floattov10(nx),floattov10(ny),floattov10(nz)); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_NORMAL(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)NORMAL_PACK(inttov10(nx),inttov10(ny),inttov10(nz)); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex3v16(v16 x, v16 y, v16 z){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//400048Ch 23h 2  9   VTX_16 - Set Vertex XYZ Coordinates (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_VERTEX16(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)(y << 16) | (x & 0xFFFF); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)((uint32)(uint16)z); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex3v10(v10 x, v10 y, v10 z){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000490h 24h 1  8   VTX_10 - Set Vertex XYZ Coordinates (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_VERTEX10(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)VERTEX_PACKv10(x, y, z); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
	}
}

//Parameters. x. Specifies the x-coordinate of a vertex. y. Specifies the y-coordinate of a vertex
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex2v16(v16 x, v16 y){
	u32 * savedDLBufferOffsetPtr = (isAnOpenGLExtendedDisplayListCallList == false) ? (u32*)&InternalUnpackedGX_DL_Binary_StandardOGLPtr : (u32*)&InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;
	u32 baseGXDLOffset = (isAnOpenGLExtendedDisplayListCallList == false) ? 0 : InternalUnpackedGX_DL_workSize;
	u32 ptrVal = (*savedDLBufferOffsetPtr);
	if(((int)(ptrVal+1) < (int)(InternalUnpackedGX_DL_workSize)) ){
		//4000494h 25h 1  8   VTX_XY - Set Vertex XY Coordinates (W)
		u8 cmd = InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)getFIFO_VTX_XY(); //Unpacked Command format
		(*savedDLBufferOffsetPtr)++;
		InternalUnpackedGX_DL_Binary[(*savedDLBufferOffsetPtr) + baseGXDLOffset] = (u32)VERTEX_PACK(x, y); (*savedDLBufferOffsetPtr)++; //Unpacked Command format
		handleInmediateGXDisplayList((u32*)&InternalUnpackedGX_DL_Binary[baseGXDLOffset], savedDLBufferOffsetPtr, cmd, (*savedDLBufferOffsetPtr) - ptrVal);
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
bool isCustomDisplayList; //Toggles either a custom DL is built or a SINGLE GX hardware one
bool isAnOpenGLExtendedDisplayListCallList; //Toggles targeting either a single, or custom DL into the Standard OpenGL GX binary pipeline, or the extended OpenGL DisplayList GX binary pipeline.

//OpenGL DL internal Display Lists enumerator: stores multiple DL pointed by current InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr, starting from 0.
GLsizei Compiled_DL_Binary_Descriptor[InternalUnpackedGX_DL_workSize];

u32 LastOpenGLDisplayListStart=0; //enumerates last list allocated by glNewList()

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
u32 * getInternalUnpackedDisplayListBuffer_StandardOGLCurOffset(){
	return (u32 *)&InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_StandardOpenGLStartOffset + InternalUnpackedGX_DL_Binary_StandardOGLPtr];
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
		LastOpenGLDisplayListStart=0; //reset to default list start
		InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr = 1; //OFFSET 0 IS DL SIZE
        return InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr; //starts from range of said length
    }
    return 0;
}

//Specifies the offset that's added to the display-list indices in glCallLists() to obtain the final display-list indices. The default display-list base is 0. The list base has no effect on glCallList(), which executes only one display list or on glNewList().
//Internally, if a new base is set, a pointer to a new section is updated
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
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
	isAnOpenGLExtendedDisplayListCallList = true;
}

//When glEndList is encountered, the display-list definition is completed 
//by associating the list with the unique name list (specified in the glNewList command). 
//If a display list with name list already exists, it is replaced only when glEndList is called.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glEndList(void){
	//If LAST display-list name is GL_COMPILE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL. Such binary will be manually executed when glCallList(display-list name) is called 
	//Else If LAST display-list name is GL_COMPILE_AND_EXECUTE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL and executes it inmediately through GX GLCallList()
	
	//define List Size
	int listSize = ((InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr - LastOpenGLDisplayListStart) * 4) + 4;
	InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_OpenGLDisplayListStartOffset + LastOpenGLDisplayListStart] = (u32)listSize;
	u32 targetGXDLOffset = InternalUnpackedGX_DL_OpenGLDisplayListStartOffset + LastOpenGLDisplayListStart;
	InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr++;
	LastOpenGLDisplayListStart = InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr;	
	if(globalGLCtx.mode == GL_COMPILE_AND_EXECUTE){
		glCallListGX((const u32*)&InternalUnpackedGX_DL_Binary[targetGXDLOffset]); //Using Unpacked Command instead
	}
	isAnOpenGLExtendedDisplayListCallList = false;
}

/*
OpenGL Display List execution which implements the lower level GX hardware Display Lists execution.
params: 
list
Specifies the integer name of the display list to be executed.
*/
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glCallList(GLuint list){
	if(list > 0){
		list--;
	}
	u32 * InternalDL = getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr();
	int curDLInCompiledDLOffset = Compiled_DL_Binary_Descriptor[list];
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
		int singleListSize = *currentPhysicalDisplayListStart;
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
__attribute__((optimize("O0")))
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glDeleteLists(GLuint list, GLsizei range){
	if(list >= 1){
		list--; //assign current InternalUnpackedGX_DL_Binary_OpenGLDisplayListPtr (new) to a List
	}
	if (list >= DL_DESCRIPTOR_MAX_ITEMS){
		return;
	}

	int lowestCurDLInCompiledDLOffset = 0;
	int i = 0;
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
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
GL_GLBEGIN_ENUM getDisplayListGLType(struct ndsDisplayListDescriptor * dlInst){
	if(dlInst != NULL){
		return (GL_GLBEGIN_ENUM)dlInst->DL[1].value;
	}
	return (GL_GLBEGIN_ENUM)DL_INVALID;
}

//Compiles a NDS GX Display List / CallList binary using the Command Packed format, from an object one. Understood by the GX hardware.
//Returns: List count (multiplied by 4 is the file size), DL_INVALID if fails.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int CompilePackedNDSGXDisplayListFromObject(u32 * bufOut, struct ndsDisplayListDescriptor * dlInst){
	if( (dlInst != NULL) && (bufOut != NULL)){
		*(bufOut) = dlInst->ndsDisplayListSize;
		bufOut++;
		int i = 0; 
		for(i = 0; i < dlInst->ndsDisplayListSize; i++){
			struct ndsDisplayList * curDL = &dlInst->DL[i];
			*(bufOut) = curDL->value;
			bufOut++;
		}
		return i;
	}

	return DL_INVALID;
}

//////////////////////////////////////////////////////////// Extended Display List OpenGL 1.x end //////////////////////////////////////////
