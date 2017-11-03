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

#include "mem_handler_shared.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "reent.h"	//sbrk
#include "sys/types.h"

#include "common_shared.h"


//linker to C proper memory layouts.

//Shared:

//these toggle the WRAM_CR register depending on linker settings

uint32 get_arm7_start_address(){
	return (uint32)MyIPC->arm7startaddress;
}

uint32 get_arm7_end_address(){
	return (uint32)MyIPC->arm7endaddress;
}

sint32 get_arm7_ext_size(){
	return (sint32)((uint8*)(uint32*)get_arm7_end_address() - (sint32)(get_arm7_start_address()));
}


uint32 get_arm9_start_address(){
	return (uint32)MyIPC->arm9startaddress;
}

uint32 get_arm9_end_address(){
	return (uint32)MyIPC->arm9endaddress;
}

sint32 get_arm9_ext_size(){
	return (sint32)((uint8*)(uint32*)get_arm9_end_address() - (sint32)(get_arm9_start_address()));
}


#ifdef ARM7
uint32 get_iwram_start(){
	return (uint32)(&_iwram_start);
}

sint32 get_iwram_size(){
	return (sint32)((uint8*)(uint32*)&_iwram_end - (sint32)(&_iwram_start));
}

uint32 get_iwram_end(){
	return (uint32)(&_iwram_end);
}

#endif

uint32 get_lma_libend(){
	return (uint32)(&__vma_stub_end__);	//linear memory top (start)
}

//(ewram end - linear memory top ) = malloc free memory (bottom, end)
uint32 get_lma_wramend(){
	#ifdef ARM7
	return (uint32)(&_iwram_end);
	#endif
	#ifdef ARM9
	return (uint32)(&_ewram_end);	
	#endif
}

#ifdef ARM9


uint32 get_ewram_start(){
	return (uint32)(&_ewram_start);
}

sint32 get_ewram_size(){
	return (sint32)((uint8*)(uint32*)&_ewram_end - (sint32)(&_ewram_start));
}

uint32 get_itcm_start(){
	return (uint32)(&_itcm_start);
}

sint32 get_itcm_size(){
	return (sint32)((uint8*)(uint32*)&_itcm_end - (sint32)(&_itcm_start));
}

uint32 get_dtcm_start(){
	return (uint32)(&_dtcm_start);
}

sint32 get_dtcm_size(){
	return (sint32)((uint8*)(uint32*)&_dtcm_end - (sint32)(&_dtcm_start));
}

#endif
#ifdef own_allocator

//at start 0
uint32 * this_heap_ptr = 0;

//libc init code
void sbrk_init(){
	this_heap_ptr = (uint32*)get_lma_ewramend();
}

//call after sbrk_init
sint32 get_available_mem(){
	return (sint32)((uint8*)(uint32*)this_heap_ptr - (sint32)(get_lma_libend()));
}
//
sint32 calc_heap(sint32 size){
	if(size >= 0){	//will alloc from heap
		sint32 offset = 0;
		if((size % sizeof(sint32)) > 0){	//ARM requires pointers to be sizeof(sint32) bytes, offset will always round to such boundary if misalignment
			offset = 1;
		}
		if(
			(get_available_mem() >= size)
			&&
			( ((uint32*)get_lma_libend()) <= (this_heap_ptr	-	(sint32)(size/sizeof(sint32))	-	offset)	)
		){	
			this_heap_ptr	= (this_heap_ptr - (size/sizeof(sint32)) - offset);	//size/4 and offset for sizeof(sint32) pointers
		}
		else{
			return -1;
		}
	}
	else{			//will free to heap		
		if( ((uint32*)get_lma_ewramend()) >= (this_heap_ptr	+	(sint32)(abs(size)/sizeof(sint32)))	){
			this_heap_ptr	= (this_heap_ptr	+	(sint32)(abs(size)/sizeof(sint32)));
		}
		else{
			return -1;
		}
	}
	
	return 0;
}

#endif

//Coto: Posix (by default) memory allocator that works on ARM7 (iwram like 3k free) and ARM9 (3M~ free)
void * _sbrk (int nbytes)
{
	//Coto: own implementation, libc's own malloc implementation does not like it.
	//but helps as standalone sbrk implementation except it will always alloc in linear way and not malloc/free (requires keeping track of pointers)

	#ifdef own_allocator
	int retcode = calc_heap(alloc);
	void * retptr = (void *)alloc_failed;
	switch(retcode){
		case(0):{
			//ok
			retptr = this_heap_ptr;
		}
		break;
		//error handling code
		case(-1):{
			
		}
		break;
	}
	
	return retptr;
	#endif
	
	//Standard newlib implementation
	#ifndef own_allocator

	static sint8 *heap_end;
	sint8 *prev_heap_end;

	if (heap_end == NULL){
		heap_end = (sint8*)get_lma_libend();
	}
	prev_heap_end = heap_end;

	if (heap_end + nbytes > (sint8*)get_lma_wramend())
	{
		//errno = ENOMEM;
		return (void *) -1;
	}

	heap_end += nbytes;

	return prev_heap_end;
	#endif
}

//IPC 

//Internal
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
uint32 getToolchainIPCAddress(){
	return (uint32)(0x027FF000);
}
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
sint32 getToolchainIPCSize(){
	return (sint32)(sizeof(tMyIPC));
}

//Internal Aligned
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
uint32 getToolchainAlignedIPCAddress(){
	return (uint32)(getToolchainIPCAddress()+getToolchainIPCSize());
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
sint32 getToolchainAlignedIPCSize(){
	return (sint32)(sizeof(struct sAlignedIPC));
}

//Printf7 Buffer
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
uint32 getPrintfBuffer(){
	return (uint32)(&AlignedIPC->arm7PrintfBuf[0]);
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
uint32 getUserIPCAddress(){
	return (uint32)(getToolchainAlignedIPCAddress()+getToolchainAlignedIPCSize());
}
