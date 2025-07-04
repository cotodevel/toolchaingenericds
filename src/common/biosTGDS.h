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

/*----------------------------------------------------------------------------*/
/*--  lzss.c - LZSS coding for Nintendo GBA/DS                              --*/
/*--  Copyright (C) 2011 CUE                                                --*/
/*--                                                                        --*/
/*--  This program is free software: you can redistribute it and/or modify  --*/
/*--  it under the terms of the GNU General Public License as published by  --*/
/*--  the Free Software Foundation, either version 3 of the License, or     --*/
/*--  (at your option) any later version.                                   --*/
/*--                                                                        --*/
/*--  This program is distributed in the hope that it will be useful,       --*/
/*--  but WITHOUT ANY WARRANTY; without even the implied warranty of        --*/
/*--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          --*/
/*--  GNU General Public License for more details.                          --*/
/*--                                                                        --*/
/*--  You should have received a copy of the GNU General Public License     --*/
/*--  along with this program. If not, see <http://www.gnu.org/licenses/>.  --*/
/*----------------------------------------------------------------------------*/

#ifndef __nds_bios_h__
#define __nds_bios_h__

#include "typedefsTGDS.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "keypadTGDS.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define swiDelay1ms (sint32)(8388)

// CUE Author Code Start

//LZSS
struct LZSSContext {
	u8 * bufferSource;
	int bufferSize;
};

/*----------------------------------------------------------------------------*/
#define CMD_DECODE    0x00       // decode
#define CMD_CODE_10   0x10       // LZSS magic number

#define LZS_NORMAL    0x00       // normal mode, (0)
#define LZS_FAST      0x80       // fast mode, (1 << 7)
#define LZS_BEST      0x40       // best mode, (1 << 6)

#define LZS_WRAM      0x00       // VRAM not compatible (LZS_WRAM | LZS_NORMAL)
#define LZS_VRAM      0x01       // VRAM compatible (LZS_VRAM | LZS_NORMAL)
#define LZS_WFAST     0x80       // LZS_WRAM fast (LZS_WRAM | LZS_FAST)
#define LZS_VFAST     0x81       // LZS_VRAM fast (LZS_VRAM | LZS_FAST)
#define LZS_WBEST     0x40       // LZS_WRAM best (LZS_WRAM | LZS_BEST)
#define LZS_VBEST     0x41       // LZS_VRAM best (LZS_VRAM | LZS_BEST)

#define LZS_SHIFT     1          // bits to shift
#define LZS_MASK      0x80       // bits to check:
                                 // ((((1 << LZS_SHIFT) - 1) << (8 - LZS_SHIFT)

#define LZS_THRESHOLD 2          // max number of bytes to not encode
#define LZS_N         0x1000     // max offset (1 << 12)
#define LZS_F         0x12       // max coded ((1 << 4) + LZS_THRESHOLD)
#define LZS_NIL       LZS_N      // index for root of binary search trees

#define RAW_MINIM     0x00000000 // empty file, 0 bytes
#define RAW_MAXIM     0x00FFFFFF // 3-bytes length, 16MB - 1

#define LZS_MINIM     0x00000004 // header only (empty RAW file)
#define LZS_MAXIM     0x01400000 // 0x01200003, padded to 20MB:
                                 // * header, 4
                                 // * length, RAW_MAXIM
                                 // * flags, (RAW_MAXIM + 7) / 8
                                 // 4 + 0x00FFFFFF + 0x00200000 + padding

//LZSS end
// CUE Author Code End

#ifdef ARM9
//Callbacks to use libutil's components in libhardware 7 or 9
typedef sint32(*HandledoMULTIDaemonWeakRefLibHardware9_fn)(); //doMULTIDaemon()
#endif

#endif

#ifdef __cplusplus
extern "C"{
#endif

//NDS7/9 Bios 
//ARM ASM NDS9 Bios (useful when vectors @ 0xffff0000 and manual processor vectors handling is required)
extern void swiDelay(uint32 delayvalue);
extern uint16 swiCRC16(uint16 crc, void * data, uint32 size);
extern void swiSleep(uint32 delayvalue);
extern int swiIsDebugger(void);
extern int swiDivide(int numerator, int divisor);
extern int swiRemainder(int numerator, int divisor);
extern void swiDivMod(int numerator, int divisor, int * result, int * remainder);
extern int swiSqrt(int value);
extern void swiSoftReset(void);

//NDS7 Bios 
#ifdef ARM7
extern void swiHalt(void);
extern uint16 swiGetSineTable(int index);
extern uint16 swiGetPitchTable(int index);
extern uint8 swiGetVolumeTable(int index);
extern void swiSetHaltCR(uint32 data);
extern void swiChangeSoundBias(int enabled, int delay);	//same as void swiChangeSndBias(int enable, int delayvalue);
#endif

extern struct LZSSContext LZS_DecodeFromBuffer(unsigned char *pak_buffer, unsigned int   pak_len);
extern void swiDecompressLZSSWram(void * source, void * destination);
extern void swiFastCopy(uint32 * source, uint32 * dest, int flags);

//weak symbols : the implementation of this is project-defined
extern void onThreadOverflowUserCode(u32 * args);

//Init SVCs
#ifdef ARM7
extern void handleARM7InitSVC();
extern void taskARM7SVC(u32 * args);
#endif

#ifdef ARM9
extern void handleARM9InitSVC();
extern void taskARM9SVC(u32 * args);

extern HandledoMULTIDaemonWeakRefLibHardware9_fn HandledoMULTIDaemonWeakRefLibHardware9Callback;

#endif

extern bool isArm7ClosedLid;

#ifdef __cplusplus
}
#endif
