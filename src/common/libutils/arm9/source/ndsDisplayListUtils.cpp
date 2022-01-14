#if defined (WIN32) || defined(ARM9)

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

//ndsDisplayListUtils version: 0.3. src: https://bitbucket.org/Coto88/ndsdisplaylistutils/src

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
#include "VideoGLExt.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ndsDisplayListUtils.h"
#include "VideoGL.h"

#ifdef ARM9
#include "posixHandleTGDS.h"
#endif

//NDS GX C Display List implementation
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u32 ID2REG_C(u8 val){	
	u8  u8val = (u8)((val) << 2);
	return (u32)(u8val | 0x04000400);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u32 REG2ID_C(u32 val){	
	return (u32)( ( ((u32)((val & ~0x04000400))) ) >> 2 );
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u32 FIFO_COMMAND_PACK_C(u8 c1, u8 c2, u8 c3, u8 c4) {
	return (u32)(((c4) << 24) | ((c3) << 16) | ((c2) << 8) | (c1));
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_NOP(){
	return REG2ID_C(GFX_FIFO_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_STATUS(){
	return REG2ID_C(GFX_STATUS_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_COLOR(){
	return REG2ID_C(GFX_COLOR_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif  
u8 getFIFO_VERTEX16(){
	return REG2ID_C(GFX_VERTEX16_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_TEX_COORD(){
	return REG2ID_C(GFX_TEX_COORD_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_TEX_FORMAT(){
	return REG2ID_C(GFX_TEX_FORMAT_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_CLEAR_COLOR(){
	return REG2ID_C(GFX_CLEAR_COLOR_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_CLEAR_DEPTH(){
	return REG2ID_C(GFX_CLEAR_DEPTH_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_LIGHT_VECTOR(){
	return REG2ID_C(GFX_LIGHT_VECTOR_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_LIGHT_COLOR(){
	return REG2ID_C(GFX_LIGHT_COLOR_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getMTX_MULT_3x3(){
	return REG2ID_C(MATRIX_MULT3x3_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getMTX_IDENTITY(){
	return REG2ID_C(MATRIX_IDENTITY_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getMTX_SCALE(){
	return REG2ID_C(MATRIX_SCALE_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getMTX_PUSH(){
	return REG2ID_C(GFX_MTX_PUSH_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getMTX_POP(){
	return REG2ID_C(GFX_MTX_POP_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getMTX_TRANS(){
	return REG2ID_C(GFX_MTX_TRANS_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_NORMAL(){
	return REG2ID_C(GFX_NORMAL_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_DIFFUSE_AMBIENT(){
	return REG2ID_C(GFX_DIFFUSE_AMBIENT_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_SPECULAR_EMISSION(){
	return REG2ID_C(GFX_SPECULAR_EMISSION_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_SHININESS(){
	return REG2ID_C(GFX_SHININESS_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_POLY_FORMAT(){
	return REG2ID_C(GFX_POLY_FORMAT_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_BEGIN(){
	return REG2ID_C(GFX_BEGIN_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_END(){
	return REG2ID_C(GFX_END_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_FLUSH(){
	return REG2ID_C(GFX_FLUSH_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
u8 getFIFO_VIEWPORT(){
	return REG2ID_C(GFX_VIEWPORT_ADDR);
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
struct unpackedCmd FIFO_COMMAND_PACKED_FMT_UNPACK(u32 cmd){
	struct unpackedCmd out;
	out.cmd1 = ((cmd >> 0) & 0xFF);
	out.cmd2 = ((cmd >> 8) & 0xFF);
	out.cmd3 = ((cmd >> 16) & 0xFF);
	out.cmd4 = ((cmd >> 24) & 0xFF);
	return out;
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

//Returns: Display List raw binary filesize.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int getRawFileSizefromNDSGXDisplayListObject(struct ndsDisplayListDescriptor * dlInst){
	if(dlInst != NULL){
		return (dlInst->ndsDisplayListSize * 4) + 4;
	}
	return DL_INVALID;
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

//Lists: All "FIFO_BEGIN" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_BEGIN(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_BEGIN());	
}

//Lists: All "FIFO_END" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_END(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_END());	
}

//Lists: All "FIFO_VERTEX16" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_VERTEX16(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_VERTEX16());	
}

//Lists: All "FIFO_TEX_COORD" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_TEX_COORD(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_TEX_COORD());	
}

//Lists: All "FIFO_COLOR" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_COLOR(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_COLOR());	
}

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFilterByCommand(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut, u8 targetCommand){
	if((dlInst != NULL) && (dlInstOut != NULL)){
		int outCmdCount = 0;
		//Initialize
		dlInstOut->ndsDisplayListSize = 0;
		for(int i = 0; i < DL_MAX_ITEMS; i++){
			struct ndsDisplayList * DLOut = (struct ndsDisplayList *)&dlInstOut->DL[i];
			DLOut->displayListType = DL_INVALID;
			DLOut->index = 0;
			DLOut->value = 0;
		}
		for(int i = 0; i < dlInst->ndsDisplayListSize; i++){
			struct ndsDisplayList * thisDL = (struct ndsDisplayList *)&dlInst->DL[i];
			int indexOurCommandIs = (int)DL_INVALID;
			u8 lastCmd = (u8)DL_INVALID;
			//Find said command here
			struct unpackedCmd unpacked = FIFO_COMMAND_PACKED_FMT_UNPACK(thisDL->value);
			if(unpacked.cmd1 == targetCommand){
				indexOurCommandIs=1;
			}
			else if(unpacked.cmd2 == targetCommand){
				indexOurCommandIs=2;
				lastCmd = unpacked.cmd1;
			}
			else if(unpacked.cmd3 == targetCommand){
				indexOurCommandIs=3;
				lastCmd = unpacked.cmd2;
			}
			else if(unpacked.cmd4 == targetCommand){
				indexOurCommandIs=4;
				lastCmd = unpacked.cmd3;
			}
			//Handle special case: FIFO_VERTEX16 cmd. It packs 2 x 32bit (4x 16v values) = 8 bytes. 
			if(indexOurCommandIs>1){
				u8 specialCommandFIFO_VERTEX16 = (u8)getFIFO_VERTEX16();
				if(lastCmd == specialCommandFIFO_VERTEX16){
					indexOurCommandIs++;
				}
			}

			//push: displayListType is our cmd now
			if(indexOurCommandIs != (int)DL_INVALID){
				struct ndsDisplayList * startDLOut = (struct ndsDisplayList *)&dlInstOut->DL[outCmdCount];
				startDLOut->index = outCmdCount;
				thisDL+=indexOurCommandIs;
				startDLOut->value = thisDL->value;
				startDLOut->displayListType = targetCommand;
				dlInstOut->ndsDisplayListSize++;
				outCmdCount++;
			}
		}

		return true;
	}
	return false;
}


//Builds a NDS GX DisplayList / CallList object from a compiled one through the Filesystem.
//Returns: Display List size, DL_INVALID if fails.
//Notes: Currently reads a Packed Format Display List, todo: add Unpacked Format Display List
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
int BuildNDSGXDisplayListObjectFromFile(char * filename, struct ndsDisplayListDescriptor * dlInst){
	int ret = DL_INVALID;
	int readFileSize = -1;
	u32 * curPtr = NULL;
	u32 * startPtr = NULL;
	if( (filename != NULL) && (strlen(filename) > 0) && (dlInst != NULL) ){
		//Initialize
		for(int i = 0; i < DL_MAX_ITEMS; i++){
			struct ndsDisplayList * initDL = (struct ndsDisplayList *)&dlInst->DL[i];
			initDL->displayListType = DL_INVALID;
			initDL->index = 0;
			initDL->value = 0;
		}
		struct ndsDisplayList * curDL = (struct ndsDisplayList *)&dlInst->DL[0];
		int curDLIndex = 0;
		printf("ReadFile: %s", filename);
		
		#ifdef WIN32
		FILE * outFileGen = fopen(filename, "rb");
		#endif
		
		#ifdef ARM9
		FILE * outFileGen = fopen(filename, "r");
		#endif
		
		char * binDisplayList = NULL;
		if(outFileGen != NULL){
			fseek(outFileGen, 0, SEEK_END);
			int FileSize = ftell(outFileGen);
			fseek(outFileGen, 0, SEEK_SET);
			binDisplayList = (char*)malloc(FileSize);
			readFileSize = fread(binDisplayList, 1, FileSize, outFileGen);
			printf("DisplayList readSize: %d", readFileSize);
			startPtr = curPtr = (u32*)binDisplayList;
			dlInst->ndsDisplayListSize = (int)*curPtr;
			curPtr++;
			
			u32 cmdv1 = FIFO_COMMAND_PACK_C( getFIFO_BEGIN() , getFIFO_COLOR() , getFIFO_TEX_COORD() , getFIFO_NORMAL() ); //DL_TYPE_FIFO_PACKED_COMMAND_V1
			u32 cmdv2 = FIFO_COMMAND_PACK_C( getFIFO_VERTEX16() , getFIFO_COLOR() , getFIFO_TEX_COORD() , getFIFO_NORMAL() ); //DL_TYPE_FIFO_PACKED_COMMAND_V2
			u32 cmdend = FIFO_COMMAND_PACK_C( getFIFO_VERTEX16() , getFIFO_END() , getFIFO_NOP() , getFIFO_NOP() ); //DL_TYPE_FIFO_PACKED_COMMAND_END
				
			//FIFO_COMMAND_PACK() CMD Descriptor (start cmd):
			//1: FIFO_VERTEX16
			//2: FIFO_COLOR
			//3: FIFO_TEX_COORD
			//4: FIFO_NORMAL
			u32 val = *(curPtr);
			while(
				!(val == cmdv1)
			){
				curPtr++; //next command...
				val = *(curPtr);
			}
			val = *(curPtr); 
			curPtr++;
			while(
				!(val == cmdv2)
				&&
				!(val == cmdend)
			){
				curDL->index = curDLIndex;
				curDL->value = val;
				(curDL)->displayListType = DL_TYPE_FIFO_PACKED_COMMAND_V1;
				curDLIndex++;
				curDL++;	
				val = *(curPtr);
				curPtr++;
			}

			//find v2 cmds
			while(
				!(val == cmdend)
			){
				curDL->index = curDLIndex;
				curDL->value = val;
				(curDL)->displayListType = DL_TYPE_FIFO_PACKED_COMMAND_V2;
				curDLIndex++;
				curDL++;	
				val = *(curPtr);
				curPtr++;
			}

			//find end cmds
			while(
				((readFileSize/4) > ((int)(curPtr - startPtr) - 1))
			){
				curDL->index = curDLIndex;
				curDL->value = val;
				(curDL)->displayListType = DL_TYPE_FIFO_PACKED_COMMAND_END;
				curDLIndex++;
				curDL++;	
				val = *(curPtr);
				curPtr++;
			}
			free(binDisplayList);
			fclose(outFileGen);
		}
	}
	ret = ((int)(curPtr - startPtr) - 1);
	return ret;
}

//Defines if it's a GX command or not
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
bool isAGXCommand(u32 val){
	bool isAGXCommand = false;
	if (val == (u32)getFIFO_BEGIN()) {
		isAGXCommand = true;
	}
	else if (val == (u32)getMTX_PUSH()) {
		isAGXCommand = true;
	}
	else if (val == (u32)getMTX_POP()) {
		isAGXCommand = true;
	}
	else if (val == (u32)getMTX_IDENTITY()) {
		isAGXCommand = true;
	}
	else if (val == (u32)getMTX_TRANS()) {
		isAGXCommand = true;
	}
	else if (val == (u32)getMTX_MULT_3x3()) {
		isAGXCommand = true;
	}
	else if (val == (u32)getFIFO_END()) {
		isAGXCommand = true;
	}
	return isAGXCommand;
}

#ifdef WIN32

//Unit Test (WIN32): reads a NDS GX Display List / Call List payload emmited from (https://bitbucket.org/Coto88/blender-nds-exporter/src/master/)
//Translates to an NDS GX Display List / Call List object, and then builds it again into the original payload.
int main(int argc, char** argv){

	//Unit Test: NDS GX DL object and building
	/*
	char cwdPath[256];
	getCWDWin(cwdPath, "\\cv\\");
	std::vector<std::string> BinFilesRead = findFiles(std::string(cwdPath), std::string("bin"));
	printf("\nPath: %s \nbin files read: %d\n", cwdPath, BinFilesRead.size());
	
	struct ndsDisplayListDescriptor * NDSDL = (struct ndsDisplayListDescriptor *)TGDSARM9Malloc(sizeof(struct ndsDisplayListDescriptor));
	int dlReadSize = BuildNDSGXDisplayListObjectFromFile((char*)BinFilesRead.at(0).c_str(), NDSDL);
	if(dlReadSize != DL_INVALID){

		GL_GLBEGIN_ENUM type = getDisplayListGLType(NDSDL);
		if(type == GL_TRIANGLES){
			printf("\nGX DisplayListType: GL_TRIANGLES\n");
		}
		else if (type == GL_QUADS){
			printf("\nGX DisplayListType: GL_QUADS\n");
		}
		else if (type == GL_TRIANGLE_STRIP){
			printf("\nGX DisplayListType: GL_TRIANGLE_STRIP\n");
		}
		else if (type == GL_QUAD_STRIP){
			printf("\nGX DisplayListType: GL_QUAD_STRIP\n");
		}
		else{
			printf("\nGX DisplayListType: ERROR\n");
		}

		//Get Binary filesize
		int displayListSize = getRawFileSizefromNDSGXDisplayListObject(NDSDL);

		//List commands
		struct ndsDisplayListDescriptor * NDSDLColorCmds = (struct ndsDisplayListDescriptor *)TGDSARM9Malloc(sizeof(struct ndsDisplayListDescriptor));
		getDisplayListFIFO_BEGIN(NDSDL, NDSDLColorCmds);
		int FIFO_BEGINCount = NDSDLColorCmds->ndsDisplayListSize;
		printf("\nGX DisplayList (%d): FIFO_BEGIN commands \n", FIFO_BEGINCount);
		getDisplayListFIFO_COLOR(NDSDL, NDSDLColorCmds);
		printf("\nGX DisplayList (%d): FIFO_COLOR commands \n", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_TEX_COORD(NDSDL, NDSDLColorCmds);
		printf("\nGX DisplayList (%d): FIFO_TEX_COORD commands \n", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_VERTEX16(NDSDL, NDSDLColorCmds);
		printf("\nGX DisplayList (%d): FIFO_VERTEX16 commands \n", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_END(NDSDL, NDSDLColorCmds);
		int FIFO_ENDCount = NDSDLColorCmds->ndsDisplayListSize;
		printf("\nGX DisplayList (%d): FIFO_END commands \n", FIFO_ENDCount);
		
		if((FIFO_BEGINCount != 0) && (FIFO_ENDCount != 0) && (FIFO_BEGINCount == FIFO_ENDCount) ){
			printf("\n(%d) GX DisplayList detected \n", FIFO_BEGINCount);
		}
		else{
			printf("\nBROKEN GX DisplayList: It won't work as expected on real Nintendo DS!\n");
		}
		TGDSARM9Free(NDSDLColorCmds);

		//Build it
		u32 * builtDisplayList = (u32*)TGDSARM9Malloc(displayListSize);
		memset(builtDisplayList, 0, displayListSize);
		int builtDisplayListSize = CompilePackedNDSGXDisplayListFromObject(builtDisplayList, NDSDL) + 1;
		
		if(dlReadSize != builtDisplayListSize){
			printf("\nNDS DisplayList Build failed\n");
			return 0;
		}
		char outPath[256];
		sprintf(outPath, "%s%s", cwdPath, "out.bin");
		printf("Rebuilding DisplayList at: %s", outPath);
		FILE * fout = fopen(outPath, "w+b");
		if(fout != NULL){
			if(fwrite(builtDisplayList, 1, displayListSize, fout) > 0){
				fclose(fout);
				printf("\nRebuild OK \n");
			}
			else{
				printf("\nRebuild ERROR \n");
				return 0;
			}
		}
		TGDSARM9Free(builtDisplayList);
	}
	else{
		printf("NDS DisplayList creation fail");
	}
	TGDSARM9Free(NDSDL);
	*/

	//Unit Test #1: Tests OpenGL DisplayLists components functionality then emitting proper GX displaylists, unpacked format.
	GLInitExt();
	int list = glGenLists(10);
	if(list){
		glListBase(list);
		bool ret = glIsList(list); //should return false (DL generated, but no displaylist-name was generated)
		glNewList(list, GL_COMPILE);
		ret = glIsList(list); //should return true (DL generated, and displaylist-name was generated)
		if(ret == true){
			for (int i = 0; i <10; i ++){ //Draw 10 cubes
				glPushMatrix();
				glRotate(36*i,0.0,0.0,1.0);
				glTranslatef(10.0,0.0,0.0);
				glPopMatrix(1);
			}
		}
		glEndList();
		
		glListBase(list + 1);
		glNewList (list + 1, GL_COMPILE);//Create a second display list and execute it
        ret = glIsList(list + 1); //should return true (DL generated, and displaylist-name was generated)
		if(ret == true){
			for (int i = 0; i <20; i ++){ //Draw 20 triangles
				glPushMatrix();
				glRotate(18*i,0.0,0.0,1.0);
				glTranslatef(15.0,0.0,0.0);
				glPopMatrix(1);
			}
		}
		glEndList();//The second display list is created
	}

	//Unit Test #2:
	//Convert Compiled_DL from unpacked format to packed format exported as C source code (.h)
	//should resemble the structure at: Cube.h
	
	char cwdPath[256];
	getCWDWin(cwdPath, "\\cv\\");
	char outPath[256];
		sprintf(outPath, "%s%s", cwdPath, "PackedDLEmitted.h");
		FILE * fout = fopen(outPath, "w+b");
		if(fout != NULL){
			u32 * listPtr = (u32*)&Compiled_DL_Binary[0];
			int listSize = (int)*listPtr;
			listPtr++;
			int currentCmdCount = 0;
			fprintf(fout, "u32 PackedDLEmitted[] = { \n%d,\n", listSize);


			u32 * rawARGBuffer = (u32 *)malloc(listSize); //raw, linear buffer args from all GX commands (without GX commands)
			u32* curRawARGBufferSave = (u32*)rawARGBuffer; //used to save args
			u32* curRawARGBufferRestore = (u32*)rawARGBuffer; //used to read and consume args 

			int curRawARGBufferCount = 0; //incremented from the args parsing part, consumed when adding args to fout stream

			for(int i = 0; i < (listSize / 4); i++){
				
				//build command(s)
				if (((currentCmdCount) % 4) == 0) {
					fprintf(fout, "FIFO_COMMAND_PACK( ");
				}

				//Note: All commands implemented here must be replicated to isAGXCommand() method
				u32 cmd = *listPtr;
				int cmdOffset = 0;
				bool isCmd = false;
				if (cmd == getFIFO_BEGIN()){
					fprintf(fout, "FIFO_BEGIN");
					currentCmdCount++;
					isCmd = true;

					//no args used by this GX command
				}

				else if(cmd == getMTX_PUSH()){
					fprintf(fout, "MTX_PUSH");
					currentCmdCount++;
					isCmd = true;

					//no args used by this GX command
				}
				
				else if(cmd == getMTX_POP()){
					fprintf(fout, "MTX_POP");
					currentCmdCount++;
					isCmd = true;

					//4000448h 12h 1  36  MTX_POP - Pop Current Matrix from Stack(W)
					u32 * curListPtr = listPtr;
					curListPtr++;
					u32 curVal = *curListPtr;
					while (isAGXCommand(curVal) == false) {
						*curRawARGBufferSave = curVal;
						curRawARGBufferSave++;
						curListPtr++;
						curVal = *curListPtr;
						curRawARGBufferCount++;
					}
				}

				else if (cmd == getMTX_IDENTITY()) {
					fprintf(fout, "MTX_IDENTITY");
					currentCmdCount++;
					isCmd = true;

					//4000454h 15h - 19  MTX_IDENTITY - Load Unit Matrix to Current Matrix(W)
					//no args used by this GX command
				}

				else if (cmd == getMTX_TRANS()) {
					fprintf(fout, "MTX_TRANS");
					currentCmdCount++;
					isCmd = true;

					//4000470h 1Ch 3  22 * MTX_TRANS - Mult.Curr.Matrix by Translation Matrix(W)
					u32* curListPtr = listPtr;
					curListPtr++;
					u32 curVal = *curListPtr;
					while (isAGXCommand(curVal) == false) {
						*curRawARGBufferSave = curVal;
						curRawARGBufferSave++;
						curListPtr++;
						curVal = *curListPtr;
						curRawARGBufferCount++;
					}
				}

				else if (cmd == getMTX_MULT_3x3()) {
					fprintf(fout, "MTX_MULT_3x3");
					currentCmdCount++;
					isCmd = true;

					//4000468h 1Ah 9  28 * MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix(W)
					u32* curListPtr = listPtr;
					curListPtr++;
					u32 curVal = *curListPtr;
					while (isAGXCommand(curVal) == false) {
						*curRawARGBufferSave = curVal;
						curRawARGBufferSave++;
						curListPtr++;
						curVal = *curListPtr;
						curRawARGBufferCount++;
					}
				}

				else if (cmd == getFIFO_END()) {
					fprintf(fout, "FIFO_END");
					currentCmdCount++;
					isCmd = true;
				}

				//add comma if next command is not closing
				if ( (((currentCmdCount) % 4) != 0) ) {
					if (isCmd == true) {
						fprintf(fout, ", ");
					}
				}
				else {
					//close command
					fprintf(fout, "),\n");


					//for each command found, add params(s)
					//curRawARGBufferCount consumed here
					if (curRawARGBufferCount > 0) {
						int j = 0;
						for (j = 0; j < curRawARGBufferCount; j++) {
							u32 curArg = curRawARGBufferRestore[j];
							fprintf(fout, "%d,\n", curArg);
						}
						curRawARGBufferRestore += curRawARGBufferCount;
						curRawARGBufferCount = 0;
					}

					//i = (listSize / 4); //debug
				}
				listPtr++;
			}

			
			
			fprintf(fout, "FIFO_COMMAND_PACK( FIFO_END, FIFO_NOP, FIFO_NOP, FIFO_NOP )\n};");
			fclose(fout);


			free(rawARGBuffer);
		}

	return 0;
}
#endif

#ifdef ARM9
//Unit Test (NDS): reads a NDS GX Display List / Call List payload emmited from (https://bitbucket.org/Coto88/blender-nds-exporter/src/master/)
//Translates to an NDS GX Display List / Call List object, and then builds it again into the original payload.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool ndsDisplayListUtilsTestCaseARM9(char * filename, char * outNDSGXBuiltDisplayList){
	bool retStatus = false;
	struct ndsDisplayListDescriptor * NDSDL = (struct ndsDisplayListDescriptor *)TGDSARM9Malloc(sizeof(struct ndsDisplayListDescriptor));
	int dlReadSize = BuildNDSGXDisplayListObjectFromFile((char*)filename, NDSDL);
	if((u32)dlReadSize != DL_INVALID){
		GL_GLBEGIN_ENUM type = getDisplayListGLType(NDSDL);
		if(type == GL_TRIANGLES){
			printf("GX DisplayListType: GL_TRIANGLES");
		}
		else if (type == GL_QUADS){
			printf("GX DisplayListType: GL_QUADS");
		}
		else if (type == GL_TRIANGLE_STRIP){
			printf("GX DisplayListType: GL_TRIANGLE_STRIP");
		}
		else if (type == GL_QUAD_STRIP){
			printf("GX DisplayListType: GL_QUAD_STRIP");
		}
		else{
			printf("GX DisplayListType: ERROR");
		}

		//Get Binary filesize
		int displayListSize = getRawFileSizefromNDSGXDisplayListObject(NDSDL);

		//List commands
		struct ndsDisplayListDescriptor * NDSDLColorCmds = (struct ndsDisplayListDescriptor *)TGDSARM9Malloc(sizeof(struct ndsDisplayListDescriptor));
		getDisplayListFIFO_BEGIN(NDSDL, NDSDLColorCmds);
		int FIFO_BEGINCount = NDSDLColorCmds->ndsDisplayListSize;
		printf("GX DisplayList (%d): FIFO_BEGIN commands ", FIFO_BEGINCount);
		getDisplayListFIFO_COLOR(NDSDL, NDSDLColorCmds);
		printf("GX DisplayList (%d): FIFO_COLOR commands", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_TEX_COORD(NDSDL, NDSDLColorCmds);
		printf("GX DisplayList (%d): FIFO_TEX_COORD commands", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_VERTEX16(NDSDL, NDSDLColorCmds);
		printf("GX DisplayList (%d): FIFO_VERTEX16 commands", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_END(NDSDL, NDSDLColorCmds);
		int FIFO_ENDCount = NDSDLColorCmds->ndsDisplayListSize;
		printf("GX DisplayList (%d): FIFO_END commands", FIFO_ENDCount);
		
		if((FIFO_BEGINCount != 0) && (FIFO_ENDCount != 0) && (FIFO_BEGINCount == FIFO_ENDCount) ){
			printf("(%d) GX DisplayList detected", FIFO_BEGINCount);
		}
		else{
			printf("BROKEN GX DisplayList: It won't work as expected on real Nintendo DS!");
		}
		TGDSARM9Free(NDSDLColorCmds);

		//Build it
		u32 * builtDisplayList = (u32*)TGDSARM9Malloc(displayListSize);
		memset(builtDisplayList, 0, displayListSize);
		int builtDisplayListSize = CompilePackedNDSGXDisplayListFromObject(builtDisplayList, NDSDL) + 1;
		
		if(dlReadSize != builtDisplayListSize){
			printf("NDS DisplayList Build failed");
		}
		printf("Rebuilding DisplayList at: %s", outNDSGXBuiltDisplayList);
		FILE * fout = fopen(outNDSGXBuiltDisplayList, "w+");
		if(fout != NULL){
			if(fwrite(builtDisplayList, 1, displayListSize, fout) > 0){
				fclose(fout);
				printf("Rebuild OK");
				retStatus = true;
			}
			else{
				printf("Rebuild ERROR");
			}
		}
		TGDSARM9Free(builtDisplayList);
	}
	else{
		printf("NDS DisplayList creation fail");
	}
	TGDSARM9Free(NDSDL);
	return retStatus;
}
#endif

#endif