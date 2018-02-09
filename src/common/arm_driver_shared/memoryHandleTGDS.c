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

#include <sys/types.h>
#include <errno.h>
#include <stddef.h>

#include "memoryHandleTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "reent.h"	//sbrk
#include "sys/types.h"
#include "ipcfifoTGDS.h"

//vram linear memory allocator 
sint32 vramABlockOfst	=	0;	//offset pointer to free memory, user alloced memory is (baseAddr + (sizeAlloced - vramBlockPtr))
sint32 vramBBlockOfst	=	0;
sint32 vramCBlockOfst	=	0;
sint32 vramDBlockOfst	=	0;

uint32 * vramAlloc(uint32 vramBlock,uint32 StartAddr,int size){
	uint32 * vramBlockAssign = NULL;
	switch(vramBlock){
		case(vramBlockA):{
			vramBlockAssign = (uint32 *)&vramABlockOfst;
		}
		break;
		case(vramBlockB):{
			vramBlockAssign = (uint32 *)&vramBBlockOfst;
		}
		break;
		case(vramBlockC):{
			vramBlockAssign = (uint32 *)&vramCBlockOfst;
		}
		break;
		case(vramBlockD):{
			vramBlockAssign = (uint32 *)&vramDBlockOfst;
		}
		break;
	}
	if(vramBlockAssign == NULL){
		return NULL;
	}
	if((StartAddr + (int)*vramBlockAssign + size) <= (StartAddr+vramSize)){
		//memset((uint32*)(StartAddr + (int)*vramBlockAssign) , 0, size);
		*vramBlockAssign = (uint32)((int)*vramBlockAssign + size);
	}
	else{
		return NULL;
	}
	uint32 AllocBuf = (StartAddr + ((int)*vramBlockAssign - size));
	if(AllocBuf < StartAddr){
		AllocBuf = StartAddr;
	}
	return (uint32*)AllocBuf;
}

uint32 * vramFree(uint32 vramBlock,uint32 StartAddr,int size){
	uint32 * vramBlockAssign = NULL;
	switch(vramBlock){
		case(vramBlockA):{
			vramBlockAssign = (uint32 *)&vramABlockOfst;
		}
		break;
		case(vramBlockB):{
			vramBlockAssign = (uint32 *)&vramBBlockOfst;
		}
		break;
		case(vramBlockC):{
			vramBlockAssign = (uint32 *)&vramCBlockOfst;
		}
		break;
		case(vramBlockD):{
			vramBlockAssign = (uint32 *)&vramDBlockOfst;
		}
		break;
	}
	if(vramBlockAssign == NULL){
		return NULL;
	}
	if(((StartAddr + (int)*vramBlockAssign) - size) >= (StartAddr)){
		*vramBlockAssign = (uint32)((int)*vramBlockAssign - size);
	}
	else{
		return NULL;
	}
	return (uint32*)(StartAddr + ((int)*vramBlockAssign));
}

//linker to C proper memory layouts.

//Shared:

//these toggle the WRAM_CR register depending on linker settings

uint32 get_arm7_start_address(){
	return (uint32)getsIPCSharedTGDS()->arm7startaddress;
}

uint32 get_arm7_end_address(){
	return (uint32)getsIPCSharedTGDS()->arm7endaddress;
}

sint32 get_arm7_ext_size(){
	return (sint32)((uint8*)(uint32*)get_arm7_end_address() - (sint32)(get_arm7_start_address()));
}


uint32 get_arm9_start_address(){
	return (uint32)getsIPCSharedTGDS()->arm9startaddress;
}

uint32 get_arm9_end_address(){
	return (uint32)getsIPCSharedTGDS()->arm9endaddress;
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

//ARM7:
//use 0x06000000 ~ 32K for playBuffer
//use 0x06008000 ~ 32K for malloc / stacks
uint32 * vramLMALibendARM7 = NULL;	//relocated end section, alloced by vram alloc, used as sbrk

#ifdef ARM7
void initvramLMALibend(){
	if(vramLMALibendARM7 == NULL){
		vramLMALibendARM7 = vramAlloc(vramBlockC,(0x06008100),((32*1024)-256));	//it is VRAMBLOCK D currently mapped to ARM7 VRAM, use vramBlockC to keep different maps
	}
}
#endif


uint32 get_lma_libend(){
	#ifdef ARM7
	return (uint32)(0x06000000 + (32*1024) + (256));
	#endif
	 
	#ifdef ARM9
	return (uint32)(&__vma_stub_end__);	//linear memory top (start)
	#endif
}

//(ewram end - linear memory top ) = malloc free memory (bottom, end)
uint32 get_lma_wramend(){
	#ifdef ARM7
	return (uint32)(0x06000000 + (32*1024) + (256) + (((32*1024)-256)));
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


/* ------------------------------------------------------------------------- */
/*!Increase program data space
   This is a minimal implementation.  Assuming the heap is growing upwards
   from __heap_start towards __heap_end.
   See linker file for definition.
   @param[in] incr  The number of bytes to increment the stack by.
   @return  A pointer to the start of the new block of memory                */
/* ------------------------------------------------------------------------- */
void *
_sbrk (int  incr)
{
	extern char __heap_start;//set by linker
	extern char __heap_end;//set by linker

	static char *heap_end;		/* Previous end of heap or 0 if none */
	char        *prev_heap_end;

	if (0 == heap_end) {
		heap_end = (sint8*)get_lma_libend();			/* Initialize first time round */
	}

	prev_heap_end  = heap_end;
	heap_end      += incr;
	//check
	if( heap_end < (char*)get_lma_wramend()) {

	} else {
		errno = ENOMEM;
		return (char*)-1;
	}
	return (void *) prev_heap_end;

}	/* _sbrk () */



///* sbrk support */
//
///* The current plan is to have one sbrk handler for all cpus.
//   Hence use `asm' for each global variable here to avoid the cpu prefix.
//   We can't intrude on the user's namespace (another reason to use asm).  */
//
//#include <sys/types.h>
///*#include <sys/syscall.h>*/
//#include <errno.h>
//#include <stddef.h>
//
///* These variables are publicly accessible for debugging purposes.
//   The user is also free to set sbrk_size to something different.
//   See mem-layout.c.  */
//
//extern int sbrk_size asm ("sbrk_size");
//
//caddr_t sbrk_start asm ("sbrk_start");
//caddr_t sbrk_loc asm ("sbrk_loc");
//extern char  end;                    /* Defined by linker.  */
//
///*caddr_t _sbrk_r (struct _reent *, size_t) asm ("__sbrk_r");*/
//
//
///* This symbol can be defined with the --defsym linker option.  */
//extern char __heap_end __attribute__((weak));
//
//
///* FIXME: We need a semaphore here.  */
//
//caddr_t
//_sbrk_r (struct _reent *r, size_t nbytes)
//{
//   extern char  end;			/* Defined by linker.  */
//   static char *heap_end;		/* Previous end of heap or 0 if none */
//   char        *prev_heap_end;
//   register char* stack asm("sp");
//
//   if (0 == heap_end)
//     {
//       heap_end = &end;			/* Initialize first time round */
//     }
//
//   prev_heap_end  = heap_end;
//
//   if (stack < (prev_heap_end+nbytes)) {
//     /* heap would overlap the current stack depth.
//      * Future:  use sbrk() system call to make simulator grow memory beyond
//      * the stack and allocate that
//      */
//     errno = ENOMEM;
//     return (char*)-1;
//   }
//
//   heap_end      += nbytes;
//
//   return (caddr_t) prev_heap_end;
//
//}	/* _sbrk () */


//caddr_t
//_sbrk_r (struct _reent *r, size_t nbytes)
//{
//  caddr_t result;
//
//  void *heap_end;
//
//  heap_end = &__heap_end;
//  if (!heap_end)
//    heap_end = sbrk_start + sbrk_size;
//  if (
//      /* Ensure we don't underflow.  */
//      sbrk_loc + nbytes < sbrk_start
//      /* Ensure we don't overflow.  */
//      || sbrk_loc + nbytes > (caddr_t)heap_end)
//    {
//      errno = ENOMEM;
//      return ((caddr_t) -1);
//    }
//
//  if (0 == sbrk_loc)
//    sbrk_loc = &end;                 /* Initialize first time round */
//  result = sbrk_loc;
//  sbrk_loc += nbytes;
//  return result;
//}

//IPC 

//Internal
uint32 getToolchainIPCAddress(){
	return (uint32)(0x027FF000);
}
sint32 getToolchainIPCSize(){
	return (sint32)(sizeof(struct sIPCSharedTGDS));
}

//Printf7 Buffer
uint32 getPrintfBuffer(){
	return (uint32)(&getsIPCSharedTGDS()->arm7PrintfBuf[0]);
}

uint32 getUserIPCAddress(){
	return (uint32)(getToolchainIPCAddress()+getToolchainIPCSize());
}
