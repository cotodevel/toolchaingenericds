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
u32 * getInternalDisplayListBuffer(){
	return (u32 *)&Compiled_DL_Binary[0];
}

//listBase
GLsizei currentListPointer = DL_INVALID;
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
GLuint glGenLists(GLsizei	range){
    if(range < MAX_Internal_DisplayList_Count){
        int i = 0;
        interCompiled_DLPtr = 1; //GL index 0 is reserved for DL size
		for(i = 0; i < (range-1); i++ ){
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
void glListBase(GLuint base){
	if(base > 0){
		base--;
	}
	interCompiled_DLPtr = interCompiled_DLPtr + base;
}

//glIsList returns GL_TRUE if list is the name of a display list and returns GL_FALSE if it is not, or if an error occurs.
//A name returned by glGenLists, but not yet associated with a display list by calling glNewList, is not the name of a display list.
GLboolean glIsList(GLuint list){
	if(list > 0){
		list--;
	}
	if(GLDLEnumerator[list] != DL_INVALID){
		return GL_TRUE;
	}

	return GL_FALSE;
	//Todo: GL_INVALID_OPERATION;
}

//list:Specifies the display-list name.
//mode:Specifies the compilation mode, which can be GL_COMPILE or GL_COMPILE_AND_EXECUTE.
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


//verbs implemented so far: glNormal3f
//verbs missing: glTexCoord2f, glEnd, glEndList
void glNormal3b(const GLbyte v){
	glNormal3i((const GLint )v);
}
 
void glNormal3d(const GLdouble v){
	glNormal3i((const GLint )v);
}
 
void glNormal3f(const GLfloat v){
	glNormal3i((const GLint )v);
}

void glNormal3s(const GLshort v){
	glNormal3i((const GLint )v);
}

void glNormal3i(const GLint v){
	if((isNdsDisplayListUtilsCallList == true) && ((int)(interCompiled_DLPtr+1) < (int)(DL_MAX_ITEMS*MAX_Internal_DisplayList_Count)) ){
		double intpart;
		double fractpart = modf((double)v, &intpart);
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)getFIFO_NORMAL(); //Unpacked Command format
		interCompiled_DLPtr++;
		Compiled_DL_Binary[interCompiled_DLPtr] = (u32)floattov10(fractpart); interCompiled_DLPtr++; //Unpacked Command format
	}
}

//Todo: Implement special Test Case (below) which also happens to be the same as https://community.khronos.org/t/glcalllist-not-working/58999
/*
GLuint list;
void CreateLists(){
    list = glGenLists (2);//Generate two display lists,
    if(list){
        glNewList (list, GL_COMPILE);//Create the first display list
        for (int i = 0; i <10; i ++){ //Draw 10 cubes
            glPushMatrix();
            glRotatef(36*i,0.0,0.0,1.0);
            glTranslatef(10.0,0.0,0.0);
            DrawCube();
            glPopMatrix(1);
        }
        glEndList ();//The first display list is created

        glNewList (list + 1, GL_COMPILE);//Create a second display list
        for (int i = 0; i <20; i ++)//Draw 20 triangles{
            glPushMatrix() ;
            glRotatef(18*i,0.0,0.0,1.0) ;
            glTranslatef(15.0,0.0,0.0) ;
            DrawPyramid ();//DrawPyramid () was introduced in the previous chapter
            glPopMatrix(1);
        }
        glEndList ();//The second display list is created
    }
}
*/