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

#ifndef __mem_handler_h__
#define __mem_handler_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "reent.h"

#ifdef ARM9
#define nds_ewram_mask (get_ewram_size()-1)
#endif

//IPC specific: Shared Work     027FF000h 4KB    -     -    -    R/W
#define IPCRegionSize	(sint32)(4*1024)

 
#endif

#ifdef __cplusplus
extern "C"{
#endif


#ifdef ARM7
extern uint32 _iwram_start;
extern uint32 _iwram_end;
extern uint32 get_iwram_start();
extern sint32 get_iwram_size();
extern uint32 get_iwram_end();
#endif

//newlib
extern uint32 get_lma_libend();		//linear memory top
extern uint32 get_lma_wramend();	//(ewram end - linear memory top ) = malloc free memory

//initNDS.c -> initHardware(); init these, dont call them before.
extern uint32 get_arm7_start_address();
extern uint32 get_arm7_end_address();
extern sint32 get_arm7_ext_size();

extern uint32 get_arm9_start_address();
extern uint32 get_arm9_end_address();
extern sint32 get_arm9_ext_size();
extern sint32 get_arm7_printfBufSize();

#ifdef ARM9
//linker script hardware address setup (get map addresses from linker file)
extern uint32 	_ewram_start;
extern uint32	_ewram_end;
extern uint32 	get_ewram_start();
extern sint32 	get_ewram_size();

extern uint32 	_dtcm_start;
extern uint32 	_dtcm_end;
extern uint32 	get_dtcm_start();
extern sint32 	get_dtcm_size();

extern uint32 	_gba_start;
extern uint32 	_gba_end;

extern uint32	_gbawram_start;
extern uint32	_gbawram_end;

extern uint32 	_itcm_start;
extern uint32 	_itcm_end;
extern uint32 	get_itcm_start();
extern sint32 	get_itcm_size();

extern uint32 	_vector_start;
extern uint32 	_vector_end;

#endif

//top NDS EWRAM allocated by toolchain for libraries.
extern uint32 	__lib__end__;		//start free linear memory (malloc starts here)
extern uint32 	__vma_stub_end__;	//__lib__end__ + itcm + dtcm + any other sections

//CP15 MPU 
extern void MPUSet();

extern void * _sbrk (int size);
extern void * _sbrk_r (struct _reent * reent, int size);

extern void Write32bitAddrExtArm(uint32 address, uint32 value);
extern void Write16bitAddrExtArm(uint32 address, uint16 value);
extern void Write8bitAddrExtArm(uint32 address, uint8 value);

extern volatile char 		*heap_end;
extern volatile char        *prev_heap_end;

#ifdef ARM9
//NDS Memory Map
extern bool isValidMap(uint32 addr);
#endif

#ifdef __cplusplus
}
#endif
