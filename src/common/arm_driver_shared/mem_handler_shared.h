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

#include "typedefs.h"
#include "dsregs.h"

//define for own linear allocator
//undefine for default newlib malloc, linear heap implementation. If unsure just leave this undefined
//#define own_allocator

#include "reent.h"


#ifdef own_allocator
#define alloc_failed (uint32)(0xffffc070)
#endif

#define nds_ewram_mask (get_ewram_size()-1)

#endif

#ifdef __cplusplus
extern "C"{
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

#ifdef ARM7
extern uint32 _iwram_start;
extern uint32 _iwram_end;
extern uint32 get_iwram_start();
extern sint32 get_iwram_size();
extern uint32 get_iwram_end();
#endif

#ifdef ARM9
//linker script hardware address setup (get map addresses from linker file)
extern uint32 	_ewram_start;
extern uint32	_ewram_end;
extern uint32 get_ewram_start();
extern sint32 get_ewram_size();


extern uint32 	_dtcm_start;
extern uint32 	_dtcm_end;
extern uint32 	get_dtcm_start();
extern sint32 	get_dtcm_size();

extern uint32 	_gba_start;
extern uint32 	_gba_end;
//todo: read from gba cart or something

extern uint32 	_itcm_start;
extern uint32 	_itcm_end;
extern uint32 	get_itcm_start();
extern sint32 	get_itcm_size();

extern uint32 	_vector_start;
extern uint32 	_vector_end;

#endif

//top NDS EWRAM allocated by toolchain for libraries.
extern uint32 	__lib__end__;		//vma_idtcm_start
extern uint32 	__vma_stub_end__;	//vma_idtcm_end

//CP15 MPU 
extern void MPUSet();


#ifdef own_allocator
extern void sbrk_init();
extern sint32 calc_heap(sint32 size);
extern uint32 * this_heap_ptr;
//must be signed
extern sint32 get_available_mem();
#endif

extern void * _sbrk (int size);

//MyIPC unaligned
extern uint32 getToolchainIPCAddress();		
extern sint32 getToolchainIPCSize();

//AlignIPC aligned
extern uint32 getToolchainAlignedIPCAddress();	
extern sint32 getToolchainAlignedIPCSize();

//Project Specific IPC
extern uint32 getUserIPCAddress();

//Printf7 Buffer
extern uint32 getPrintfBuffer();
#ifdef __cplusplus
}
#endif
