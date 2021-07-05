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

#ifndef __loader_common_h__
#define __loader_common_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "utilsTGDS.h"
#include "biosTGDS.h"

//loader defs
#define ARM7COMMAND_RELOADNDS (u32)(0xFFFFFF01)
#define NDSLOADER_LOAD_OK (u32)(0xFF222219)	//File read OK -> (NDSLoader context generated properly) ARM7 must reload ARM7.bin into arm7 target address
#define NDSLOADER_START (u32)(0xFF22221A)	//Reload ARM7.bin -> ARM7 target addresses && ARM9.bin -> ARM9 target addresses and boot NDS homebrew
#define NDSLOADER_INITDLDIARM7_BUSY (u32)(0xFF222221)	//DLDI SETUP ARM9 -> ARM7 Taking place...
#define NDSLOADER_INITDLDIARM7_DONE (u32)(0xFF222224)	//DLDI SETUP ARM9 -> ARM7 Done!

//other misc loader defs (not related to tgds-multiboot)
#define ARM7COMMAND_RELOADARM7 (u32)(0xFFFFFF02)

struct ndsloader_s
{
	//raw NDS Binary
	u32 sectorTableBootCode[(4 * 1024 * 1024)/512];	//32K bytes in sectors = 4MB file addressed (NDS max binary size)
	int fileSize;
	
	//ARM7 BootCode
	int bootCode7FileSize;
	u32 arm7EntryAddress;
	int sectorOffsetStart7;
	int sectorOffsetEnd7;
	u32 arm7BootCodeOffsetInFile;
	
	//ARM9 BootCode
	int bootCode9FileSize;
	u32 arm9EntryAddress;
	int sectorOffsetStart9;
	int sectorOffsetEnd9;
	u32 arm9BootCodeOffsetInFile;
	
	//SD specific
	int sectorsPerCluster;
	int sectorSize;
	int ndsloaderInitStatus;
};

//Shared IPC : 

//NDSLoaderContext = EWRAM - 33K shared IPC (NDS_LOADER_IPC_CTX_SIZE)
#define NDS_LOADER_IPC_CTX_SIZE (48*1024)	//about 34K used anyway, but for addressing purposes we align it

#define NDS_LOADER_IPC_CTXADDR		(0x02400000 - (int)NDS_LOADER_IPC_CTX_SIZE)	//0x023F4000
#define NDS_LOADER_IPC_CTX_CACHED ((struct ndsloader_s*)NDS_LOADER_IPC_CTXADDR)
#define NDS_LOADER_IPC_CTX_UNCACHED_NTR ((struct ndsloader_s*)(NDS_LOADER_IPC_CTXADDR | 0x400000))
#define NDS_LOADER_IPC_CTX_UNCACHED_TWL ((struct ndsloader_s*)(NDS_LOADER_IPC_CTXADDR | 0x400000))

//ARM7.bin
#define NDS_LOADER_IPC_ARM7BIN_CACHED		(NDS_LOADER_IPC_CTXADDR - (96*1024))	//0x023DC000
#define NDS_LOADER_IPC_ARM7BIN_UNCACHED_NTR		(NDS_LOADER_IPC_ARM7BIN_CACHED | 0x400000)
#define NDS_LOADER_IPC_ARM7BIN_UNCACHED_TWL		(NDS_LOADER_IPC_ARM7BIN_CACHED | 0x400000)

//ARM7 NDSLoader code: Bootstrap that reloads ARM7.bin into arm7 EntryAddress (arm7bootldr/arm7bootldr.bin)
#define NDS_LOADER_IPC_BOOTSTUBARM7_CACHED		(NDS_LOADER_IPC_ARM7BIN_CACHED - (96*1024))	//0x023C4000	-> same as IWRAMBOOTCODE	(rwx)	: ORIGIN = 0x023C4000, LENGTH = 96K (arm7bootldr.bin) entry address
#define NDS_LOADER_IPC_BOOTSTUBARM7_UNCACHED_NTR 	(NDS_LOADER_IPC_BOOTSTUBARM7_CACHED | 0x400000)
#define NDS_LOADER_IPC_BOOTSTUBARM7_UNCACHED_TWL 	(NDS_LOADER_IPC_BOOTSTUBARM7_CACHED | 0x400000)

//ARM7 DLDI section code
#define NDS_LOADER_DLDISECTION_CACHED		(NDS_LOADER_IPC_BOOTSTUBARM7_CACHED - (16*1024))	//0x023AC000
#define NDS_LOADER_DLDISECTION_UNCACHED_NTR 	(NDS_LOADER_DLDISECTION_CACHED | 0x400000)
#define NDS_LOADER_DLDISECTION_UNCACHED_TWL 	(NDS_LOADER_DLDISECTION_CACHED | 0x400000)

static inline void reloadARMCore(u32 targetAddress){
	REG_IF = REG_IF;	// Acknowledge interrupt
	REG_IE = 0;
	REG_IME = IME_DISABLE;	// Disable interrupts
	swiDelay(800);
	swiSoftResetByAddress(targetAddress);	// Jump to boot loader
}

static inline void runBootstrapARM7(void){
	#ifdef ARM9
	SendFIFOWords(ARM7COMMAND_RELOADNDS);
	#endif
	
	#ifdef ARM7
	reloadARMCore((u32)NDS_LOADER_IPC_BOOTSTUBARM7_CACHED);	//Run Bootstrap7 
	#endif	
}

#endif


#ifdef __cplusplus
extern "C"{
#endif

extern void setNDSLoaderInitStatus(int ndsloaderStatus);
extern void waitWhileNotSetStatus(u32 status);	//[Blocking]: Local ARM Core waits until External action takes place, waits while resolving internal NDS hardware wait states.

#ifdef ARM9
extern void ARM7JumpTo(u32 ARM7Entrypoint);
#endif

#ifdef __cplusplus
}
#endif