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

#ifndef __ndsDisplayListUtils_h__
#define __ndsDisplayListUtils_h__

//GX Payload source sample: PackedDisplayListCompiled.bin: 
//Packed Display List commands generated from VS2012 by running unit tests 1,2,3,4 and 5 
//(emited DL unpacked, then packed to a GX binary, whose dump ends up being this file)

#ifdef WIN32
#include "TGDSTypes.h"
#define testSourceFileLocation (char*)"\\cv\\PackedDisplayListCompiled.bin"
#endif

#ifdef ARM9
#include "typedefsTGDS.h"
//testSourceFileLocation is embedded into ARM9
#endif

#include "VideoGL.h"

//NDS GX C Display List defines
struct unpackedCmd {
	u8 cmd1;
	u8 cmd2;
	u8 cmd3;
	u8 cmd4;
};

#define CRC16_POLYNOMIAL 0xa001
#define CRC32_POLYNOMIAL 0xedb88320

#endif

extern u32 ID2REG_C(u8 val);
extern u32 REG2ID_C(u32 val);
extern u32 FIFO_COMMAND_PACK_C(u8 c1, u8 c2, u8 c3, u8 c4);
extern u8 getFIFO_NOP();
extern u8 getFIFO_STATUS();
extern u8 getFIFO_COLOR();
extern u8 getFIFO_VERTEX10();
extern u8 getFIFO_VERTEX16();
extern u8 getFIFO_TEX_COORD();
extern u8 getFIFO_VTX_XY();
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
extern u8 getMTX_MULT_4x4();
extern u8 getMTX_SCALE();

extern u8 getMTX_PUSH();
extern u8 getMTX_POP();
extern u8 getNOP();
extern u8 getMTX_TRANS();
extern u8 getMTX_STORE();
extern u8 getMTX_IDENTITY();
extern u8 getMTX_LOAD_4x4();
extern u8 getMTX_LOAD_4x3();
extern u8 getMTX_MODE();
extern u8 getVIEWPORT();
extern struct unpackedCmd FIFO_COMMAND_PACKED_FMT_UNPACK(u32 cmd);
extern int BuildNDSGXDisplayListObjectFromFile(char * filename, struct ndsDisplayListDescriptor * dlInst);
extern int getRawFileSizefromNDSGXDisplayListObject(struct ndsDisplayListDescriptor * dlInst);
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
extern bool packAndExportSourceCodeFromRawUnpackedDisplayListFormat(char * filenameOut, u32 * rawUnpackedDisplayList);
extern bool rawUnpackedToRawPackedDisplayListFormat(u32 * inRawUnpackedDisplayList, u32 * outRawPackedDisplayList);
extern int getAGXParamsCountFromCommand(u32 command);

extern void swap1(char *x, char *y);
extern char* reverse1(char *buffer, int i, int j);
extern char* itoa1(int value, char* buffer, int base);
extern bool isNDSDLUtilsAPIStable();

//crc32 bits
extern void init_crc_table (void *table, unsigned int polynomial);
extern unsigned int *crc32_table;
extern void free_crc32_table (void);
extern unsigned int crc32 (unsigned int *crc, const void *buffer, unsigned int size);
extern int crc32file( FILE *file, unsigned int *outCrc32);
extern GLuint	texture[1];			// Storage For 1 Texture
extern GLuint	box;				// Storage For The Box Display List
extern GLuint	top;				// Storage For The Top Display List
extern GLuint	xloop;				// Loop For X Axis
extern GLuint	yloop;				// Loop For Y Axis

extern GLfloat	xrot;				// Rotates Cube On The X Axis
extern GLfloat	yrot;				// Rotates Cube On The Y Axis

extern GLfloat boxcol[5][3];
extern GLfloat topcol[5][3];
extern GLvoid BuildLists();
extern GLvoid ReSizeGLScene(GLsizei width, GLsizei height);
extern int InitGL();
extern int DrawGLScene();
