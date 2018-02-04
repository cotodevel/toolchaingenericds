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

//TGDS IPC Version: 1.3

#ifndef __nds_ipc_h__
#define __nds_ipc_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdbool.h>
#include "spitscTGDS.h"
#include "usrsettingsTGDS.h"
#include <time.h>
#include "memoryHandleTGDS.h"
#include "audioTGDS.h"

//printf size buffer
#define consolebuf_size (sint32)(100)

//irqs
#define VCOUNT_LINE_INTERRUPT 0

//void writemap_ext_armcore(0x04000208,0x000000ff,WRITE_VALUE_8);
#define WRITE_VALUE_8	0xf0
#define WRITE_VALUE_16	0xf1
#define WRITE_VALUE_32	0xf2

//FIFO Hardware -> FIFO Software: GetSoftFIFO / SetSoftFIFO	/ 
#define FIFO_NDS_HW_SIZE (16*4)
#define FIFO_SOFTFIFO_WRITE_EXT	(uint32)(0xffff1017)
#define FIFO_SOFTFIFO_READ_EXT	(uint32)(0xffff1018)

//PowerCnt Read / PowerCnt Write
#define FIFO_POWERCNT_ON	0xffff0004
#define FIFO_POWERCNT_OFF	0xffff0005

//FIFO - WIFI
#define WIFI_SYNC 0xffff0006
#define WIFI_STARTUP 0xffff0007

//Exception Handling
#define EXCEPTION_ARM7 0xffff0008
#define EXCEPTION_ARM9 0xffff0009

//PowerManagementWrite
#define FIFO_POWERMGMT_WRITE	(uint32)(0xffff1019)

//printf7 FIFO
#define FIFO_PRINTF_7	(uint32)(0xffff101a)

#define FIFO_IPC_MESSAGE	(uint32)(0xffff1010)
#define ARM7ARM9SHAREDBUFFERSIZE	(sint32)(1024*2)	//2K Shared buffer. ARM9 defines it.

#define SEND_FIFO_IPC_EMPTY	(1<<0)	
#define SEND_FIFO_IPC_FULL	(1<<1)	
#define SEND_FIFO_IPC_IRQ	(1<<2)		
#define SEND_FIFO_IPC_CLEAR	(1<<3)	
#define RECV_FIFO_IPC_EMPTY	(1<<8)	
#define RECV_FIFO_IPC_FULL	(1<<9)	
#define RECV_FIFO_IPC_IRQ	(1<<10)	

#define FIFO_IPC_ERROR	(1<<14)	
#define FIFO_IPC_ENABLE	(1<<15)

//fifo 
#define REG_IPC_FIFO_TX		(*(vuint32*)0x4000188)
#define REG_IPC_FIFO_RX		(*(vuint32*)0x4100000)
#define REG_IPC_FIFO_CR		(*(vuint16*)0x4000184)

//ipc fifo sync
#define REG_IPC_SYNC	(*(vuint16*)0x04000180)

//audio API
#define FIFO_STOPSAMPLE		(uint32)(0xffff1020)
#define FIFO_STARTSAMPLE	(uint32)(0xffff1021)

#define FIFO_SETSHAREDAUDIOHANDLER	(uint32)(0xffff1022)

struct sIPCSharedTGDS {
    tEXTKEYIN	EXTKEYINInst;
	uint16 buttons7;  			// X, Y, /PENIRQ buttons
    uint16 KEYINPUT7;			//REG_KEYINPUT ARM7
	uint16 touchX,   touchY;   // raw x/y TSC SPI
	sint16 touchXpx, touchYpx; // TFT x/y pixel (converted)
	
	sint16 touchZ1,  touchZ2;  // TSC x-panel measurements
    uint16 tdiode1,  tdiode2;  // TSC temperature diodes
    uint32 temperature;        // TSC computed temperature
		
	struct tm tmInst;	//DateTime
	uint8	dayOfWeek;	//Updated when the above inst is updated
	
	uint8 consoletype;
	/*
	Language and Flags (Entry 064h)
	Bit
	0..2 Language (0=Japanese, 1=English, 2=French, 3=German,
		4=Italian, 5=Spanish, 6..7=Reserved) (for Chinese see Entry 075h)
		(the language setting also implies time/data format)
	3    GBA mode screen selection (0=Upper, 1=Lower)
	4-5  Backlight Level    (0..3=Low,Med,High,Max) (DS-Lite only)
	6    Bootmenu Disable   (0=Manual/bootmenu, 1=Autostart Cartridge)
	9    Settings Lost (1=Prompt for User Info, and Language, and Calibration)
	10   Settings Okay (0=Prompt for User Info)
	11   Settings Okay (0=Prompt for User Info) (Same as Bit10)
	12   No function
	13   Settings Okay (0=Prompt for User Info, and Language)
	14   Settings Okay (0=Prompt for User Info) (Same as Bit10)
	15   Settings Okay (0=Prompt for User Info) (Same as Bit10)
	The Health and Safety message is skipped if Bit9=1, or if one or more of the following bits is zero: Bits 10,11,13,14,15. However, as soon as entering the bootmenu, the Penalty-Prompt occurs.
	*/
	uint8 nickname_schar8[0x20];	//converted from UTF-16 to char*
	
	//DS Firmware	Settings default set
	tDSFWSETTINGS DSFWSETTINGSInst;
	bool valid_dsfwsettings;	//true or false
	
	uint8 lang_flags[0x2];
	
	uint32 * arm7arm9sharedBuffer;	//set up by ARM9. Both cores must see the definition.
	
	//Internal use, use functions inside mem_handler_shared.c for accessing those from BOTH ARM Cores.
	uint32 arm7startaddress;
	uint32 arm7endaddress;
	
	uint32 arm9startaddress;
	uint32 arm9endaddress;
	
	uint32 WRAM_CR_ISSET;	//0 when ARM7 boots / 1 by ARM9 when its done
	
	uint8 arm7PrintfBuf[consolebuf_size];
	
	//used by softFIFO
	uint32 FIFO_BUF_SOFT[FIFO_NDS_HW_SIZE/4];
	
	struct sIPCSharedTGDSAudioGlobal audioglobal;
};

//Shared Work     027FF000h 4KB    -     -    -    R/W

#endif

#ifdef __cplusplus
extern "C"{
#endif

//weak symbols : the implementation of these is project-defined, also abstracted from the hardware IPC FIFO Implementation for easier programming.
extern __attribute__((weak))	void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4);
extern __attribute__((weak))	void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4);


//FIFO 
extern int GetSoftFIFOCount();
extern bool SetSoftFIFO(uint32 value);
extern bool GetSoftFIFO(uint32 * var);

extern volatile int FIFO_SOFT_PTR;
extern volatile uint32 FIFO_IN_BUF[FIFO_NDS_HW_SIZE/4];

extern void HandleFifoNotEmpty();
extern void HandleFifoEmpty();

extern void Handle_SoftFIFORECV();
extern void SoftFIFOSEND(uint32 value0,uint32 value1,uint32 value2,uint32 value3);

extern void writeuint32extARM(uint32 address,uint32 value);

extern int SendFIFOCommand(uint32 * buf,int size);
extern int RecvFIFOCommand(uint32 * buf);

extern void powerON(uint16 values);
extern void powerOFF(uint16 values);

extern void SendMultipleWordByFifo(uint32 data0, uint32 data1, uint32 data2, uint32 * buffer_shared);
extern void SendMultipleWordACK(uint32 data0, uint32 data1, uint32 data2, uint32 * buffer_shared);

#ifdef ARM9
extern volatile uint8 arm7arm9sharedBuffer[ARM7ARM9SHAREDBUFFERSIZE];
#endif

extern void setARM7ARM9SharedBuffer(uint32 * shared_buffer_address);
extern uint32 * getARM7ARM9SharedBuffer();
extern struct sIPCSharedTGDS* getsIPCSharedTGDS();

#ifdef __cplusplus
}
#endif