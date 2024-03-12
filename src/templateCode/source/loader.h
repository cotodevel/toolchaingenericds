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