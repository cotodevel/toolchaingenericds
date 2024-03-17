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

//TGDS-MB v3 VRAM Loader
#define BOOT_FILE_TGDSMB ((u32)0xFF883232)
#define FIFO_ARM7_RELOAD (u32)(0xFFFFABCA)
#define TGDS_MB_V3_WORKBUFFER ((int)(0x02000000 + (512*1024)))
#define TGDS_MB_V3_MEMBASE ((int)0x02FFE000) //can't be 0x02FFF000 or 0x027FF000 due to shared ram IO and header section otherwise breaks stuff
#define ARM9_STRING_PTR ((u32*)(TGDS_MB_V3_MEMBASE-(4*0)))
#define ARM9_BOOT_SIZE ((u32*)(TGDS_MB_V3_MEMBASE-(4*1)))
#define ARM7_BOOT_SIZE ((u32*)(TGDS_MB_V3_MEMBASE-(4*2)))
#define ARM7_BOOTCODE_OFST ((u32*)(TGDS_MB_V3_MEMBASE-(4*3)))
#define ARM9_BOOTCODE_OFST ((u32*)(TGDS_MB_V3_MEMBASE-(4*4)))
#define ARM9_TWLORNTRPAYLOAD_MODE ((u32*)(TGDS_MB_V3_MEMBASE-(4*5)))
#define ARM7_ARM9_DLDI_STATUS ((u32*)(TGDS_MB_V3_MEMBASE-(4*6))) //0x02FFDFE8 global TGDS ARM7DLDI register
#define ARM7_ARM9_SAVED_DSFIRMWARE ((u32*)(TGDS_MB_V3_MEMBASE-(4*7)))	//0x02FFDFE4
#define ARM7i_HEADER_SCFG_EXT7 ((u32*)(TGDS_MB_V3_MEMBASE-(4*8)))	//1B8h 4    ARM7 SCFG_EXT7 setting (bit0,1,2,10,18,31) //Note: TWL Header does not have a SCFG_EXT9 section due to important custom layout memory for TWL mode is being set through SCFG_EXT7 flags such as "Access to New Shared WRAM" as well "Access to 2nd NDS Cart Slot". The rest is entirely ARM9i dependant for TWL circuitry and does not affect IO memory mapping for execution, but for extended cuircuitry features not needed for base TWL homebrew. 

#define ARM9i_RAM_ADDRESS ((u32*)(TGDS_MB_V3_MEMBASE-(4*9)))
#define ARM9i_BOOT_SIZE ((u32*)(TGDS_MB_V3_MEMBASE-(4*10)))
#define ARM7i_RAM_ADDRESS ((u32*)(TGDS_MB_V3_MEMBASE-(4*11)))
#define ARM7i_BOOT_SIZE ((u32*)(TGDS_MB_V3_MEMBASE-(4*12)))

//The base address all four ARM7/ARM9/ARM7i/ARM9i should have at least to be ran through TGDS-MB v3
#define ARM_MININUM_LOAD_ADDR ((u32)0x01000000)


//tgds_mb_payload.bin (NTR/TWL): 
//304K  0x02280000 -> 0x02400000 - 304K (0x4C000) = Entrypoint: 0x023B0000
//Workram (0x02000000 ~ 0x80000) = 512K 
#define TGDS_MB_V3_PAYLOAD_ADDR ((u32*)0x023B0000)


#endif


#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM9
extern u32 reloadStatus;
extern bool TGDSMultibootRunNDSPayload(char * filename);
#endif

#ifdef __cplusplus
}
#endif