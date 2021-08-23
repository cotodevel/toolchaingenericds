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

#ifndef __DldiBulk_h__
#define __DldiBulk_h__

#if defined(ARM7) || defined(ARM9)
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "limitsTGDS.h"
#endif

#ifdef ARM9
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "dldi.h"
#endif

#ifdef ARM7
#define BulkBufferARM7 (u32)(0x0601F000) //last 4k
#endif

#if defined(WIN32)
#include "..\..\..\..\ToolchainGenericDSFS\fatfslayerTGDS.h"
#include "TGDSTypes.h"
#include "..\..\..\..\ToolchainGenericDSFS\dldi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(ARM7)
extern int DldiBulkReadFromFileIntoEwramArm7(int fileOffsetInFileHandle, u8 * EwramSourceBuffer, int readSize, u32 wholeFileSize, int sectorSize, int sectorsPerCluster, u32* sector_tablePtrUser);
#endif

#if defined(WIN32) || defined(ARM9)
extern int DldiBulkReadSetupFromTGDSVideoFile(struct fd * videoHandleFD, u32* sector_table);
extern int DldiBulkReadFromFileIntoEwramArm9(int fileOffsetInFileHandle, u8 * EwramSourceBuffer, int readSize, struct fd * StructFDHandle, u32 * sector_table);
#endif

#if defined(WIN32)
extern void clrscr();
#endif

#ifdef __cplusplus
}
#endif

#endif

