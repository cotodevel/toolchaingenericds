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

#include "VideoGLExt.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "VideoGL.h"
#include "ndsDisplayListUtils.h"

#ifdef ARM9
#include <typedefsTGDS.h>
#include "videoTGDS.h"
#include "arm9math.h"
#include "dsregs.h"
#endif

//DL Notes: Are sent using unpacked format.
//Unpacked Command format:
//Opcodes that write more than one 32bit value (ie. STRD and STM) can be used to send ONE UNPACKED command, 
//plus any parameters which belong to that command. 
//After that, there must be a 1 cycle delay before sending the next command 
//(ie. one cannot sent more than one command at once with a single opcode, each command must be invoked by a new opcode).

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//GX hardware object descriptor: Holds current callList GL context from a GL index
//struct ndsDisplayListDescriptor Internal_DL[MAX_Internal_DisplayList_Count];
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
u32 interCompiled_DLPtr=0;	//interCompiled_DLPtr is the base list index surrounding multiple CallLists. 
//GX hardware DL Commands, GL index is above descriptor
u32 Compiled_DL_Binary[DL_MAX_ITEMS*MAX_Internal_DisplayList_Count]; //Packed / Unpacked FIFO Format
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//OpenGL DL internal Display Lists enumerator, holds current DL index pointed by internal interCompiled_DLPtr, starting from 0.
GLsizei GLDLEnumerator[MAX_Internal_DisplayList_Count];

//Gets the internal Display List buffer used by Display Lists Open GL opcodes.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
u32 * getInternalDisplayListBuffer(){
	return (u32 *)&Compiled_DL_Binary[0];
}

//listBase
GLsizei currentListPointer = DL_INVALID;

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void GLInitExt(){
	//memset((char*)&Internal_DL[0], 0, sizeof(Internal_DL));
	globalGLCtx.mode = GL_COMPILE;
	
	/*
	int i = 0;
	for(i = 0; i < MAX_Internal_DisplayList_Count; i++ ){
		Internal_DL[i].DisplayListNameAssigned = DL_INVALID;
		Internal_DL[i].isDisplayListAssigned = false;
        Internal_DL[i].ndsDisplayListSize = DL_MAX_ITEMS;
		int j = 0;
        for(j = 0; j < Internal_DL[i].ndsDisplayListSize; j++){
            Internal_DL[i].DL[j].displayListType = DL_INVALID;
            Internal_DL[i].DL[j].index = j;
            Internal_DL[i].DL[j].value = DL_INVALID;
        }
	}*/
    isNdsDisplayListUtilsCallList = false;
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
    if(range < MAX_Internal_DisplayList_Count){
        int i = 0;
        interCompiled_DLPtr = 1; //GL index 0 is reserved for DL size
		for(i = 0; i < 128; i++ ){
            /*
			Internal_DL[i].DisplayListNameAssigned = DL_INVALID;//glNewList() assigns the index (name of a display list) //=i;
            Internal_DL[i].isDisplayListAssigned = true;
            Internal_DL[i].ndsDisplayListSize = DL_MAX_ITEMS;
			int j = 0;
            for(j = 0; j < Internal_DL[i].ndsDisplayListSize; j++){
                Internal_DL[i].DL[j].displayListType = DL_INVALID;
                Internal_DL[i].DL[j].index = j;
                Internal_DL[i].DL[j].value = DL_INVALID;
            }
			*/
			GLDLEnumerator[i] = DL_INVALID; //GLDLEnumerator[i] = interCompiled_DLPtr; //only mapped from glNewList()
		}
        return interCompiled_DLPtr; //starts from range of said length
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
	interCompiled_DLPtr = interCompiled_DLPtr + base;
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
	if((u32)GLDLEnumerator[list] != (u32)DL_INVALID){
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
		list--; //assign current interCompiled_DLPtr (new) to a List
	}
	GLDLEnumerator[list] = interCompiled_DLPtr;
	globalGLCtx.mode = (u32)mode;
	isNdsDisplayListUtilsCallList = true;
	memset((char*)&Compiled_DL_Binary[interCompiled_DLPtr], 0, DL_MAX_ITEMS - ((interCompiled_DLPtr*4) % DL_MAX_ITEMS) );
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
	int listSize = (interCompiled_DLPtr * 4) + 4;
	Compiled_DL_Binary[0] = (u32)listSize;

	//Close the list, build the list, execute it if needed
	Compiled_DL_Binary[interCompiled_DLPtr] = getFIFO_END();
	interCompiled_DLPtr++;

	//not using Packed Command... int builtDisplayListSize = CompilePackedNDSGXDisplayListFromObject((u32*)&Compiled_DL_Binary[0], &Compiled_DL) + 1;
	if(globalGLCtx.mode == GL_COMPILE_AND_EXECUTE){
		glCallListGX((const u32*)&Compiled_DL_Binary[0]); //Using Unpacked Command instead

		//List already consumed. Reset GX Compiled Display List
		interCompiled_DLPtr = 1;
		memset((char*)&Compiled_DL_Binary[0], 0, sizeof(Compiled_DL_Binary));
	}
	isNdsDisplayListUtilsCallList = false;
}

/*
OpenGL Display List execution which implements the lower level GX hardware Display Lists execution.

params: 
list
Specifies the integer name of the display list to be executed.
*/
#ifdef ARM9
u32 singleOpenGLCompiledDisplayList[2048]; //2048 cmds per a single List should be enough

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glCallList(GLuint list){
	u32 * InternalDL = getInternalDisplayListBuffer();
	int curDLInCompiledDLOffset = GLDLEnumerator[list];
	if((u32)curDLInCompiledDLOffset != DL_INVALID){
		u32 * currentPhysicalDisplayListStart = (u32 *)&InternalDL[curDLInCompiledDLOffset];
		u32 * currentPhysicalDisplayListEnd = NULL;

		int nextDLInCompiledDLOffset = GLDLEnumerator[list+1];
		if(
			((u32)nextDLInCompiledDLOffset != DL_INVALID)
			||
			( (u32)InternalDL[nextDLInCompiledDLOffset] == (u32)getFIFO_END() ) //means no last cmd End
			){
			currentPhysicalDisplayListEnd = (u32 *)&InternalDL[nextDLInCompiledDLOffset];
		}
		//means we are at the last list. Copy everything until getFIFO_END()
		else{
			u32 * currentPhysicalDisplayListEndCopy = currentPhysicalDisplayListStart;
			while( ((u32)*(currentPhysicalDisplayListEndCopy)) != (u32)getFIFO_END() ){
				currentPhysicalDisplayListEndCopy++;
			}
			currentPhysicalDisplayListEnd = currentPhysicalDisplayListEndCopy + 1; //a whole packed command is 4 bytes
			//printf("[List: %d]got end of list: %x", list, currentPhysicalDisplayListEnd);
			//printf("liststart: %x", currentPhysicalDisplayListStart);			
		}
		int singleListSize = (currentPhysicalDisplayListEnd - currentPhysicalDisplayListStart) * 4;
		if(singleListSize > 0){
			//Run a single GX Display List, having proper DL size
			memcpy((u8*)&singleOpenGLCompiledDisplayList[1], currentPhysicalDisplayListStart, singleListSize);
			singleOpenGLCompiledDisplayList[0] = (u32)singleListSize;
			glCallListGX((const u32*)&singleOpenGLCompiledDisplayList[0]);
			printf("glCallList():glCallListGX() List(%d) exec. OK", list);
		}
		else{
			//printf("glCallList():This OpenGL list name(%d)'s InternalDL offset points to InternalDL GX end (no more GX DL after this)", (u32)list);
		}
	}
}

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
				//printf("glCallLists(): trying list: %d", listName);
				glCallList(listName);
			}
		}
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
void glDeleteLists(GLuint list, GLsizei range){
	if(list >= 1){
		list--; //assign current interCompiled_DLPtr (new) to a List
	}
	if (list >= sizeof(GLDLEnumerator)){
		return;
	}

	int lowestCurDLInCompiledDLOffset = -1;
	int i = 0;
	for(i = 0; i < range; i++){
		u32 * InternalDL = getInternalDisplayListBuffer();
		int curDLInCompiledDLOffset = GLDLEnumerator[list + i];
		if((u32)curDLInCompiledDLOffset != DL_INVALID){
			u32 * currentPhysicalDisplayListStart = (u32 *)&InternalDL[curDLInCompiledDLOffset];
			GLDLEnumerator[list + i] = DL_INVALID;

			if(lowestCurDLInCompiledDLOffset < curDLInCompiledDLOffset){
				lowestCurDLInCompiledDLOffset = curDLInCompiledDLOffset;
			}
		}
	}
	globalGLCtx.mode = (u32)GL_NONE;
	isNdsDisplayListUtilsCallList = false;
	
	//Find the lowest internal buffer offset assigned, just deleted, and rewind it so it points to unallocated Internal DL memory
	if(lowestCurDLInCompiledDLOffset != -1){
		interCompiled_DLPtr = lowestCurDLInCompiledDLOffset;
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
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_TEX_COORD(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)uv; interCompiled_DLPtr++; //Unpacked Command format
	}
	else{
		GFX_TEX_COORD = uv;
	}
}

//////////////////////////////////////////////////////////////////////
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glTexCoord2t16(t16 u, t16 v){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_TEX_COORD(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)TEXTURE_PACK(u, v); interCompiled_DLPtr++; //Unpacked Command format
	}
	else{
		GFX_TEX_COORD = (u << 16) + v;
	}
}
//////////////////////////////////////////////////////////////////////

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
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//4000500h 40h 1  1   BEGIN_VTXS - Start of Vertex List (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_BEGIN(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)primitiveType; interCompiled_DLPtr++; //Unpacked Command format
	}
	else{
		GFX_BEGIN = (u32)primitiveType;
	}
}

//////////////////////////////////////////////////////////////////////
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glEnd( void){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//4000504h 41h -  1   END_VTXS - End of Vertex List (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_END(); //Unpacked Command format
		interCompiled_DLPtr++;
		//no args used by this GX command
	}
	else{
		GFX_END = 0;
	}
}

//////////////////////////////////////////////////////////////////////

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
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//4000480h 20h 1  1   COLOR - Directly Set Vertex Color (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_COLOR(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)RGB15(red, green, blue); interCompiled_DLPtr++; //Unpacked Command format
	}
	else{
		switch(globalGLCtx.primitiveShadeModelMode){
			//light vectors are todo
			case(GL_FLAT):{
				//otherwise override all colors to be the same subset of whatever color was passed here
				if(lastVertexColor == 0){
					lastVertexColor = RGB15(red, green, blue);
				}
				GFX_COLOR = lastVertexColor;
			}
			break;
			
			case(GL_SMOOTH):{
				//Smooth shading, the default by DS, causes the computed colors of vertices to be interpolated as the primitive is rasterized, 
				//typically assigning different colors to each resulting pixel fragment. 
				GFX_COLOR = (vuint32)RGB15(red, green, blue);			
			}
			break;
			
			default:{
				//error! call glInit(); first
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3b(const GLbyte v){
	glNormal3i((const GLint )v);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3d(const GLdouble v){
	glNormal3i((const GLint )v);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3f(const GLfloat v){
	glNormal3i((const GLint )v);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3s(const GLshort v){
	glNormal3i((const GLint )v);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glNormal3i(const GLint v){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		double intpart;
		double fractpart = modf((double)v, &intpart);
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_NORMAL(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)floattov10(fractpart); interCompiled_DLPtr++; //Unpacked Command format
	}
}

//////////////////////////////////////////////////////////////////////
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex3v16(v16 x, v16 y, v16 z){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//400048Ch 23h 2  9   VTX_16 - Set Vertex XYZ Coordinates (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_VERTEX16(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)VERTEX_PACK(x, y); interCompiled_DLPtr++; //Unpacked Command format
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)VERTEX_PACK(z, 0); interCompiled_DLPtr++; //Unpacked Command format
	}
	else{
		GFX_VERTEX16 = (y << 16) | (x & 0xFFFF);
		GFX_VERTEX16 = ((uint32)(uint16)z);
	}
}

//////////////////////////////////////////////////////////////////////
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void glVertex3v10(v10 x, v10 y, v10 z){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//4000490h 24h 1  8   VTX_10 - Set Vertex XYZ Coordinates (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_VERTEX10(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)VERTEX_PACKv10(x, y, z); interCompiled_DLPtr++; //Unpacked Command format
	}
	else{
		GFX_VERTEX16 = (y << 16) | (x & 0xFFFF);
		GFX_VERTEX16 = ((uint32)(uint16)z);
	}
}

//////////////////////////////////////////////////////////////////////
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
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		//4000494h 25h 1  8   VTX_XY - Set Vertex XY Coordinates (W)
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_VTX_XY(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)VERTEX_PACK(x, y); interCompiled_DLPtr++; //Unpacked Command format
	}
	else{
		GFX_VERTEX16 = (y << 16) | (x & 0xFFFF);
	}
}

//////////////////////////////////////////////////////////////////////
//Todo
//4000498h 26h 1  8   VTX_XZ - Set Vertex XZ Coordinates (W)
//400049Ch 27h 1  8   VTX_YZ - Set Vertex YZ Coordinates (W)
//40004A0h 28h 1  8   VTX_DIFF - Set Relative Vertex Coordinates (W)
