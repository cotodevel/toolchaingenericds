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

#ifndef __ndsDisplayListUtils_h__
#define __ndsDisplayListUtils_h__

#ifdef WIN32
#include "TGDSTypes.h"
#endif

#ifdef ARM9
#include "typedefsTGDS.h"
#endif

#include "VideoGL.h"

//NDS GX C Display List defines
struct unpackedCmd {
	u8 cmd1;
	u8 cmd2;
	u8 cmd3;
	u8 cmd4;
};

//Display List Descriptor
#define DL_INVALID (u32)(-1)
#define DL_TYPE_SIZE (u32)(1)
#define DL_TYPE_FIFO_PACKED_COMMAND_V1 (u32)(DL_TYPE_SIZE+1)	//FIFO_COMMAND_PACK( FIFO_BEGIN , FIFO_COLOR , FIFO_TEX_COORD , FIFO_NORMAL )
#define DL_TYPE_FIFO_PACKED_COMMAND_V2 (u32)(DL_TYPE_FIFO_PACKED_COMMAND_V1+1)	//FIFO_COMMAND_PACK( FIFO_VERTEX16 , FIFO_COLOR , FIFO_TEX_COORD , FIFO_NORMAL )
#define DL_TYPE_FIFO_PACKED_COMMAND_END (u32)(DL_TYPE_FIFO_PACKED_COMMAND_V2+1)	//FIFO_COMMAND_PACK( FIFO_VERTEX16 , FIFO_END , FIFO_NOP , FIFO_NOP )
#define DL_MAX_ITEMS (int)(256) //256 NDS GX commands(words, 4 bytes each) = 1024 per DisplayList object

struct ndsDisplayList {
	int index;
	u32 displayListType; //Display List Descriptor: FIFO_BEGIN, FIFO_COLOR, FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_END, FIFO_NOP, etc... 
	u32 value; //RGB15(31,31,31), TEXTURE_PACK(floattot16(0.000000),floattot16(128.000000)), NORMAL_PACK(floattov10(0.577349),floattov10(0.577349),floattov10(-0.577349)), VERTEX_PACK(floattov16(1.000000),floattov16(1.000000)) , VERTEX_PACK(floattov16(-1.000000),0), FIFO_END (no value after), FIFO_NOP (no value)
} 
#ifdef ARM9
__attribute__((packed)) ;
#endif
#ifdef WIN32
;
#endif

struct ndsDisplayListDescriptor {
	int DisplayListNameAssigned; //Used by the GL List API as a display-list name
	bool isDisplayListAssigned;
	int ndsDisplayListSize;
	struct ndsDisplayList DL[DL_MAX_ITEMS];
}
#ifdef ARM9
__attribute__((packed)) ;
#endif
#ifdef WIN32
;
#endif

#endif

#ifdef __cplusplus 
extern "C" {
#endif

extern u32 ID2REG_C(u8 val);
extern u32 REG2ID_C(u32 val);
extern u32 FIFO_COMMAND_PACK_C(u8 c1, u8 c2, u8 c3, u8 c4);
extern u8 getFIFO_NOP();
extern u8 getFIFO_STATUS();
extern u8 getFIFO_COLOR();
extern u8 getFIFO_VERTEX16();
extern u8 getFIFO_TEX_COORD();
extern u8 getFIFO_TEX_FORMAT();
extern u8 getFIFO_CLEAR_COLOR();
extern u8 getFIFO_CLEAR_DEPTH();
extern u8 getFIFO_LIGHT_VECTOR();
extern u8 getFIFO_LIGHT_COLOR();
extern u8 getFIFO_NORMAL();
extern u8 getFIFO_DIFFUSE_AMBIENT();
extern u8 getFIFO_SPECULAR_EMISSION();
extern u8 getFIFO_SHININESS();
extern u8 getFIFO_POLY_FORMAT();
extern u8 getFIFO_BEGIN();
extern u8 getFIFO_END();
extern u8 getFIFO_FLUSH();
extern u8 getFIFO_VIEWPORT();
extern u8 getMTX_MULT_3x3();
extern u8 getMTX_IDENTITY();
extern u8 getMTX_SCALE();

extern u8 getMTX_PUSH();
extern u8 getMTX_POP();
extern u8 getMTX_TRANS();
extern struct unpackedCmd FIFO_COMMAND_PACKED_FMT_UNPACK(u32 cmd);
extern int CompilePackedNDSGXDisplayListFromObject(u32 * bufOut, struct ndsDisplayListDescriptor * dlInst);
extern int BuildNDSGXDisplayListObjectFromFile(char * filename, struct ndsDisplayListDescriptor * dlInst);
extern int getRawFileSizefromNDSGXDisplayListObject(struct ndsDisplayListDescriptor * dlInst);
extern GL_GLBEGIN_ENUM getDisplayListGLType(struct ndsDisplayListDescriptor * dlInst);

extern bool getDisplayListFilterByCommand(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut, u8 targetCommand);
extern bool getDisplayListFIFO_COLOR(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut);
extern bool getDisplayListFIFO_TEX_COORD(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut);
extern bool getDisplayListFIFO_VERTEX16(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut);
extern bool getDisplayListFIFO_BEGIN(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut);
extern bool getDisplayListFIFO_END(struct ndsDisplayListDescriptor * dlInst, struct ndsDisplayListDescriptor * dlInstOut);

#ifdef ARM9
extern bool ndsDisplayListUtilsTestCaseARM9(char * filename, char * outNDSGXBuiltDisplayList);
#endif

extern bool isAGXCommand(u32 val);
extern u8 clzero(u32 var);
#ifdef __cplusplus 
}
#endif

#endif