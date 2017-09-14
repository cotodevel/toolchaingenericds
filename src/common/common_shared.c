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

//Coto: these are my FIFO handling libs. Works fine with NIFI (trust me this is very tricky to do without falling into freezes).
//Use it at your will, just make sure you read WELL the descriptions below.


#include "common_shared.h"
#include "InterruptsARMCores_h.h"
#include "ipc.h"


#ifdef ARM7
#include <string.h>
#endif

#ifdef ARM9
#include <stdbool.h>

#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm9.h"

#endif







//Software FIFO calls, Rely on Hardware FIFO calls so it doesnt matter if they are in different maps 
#ifdef ARM9
__attribute__((section(".dtcm")))
#endif    
volatile int FIFO_SOFT_PTR = 0;
#ifdef ARM9
__attribute__((section(".dtcm")))
#endif    
volatile uint32 FIFO_BUF_SOFT[FIFO_NDS_HW_SIZE/4];

//GetSoftFIFO: Stores up to FIFO_NDS_HW_SIZE. Exposed to usercode for fetching 64 bytes sent from other core, until it returns false (empty buffer).

//Example: 
//uint32 n = 0;
//while(GetSoftFIFO(&n)== true){
//	//n has 4 bytes from the other ARM Core.
//}
#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
inline bool GetSoftFIFO(uint32 * var)
{
	if(FIFO_SOFT_PTR > 0){
		FIFO_SOFT_PTR--;
		
		*var = (uint32)FIFO_BUF_SOFT[FIFO_SOFT_PTR];
		FIFO_BUF_SOFT[FIFO_SOFT_PTR] = (uint32)0;
		
		return true;
	}
	else
		return false;
}

//SetSoftFIFO == false means FULL
#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
//returns ammount of inserted uint32 blocks into FIFO hardware regs
inline bool SetSoftFIFO(uint32 value)
{
	if(FIFO_SOFT_PTR < (int)(FIFO_NDS_HW_SIZE/4)){
		FIFO_BUF_SOFT[FIFO_SOFT_PTR] = value;
		FIFO_SOFT_PTR++;
		return true;
	}
	else
		return false;
}





//Software FIFO Handler receiver, add it wherever its required. (This is a FIFO that runs on software logic, rather than NDS FIFO).
//Supports multiple arguments, in FIFO format.
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Handle_SoftFIFORECV()
{
	uint32 msg = 0;
	
	while(GetSoftFIFO((uint32*)&msg) == true){
		
		//process incoming packages
		switch(msg){
			
			
		}
		
	}
}

//Software FIFO Sender. Send from external ARM Core, receive from the above function. Uses hardware FIFO.
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void SoftFIFOSEND(uint32 value0,uint32 value1,uint32 value2,uint32 value3){
	
}


void writeuint32extARM(uint32 address,uint32 value){
	
}





//int PXI_SendWordByFifo(int fifotag, u32 data, bool err);




//Hardware FIFO
//deprecated
/*
//SendArm[7/9]Command: These send a command and up to 15 arguments. 
//The other ARM Core through a FIFO interrupt will execute HandleFifo()
//By default I use 4 (you can fill them with 0s if you want to use fewer)
#ifdef ARM9
__attribute__((section(".itcm")))
inline void SendArm7Command(uint32 command1, uint32 command2, uint32 command3, uint32 command4)
#endif
#ifdef ARM7
inline void SendArm9Command(uint32 command1, uint32 command2, uint32 command3, uint32 command4)
#endif    
{	
	FIFO_DRAINWRITE();
	
	REG_IPC_FIFO_TX = command1;
	REG_IPC_FIFO_TX = command2;
	REG_IPC_FIFO_TX = command3;
	REG_IPC_FIFO_TX = command4;
	
	FIFO_DRAINWRITE();
}
*/

// Ensures a SendArm[7/9]Command (FIFO message) command to be forcefully executed at target ARM Core, while the host ARM Core awaits. 
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline void FIFO_DRAINWRITE(){
	while (!(REG_IPC_FIFO_CR & SEND_FIFO_IPC_EMPTY)){}
}


#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline bool SendMultipleWordACK(uint32 data0, uint32 data1, uint32 data2, uint32 data3){
	uint32 reply = SendMultipleWordByFifo(fifo_requires_ack, data0, data1, data2, data3);
	if(reply == fifo_requires_ack_execok){
		return true;
	}
	return false;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline uint32 SendMultipleWordByFifo(uint32 data0, uint32 data1, uint32 data2, uint32 data3, uint32 data4)	//channel / multiple words / err reporting
{	
	FIFO_DRAINWRITE();
	
	//data4 to be used from other core
	REG_IPC_FIFO_TX =	(uint32)data0; 	//raise irq here
	REG_IPC_FIFO_TX = 	(uint32)data1;
	REG_IPC_FIFO_TX = 	(uint32)data2;
	REG_IPC_FIFO_TX = 	(uint32)data3;
	REG_IPC_FIFO_TX = 	(uint32)data4;
	
	FIFO_DRAINWRITE();
	 
	REG_IPC_FIFO_CR |= (1<<3); //3     W    Send Fifo Clear             (0=Nothing, 1=Flush Send Fifo)
	
	return MyIPC->fiforeply;
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmpty(){
	HandleFifoEmptyWeakRef((uint32)0,(uint32)0,(uint32)0,(uint32)0,(uint32)0);
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmpty(){
	
	volatile uint32 cmd1 = 0,cmd2 = 0,cmd3 = 0,cmd4 = 0,cmd5 = 0,cmd6 = 0,cmd7 = 0,cmd8 = 0,cmd9 = 0,cmd10 = 0,cmd11 = 0,cmd12 = 0,cmd13 = 0,cmd14 = 0,cmd15 = 0;
	
	if(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		cmd1 = REG_IPC_FIFO_RX;
	}
	
	if(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		cmd2 = REG_IPC_FIFO_RX;
	}
	
	if(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		cmd3 = REG_IPC_FIFO_RX;
	}
	
	if(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		cmd4 = REG_IPC_FIFO_RX;
	}
	
	if(!(REG_IPC_FIFO_CR & RECV_FIFO_IPC_EMPTY)){
		cmd5 = REG_IPC_FIFO_RX;
	}
	
	HandleFifoNotEmptyWeakRef(cmd1,cmd2,cmd3,cmd4,cmd5);
	
	//ok, send empty signal
	REG_IPC_FIFO_CR |= FIFO_IPC_ERROR;
	
	//can we clear then send again, to recv parses the response into shared RAM?
}





//FIFO HANDLER END


//Hardware Shared Functions
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline void writemap_ext_armcore(uint32 address, uint32 value, uint32 mode){
	
	//ARM7 command handler
	#ifdef ARM7
	//SendArm9Command(FIFO_WRITE_ADDR_EXT, address, value, mode);
	#endif
	
	//ARM9 command handler
	#ifdef ARM9
	//SendArm7Command(FIFO_WRITE_ADDR_EXT, address, value, mode);
	#endif
}

//writemap_ext_armcore(REG_POWERCNT_ADDR, (uint32)value, WRITE_VALUE_16);

void powerON(u16 values){
	#ifdef ARM7
	REG_POWERCNT |= values;
	#endif
	
	#ifdef ARM9
	if(!(values & POWERMAN_ARM9)){
		//SendArm7Command(FIFO_POWERCNT_ON, (uint32)values, 0x00000000, 0x00000000);
		SendMultipleWordACK(FIFO_POWERCNT_ON, (uint32)values, 0, 0);
	}
	else{
		REG_POWERCNT |= values;
	}
	#endif
}

void powerOFF(u16 values){
	#ifdef ARM7
	REG_POWERCNT &= ~values;
	#endif
	
	#ifdef ARM9
	if(!(values & POWERMAN_ARM9)){
		//SendArm7Command(FIFO_POWERCNT_OFF, (uint32)values, 0x00000000, 0x00000000);
		SendMultipleWordACK(FIFO_POWERCNT_OFF, (uint32)values, 0, 0);
	}
	else{
		REG_POWERCNT &= ~values;
	}
	#endif
}


