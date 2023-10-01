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

//ndsDisplayListUtils. src: https://bitbucket.org/Coto88/ndsdisplaylistutils/src
//Changelog:
//0.4: Implementation finished.
//0.3: Standardized format so it's conformant with Open GL Display Lists: (https://bitbucket.org/Coto88/toolchaingenericds-displaylist/src)
//0.2: Ported to NintendoDS, unit test implemented at: https://bitbucket.org/Coto88/toolchaingenericds-unittest/src (5th Test Case)
//0.1: First release, WIN32 only.

#ifdef WIN32
#include "winDir.h"
#include "stdafx.h"
#include "TGDSTypes.h"
#include <assert.h>
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ndsDisplayListUtils.h"

#if !defined(TGDSPROJECT_WIN32) || defined(DIRECT_VS2012_NDSDL_EXEC)

#ifdef ARM9
#include "videoGL.h"
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u32 FIFO_COMMAND_PACK_C(u8 c1, u8 c2, u8 c3, u8 c4) {
	return (u32)(((c4) << 24) | ((c3) << 16) | ((c2) << 8) | (c1));
}

/////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
 
// Function to swap two numbers
void swap1(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}
 
// Function to reverse `buffer[i�j]`
char* reverse1(char *buffer, int i, int j)
{
    while (i < j) {
        swap1(&buffer[i++], &buffer[j--]);
    }
 
    return buffer;
}
 
// Iterative function to implement `itoa()` function in C
char* itoa1(int value, char* buffer, int base)
{
	// consider the absolute value of the number
    int n = abs(value);
    int i = 0;
    // invalid input
    if (base < 2 || base > 32) {
        return buffer;
    }
    while (n)
    {
        int r = n % base;
 
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }
 
        n = n / base;
    }
 
    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }
 
    // If the base is 10 and the value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }
 
    buffer[i] = '\0'; // null terminate string
 
    // reverse the string and return it
    return reverse1(buffer, 0, i - 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

//Defines if it's a GX command or not
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
bool isAGXCommand(u32 val){
	bool isAGXCommand = false;
	switch(val){
		case((u32)getMTX_STORE):
		case((u32)getMTX_TRANS):
		case((u32)getMTX_IDENTITY):
		case((u32)getVIEWPORT):
		case((u32)getFIFO_TEX_COORD):
		case((u32)getFIFO_BEGIN):
		case((u32)getFIFO_END):
		case((u32)getFIFO_COLOR):
		case((u32)getFIFO_NORMAL):
		case((u32)getFIFO_VERTEX16):
		case((u32)getFIFO_VERTEX10):
		case((u32)getFIFO_VTX_XY):
		case((u32)getMTX_PUSH):
		case((u32)getMTX_POP):
		case((u32)getMTX_MULT_3x3):
		case((u32)getMTX_MULT_4x4):
		case((u32)getMTX_LOAD_4x4):
		case((u32)getMTX_LOAD_4x3):
		case((u32)getNOP):
		{
			isAGXCommand = true;
		}
		break;
	}
	return isAGXCommand;
}

//returns: -1 if ain't a command, or command count
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
int getAGXParamsCountFromCommand(u32 val){
	//same as above method
	int AGXParamsCount = -1;
	switch(val){
			case((u32)getMTX_STORE):{
				//400044Ch 13h 1  17  MTX_STORE - Store Current Matrix on Stack (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getMTX_TRANS):{
				//  4000470h 1Ch 3  22* MTX_TRANS - Mult. Curr. Matrix by Translation Matrix (W)
				AGXParamsCount = 3;
			}break;
			case((u32)getMTX_IDENTITY):{
				//4000454h 15h - 19  MTX_IDENTITY - Load Unit Matrix to Current Matrix(W)
				//no args used by this GX command
				AGXParamsCount = 0;
			}break;
			case((u32)getMTX_MODE):{
				//4000440h 10h 1  1   MTX_MODE - Set Matrix Mode (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getVIEWPORT):{
				//4000580h 60h 1  1   VIEWPORT - Set Viewport (W)
				AGXParamsCount = 1;					   
			}break;
			case((u32)getFIFO_TEX_COORD):{
				//4000488h 22h 1  1   TEXCOORD - Set Texture Coordinates (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getFIFO_BEGIN):{
				//4000500h 40h 1  1   BEGIN_VTXS - Start of Vertex List (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getFIFO_END):{
				//4000504h 41h -  1   END_VTXS - End of Vertex List (W)
				//no args used by this GX command
				AGXParamsCount = 0;
			}break;
			case((u32)getFIFO_COLOR):{
				//4000480h 20h 1  1   COLOR - Directly Set Vertex Color (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getFIFO_NORMAL):{
				//4000484h 21h 1  9*  NORMAL - Set Normal Vector (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getFIFO_VERTEX16):{
				//400048Ch 23h 2  9   VTX_16 - Set Vertex XYZ Coordinates (W)
				AGXParamsCount = 2;
			}break;
			case((u32)getFIFO_VERTEX10):{
				//4000490h 24h 1  8   VTX_10 - Set Vertex XYZ Coordinates (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getFIFO_VTX_XY):{
				//4000494h 25h 1  8   VTX_XY - Set Vertex XY Coordinates (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getMTX_PUSH):{
				//4000444h 11h -  17  MTX_PUSH - Push Current Matrix on Stack (W)
				//no args used by this GX command
				AGXParamsCount = 0;
			}break;
			case((u32)getMTX_POP):{
				//4000448h 12h 1  36  MTX_POP - Pop Current Matrix from Stack (W)
				AGXParamsCount = 1;
			}break;
			case((u32)getMTX_MULT_3x3):{
				//  4000468h 1Ah 9  28* MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix (W)
				AGXParamsCount = 9;
			}break;
			case((u32)getMTX_MULT_4x4):{
				//  4000460h 18h 16 35* MTX_MULT_4x4 - Multiply Current Matrix by 4x4 Matrix (W)
				AGXParamsCount = 16;
			}break;
			case((u32)getMTX_LOAD_4x4):{
				//  4000458h 16h 16 34  MTX_LOAD_4x4 - Load 4x4 Matrix to Current Matrix (W)
				AGXParamsCount = 16;
			}break;
			case((u32)getMTX_LOAD_4x3):{
				//  400045Ch 17h 12 30  MTX_LOAD_4x3 - Load 4x3 Matrix to Current Matrix (W
				AGXParamsCount = 12;
			}break;
			case((u32)getNOP):{
				//  N/A      00h -  -   NOP - No Operation (for padding packed GXFIFO commands)
				AGXParamsCount = 0;
			}break;
	}
	return AGXParamsCount;
}

//counts leading zeroes :)
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
u8 clzero(u32 var){   
    u8 cnt=0;
    u32 var3;
    if (var>0xffffffff) return 0;
   
    var3=var; //copy
    var=0xFFFFFFFF-var;
    while((var>>cnt)&1){
        cnt++;
    }
    if ( (((var3&0xf0000000)>>28) >0x7) && (((var3&0xff000000)>>24)<0xf)){
        var=((var3&0xf0000000)>>28);
        var-=8; //bit 31 can't count to zero up to this point
            while(var&1) {
                cnt++; var=var>>1;
            }
    }
	return cnt;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void init_crc_table (void *table, unsigned int polynomial){ // works for crc16 and crc32
  unsigned int crc, i, j;

  for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
        if (crc & 1)
          crc = (crc >> 1) ^ polynomial;
        else
          crc >>= 1;

      if (polynomial == CRC32_POLYNOMIAL)
        ((unsigned int *) table)[i] = crc;
      else
        ((unsigned short *) table)[i] = (unsigned short) crc;
    }
}

unsigned int *crc32_table = NULL;

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
void free_crc32_table (void){
	TGDSARM9Free(crc32_table);
	crc32_table = NULL;
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
unsigned int crc32 (unsigned int *crc, const void *buffer, unsigned int size){
	unsigned char *p = (unsigned char *)buffer;
	if (!crc32_table){
		crc32_table = (unsigned int *) TGDSARM9Malloc(256 * 4);
		init_crc_table (crc32_table, CRC32_POLYNOMIAL);
	}
	*crc = ~(*crc);
	while (size--){
		*crc = ((*crc) >> 8) ^ crc32_table[((*crc) ^ *p++) & 0xff];
	}
	free_crc32_table();
	return ~(*crc);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int crc32file( FILE *file, unsigned int *outCrc32){
	#define CRC_BUFFER_SIZE  (int)(1024*64)
    char * buf = (char *)TGDSARM9Malloc(CRC_BUFFER_SIZE);
	size_t bufLen;
    /** accumulate crc32 from file **/
    *outCrc32 = 0;
    while (1) {
        bufLen = fread(buf, 1, CRC_BUFFER_SIZE, file);
        if (bufLen == 0) {
            //if (ferror(file)) {
            //    fprintf( stderr, "error reading file\n" );
            //    goto ERR_EXIT;
            //}
            break;
        }
        *outCrc32 = crc32(outCrc32, buf, bufLen );
    }
	TGDSARM9Free(buf);
    return( 0 );
}

#if defined(_MSC_VER) && !defined(ARM9) //VS2012?


int main(int argc, char** argv){
	float rotateX = 0.0;
	float rotateY = 0.0;
	//DS can just call glInit(); a lot of times
	{
		int TGDSOpenGLDisplayListWorkBufferSize = (256*1024);
		#ifdef WIN32
		InitGLOnlyOnce = false;
		glInit(TGDSOpenGLDisplayListWorkBufferSize);
		#endif

		//set mode 0, enable BG0 and set it to 3D
		#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
		SETDISPCNT_MAIN(MODE_0_3D);
		#endif
		//this should work the same as the normal gl call
		glViewport(0,0,255,191);
		
		glClearColor(0,0,0);
		glClearDepth(0x7FFF);
		
	}

	//NintendoDS: Triangle demo using standard OpenGL 1.0 calls
	/*
	ReSizeGLScene(255, 191);  
	InitGL();	

	while (1){
		glReset();
	
		//any floating point gl call is being converted to fixed prior to being implemented
		gluPerspective(35, 256.0 / 192.0, 0.1, 40);

		gluLookAt(	0.0, 0.0, 1.0,		//camera possition 
					0.0, 0.0, 0.0,		//look at
					0.0, 1.0, 0.0);		//up

		glPushMatrix();

		//move it away from the camera
		glTranslate3f32(0, 0, floattof32(-1));
					
		glRotateX(rotateX);
		glRotateY(rotateY);			
		glMatrixMode(GL_MODELVIEW);
			
		//not a real gl function and will likely change
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
			
		//glShadeModel(GL_FLAT); //forces the fill color to be the first glColor3b call
			
		//draw the obj
		glBegin(GL_TRIANGLE);
				
			glColor3b(31,0,0);
			glVertex3v16(inttov16(-1),inttov16(-1),0);

			glColor3b(0,31,0);
			glVertex3v16(inttov16(1), inttov16(-1), 0);

			glColor3b(0,0,31);
			glVertex3v16(inttov16(0), inttov16(1), 0);
				
		glEnd();
		glPopMatrix(1);
		glFlush();
	}
	*/

	//Unit Test #0: Multiple spawn glGenLists();
	{
		int list = 0;
		int first = -1;
		int * dlist = &list;
		int i = 0;
		struct TGDSOGL_DisplayListContext * Inst = NULL;
		int first256unusedDLs = 0;
		first256unusedDLs = glGenLists(256);
		*dlist = first = glGenLists(1);

		for(i = 0; i < 7; i++){
			*dlist = glGenLists(1);
			if (!glIsList(*dlist)) {
				printf("TGDS ERROR: glIsList() FALSE!: List (%d)", *dlist);
				while(1==1){}
				return;
			}
			glNewList(*dlist, GL_COMPILE);
			glBegin(GL_TRIANGLES);
			glPushMatrix();
			
			glPopMatrix(1);
			glEnd();
			glEndList();
			list++;
		}

		for(i = 0; i < 10; i++){
			if(i == 6){
				int debug = 0;
				debug = debug;
			}
			glCallList(i+1);
		}
		

		
		glNewList(first, GL_COMPILE_AND_EXECUTE);
		glBegin(GL_TRIANGLES);
		glPushMatrix();
			
		glPopMatrix(1);
		glEnd();
		glEndList();

		//Inst->CurrentSpawnOGLDisplayList+1
	}
	

	//Unit Test #1: Tests OpenGL DisplayLists components functionality then emitting proper GX displaylists, unpacked format.
	{
		int list = glGenLists(10);
		if(list){
			int i = 0;
			bool ret = 0;
			glListBase(list);
			ret = glIsList(list); //should return false (DL generated, but no displaylist-name was generated)
			glNewList(list, GL_COMPILE);
			ret = glIsList(list); //should return true (DL generated, and displaylist-name was generated)
			if(ret == true){
				glBegin(GL_TRIANGLES);
				for (i = 0; i <10; i ++){ //Draw 10 cubes
					glPushMatrix();
					glRotatef(36*i,0.0,0.0,1.0);
					glTranslatef(10.0,0.0,0.0);
					glPopMatrix(1);
				}
				glEnd();
			}
			glEndList(); 
		
			glListBase(list + 1);
			glNewList (list + 1, GL_COMPILE);//Create a second display list and execute it
			ret = glIsList(list + 1); //should return true (DL generated, and displaylist-name was generated)
			if(ret == true){
				glBegin(GL_TRIANGLES);
				for (i = 0; i <20; i ++){ //Draw 20 triangles
					glPushMatrix();
					glRotatef(18*i,0.0,0.0,1.0);
					glTranslatef(15.0,0.0,0.0);
					glPopMatrix(1);
				}
				glEnd();
			}
			glEndList();//The second display list is created
		}
		glCallList(list);
		
		glCallList(list + 1);

		//delete
		glDeleteLists(list, 2); //remove 2 of them
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Sources: https://songho.ca/opengl/gl_vbo.html
	
	//NintendoDS: VBO Test #1: Multiple vertex arrays: https://io7m.com/documents/history-vertex-spec/#st200_s4fo3
	{
		glReset();
	
		//any floating point gl call is being converted to fixed prior to being implemented
		gluPerspective(35, 256.0 / 192.0, 0.1, 40);

		gluLookAt(	0.0, 0.0, 1.0,		//camera possition 
					0.0, 0.0, 0.0,		//look at
					0.0, 1.0, 0.0		//up
		);		

		glPushMatrix();

		//move it away from the camera
		glTranslate3f32(0, 0, floattof32(-1));
				
		glRotateX(rotateX);
		glRotateY(rotateY);
		glMatrixMode(GL_MODELVIEW);
		
		updateGXLights(); //Update GX 3D light scene!
		
		
		{
			float vertices[9] = {
				 -1.0f, -1.0f,  0.0f ,
				 1.0f,  -1.0f,  0.0f ,
				 0.0f,   1.0f,  0.0f 
			};
			float colors[9] = {
				 31.0f, 0.0f, 0.0f ,
				 0.0f, 31.0f, 0.0f ,
				 0.0f, 0.0f, 31.0f 
			};

			int vtxSize = (sizeof(vertices)/sizeof(GLfloat));
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			system("cls");

			glVertexPointer(3, GL_FLOAT, 0, vertices);
			glColorPointer(3, GL_FLOAT, 0, colors);
			glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should build

			glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should be called from DL

			vertices[0] = 0.9f;
			glVertexPointer(3, GL_FLOAT, 0, vertices);
			glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should build

			glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should be called from DL

			vertices[0] = -1.0f;
			glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should be called from DL

			//now refill the cached DLists and overwrite a new one

			{
				int j = 0;
				for(j = 0; j < (VBO_CACHED_PREBUILT_DL_SIZE-2); j++){
					vertices[0] = (float)j + 2.0f;
					glVertexPointer(3, GL_FLOAT, 0, vertices);

					if(j == 37){
						j = j;
					}

					glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should build
				}

				vertices[0] = (float)0 + 2.0f;
				glVertexPointer(3, GL_FLOAT, 0, vertices);
				glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should be called from DL

				vertices[0] = ((float)100.9) + 2.9f;
				glVertexPointer(3, GL_FLOAT, 0, vertices);
				glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should build

				glVertexPointer(3, GL_FLOAT, 0, vertices);
				glDrawArrays(GL_TRIANGLES, 0, vtxSize); //should be called from DL
			}

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		}

		glFlush(); //DS: Update 3D buffer onto screen
	}

	//VBO Test #2: Creating a single VBO for vertex coordinates + upload to VBO + drawing VBOs (Unsupported by NDS hardware)
	{
		GLuint vboId1 = 1; // ID of VBO: vertex
		//GLuint vboId2 = 2; // ID of VBO: indices //todo

		const GLvoid * offset1 = (const GLvoid *)0; // vertex array offset start
		const GLvoid * offset2 = (const GLvoid *)0; // normal array offset start
		const GLvoid * offset3 = (const GLvoid *)0; // texture coordinate array offset start

		GLsizei stride = 0; //array is packed (not aligned, natural size)
		int vCount = 12; //4 glVertex + 4 glNormal + 4 glTexCoord calls crammed into 1 vertex array
		int dataSize = sizeof(GLfloat)*vCount*3;
		GLfloat* vertices = (GLfloat*)malloc(dataSize); // create vertex array
		
		//fill these vertices using the array format!
		vertices[0] = (GLfloat)(1.0f);
		vertices[1] = (GLfloat)(-1.0f);
		vertices[2] = (GLfloat)(-1.0f);

		vertices[3] = (GLfloat)(-4.3f);
		vertices[4] = (GLfloat)(-4.3f);
		vertices[5] = (GLfloat)(-4.3f);

		vertices[6] = (GLfloat)(1.0f);
		vertices[7] = (GLfloat)(-1.0f);
		vertices[8] = (GLfloat)(0.0f);

		vertices[9] = (GLfloat)(1.0f);
		vertices[10] = (GLfloat)(-1.0f);
		vertices[11] = (GLfloat)(-1.0f);

		vertices[12] = (GLfloat)(-4.3f);
		vertices[13] = (GLfloat)(-4.3f);
		vertices[14] = (GLfloat)(-4.3f);

		vertices[15] = (GLfloat)(1.0f);
		vertices[16] = (GLfloat)(-1.0f);
		vertices[17] = (GLfloat)(0.0f);

		vertices[18] = (GLfloat)(1.0f);
		vertices[19] = (GLfloat)(-1.0f);
		vertices[20] = (GLfloat)(-1.0f);

		vertices[21] = (GLfloat)(-4.3f);
		vertices[22] = (GLfloat)(-4.3f);
		vertices[23] = (GLfloat)(-4.3f);

		vertices[24] = (GLfloat)(1.0f);
		vertices[25] = (GLfloat)(-1.0f);
		vertices[26] = (GLfloat)(0.0f);

		vertices[27] = (GLfloat)(1.0f);
		vertices[28] = (GLfloat)(-1.0f);
		vertices[29] = (GLfloat)(-1.0f);

		vertices[30] = (GLfloat)(-4.3f);
		vertices[31] = (GLfloat)(-4.3f);
		vertices[32] = (GLfloat)(-4.3f);

		vertices[33] = (GLfloat)(1.0f);
		vertices[34] = (GLfloat)(-1.0f);
		vertices[35] = (GLfloat)(0.0f);

		// generate a new VBO and get the associated ID
		glGenBuffers(1, &vboId1);

		// bind VBO in order to use vertex array and index array
		glBindBuffer(GL_ARRAY_BUFFER, vboId1); // for vertex attributes
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId2);    // for indices //todo

		// upload data to VBO
		glBufferData(GL_ARRAY_BUFFER, dataSize, vertices, GL_STATIC_DRAW);

		//vertex data uploaded; It's safe to delete after copying data to VBO
		free(vertices);
		
		////part 2: draw
		glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex position array
		glEnableClientState(GL_NORMAL_ARRAY);             // activate vertex normal array
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);      // activate texture coord array

		glVertexPointer(3, GL_FLOAT, stride, offset1);   
		glNormalPointer(GL_FLOAT, stride, offset2);
		glTexCoordPointer(2, GL_FLOAT, stride, offset3);

		// draw 6 faces using offset of index array
		glDrawElements(GL_TRIANGLES, (dataSize/sizeof(GLfloat)), GL_UNSIGNED_BYTE, 0);

		glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex position array
		glDisableClientState(GL_NORMAL_ARRAY);            // deactivate vertex normal array
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);     // deactivate vertex tex coord array

		// bind with 0, so, switch back to normal pointer operation
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		// delete VBO when program terminated
		glDeleteBuffers(1, &vboId1);
	}

	//misc tests
	{
		GLfloat mat[16] = {1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f};
		glLoadMatrixf((const GLfloat *)&mat);
	}
	
	{
		GLint shadeNow = 0;
		GLfloat vec[2];
		vec[0] = 0.1;
		vec[1] = 0.2;
		glTexCoord2fv((const GLfloat *)&vec);

		glGetIntegerv(GL_SHADE_MODEL, &shadeNow);

		/////////////////////////////////////// Unit Test #5: Multiple glGenLists(); ///////////////////////////////////////
		{
			GLuint index = glGenLists(5);
			GLubyte lists[5];
			int i = 0;
			int DLCount = 0;
			
			//Compile display lists: 0
			for(DLCount = 0; DLCount < 5; DLCount++){
				glNewList(index + DLCount, GL_COMPILE);   // compile each one

				for (i = 0; i <20; i ++){ //Draw triangles
					glPushMatrix();
					glRotatef(18*i,0.0,0.0,1.0);
					glTranslatef(15.0,0.0,0.0);
					glPopMatrix(1);
				}

				glEndList();
			}
			// draw odd placed display lists names only (1st, 3rd, 5th, 7th, 9th)
			lists[0]=0; lists[1]=2; lists[2]=4; lists[3]=6; lists[4]=8;
			glListBase(index);              // set base offset
			glCallLists(5, GL_UNSIGNED_BYTE, lists); //only OpenGL Display List names set earlier will run!

			//Compile display list: 1
			index = glGenLists(5);
		
			//Compile display lists
			for(DLCount = 0; DLCount < 5; DLCount++){
				glNewList(index + DLCount, GL_COMPILE);   // compile each one

				for (i = 0; i <20; i ++){ //Draw triangles
					glPushMatrix();
					glRotatef(18*i,0.0,0.0,1.0);
					glTranslatef(15.0,0.0,0.0);
					glPopMatrix(1);
				}

				glEndList();
			}
			// draw odd placed display lists names only (1st, 3rd, 5th, 7th, 9th)
			lists[0]=0; lists[1]=2; lists[2]=4; lists[3]=6; lists[4]=8;
			glListBase(index);              // set base offset
			glCallLists(5, GL_UNSIGNED_BYTE, lists); //only OpenGL Display List names set earlier will run!
		}
	}
	
	return 0;
}
#endif

#ifdef WIN32
int TWLPrintf(const char *fmt, ...){
	return 0;
}
#endif
#endif

//glut shapes 
GLint DLSOLIDCUBE0_06F=-1;
GLint DLSPHERE=-1;
GLint DLCYLINDER=-1;

void glut2SolidCube(float x, float y, float z){
#ifdef ARM9
	updateGXLights(); //Update GX 3D light scene!
#endif
	glScalef(x, y, y);
	glCallList(DLSOLIDCUBE0_06F);
}

//glutSolidSphere(radius, 16, 16);  -> NDS GX Replacement
void drawSphere(float r, int lats, int longs) {	
#ifdef ARM9
	#include "Sphere008.h"
	glScalef(r, r, r);
	glCallListGX((u32*)&Sphere008); //comment out when running on NDSDisplayListUtils
	#endif

	#ifdef WIN32
	glCallList(DLSPHERE);
	#endif
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
void setupGLUTObjects(){
	DLSOLIDCUBE0_06F = (GLint)glGenLists(1);
	//glut2SolidCube(); -> NDS GX Implementation
	glNewList(DLSOLIDCUBE0_06F, GL_COMPILE);
	{
		float size = 0.06f;
		GLfloat n[6][3] =
		{
			{-1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, -1.0f}
		};
		GLint faces[6][4] =
		{
			{0, 1, 2, 3},
			{3, 2, 6, 7},
			{7, 6, 5, 4},
			{4, 5, 1, 0},
			{5, 6, 2, 1},
			{7, 4, 0, 3}
		};
		GLfloat v[8][3];
		GLint i;

		v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
		v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
		v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
		v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
		v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
		v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

		glScalef(32.0f, 32.0f, 32.0f);
		for (i = 5; i >= 0; i--)
		{
			glBegin(GL_QUADS);
			//glNormal3fv(&n[i][0]); //object is black when lighting is off
			glTexCoord2f(0, 0);
			glVertex3fv(&v[faces[i][0]][0]);
			glTexCoord2f(1, 0);
			glVertex3fv(&v[faces[i][1]][0]);
			glTexCoord2f(1, 1);
			glVertex3fv(&v[faces[i][2]][0]);
			glTexCoord2f(0, 1);
			glVertex3fv(&v[faces[i][3]][0]);
			glEnd();
		}
	}
	glEndList();

	DLSPHERE = (GLint)glGenLists(1);
	//drawSphere(); -> NDS GX Implementation
	glNewList(DLSPHERE, GL_COMPILE);
	{
		float r=1; 
		int lats=8; 
		int longs=8;
		int i, j;
		for (i = 0; i <= lats; i++) {
			float lat0 = PI * (-0.5 + (float)(i - 1) / lats);
			float z0 = sin((float)lat0);
			float zr0 = cos((float)lat0);

			float lat1 = PI * (-0.5 + (float)i / lats);
			float z1 = sin((float)lat1);
			float zr1 = cos((float)lat1);
			glBegin(GL_QUAD_STRIP);
			for (j = 0; j <= longs; j++) {
				float lng = 2 * PI * (float)(j - 1) / longs;
				float x = cos(lng);
				float y = sin(lng);
				//glNormal3f(x * zr0, y * zr0, z0); //lights are off
				glVertex3f(r * x * zr0, r * y * zr0, r * z0);
				//glNormal3f(x * zr1, y * zr1, z1);
				glVertex3f(r * x * zr1, r * y * zr1, r * z1);
			}
			glEnd();
		}
	}
	glEndList();
}

//gluDisk(qObj, 0.0, RADIUS, 16, 16); -> NDS GX Implementation
void drawCircle(GLfloat x, GLfloat y, GLfloat r, GLfloat BALL_RADIUS)
{
	#define SLICES_PER_CIRCLE ((int)16)
	float angle = 360.f / SLICES_PER_CIRCLE;
	float anglex = cos(angle);
	float angley = sin(angle);
	GLfloat lastX = 1;
	GLfloat lastY = 0;
	int c = 0; 
	glBegin(GL_TRIANGLE_STRIP);
	for (c = 1; c < SLICES_PER_CIRCLE; c++)
	{
		x = lastX * anglex - lastY * angley;
		y = lastX * angley + lastY * anglex;
		glVertex2f(x * BALL_RADIUS, y * BALL_RADIUS);
		lastX = x;
		lastY = y;
	}
	glEnd();
}

void drawCylinder(int numMajor, int numMinor, float height, float radius){
	double majorStep = height / numMajor;
	double minorStep = 2.0 * PI / numMinor;
	int i, j;

	for (i = 0; i < numMajor; ++i) {
		GLfloat z0 = 0.5 * height - i * majorStep;
		GLfloat z1 = z0 - majorStep;

		//glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j <= numMinor; ++j) {
			double a = j * minorStep;
			GLfloat x = radius * cos(a);
			GLfloat y = radius * sin(a);
			glNormal3f(x / radius, y / radius, 0.0);
			
			glTexCoord2f(j / (GLfloat) numMinor, i / (GLfloat) numMajor);
			glVertex3f(x, y, z0);

			glNormal3f(x / radius, y / radius, 0.0);
			glTexCoord2f(j / (GLfloat) numMinor, (i + 1) / (GLfloat) numMajor);
			glVertex3f(x, y, z1);
		}
		//glEnd();
	}
}

//slower, but allows multiple cubes of different sizes to upscale textures accurately
void glut2SolidCubeSlow(GLdouble size){
    static GLfloat n[6][3] =
    {
        {-1.0, 0.0, -1.0},
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 0.0, -1.0}
    };
    static GLint faces[6][4] =
    {
        {0, 1, 2, 3},
        {3, 2, 6, 7},
        {7, 6, 5, 4},
        {4, 5, 1, 0},
        {5, 6, 2, 1},
        {7, 4, 0, 3}
    };
    GLfloat v[8][3];
    GLint i;

    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

    for (i = 5; i >= 0; i--)
    {
		glBegin(GL_QUADS);
			//(snake head object)
			glColor3f(1.0f, 1.0f, 14.0f);
            glNormal3fv(&n[i][0]);
            glTexCoord2f(0, 0);
			glVertex3fv(&v[faces[i][0]][0]);
            glTexCoord2f(1, 0);
            glVertex3fv(&v[faces[i][1]][0]);
            glTexCoord2f(1, 1);
            glVertex3fv(&v[faces[i][2]][0]);
            glTexCoord2f(0, 1);
            glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}
