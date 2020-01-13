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

//IPC FIFO Description: 
//		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 														// Access to TGDS internal IPC FIFO structure. 		(ipcfifoTGDS.h)
//		struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;		// Access to TGDS Project (User) IPC FIFO structure	(ipcfifoTGDSUser.h)

#ifndef __nds_ipc_h__
#define __nds_ipc_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdbool.h>
#include "spitscTGDS.h"
#include "usrsettingsTGDS.h"
#include <time.h>
#include "utilsTGDS.h"
#include "wifi_shared.h"
#include "soundTGDS.h"
#include "global_settings.h"

//FIFO Hardware
//void Write8bitAddrExtArm
//void Write16bitAddrExtArm
//void Write32bitAddrExtArm
#define WRITE_EXTARM_8	(uint32)(0xffff0200)
#define WRITE_EXTARM_16	(uint32)(0xffff0201)
#define WRITE_EXTARM_32	(uint32)(0xffff0202)
	
//Linear sound playback (ARM9 -> ARM7)
#define FIFO_PLAYSOUND	(uint32)(0xffff0203)
#define FIFO_INITSOUND	(uint32)(0xffff0204)
#define FIFO_FLUSHSOUNDCONTEXT	(uint32)(0xffff0210)

//PowerCnt Read / PowerCnt Write
#define FIFO_POWERCNT_ON	(uint32)(0xffff0205)
#define FIFO_POWERCNT_OFF	(uint32)(0xffff0206)

//FIFO - WIFI
#define WIFI_SYNC (uint32)(0xffff0207)
#define WIFI_INIT (uint32)(0xffff0208)
#define WIFI_DEINIT (uint32)(0xffff0209)

//Exception Handling
#define EXCEPTION_ARM7 (uint32)(0xffff020A)
#define EXCEPTION_ARM9 (uint32)(0xffff020B)

//PowerManagementWrite
#define FIFO_POWERMGMT_WRITE	(uint32)(0xffff020C)
	//power management commands:
	//screen power write
	#define FIFO_SCREENPOWER_WRITE	(uint32)(0xffff020D)

//DLDI ARM7
#define TGDS_DLDI_ARM7_STATUS_INIT	(int)(0xffff0300)
	#define TGDS_DLDI_ARM7_STATUS_STAGE0	(int)(0)	//ARM7 Inits ARM7 DLDI context. Passes the target DLDI address and waits for DLDI to be relocated to such address.
	#define TGDS_DLDI_ARM7_STATUS_STAGE1	(int)(-1)
	
#define TGDS_DLDI_ARM7_INIT_OK	(uint32)(0xffff0301)
#define TGDS_DLDI_ARM7_INIT_ERROR	(uint32)(0xffff0302)

#define TGDS_DLDI_ARM7_READ	(uint32)(0xffff0303)
	#define TGDS_DLDI_ARM7_STATUS_BUSY_READ	(int)(0xffff0304)	//ARM7 Inits ARM7 DLDI context. Passes the target DLDI address and waits for DLDI to be relocated to such address.
	#define TGDS_DLDI_ARM7_STATUS_IDLE_READ	(int)(0xffff0305)

#define TGDS_DLDI_ARM7_WRITE	(uint32)(0xffff0306)
	#define TGDS_DLDI_ARM7_STATUS_BUSY_WRITE	(int)(0xffff0307)	//ARM7 Inits ARM7 DLDI context. Passes the target DLDI address and waits for DLDI to be relocated to such address.
	#define TGDS_DLDI_ARM7_STATUS_IDLE_WRITE	(int)(0xffff0308)

#define SEND_FIFO_IPC_EMPTY	(uint32)(1<<0)	
#define SEND_FIFO_IPC_FULL	(uint32)(1<<1)	
#define SEND_FIFO_IPC_IRQ	(uint32)(1<<2)		
#define SEND_FIFO_IPC_CLEAR	(uint32)(1<<3)	
#define RECV_FIFO_IPC_EMPTY	(uint32)(1<<8)	
#define RECV_FIFO_IPC_FULL	(uint32)(1<<9)	
#define RECV_FIFO_IPC_IRQ	(uint32)(1<<10)	

#define FIFO_IPC_ERROR	(uint32)(1<<14)	
#define FIFO_IPC_ENABLE	(uint32)(1<<15)

//LID signaling at ARM9
#define FIFO_IRQ_LIDHASOPENED_SIGNAL	(uint32)(0xffff020E)
#define FIFO_IRQ_LIDHASCLOSED_SIGNAL	(uint32)(0xffff020F)

//IPC bits
#define REG_IPC_FIFO_TX		(*(vuint32*)0x4000188)
#define REG_IPC_FIFO_RX		(*(vuint32*)0x4100000)
#define REG_IPC_FIFO_CR		(*(vuint16*)0x4000184)

#define REG_IPC_SYNC	(*(vuint16*)0x04000180)
#define IPC_SYNC_IRQ_ENABLE		(uint16)(1<<14)
#define IPC_SYNC_IRQ_REQUEST	(uint16)(1<<13)
#define IPC_FIFO_SEND_EMPTY		(uint16)(1<<0)
#define IPC_FIFO_SEND_FULL		(uint16)(1<<1)
#define IPC_FIFO_SEND_IRQ		(uint16)(1<<2)
#define IPC_FIFO_SEND_CLEAR		(uint16)(1<<3)
#define IPC_FIFO_RECV_EMPTY		(uint16)(1<<8)
#define IPC_FIFO_RECV_FULL		(uint16)(1<<9)
#define IPC_FIFO_RECV_IRQ		(uint16)(1<<10)
#define IPC_FIFO_ERROR			(uint16)(1<<14)
#define IPC_FIFO_ENABLE			(uint16)(1<<15)

#define IPC_SEND_MULTIPLE_CMDS			(u8)(1)

//Read callback between ARM processors (in chunks)
#define READ_EXTARM_FIFO	(uint8)(0xffff22fe)
	#define READ_EXTARM_FIFO_READY	(uint32)(0xffff22ff)
	#define READ_EXTARM_FIFO_BUSY	(uint32)(0xffff11ff)
	#define READ_EXTARM_FIFO_SIZE	(sint32)(32*1024)


struct sTGDSDLDIARM7DLDICmd {
    uint32_t sector;
	uint32_t numSectors;
	void* buffer;
	int DLDIStatus;
} __attribute__((aligned (4)));

struct sTGDSDLDIARM7Context {
    u32 DLDISourceAddress;
	int DLDISize;
	int TGDSDLDIStatus;
	struct sTGDSDLDIARM7DLDICmd * dldiCmdSharedCtx;
} __attribute__((aligned (4)));

struct sSharedSENDCtx {
    u32 targetAddr;
	u32 srcAddr;
	int size;		//buffer source, accounts size
	int lastCopySz;
	int status;	//0 not ready, 1 ready to take orders
} __attribute__((aligned (4)));

struct sIPCSharedTGDS {
    struct soundSampleContextList soundContextShared;
	uint16 buttons7;  			// X, Y, /PENIRQ buttons
    uint16 KEYINPUT7;			//REG_KEYINPUT ARM7
	uint16 touchX,   touchY;   // raw x/y TSC SPI
	sint16 touchXpx, touchYpx; // TFT x/y pixel (converted)
	
	sint16 touchZ1,  touchZ2;  // TSC x-panel measurements
    uint16 tdiode1,  tdiode2;  // TSC temperature diodes
    uint32 temperature;        // TSC computed temperature
	
	struct tm tmInst;	//DateTime
	ulong ndsRTCSeconds; //DateTime in epoch time (seconds) starting from January 1, 1970 (midnight UTC/GMT)
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
	bool valid_dsfwsettings;	//true or false
	uint8 lang_flags[0x2];
	//Internal use, use functions inside mem_handler_shared.c for accessing those from BOTH ARM Cores.
	uint32 arm7startaddress;
	uint32 arm7endaddress;
	
	uint32 arm9startaddress;
	uint32 arm9endaddress;
	
	uint32 WRAM_CR_ISSET;	//0 when ARM7 boots / 1 by ARM9 when its done
	
	//DS Firmware	Settings default set
	struct sDSFWSETTINGS DSFWSETTINGSInst;
	struct sEXTKEYIN	EXTKEYINInst;
	
	//FIFO Mesagging: used when 3+ args sent between ARM cores through FIFO interrupts.
	uint32 fifoMesaggingQueue[0x10];
	
	//IPC Mesagging: used when 1+ args sent between ARM Cores through IPC interrupts.
	u8 ipcMesaggingQueue[0x10];
	
	//ARM7 DLDI implementation
	#ifdef ARM7_DLDI
	struct sTGDSDLDIARM7Context dldi7TGDSCtx;
	#endif
	
} __attribute__((aligned (4)));

//Shared Work     027FF000h 4KB    -     -    -    R/W
#define TGDSIPCStartAddress (__attribute__((aligned (4))) struct sIPCSharedTGDS*)(0x027FF000)
#define TGDSIPCSize (int)(sizeof(struct sIPCSharedTGDS))
#define TGDSIPCUserStartAddress (u32)(0x027FF000 + TGDSIPCSize)	//u32 because it`s unknown at this point. TGDS project will override it to specific USER IPC struct

#endif

#ifdef __cplusplus
extern "C"{
#endif

//weak symbols : the implementation of these is project-defined, also abstracted from the hardware IPC FIFO Implementation for easier programming.
extern __attribute__((weak))	void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2);
extern __attribute__((weak))	void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2);

extern void HandleFifoNotEmpty();
extern void HandleFifoEmpty();
extern void SendFIFOWords(uint32 data0, uint32 data1);

extern void idleIPC();
extern uint8 receiveByteIPC();
extern void sendByteIPC(uint8 inByte);
extern void sendMultipleByteIPC(uint8 inByte0, uint8 inByte1, uint8 inByte2, uint8 inByte3);

#ifdef __cplusplus
}
#endif