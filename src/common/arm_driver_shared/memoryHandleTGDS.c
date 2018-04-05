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

sint32 HeapBlockOfst	=	0;

//if ret ptr == NULL, invalid operation  not enough space
uint32 * vramHeapAlloc(uint32 vramBlock,uint32 StartAddr,int size){
	uint32 * BlockAssign = NULL;
	bool isVram = false;
	switch(vramBlock){
		case(vramBlockA):{
			BlockAssign = (uint32 *)&vramABlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockB):{
			BlockAssign = (uint32 *)&vramBBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockC):{
			BlockAssign = (uint32 *)&vramCBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockD):{
			BlockAssign = (uint32 *)&vramDBlockOfst;
			isVram = true;
		}
		break;
		case(HeapBlock):{
			BlockAssign = (uint32 *)&HeapBlockOfst;
		}
		break;
	}
	if(BlockAssign == NULL){
		return NULL;
	}
	sint32 heapDetected = (isVram == true) ? vramSize : HeapSize;
	if((StartAddr + (int)*BlockAssign + size) <= (StartAddr+heapDetected)){
		//memset((uint8*)(StartAddr + (int)*BlockAssign) , 0, size);
		*BlockAssign = (uint32)((int)*BlockAssign + size);
	}
	else{
		return NULL;
	}
	uint32 AllocBuf = (StartAddr + ((int)*BlockAssign - size));
	if(AllocBuf < StartAddr){
		AllocBuf = StartAddr;
	}
	return (uint32*)AllocBuf;
}

//if ret ptr == NULL, invalid operation  not enough space
uint32 * vramHeapFree(uint32 vramBlock,uint32 StartAddr,int size){
	uint32 * BlockAssign = NULL;
	bool isVram = false;
	switch(vramBlock){
		case(vramBlockA):{
			BlockAssign = (uint32 *)&vramABlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockB):{
			BlockAssign = (uint32 *)&vramBBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockC):{
			BlockAssign = (uint32 *)&vramCBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockD):{
			BlockAssign = (uint32 *)&vramDBlockOfst;
			isVram = true;
		}
		break;
		case(HeapBlock):{
			BlockAssign = (uint32 *)&HeapBlockOfst;
		}
		break;
	}
	if(BlockAssign == NULL){
		return NULL;
	}
	if(((StartAddr + (int)*BlockAssign) - size) >= (StartAddr)){
		*BlockAssign = (uint32)((int)*BlockAssign - size);
	}
	else{
		return NULL;
	}
	return (uint32*)(StartAddr + ((int)*BlockAssign));
}

/*
int _tmain(int argc, _TCHAR* argv[])
{
	uint32 startLinearVramAddr = 0x06000000;
	uint32 startLinearHeapAddr = 0x02040100;	//fake ewram address

	sint32 size = 1024 * 32;
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06000000
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06008000
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06010000
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06018000

	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02040100
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02048100
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02050100
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02058100
	
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0 (invalid)
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06018000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02058100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06010000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02050100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06008000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02048100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06000000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02040100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0 (invalid)
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0 (invalid)
	
	while(1==1){}
	return 0;
}
*/

//

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

#ifdef ARM7
//ARM7:
//use 0x06000000 ~ 32K for playBuffer
//use 0x06008000 ~ 32K for malloc / stacks
uint32 * vramLMALibendARM7 = NULL;	//relocated end section, alloced by vram alloc, used as sbrk
#endif

#ifdef ARM9
//ARM9:
//heap alloced from malloc
//this heap is used as:
//alloc: ptr to start allocated = (start linear ptr *)vramHeapAlloc(HeapBlock,vramLMAstartARM9,int size);
//free: ptr to freed start unallocated = (start linear ptr *)vramHeapFree(HeapBlock,vramLMAstartARM9,int size);
uint32 * vramLMAstartARM9 = NULL;
#endif
void initvramLMALibend(){
	#ifdef ARM9
	if(vramLMAstartARM9 == NULL){
		vramLMAstartARM9 = malloc((int)HeapSize);
	}
	#endif
	#ifdef ARM7
	if(vramLMALibendARM7 == NULL){
		//vramLMALibendARM7 = vramHeapAlloc(vramBlockC,(0x06008100),((32*1024)-256));	//it is VRAMBLOCK D currently mapped to ARM7 VRAM, use vramBlockC to keep different maps
	}
	#endif
}


uint32 get_lma_libend(){
	return (uint32)(&__vma_stub_end__);	//linear memory top (start)
}

//(ewram end - linear memory top ) = malloc free memory (bottom, end)
uint32 get_lma_wramend(){
	#ifdef ARM7
	extern uint32 sp_SYS;	//the farthest stack from the end memory is our free memory (in ARM7, shared stacks)
	return (uint32)(&sp_SYS);
	#endif
	#ifdef ARM9
	return (uint32)(&_ewram_end);	//EWRAM has no stacks shared so we use the end memory 
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

void * _sbrk_r (struct _reent * reent, int size){
	return _sbrk (size);
}

//NDS Memory Map (valid):
//todo: detect valid maps according to MPU settings
__attribute__ ((hot))
__attribute__((section(".itcm")))
bool isValidMap(uint32 addr){
	if( 
		#ifdef ARM9
		((addr >= (uint32)0x06000000) && (addr < (uint32)0x06020000))	//NDS VRAM BG Engine Region protected	0x06000000 - 0x0601ffff
		||
		((addr >= (uint32)0x06020000) && (addr < (uint32)0x06040000))	//NDS VRAM BG Engine Region protected	0x06020000 - 0x0603ffff
		||
		((addr >= (uint32)0x06040000) && (addr < (uint32)0x06060000))	//NDS VRAM BG Engine Region protected	0x06040000 - 0x0605ffff
		||
		((addr >= (uint32)0x06060000) && (addr < (uint32)0x06080000))	//NDS VRAM BG Engine Region protected	0x06060000 - 0x0607ffff
		||
		((addr >= (uint32)0x06200000) && (addr < (uint32)(0x06200000 + 0x20000)))	//NDS VRAM Engine Region protected	//theoretical map
		||
		((addr >= (uint32)0x06400000) && (addr < (uint32)(0x06400000 + 0x14000)))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x06600000) && (addr < (uint32)(0x06600000 + 0x20000)))	//NDS VRAM Engine Region protected	// theoretical map
		||
		((addr >= (uint32)0x06800000) && (addr < (uint32)(0x06800000 + (656 * 1024) )))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x07000000) && (addr < (uint32)(0x07000000 + (2 * 1024) )))	//NDS VRAM OAM Region protected	// theoretical map?
		||
		((addr >= (uint32)get_ewram_start()) && (addr <= (uint32)(get_ewram_start() + get_ewram_size())))	//NDS EWRAM Region protected
		||
		((addr >= (uint32)(get_itcm_start())) && (addr <= (uint32)(get_itcm_start()+get_itcm_size())))	//NDS ITCM Region protected
		||
		((addr >= (uint32)(get_dtcm_start())) && (addr <= (uint32)(get_dtcm_start()+get_dtcm_size())))	//NDS DTCM Region protected
		||
		((addr >= (uint32)(0x05000000)) && (addr <= (uint32)(0x05000000 + 2*1024)))	//NDS Palette Region protected
		||
		( (WRAM_CR & WRAM_32KARM9_0KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 32K Region protected
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )
		#endif
		#ifdef ARM7
		((addr >= (uint32)get_iwram_start()) && (addr <= (uint32)(get_iwram_start() + get_iwram_size())))	//NDS EWRAM Region protected
		||
		( (WRAM_CR & WRAM_0KARM9_32KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 32K Region protected
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )
		#endif
		||
		((addr >= (uint32)(0x04000000)) && (addr <= (uint32)(0x04000000 + 4*1024)))	//NDS IO Region protected
		
	){
		return true;
	}
	return false;
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
	return (sint32)(sizeof(struct sIPCSharedTGDS));
}

//Printf7 Buffer
uint32 getPrintfBuffer(){
	return (uint32)(&getsIPCSharedTGDS()->arm7PrintfBuf[0]);
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
uint32 getUserIPCAddress(){
	return (uint32)(getToolchainIPCAddress()+getToolchainIPCSize());
}
