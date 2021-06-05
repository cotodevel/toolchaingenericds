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

#ifndef __linker_ext_shared_h__
#define __linker_ext_shared_h__

#include "exceptionTGDS.h"

#define nds_ewram_mask (get_ewram_size()-1)

//Interrupt Routines
#ifdef ARM7
	#define INTERRUPT_VECTOR *((uint32*)&_arm7_irqhandler)
	#define SWI_CHECKBITS *((uint32*)&_arm7_irqcheckbits)
#endif

#ifdef ARM9
	#define INTERRUPT_VECTOR *((uint32*)&_arm9_irqhandler)
	#define SWI_CHECKBITS *((uint32*)&_arm9_irqcheckbits)
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif


extern uint32 	__lib__end__;		//vma_idtcm_start
extern uint32 	__vma_stub_end__;	//vma_idtcm_end

#ifdef ARM9
//CP15 MPU 
extern void MPUSet();

extern 	vuint32	_arm9_irqhandler;
extern	vuint32	_arm9_irqcheckbits;

extern uint32 	_ewram_start;
extern uint32	_ewram_end;

extern uint32 	_dtcm_start;
extern uint32 	_dtcm_end;

extern uint32 	_gba_start;
extern uint32 	_gba_end;
//todo: read from gba cart or something

extern uint32	_gbawram_start;
extern uint32	_gbawram_end;

extern uint32 	_itcm_start;
extern uint32 	_itcm_end;

extern uint32 	_vector_start;
extern uint32 	_vector_end;


#endif

#ifdef ARM7

extern uint32 _iwram_start;
extern uint32 _iwram_end;

extern 	vuint32	_arm7_irqhandler;
extern	vuint32	_arm7_irqcheckbits;
#endif



#ifdef __cplusplus
}
#endif