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

//DL object workspace
struct ndsDisplayListDescriptor Internal_DL[MAX_Internal_DisplayList_Count];

//DL compiled workspace
struct ndsDisplayListDescriptor Compiled_DL;
u8 Compiled_DL_Binary[MAX_Internal_DisplayList_Count*4];

	//Current ndsDisplayList offset used by dynamic DL builder
	struct ndsDisplayList * curDLinternalCompiledDL;
	//Get Binary filesize
	//int displayListSize = getRawFileSizefromNDSGXDisplayListObject(NDSDL);

//listBaseOffset is the base list index surrounding multiple CallLists. 
GLsizei listBaseOffset;

//Indices in the indiceOffset array are counter by considering that listBaseOffset is 0.
GLsizei indiceOffset;

//listBase
GLsizei currentListPointer = DL_INVALID;
void GLInitExt(){
	memset((char*)&Internal_DL[0], 0, sizeof(Internal_DL));
	int i = 0;
	for(i = 0; i < MAX_Internal_DisplayList_Count; i++ ){
		Internal_DL[i].DisplayListNameAssigned = DL_INVALID;
		Internal_DL[i].isDisplayListAssigned = false;
        Internal_DL[i].ndsDisplayListSize = DL_MAX_ITEMS;
		Internal_DL[i].mode = DL_INVALID;
        int j = 0;
        for(j = 0; j < Internal_DL[i].ndsDisplayListSize; j++){
            Internal_DL[i].DL[j].displayListType = DL_INVALID;
            Internal_DL[i].DL[j].index = j;
            Internal_DL[i].DL[j].value = DL_INVALID;
        }
	}
    listBaseOffset = 0;
	isNdsDisplayListUtilsCallList = false;
	curVec_packed = (GLint*)&vec_packed[0]; 
}

//glGenLists returns the first list name in a range of the length you pass to glGenLists.
GLuint glGenLists(GLsizei	range){
    if((listBaseOffset + range - 1) < range){
        int firstItem = listBaseOffset + 1; //starts from range of said length
        int i = 0;
        for(i = 0; i < (range-1); i++ ){
            Internal_DL[i].DisplayListNameAssigned = DL_INVALID;//glNewList() assigns the index (name of a display list) //=i;
            Internal_DL[i].isDisplayListAssigned = true;
            Internal_DL[i].ndsDisplayListSize = DL_MAX_ITEMS;
			Internal_DL[i].mode = DL_INVALID;
            int j = 0;
            for(j = 0; j < Internal_DL[i].ndsDisplayListSize; j++){
                Internal_DL[i].DL[j].displayListType = DL_INVALID;
                Internal_DL[i].DL[j].index = j;
                Internal_DL[i].DL[j].value = DL_INVALID;
            }
        }

        listBaseOffset=(firstItem-1);
        return firstItem;
    }
    return 0;
}

//Specifies the offset that's added to the display-list indices in glCallLists() to obtain the final display-list indices. The default display-list base is 0. The list base has no effect on glCallList(), which executes only one display list or on glNewList().
void glListBase(GLuint base){
	listBaseOffset=base;
	if(listBaseOffset>=1){
		listBaseOffset--;
	}
}

//glIsList returns GL_TRUE if list is the name of a display list and returns GL_FALSE if it is not, or if an error occurs.
//A name returned by glGenLists, but not yet associated with a display list by calling glNewList, is not the name of a display list.
GLboolean glIsList(GLuint list){
	if(list >= 1){
		list--;
	}
	if((Internal_DL[list].isDisplayListAssigned == true) && (Internal_DL[list].DisplayListNameAssigned != DL_INVALID)){
		return GL_TRUE;
	}
	return GL_FALSE;
	//Todo: GL_INVALID_OPERATION;
}

//list:Specifies the display-list name.
//mode:Specifies the compilation mode, which can be GL_COMPILE or GL_COMPILE_AND_EXECUTE.
void glNewList(GLuint list, GLenum mode){
	if(list >= 1){
		list--;
	}

	listBaseOffset = listBaseOffset + list;
	Internal_DL[listBaseOffset].mode = (u32)mode;
	Internal_DL[listBaseOffset].DisplayListNameAssigned = (listBaseOffset + 1);
	isNdsDisplayListUtilsCallList = true;

	curDLinternalCompiledDL = &Compiled_DL.DL[0];

	//Prepare for upcoming FIFO_PACK_COMMAND
	curDLinternalCompiledDL++;
}

//When glEndList is encountered, the display-list definition is completed 
//by associating the list with the unique name list (specified in the glNewList command). 
//If a display list with name list already exists, it is replaced only when glEndList is called.
void glEndList(void){
	//Todo: 
		//If LAST display-list name is GL_COMPILE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL. Such binary will be manually executed when glCallList(display-list name) is called 
		//Else If LAST display-list name is GL_COMPILE_AND_EXECUTE: actually builds ALL the Display-list generated through the LAST display-list name generated from glNewList(), then compiles it into a GX binary DL and executes it inmediately through GX GLCallList()
	
	//define List Size
	int listSize = ((curDLinternalCompiledDL->index * 4) + 4);
	Compiled_DL.DL[0].index = 0;
	Compiled_DL.DL[0].value = (u32)listSize;

	//Close the list, build the list, execute it if needed
	curDLinternalCompiledDL->value = FIFO_COMMAND_PACK_C(getFIFO_END(), getFIFO_NOP(), getFIFO_NOP(), getFIFO_NOP());
	int builtDisplayListSize = CompileNDSGXDisplayListFromObject((u32*)&Compiled_DL_Binary[0], &Compiled_DL) + 1;
	if(Internal_DL[listBaseOffset].mode == GL_COMPILE_AND_EXECUTE){
		glCallListGX((const u32*)&Compiled_DL_Binary[0]);
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

GLint vec_packed[3]; //packed FIFO_NORMAL
GLint * curVec_packed; //Offset

void glNormal3i(const GLint v){
	*curVec_packed = v;

	if((GLint *)curVec_packed != (GLint *)&vec_packed[2]){
		curVec_packed++;
	}
	else{
		curVec_packed = (GLint *)&vec_packed[0];
		if((isNdsDisplayListUtilsCallList == true) && (curDLinternalCompiledDL != NULL)){
			double intpart;
			double fractpart0 = modf((double)curVec_packed[0], &intpart);
			double fractpart1 = modf((double)curVec_packed[1], &intpart);
			double fractpart2 = modf((double)curVec_packed[2], &intpart);
			curDLinternalCompiledDL->displayListType = getFIFO_NORMAL();
			curDLinternalCompiledDL->value = NORMAL_PACK(floattov10(fractpart0),floattov10(fractpart1),floattov10(fractpart2));
		}
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