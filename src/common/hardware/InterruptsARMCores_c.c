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

#ifdef ARM7
#include "wifi_arm7.h"
#endif

#ifdef ARM9
#include "wifi_arm9.h"
#include "dswnifi_lib.h"
#endif

#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDS.h"
#include "linkerTGDS.h"

#ifdef ARM7
uint32 interrupts_to_wait_arm7 = 0;
#endif

uint32 GLOBAL_IME = 0;

#ifdef ARM9
__attribute__((section(".dtcm")))
volatile uint32 interrupts_to_wait_arm9 = 0;
#endif

//stubs because we don't disable IME on DSWIFI
void SGIP_INTR_PROTECT(){}
void SGIP_INTR_REPROTECT(){}
void SGIP_INTR_UNPROTECT(){}
void SGIP_WAITEVENT(){}

void IRQInit(){
	
	//fifo setups
	REG_IME = 0;
	
	int i = 0;
	for(i = 0; i <32 ; i++){
		if((REG_IF & (1 << i)) == (1 << i)){
			REG_IF = (1 << i);
		}
	}
	
	REG_IE = 0;
	REG_IPC_SYNC = 0;
    REG_IPC_FIFO_CR = RECV_FIFO_IPC_IRQ  | FIFO_IPC_ENABLE;
	
	//set up ppu: do irq on hblank/vblank/vcount/and vcount line is 159
    REG_DISPSTAT = REG_DISPSTAT | DISP_HBLANK_IRQ | DISP_VBLANK_IRQ | (DISP_YTRIGGER_IRQ | (VCOUNT_LINE_INTERRUPT << 15));
	
	uint32 interrupts_to_wait_armX = 0;
	
	#ifdef ARM7
	interrupts_to_wait_armX = interrupts_to_wait_arm7 = IRQ_TIMER1 | IRQ_HBLANK | IRQ_VBLANK | IRQ_VCOUNT | IRQ_RECVFIFO_NOT_EMPTY ;
	#endif
	
	#ifdef ARM9
	interrupts_to_wait_armX = interrupts_to_wait_arm9 = IRQ_HBLANK| IRQ_VBLANK | IRQ_VCOUNT | IRQ_RECVFIFO_NOT_EMPTY;
	#endif
	
	REG_IE = interrupts_to_wait_armX; 
	
	INTERRUPT_VECTOR = (uint32)&InterruptServiceRoutineARMCores;
	REG_IME = 1;
	
}



//Software bios irq more or less emulated. (replaces default NDS bios for some parts)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
__attribute__((aligned(4)))
void NDS_IRQHandler(){
	uint32 REG_IE_SET = (REG_IE & REG_IF);
	if(REG_IE_SET & IRQ_HBLANK){
		HblankUser();
		SWI_CHECKBITS = REG_IF = IRQ_HBLANK;
	}
	if(REG_IE_SET & IRQ_VBLANK){
		#ifdef ARM7
		doSPIARM7IO();
		Wifi_Update();
		#endif
		
		#ifdef ARM9
		//handles DS-DS Comms
		sint32 currentDSWNIFIMode = doMULTIDaemonStage1();
		#endif
		
		//key event between frames
		do_keys();
		VblankUser();
		SWI_CHECKBITS = REG_IF = IRQ_VBLANK;
	}
	
	if(REG_IE_SET & IRQ_VCOUNT){
		VcounterUser();
		SWI_CHECKBITS = REG_IF = IRQ_VCOUNT;
	}
	
	////			Common
	if(REG_IE_SET & IRQ_TIMER0){
		Timer0handlerUser();
		REG_IF = IRQ_TIMER0;
	}
	
	if(REG_IE_SET & IRQ_TIMER1){
		#ifdef ARM7
		//arm7 sound here...
		#endif
		Timer1handlerUser();
		REG_IF = IRQ_TIMER1;
	}
	
	if(REG_IE_SET & IRQ_TIMER2){
		Timer2handlerUser();
		REG_IF = IRQ_TIMER2;
	}
	
	if(REG_IE_SET & IRQ_TIMER3){
		#ifdef ARM9
		//wifi arm9 irq
		Timer_50ms();
		#endif
		Timer3handlerUser();
		REG_IF = IRQ_TIMER3;
	}
	if(REG_IE_SET & IRQ_SENDFIFO_EMPTY){
		HandleFifoEmpty();
		REG_IF = IRQ_SENDFIFO_EMPTY;
	}
	
	if(REG_IE_SET & IRQ_RECVFIFO_NOT_EMPTY){
		HandleFifoNotEmpty();
		REG_IF=IRQ_RECVFIFO_NOT_EMPTY;
	}
	
	#ifdef ARM7
	//arm7 wifi cart irq
	if(REG_IE_SET & IRQ_WIFI){
		Wifi_Interrupt();
		REG_IF = IRQ_WIFI;
	}
	
	//clock //could cause freezes
	if(REG_IE_SET & IRQ_RTCLOCK){
		REG_IF = IRQ_RTCLOCK;
	}
	#endif
}


void EnableIrq(uint32 IRQ){
	#ifdef ARM7
	interrupts_to_wait_arm7	|=	IRQ;
	#endif
	
	#ifdef ARM9
	interrupts_to_wait_arm9	|=	IRQ;
	#endif
	
	REG_IE	|=	IRQ;
	
}

void DisableIrq(uint32 IRQ){
	#ifdef ARM7
	interrupts_to_wait_arm7	&=	~(IRQ);
	#endif
	
	#ifdef ARM9
	interrupts_to_wait_arm9	&=	~(IRQ);
	#endif
	
	REG_IE	&=	~(IRQ);
}



void RemoveQueuedIrq(uint32 IRQ){
	if(REG_IF & IRQ){
		REG_IF|=IRQ;
	}
}

void QueueIrq(uint32 IRQ){
	if(!(REG_IF & IRQ)){
		REG_IF|=IRQ;
	}
}