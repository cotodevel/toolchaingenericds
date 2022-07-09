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
#include "VideoGL.h"
#include "ndsDisplayListUtils.h"

#ifdef ARM9
#include "posixHandleTGDS.h"
#include "PackedDisplayListCompiledVS2012.h"
#endif

#ifdef ARM9
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

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
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

//Returns: Display List raw binary filesize.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
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

//Lists: All "FIFO_BEGIN" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_BEGIN(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_BEGIN);	
}

//Lists: All "FIFO_END" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_END(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_END);	
}

//Lists: All "FIFO_VERTEX16" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_VERTEX16(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_VERTEX16);	
}

//Lists: All "FIFO_TEX_COORD" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_TEX_COORD(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_TEX_COORD);	
}

//Lists: All "FIFO_COLOR" commands inside dlInst
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFIFO_COLOR(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut){
	return getDisplayListFilterByCommand(dlInst, dlInstOut, (u8)getFIFO_COLOR);	
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
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
bool getDisplayListFilterByCommand(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut, u8 targetCommand){
	int i = 0; 
	if((dlInst != NULL) && (dlInstOut != NULL)){
		int outCmdCount = 0;
		//Initialize
		dlInstOut->ndsDisplayListSize = 0;
		for(i = 0; i < DL_DESCRIPTOR_MAX_ITEMS; i++){
			struct ndsDisplayList * DLOut = (struct ndsDisplayList *)&dlInstOut->DL[i];
			DLOut->displayListType = DL_INVALID;
			DLOut->index = 0;
			DLOut->value = 0;
		}
		for(i = 0; i < dlInst->ndsDisplayListSize; i++){
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
				u8 specialCommandFIFO_VERTEX16 = (u8)getFIFO_VERTEX16;
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


//Builds a PACKED DISPLAY LIST (GX binary to be used with CallList command)/ CallList object from a compiled one through the Filesystem.
//Returns: Display List size, DL_INVALID if fails.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
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
	char * binDisplayList = NULL;
	u32 cmdv1 = FIFO_COMMAND_PACK_C( getFIFO_BEGIN , getFIFO_COLOR , getFIFO_TEX_COORD , getFIFO_NORMAL ); //DL_TYPE_FIFO_PACKED_COMMAND_V1
	u32 cmdv2 = FIFO_COMMAND_PACK_C( getFIFO_VERTEX16 , getFIFO_COLOR , getFIFO_TEX_COORD , getFIFO_NORMAL ); //DL_TYPE_FIFO_PACKED_COMMAND_V2
	u32 cmdend = FIFO_COMMAND_PACK_C( getFIFO_VERTEX16 , getFIFO_END , getFIFO_NOP , getFIFO_NOP ); //DL_TYPE_FIFO_PACKED_COMMAND_END	
	struct ndsDisplayList * curDL = NULL;
	int curDLIndex = 0;
	int FileSize = 0;
	FILE * outFileGen = NULL;
	u32 val = 0;
	if( (filename != NULL) && (strlen(filename) > 0) && (dlInst != NULL) ){
		//Initialize
		int i = 0; 
		for(i = 0; i < DL_DESCRIPTOR_MAX_ITEMS; i++){
			struct ndsDisplayList * initDL = (struct ndsDisplayList *)&dlInst->DL[i];
			initDL->displayListType = DL_INVALID;
			initDL->index = 0;
			initDL->value = 0;
		}
		curDL = (struct ndsDisplayList *)&dlInst->DL[0];
		#ifdef WIN32
		outFileGen = fopen(filename, "rb");
		#endif
		
		#ifdef ARM9
		outFileGen = fopen(filename, "r");
		#endif
		if(outFileGen != NULL){
			fseek(outFileGen, 0, SEEK_END);
			FileSize = ftell(outFileGen);
			fseek(outFileGen, 0, SEEK_SET);
			binDisplayList = (char*)TGDSARM9Malloc(FileSize);
			readFileSize = fread(binDisplayList, 1, FileSize, outFileGen);
			//printf("DisplayList readSize: %d", readFileSize);
			startPtr = curPtr = (u32*)binDisplayList;
			dlInst->ndsDisplayListSize = (int)*curPtr;
			curPtr++;
				
			//FIFO_COMMAND_PACK() CMD Descriptor (start cmd):
			//1: FIFO_VERTEX16
			//2: FIFO_COLOR
			//3: FIFO_TEX_COORD
			//4: FIFO_NORMAL
			val = *(curPtr);
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
			TGDSARM9Free(binDisplayList);
			fclose(outFileGen);
		}
		ret = ((int)(curPtr - startPtr) - 1);
	}
	return ret;
}

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
__attribute__((optnone))
#endif
#endif
bool packAndExportSourceCodeFromRawUnpackedDisplayListFormat(char * filenameOut, u32 * rawUnpackedDisplayList){
	int foundOffset = 0;
	if( (filenameOut != NULL) && (rawUnpackedDisplayList != NULL) ){
		#ifdef WIN32
		FILE * fout = fopen(filenameOut, "w+b");
		#endif
		
		#ifdef ARM9
		FILE * fout = fopen(filenameOut, "w+");
		#endif
		
		if(fout != NULL){
			u32 * listPtr = rawUnpackedDisplayList;
			int listSize = (int)*listPtr;
			int currentCmdCount = 0;
			u32 * rawARGBuffer = (u32 *)TGDSARM9Malloc(listSize); //raw, linear buffer args from all GX commands (without GX commands)
			u32* curRawARGBufferSave = (u32*)rawARGBuffer; //used to save args
			u32* curRawARGBufferRestore = (u32*)rawARGBuffer; //used to read and consume args 
			int curRawARGBufferCount = 0; //incremented from the args parsing part, consumed when adding args to fout stream
			int packedCommandCount = 0;
			int i = 0;
			u8 leadingZeroes = 0;
			char readBuf[128];
			char listSizeChar[128];
			fprintf(fout, "#include \"VideoGL.h\"\n\nu32 PackedDLEmitted[] = { \n%d,\n", listSize);
			listPtr++;
			{
				for(i = 0; i < (listSize / 4); i++){
					//Note: All commands implemented here must be replicated to isAGXCommand() method
					bool isCmd = false;
					u32 cmd = *listPtr;
					//build command(s)
					if ( (isAGXCommand(cmd) == true)  && (((currentCmdCount) % 4) == 0) ) {
						fprintf(fout, "FIFO_COMMAND_PACK(");
					}

					if (cmd == getFIFO_BEGIN){
						fprintf(fout, "FIFO_BEGIN");
						currentCmdCount++;
						isCmd = true;

						//no args used by this GX command
					}
					else if(cmd == getMTX_PUSH){
						fprintf(fout, "MTX_PUSH");
						currentCmdCount++;
						isCmd = true;

						//no args used by this GX command
					}
					else if(cmd == getMTX_POP){
						//4000448h 12h 1  36  MTX_POP - Pop Current Matrix from Stack(W)
						u32 * curListPtr = listPtr;
						curListPtr++;
						{
							u32 curVal = *curListPtr;
							while (isAGXCommand(curVal) == false) {
								*curRawARGBufferSave = curVal;
								curRawARGBufferSave++;
								curListPtr++;
								curVal = *curListPtr;
								curRawARGBufferCount++;
							}
							fprintf(fout, "MTX_POP");
							currentCmdCount++;
							isCmd = true;
						}
					}
					else if (cmd == getMTX_IDENTITY) {
						fprintf(fout, "MTX_IDENTITY");
						currentCmdCount++;
						isCmd = true;

						//4000454h 15h - 19  MTX_IDENTITY - Load Unit Matrix to Current Matrix(W)
						//no args used by this GX command
					}
					else if (cmd == getMTX_TRANS) {
						//4000470h 1Ch 3  22 * MTX_TRANS - Mult.Curr.Matrix by Translation Matrix(W)
						u32* curListPtr = listPtr;
						curListPtr++;
						{
							u32 curVal = *curListPtr;
							while (isAGXCommand(curVal) == false) {
								*curRawARGBufferSave = curVal;
								curRawARGBufferSave++;
								curListPtr++;
								curVal = *curListPtr;
								curRawARGBufferCount++;
							}
						}
						fprintf(fout, "MTX_TRANS");
						currentCmdCount++;
						isCmd = true;
					}
					else if (cmd == getMTX_MULT_3x3) {
						//4000468h 1Ah 9  28 * MTX_MULT_3x3 - Multiply Current Matrix by 3x3 Matrix(W)
						u32* curListPtr = listPtr;
						curListPtr++;
						{
							u32 curVal = *curListPtr;
							while (isAGXCommand(curVal) == false) {
								*curRawARGBufferSave = curVal;
								curRawARGBufferSave++;
								curListPtr++;
								curVal = *curListPtr;
								curRawARGBufferCount++;
							}
						}
						fprintf(fout, "MTX_MULT_3x3");
						currentCmdCount++;
						isCmd = true;
					}
					else if (cmd == getFIFO_END) {
						fprintf(fout, "FIFO_END");
						currentCmdCount++;
						isCmd = true;
						//no args used by this GX command
					}


					//IS closing cmd... Process all remaining FIFO commands, then stub out the rest as FIFO_NOPs
					if( i == ((listSize / 4) - 1)){
						int currentCommandOffset = (currentCmdCount % 4);
						int currentCommandOffsetCopy = currentCommandOffset;
						int j = 0;
					
						//FIFO_END not last aligned command? Fill it with FIFO_NOP
						if (currentCommandOffset > 0){
							for (j = 0; j < currentCommandOffset; j++) {
								fprintf(fout, "FIFO_NOP");
								currentCommandOffsetCopy++;
								if((currentCommandOffsetCopy % 4) == 0){
									//LAST CMD BATCH: for each command found, add params(s)
									//curRawARGBufferCount consumed here
									if (curRawARGBufferCount > 0) {
										fprintf(fout, "),\n");
										packedCommandCount++; //a whole packed command is 4 bytes
										for (j = 0; j < curRawARGBufferCount; j++) {
											u32 curArg = curRawARGBufferRestore[j];
											fprintf(fout, "%d", curArg);
											if( j != (curRawARGBufferCount - 1)){
												fprintf(fout, ",");
											}
											fprintf(fout, "\n");
											packedCommandCount++; //each param is 4 bytes
										}
										curRawARGBufferRestore += curRawARGBufferCount;
										curRawARGBufferCount = 0;
									}
									else{
										fprintf(fout, ")\n");
									}

									fprintf(fout, "};");
								}
								else{
									fprintf(fout, ", ");
								}
							}
						}
						//FIFO_END IS last aligned command. Close bracket and remove last comma
						else{
							int pos = ftell(fout) - 2;
							if(pos <= 0){
								pos = 0;
							}
							fseek(fout, pos, SEEK_SET);
							fprintf(fout, "\n};");
							packedCommandCount++; //a whole packed command is 4 bytes
						}
					}
					//NOT closing cmd!...  
					else{
						//add comma if next command is not closing
						if ( (((currentCmdCount) % 4) != 0) ) {
							if (isCmd == true) {
								fprintf(fout, ", ");
							}
						}
						else {
							//close command
							if (isAGXCommand(cmd) == true){
								fprintf(fout, "),\n");
								packedCommandCount++; //a whole packed command is 4 bytes
							}
							//NOT LAST CMD BATCH: for each command found, add params(s): curRawARGBufferCount consumed here
							if (curRawARGBufferCount > 0) {
								int j = 0;
								for (j = 0; j < curRawARGBufferCount; j++) {
									u32 curArg = curRawARGBufferRestore[j];
									fprintf(fout, "%d,\n", curArg);
									packedCommandCount++; //each param is 4 bytes
								}
								curRawARGBufferRestore += curRawARGBufferCount;
								curRawARGBufferCount = 0;
							}
						}
					}
				
					listPtr++;
				}
			}

			//Now find listSize inside file handle, and replace it with new count
			fseek(fout, 0, SEEK_SET);
			fread(readBuf, 1, sizeof(readBuf), fout);
			
			leadingZeroes = clzero((u32)listSize)/4; //bit -> decimal zeroes
			leadingZeroes++; //count last decimal

			memset(listSizeChar, 0, leadingZeroes);
			itoa1(listSize, (char*)&listSizeChar[0], 10);
			foundOffset = 0;
			{
				int j = 0;
				int k = 0;
				for(j = 0; j < 128/leadingZeroes; j++){
					char * looker = (char *)&readBuf[j * leadingZeroes-1];
					if(strncmp (looker, (const char *)&listSizeChar[0], leadingZeroes) == 0){
						foundOffset--;

						//erase the value
						fseek(fout, foundOffset, SEEK_SET);
						for(k = 0; k < (int)leadingZeroes + 1; k++){ //+1 the comma
							fprintf(fout, " ");
						}

						//set packed count
						fseek(fout, foundOffset, SEEK_SET);
						fprintf(fout, "%d,\n", packedCommandCount*4);

					}
					else{
						foundOffset+=leadingZeroes;
					}
				}
			}
			fclose(fout);
			TGDSARM9Free(rawARGBuffer);
		}

		return true;
	}
	return false;
}


#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
bool rawUnpackedToRawPackedDisplayListFormat(u32 * inRawUnpackedDisplayList, u32 * outRawPackedDisplayList){
	if( (inRawUnpackedDisplayList != NULL) && (outRawPackedDisplayList != NULL) ){
		u32 * listPtr = inRawUnpackedDisplayList;
		u32 * listPtrOut = outRawPackedDisplayList;

		listPtrOut++; //reserved: packed DL Size: index 0
		int listSize = (int)*listPtr;
		listPtr++;
		int currentCmdCount = 0;
		u32 * rawARGBuffer = (u32 *)TGDSARM9Malloc(listSize); //raw, linear buffer args from all GX commands (without GX commands)
		u32* curRawARGBufferSave = (u32*)rawARGBuffer; //used to save args
		u32* curRawARGBufferRestore = (u32*)rawARGBuffer; //used to read and consume args 
		
		u32 * rawStandaloneCmdBuffer = (u32 *)TGDSARM9Malloc(listSize); //raw, linear buffer args from all GX commands (without GX commands)
		u32 * rawStandaloneCmdBufferCopy = rawStandaloneCmdBuffer;
		u32 * rawStandaloneArgBuffer = (u32 *)TGDSARM9Malloc(listSize); //raw, linear buffer args from all GX commands (without GX commands)
		u32 * rawStandaloneArgBufferCopy = rawStandaloneArgBuffer;

		int curRawARGBufferCount = 0; //incremented from the args parsing part, consumed when adding args to fout stream
		int packedCommandCount = 0;
		int i = 0;
		for(i = 0; i < (listSize / 4); i++){
			//Note: All commands implemented here must be replicated to isAGXCommand() method
			u32 cmd = *listPtr;
			bool iscmdEnd = false;
			int cmdOffset = 0;
			bool isCmd = false;
			if (cmd == getFIFO_BEGIN){
				//fprintf(fout, "FIFO_BEGIN");
				currentCmdCount++;
				isCmd = true;

				//no args used by this GX command
			}
			else if(cmd == getMTX_PUSH){
				//fprintf(fout, "MTX_PUSH");
				currentCmdCount++;
				isCmd = true;

				//no args used by this GX command
			}
			else if(cmd == getMTX_POP){
				//fprintf(fout, "MTX_POP");
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
			else if (cmd == getMTX_IDENTITY) {
				//fprintf(fout, "MTX_IDENTITY");
				currentCmdCount++;
				isCmd = true;

				//4000454h 15h - 19  MTX_IDENTITY - Load Unit Matrix to Current Matrix(W)
				//no args used by this GX command
			}
			else if (cmd == getMTX_TRANS) {
				//fprintf(fout, "MTX_TRANS");
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
			else if (cmd == getMTX_MULT_3x3) {
				//fprintf(fout, "MTX_MULT_3x3");
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
			else if (cmd == getFIFO_END) {
				//fprintf(fout, "FIFO_END");
				currentCmdCount++;
				isCmd = true;
			}

			if(isCmd == true){
				*(rawStandaloneCmdBufferCopy) = cmd;
				rawStandaloneCmdBufferCopy++;
				
				//save args
				if (curRawARGBufferCount > 0) {

					int j = 0;
					for (j = 0; j < curRawARGBufferCount; j++) {
						u32 curArg = curRawARGBufferRestore[j];
						*(rawStandaloneArgBufferCopy) = curArg;
						rawStandaloneArgBufferCopy++;
					}
					curRawARGBufferRestore += curRawARGBufferCount;
					curRawARGBufferCount = 0;
				}
			}
			
			listPtr++;
		}

		//enumerate commands & enumerate args:
		//for each command take n args from the current arg stack and build each packed command accordingly
		int detectedCmdCount = (rawStandaloneCmdBufferCopy - rawStandaloneCmdBuffer);

		//reset rawStandaloneArgBufferCopy so we can re-use it
		rawStandaloneArgBufferCopy = rawStandaloneArgBuffer;

		int j = 0;
		for(j = 0; j < detectedCmdCount; j+=4){
			*listPtrOut = FIFO_COMMAND_PACK_C(rawStandaloneCmdBuffer[j + 0], rawStandaloneCmdBuffer[j + 1], rawStandaloneCmdBuffer[j + 2], rawStandaloneCmdBuffer[j + 3]); 
			listPtrOut++;
			packedCommandCount++; //a whole packed command is 4 bytes

			int curCmdparamsCount0 = getAGXParamsCountFromCommand(rawStandaloneCmdBuffer[j + 0]);
			int k = 0;
			for(k = 0; k < curCmdparamsCount0; k++){
				*(listPtrOut) = *(rawStandaloneArgBufferCopy);
				listPtrOut++;
				rawStandaloneArgBufferCopy++;
				packedCommandCount++; //each param is 4 bytes
			}

			int curCmdparamsCount1 = getAGXParamsCountFromCommand(rawStandaloneCmdBuffer[j + 1]);
			for(k = 0; k < curCmdparamsCount1; k++){
				*(listPtrOut) = *(rawStandaloneArgBufferCopy);
				listPtrOut++;
				rawStandaloneArgBufferCopy++;
				packedCommandCount++; //each param is 4 bytes
			}

			int curCmdparamsCount2 = getAGXParamsCountFromCommand(rawStandaloneCmdBuffer[j + 2]);
			for(k = 0; k < curCmdparamsCount2; k++){
				*(listPtrOut) = *(rawStandaloneArgBufferCopy);
				listPtrOut++;
				rawStandaloneArgBufferCopy++;
				packedCommandCount++; //each param is 4 bytes
			}

			int curCmdparamsCount3 = getAGXParamsCountFromCommand(rawStandaloneCmdBuffer[j + 3]);
			for(k = 0; k < curCmdparamsCount3; k++){
				*(listPtrOut) = *(rawStandaloneArgBufferCopy);
				listPtrOut++;
				rawStandaloneArgBufferCopy++;
				packedCommandCount++; //each param is 4 bytes
			}
		}

		//copy last param
		*(listPtrOut) = *(rawStandaloneArgBufferCopy);
		listPtrOut++;
		rawStandaloneArgBufferCopy++;
		packedCommandCount++; //each param is 4 bytes

		
		//Update Packed Command + args count
		*outRawPackedDisplayList = (u32)(packedCommandCount*4);
		
		TGDSARM9Free(rawStandaloneCmdBuffer);
		TGDSARM9Free(rawStandaloneArgBuffer);
		TGDSARM9Free(rawARGBuffer);
		return true;
	}
	return false;
}
#endif

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

#ifdef WIN32
int main(int argc, char** argv){
	//Quick Unit Test Triangle rendering example: direct OpenGL commands, running in either WIN32 or NDS GX hardware
	//Simple Triangle GL init
	
	float rotateX = 0.0;
	float rotateY = 0.0;
	{
		//set mode 0, enable BG0 and set it to 3D
		#ifdef ARM9
		SETDISPCNT_MAIN(MODE_0_3D);
		#endif
		//this should work the same as the normal gl call
		glViewport(0,0,255,191);
		
		glClearColor(0,0,0);
		glClearDepth(0x7FFF);
		
	}
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
		glTranslate3f32(0, 0, floattof32(-1)); //bugged func 1 //so far here
					
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
	

	// OpenGL 1.1 Dynamic Display List 
	//ReSizeGLScene(255, 191);
	InitGL();
	
	while (1){
		DrawGLScene();
	}

	//Unit Test #1: Tests OpenGL DisplayLists components functionality then emitting proper GX displaylists, unpacked format.
	int list = glGenLists(10);
	if(list){
		glListBase(list);
		bool ret = glIsList(list); //should return false (DL generated, but no displaylist-name was generated)
		glNewList(list, GL_COMPILE);
		ret = glIsList(list); //should return true (DL generated, and displaylist-name was generated)
		if(ret == true){
			for (int i = 0; i <10; i ++){ //Draw 10 cubes
				glPushMatrix();
				glRotatef(36*i,0.0,0.0,1.0);
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
				glRotatef(18*i,0.0,0.0,1.0);
				glTranslatef(15.0,0.0,0.0);
				glPopMatrix(1);
			}
		}
		glEndList();//The second display list is created
	}
	
	u32 * CompiledDisplayListsBuffer =(u32 *)&InternalUnpackedGX_DL_Binary[InternalUnpackedGX_DL_OpenGLDisplayListStartOffset]; //Lists called earlier are written to this buffer, using the unpacked GX command format.
	//Unit Test #2:
	//Takes an unpacked format display list, gets converted into packed format then exported as C Header file source code
	char cwdPath[256];
	getCWDWin(cwdPath, "\\cv\\");
	char outPath[256];
	sprintf(outPath, "%s%s", cwdPath, "PackedDisplayList.h");
	bool result = packAndExportSourceCodeFromRawUnpackedDisplayListFormat(outPath, CompiledDisplayListsBuffer);
	if(result == true){
		//printf("Unpacked Display List successfully packed and exported as C source file at: \n%s", outPath);
	}
	else{
		//printf("Unpacked Display List generation failure");
	}
	
	//Unit Test #3: Using rawUnpackedToRawPackedDisplayListFormat() in target platform builds a packed Display List from an unpacked one IN MEMORY.
	//Resembles the same behaviour as if C source file generated in Unit Test #2 was then built through ToolchainGenericDS and embedded into the project.
	u32 Packed_DL_Binary[4096];
	memset(Packed_DL_Binary, 0, sizeof(Packed_DL_Binary));
	bool result2 = rawUnpackedToRawPackedDisplayListFormat(CompiledDisplayListsBuffer, (u32*)&Packed_DL_Binary[0]);
	if(result2 == true){
		//printf("Unpacked Display List into packed format: success!\n");
		//Save to file
		sprintf(outPath, "%s%s", cwdPath, "PackedDisplayListCompiled.bin");
		#ifdef WIN32
		FILE * fout = fopen(outPath, "w+b");
		#endif
		#ifdef ARM9
		FILE * fout = fopen(outPath, "w+");
		#endif
		if(fout != NULL){
			int packedSize = Packed_DL_Binary[0];
			int written = fwrite((u8*)&Packed_DL_Binary[0], 1, packedSize, fout);
			//printf("Written: %d bytes", packedSize);
			fclose(fout);
		}
	}
	else{
		//printf("Unpacked Display List into packed format: failure!\n");
	}

	//Unit Test #4: glCallLists test
	GLuint index = glGenLists(10);  // create 10 display lists
	GLubyte lists[10];              // allow maximum 10 lists to be rendered

	//Compile 10 display lists
	int DLCount = 0;
	for(DLCount = 0; DLCount < 10; DLCount++){
		glNewList(index + DLCount, GL_COMPILE);   // compile each one until the 10th

		for (int i = 0; i <20; i ++){ //Draw 20 triangles
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
	glCallLists(10, GL_UNSIGNED_BYTE, lists); //only OpenGL Display List names set earlier will run!

	//Unit Test #5: glDeleteLists test
	glDeleteLists(index, 5); //remove 5 of them
	*/
	return 0;
}
#endif

/*
//returns:
//	true: ndsDisplayListUtils behaves 1:1 on NDS hardware and Visual Studio, and it's 100% guaranteed to work in this session.
//	false: ndsDisplayListUtils on NDS hardware was wrongly compiled in this session, rebuild again.
#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
#endif
bool isNDSDLUtilsAPIStable(){
	bool ret = false;
	unsigned int crc32source = 0;
	int sourceFileSize = 0;
	#ifdef WIN32
	char cwdPathMP4[256];
	getCWDWin(cwdPathMP4, testSourceFileLocation);
	FILE * outFileGen = fopen(cwdPathMP4, "rb");
	if(outFileGen != NULL){
		fseek(outFileGen, 0, SEEK_END);
		sourceFileSize = ftell(outFileGen);
		fseek(outFileGen, 0, SEEK_SET);

		crc32source = -1;
		int err = crc32file(outFileGen, &crc32source);
		fclose(outFileGen);
	}
	#endif
	
	#ifdef ARM9
	crc32source = 0;
	sourceFileSize = PackedDisplayListCompiledVS2012_size;	//(int)PackedDisplayListCompiledVS2012[0]; //<--- this is PACKED SIZE, SINCE IT'S PACKED, IT DOESN'T GENERATE THE SAME UNPACKED OUTPUT AS NORMAL NDSDLUTILSCALLS
	crc32source = crc32(&crc32source, (u8*)&PackedDisplayListCompiledVS2012[0], PackedDisplayListCompiledVS2012_size);
	#endif
	//Packed GX Command list generated from VS2012 must be the same as the one dinamically generated on runtime (NDS/VS2012)
	if(sourceFileSize > InternalUnpackedGX_DL_workSize){
		sourceFileSize = InternalUnpackedGX_DL_workSize;
	}
	int list = glGenLists(10);
	if(list){
		glListBase(list);
		bool ret = glIsList(list); //should return false (DL generated, but no displaylist-name was generated)
		glNewList(list, GL_COMPILE);
		ret = glIsList(list); //should return true (DL generated, and displaylist-name was generated)
		if(ret == true){
			for (int i = 0; i <10; i ++){ //Draw 10 cubes
				glPushMatrix();
				glRotatef(36*i,0.0,0.0,1.0);
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
				glRotatef(18*i,0.0,0.0,1.0);
				glTranslatef(15.0,0.0,0.0);
				glPopMatrix(1);
			}
		}
		glEndList();//The second display list is created
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	u32 * CompiledDisplayListsBuffer = getInternalUnpackedDisplayListBuffer_OpenGLDisplayListBaseAddr(); //Lists called earlier are written to this buffer, using the unpacked GX command format.
	u32 TestPacked_DL_Binary[InternalUnpackedGX_DL_workSize];
	memset((u8*)&TestPacked_DL_Binary[0], 0, sourceFileSize);
	bool result2 = rawUnpackedToRawPackedDisplayListFormat(CompiledDisplayListsBuffer, (u32*)TestPacked_DL_Binary);
	if(result2 == true){
		unsigned int crc32dest = 0;
		int packedSize = (int)TestPacked_DL_Binary[0];
		crc32dest = crc32(&crc32dest, (u8*)&TestPacked_DL_Binary[0], packedSize);
		if(crc32source == crc32dest){
			ret = true;
		}
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef ARM9
//Unit Test (NDS): reads a NDS GX Display List / Call List payload emmited from (https://bitbucket.org/Coto88/blender-nds-exporter/src/master/)
//Translates to an NDS GX Display List / Call List object, and then builds it again into the original payload.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os"))) __attribute__((section(".itcm")))
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
			//printf("GX DisplayListType: GL_TRIANGLES");
		}
		else if (type == GL_QUADS){
			//printf("GX DisplayListType: GL_QUADS");
		}
		else if (type == GL_TRIANGLE_STRIP){
			//printf("GX DisplayListType: GL_TRIANGLE_STRIP");
		}
		else if (type == GL_QUAD_STRIP){
			//printf("GX DisplayListType: GL_QUAD_STRIP");
		}
		else{
			//printf("GX DisplayListType: ERROR");
		}

		//Get Binary filesize
		int displayListSize = getRawFileSizefromNDSGXDisplayListObject(NDSDL);

		//List commands
		struct ndsDisplayListDescriptor * NDSDLColorCmds = (struct ndsDisplayListDescriptor *)TGDSARM9Malloc(sizeof(struct ndsDisplayListDescriptor));
		getDisplayListFIFO_BEGIN(NDSDL, NDSDLColorCmds);
		int FIFO_BEGINCount = NDSDLColorCmds->ndsDisplayListSize;
		//printf("GX DisplayList (%d): FIFO_BEGIN commands ", FIFO_BEGINCount);
		getDisplayListFIFO_COLOR(NDSDL, NDSDLColorCmds);
		//printf("GX DisplayList (%d): FIFO_COLOR commands", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_TEX_COORD(NDSDL, NDSDLColorCmds);
		//printf("GX DisplayList (%d): FIFO_TEX_COORD commands", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_VERTEX16(NDSDL, NDSDLColorCmds);
		//printf("GX DisplayList (%d): FIFO_VERTEX16 commands", NDSDLColorCmds->ndsDisplayListSize);
		getDisplayListFIFO_END(NDSDL, NDSDLColorCmds);
		int FIFO_ENDCount = NDSDLColorCmds->ndsDisplayListSize;
		//printf("GX DisplayList (%d): FIFO_END commands", FIFO_ENDCount);
		
		if((FIFO_BEGINCount != 0) && (FIFO_ENDCount != 0) && (FIFO_BEGINCount == FIFO_ENDCount) ){
			//printf("(%d) GX DisplayList detected", FIFO_BEGINCount);
		}
		else{
			//printf("BROKEN GX DisplayList: It won't work as expected on real Nintendo DS!");
		}
		TGDSARM9Free(NDSDLColorCmds);

		//Build it
		u32 * builtDisplayList = (u32*)TGDSARM9Malloc(displayListSize);
		memset(builtDisplayList, 0, displayListSize);
		int builtDisplayListSize = CompilePackedNDSGXDisplayListFromObject(builtDisplayList, NDSDL) + 1;
		
		if(dlReadSize != builtDisplayListSize){
			//printf("NDS DisplayList Build failed");
		}
		//printf("Rebuilding DisplayList at: %s", outNDSGXBuiltDisplayList);
		FILE * fout = fopen(outNDSGXBuiltDisplayList, "w+");
		if(fout != NULL){
			if(fwrite(builtDisplayList, 1, displayListSize, fout) > 0){
				//printf("Rebuild OK");
				retStatus = true;
			}
			else{
				//printf("Rebuild ERROR");
			}
			fclose(fout);
		}
		TGDSARM9Free(builtDisplayList);
	}
	else{
		//printf("NDS DisplayList creation fail");
	}
	TGDSARM9Free(NDSDL);
	return retStatus;
}
#endif
*/
