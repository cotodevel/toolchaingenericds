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

#ifndef __ipcfifoTGDS_h__
#define __ipcfifoTGDS_h__

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
#include "cartHeader.h"
#include "posixHandleTGDS.h"

#ifdef ARM9
#include "nds_cp15_misc.h"
#endif

//FIFO Hardware
//void Write8bitAddrExtArm
//void Write16bitAddrExtArm
//void Write32bitAddrExtArm
#define WRITE_EXTARM_8	(uint32)(0xffff0200)
#define WRITE_EXTARM_16	(uint32)(0xffff0201)
#define WRITE_EXTARM_32	(uint32)(0xffff0202)

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
	//shut down (NTR) DS unit
	#define FIFO_SHUTDOWN_DS	(uint32)(0xffff021A)
	
//DLDI ARM7
#define TGDS_DLDI_ARM7_STATUS_INIT	(int)(0xffff0300)
	#define TGDS_DLDI_ARM7_STATUS_STAGE0	(int)(0xffff1010)	//ARM9 Inits ARM7 DLDI context. Passes the target DLDI address and waits for DLDI to be relocated to such address.
	
#define TGDS_DLDI_ARM7_INIT_OK	(uint32)(0xffff0301)
#define TGDS_DLDI_ARM7_INIT_ERROR	(uint32)(0xffff0302)

#define TGDS_DLDI_ARM7_STATUS_DEINIT	(int)(0xffff0309)

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

#define TGDS_ARM7_TURNOFF_BACKLIGHT (uint32)(0xFFFF0220)
#define TGDS_ARM7_TURNON_BACKLIGHT (uint32)(0xFFFF0227)
#define TGDS_ARM7_DISABLE_SLEEPMODE (uint32)(0xFFFF0221)
#define TGDS_ARM7_ENABLE_SLEEPMODE (uint32)(0xFFFF0222)
#define TGDS_ARM7_ENABLE_EVENT_HANDLING (uint32)(0xFFFF0223)
#define TGDS_ARM7_DISABLE_EVENT_HANDLING (uint32)(0xFFFF0224)
#define TGDS_ARM7_ENABLE_SLEEPMODE_TIMEOUT (uint32)(0xFFFF0225)
#define TGDS_ARM7_SET_EVENT_HANDLING (uint32)(0xFFFF0226)
#define TGDS_ARM7_PRINTF7SETUP (uint32)(0xFFFF0228)
#define TGDS_ARM7_PRINTF7 (uint32)(0xFFFF0229)
#define TGDS_ARM7_SETUPEXCEPTIONHANDLER (uint32)(0xFFFF022A)
#define TGDS_ARM7_SETUPARMCPUMALLOCANDDLDI (uint32)(0xFFFF022C)
#define TGDS_ARM7_DETECTTURNOFFCONSOLE (uint32)(0xFFFF022D)
#define TGDS_ARM7_ENABLESOUNDSAMPLECTX (uint32)(0xFFFF022E)
#define TGDS_ARM7_DISABLESOUNDSAMPLECTX (uint32)(0xFFFF022F)
#define TGDS_ARM7_INITSTREAMSOUNDCTX (uint32)(0xFFFF0230)

#define TGDS_ARM7_REQ_SLOT1_DISABLE (uint32)(0xFFFF0231)
#define TGDS_ARM7_REQ_SLOT1_ENABLE (uint32)(0xFFFF0232)

#define TGDS_ARM7_READFLASHMEM (u32)(0xffffAAC0)		
#define TGDS_ARM7_RELOADFLASH (u32)(0xFFFFABC0)		
#define TGDS_ARM7DLDI_ENABLED (u32)(0xFFFFABC1)
#define TGDS_ARM7DLDI_DISABLED (u32)(0xFFFFABC2)

#define TGDS_ARMCORES_REPORT_PAYLOAD_MODE (u32)(0xFFFFABC3)


//ARM7 FS IPC Commands
#define TGDS_LIBNDSFIFO_COMMAND (u32)(0xFFFFAAC1)	//Bottom 8 bits act as the FIFO Channel Index

//TGDS IPC Command Interrupt Index
#define IPC_NULL_CMD					(u8)(0)	//NULL cmd is unused by TGDS, fallbacks to TGDS project IPCIRQ Handler
#define IPC_SEND_MULTIPLE_CMDS			(u8)(1)

//ARM7 Direct Memory
#define IPC_ARM7READMEMORY_REQBYIRQ		(u8)(2)
#define IPC_ARM7SAVEMEMORY_REQBYIRQ		(u8)(3)

//ARM7DLDI Hardware
	//Slot1 or slot2 access
	#define IPC_READ_ARM7DLDI_REQBYIRQ		(u8)(4)
	#define IPC_WRITE_ARM7DLDI_REQBYIRQ		(u8)(5)

	//SD TWL Access
	#define IPC_READ_ARM7_TWLSD_REQBYIRQ		(u8)(6)
	#define IPC_WRITE_ARM7_TWLSD_REQBYIRQ		(u8)(7)
	
	#define IPC_STARTUP_ARM7_TWLSD_REQBYIRQ		(u8)(8)
	#define IPC_SD_IS_INSERTED_ARM7_TWLSD_REQBYIRQ		(u8)(9)
	
#define IPC_TGDSUSER_START_FREE_INDEX	(u8)(10)	//TGDS User Project rely on it

//TGDS -> Libnds FIFO compatibility API. Ensures the behaviour of the FIFO messaging system works.
// FIFO_CHANNEL_BITS - number of bits used to specify the channel in a packet - default=4
#define FIFO_CHANNEL_BITS				(int)(4)

// FIFO_MAX_DATA_WORDS - maximum number of bytes that can be sent in a fifo message
#define FIFO_MAX_DATA_BYTES				(int)(128)

// FIFO_CHANNELS - fifo channel number allowed to work with
#define FIFO_CHANNELS				(int)(16)

//! Enum values for the different fifo channels.
typedef enum {
	FIFO_PM			= (int)(0),	/*!< \brief fifo channel reserved for power management. */
	FIFO_SOUND		= (int)(1),	/*!< \brief fifo channel reserved for sound access. */
	FIFO_SYSTEM		= (int)(2),	/*!< \brief fifo channel reserved for system functions. */
	FIFO_MAXMOD		= (int)(3),	/*!< \brief fifo channel reserved for the maxmod library. */
	FIFO_DSWIFI		= (int)(4),	/*!< \brief fifo channel reserved for the dswifi library. */
	FIFO_SDMMC		= (int)(5),	/*!< \brief fifo channel reserved for dsi sdmmc control. */
	FIFO_FIRMWARE	= (int)(6),	/*!< \brief fifo channel reserved for firmware access. */
	FIFO_RSVD_01	= (int)(7),	/*!< \brief fifo channel reserved for future use. */
	FIFO_USER_01	= (int)(8),	/*!< \brief fifo channel available for users. */
	FIFO_USER_02	= (int)(9),	/*!< \brief fifo channel available for users. */
	FIFO_USER_03	= (int)(10),	/*!< \brief fifo channel available for users. */
	FIFO_USER_04	= (int)(11),	/*!< \brief fifo channel available for users. */
	FIFO_USER_05	= (int)(12),	/*!< \brief fifo channel available for users. */
	FIFO_USER_06	= (int)(13),	/*!< \brief fifo channel available for users. */
	FIFO_USER_07	= (int)(14),	/*!< \brief fifo channel available for users. */
	FIFO_USER_08	= (int)(15),	/*!< \brief fifo channel available for users. */
} FifoChannels;

/* Standardized base handler compatible with all the others */
typedef void (*FifoHandlerFunc)(int bufferSize, u32 userdata);

struct libndsFIFOs {
	//Libnds FIFO implementation
	int channelBufferSize[FIFO_CHANNELS];	//if == 0: empty, if > 0: used
	u8 channelBuffer[FIFO_CHANNELS*FIFO_MAX_DATA_BYTES];	//value: usually used as a void * address or u32 value 
};

typedef struct sIPCSharedTGDS {
    
	//NDS Header: Set up by ARM9 on boot.
	struct sDSCARTHEADER DSHeader;
	
	//Load firmware from FLASH
	struct sDSFWHEADER DSFWHEADERInst;
	
	uint16 rawx,   rawy;   // raw x/y TSC SPI
	sint16 touchXpx, touchYpx; // TFT x/y pixel (converted)
	
	sint16 touchZ1,  touchZ2;  // TSC x-panel measurements
    uint16 tdiode1,  tdiode2;  // TSC temperature diodes
    uint32 temperature;        // TSC computed temperature
	
	struct tm tmInst;	//DateTime
	ulong ndsRTCSeconds; //DateTime in epoch time (seconds) starting from January 1, 1970 (midnight UTC/GMT)
	uint8	dayOfWeek;	//Updated when the above inst is updated
	
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
	
	uint32 WRAM_CR_ISSET;	//0 when ARM7 boots / 1 by ARM9 when its done
	
	//IPC Mesagging: used when 1+ args sent between ARM Cores through IPC interrupts.
	u8 ipcMesaggingQueue[0x10];
	
	struct soundSampleContextList soundContextShared;
	int screenOrientationMainEngine;
	int screenOrientationSubEngine;
	bool touchScreenEnabled;
	
	//args through ARM7 print debugger
	int argvCount;
	
	//ARM7 Filesystem
	bool initStatus;
	u32 ARM7FSOperationStatus;	//IO busy/idle
	
	//File from ARM7
	int IR;				//Command Status. This is used as acknowledge from IR sent through IPC IRQ Index Command
	int IR_filesize;	//file size
	u32 *IR_readbuf;
	int IR_readbufsize;
	int IR_ReadOffset;
	int IR_WrittenOffset;
	
	//DS Firmware	Settings default set
	struct sDSFWSETTINGS DSFWSETTINGSInst;
	
	u16 ARM7REG_KEYXY;	//uint16 buttons7;  // X, Y, /PENIRQ buttons
    u16 ARM7REG_KEYINPUT; //uint16 KEYINPUT7;			//REG_KEYINPUT ARM7
	
	//Soundstream
	SoundRegion soundIPC;
	
	struct libndsFIFOs libndsFIFO;
	
	//FIFO Mesagging: used by TGDS-multiboot loader code only.
	uint32 fifoMesaggingQueueSharedRegion[4];	//4 Words for various command handling which can't use the NDS_CACHED_SCRATCHPAD / NDS_UNCACHED_SCRATCHPAD at the time
	
} IPCSharedTGDS __attribute__((aligned(4)));

//FIFO Mesagging: used when 3+ args sent between ARM cores through FIFO interrupts.
#define NDS_CACHED_SCRATCHPAD	(int)(((int)0x02400000 - (4096)) | 0x400000)
#define NDS_UNCACHED_SCRATCHPAD	(int)(NDS_CACHED_SCRATCHPAD | 0x400000)

//Shared Work     027FF000h 4KB    -     -    -    R/W
#define TGDSIPCStartAddress (struct sIPCSharedTGDS*)(0x027FF000)
#define TGDSIPCSize (int)(sizeof(struct sIPCSharedTGDS))
#define TGDSIPCUserStartAddress (u32)( ((u32)TGDSIPCStartAddress) + TGDSIPCSize)	//u32 because it`s unknown at this point. TGDS project will override it to specific USER IPC struct. Known as GetUserIPCAddress()
#define IPCRegionSize	(sint32)(4*1024)

//TouchScreen
struct XYTscPos{
	uint16	rawx; 
	uint16	rawy; 
	uint16	touchXpx;   
	uint16	touchYpx;
	uint16	z1;   
	uint16	z2;
};

struct xyCoord{
	uint16 x;
	uint16 y;
};

static inline void sendIPCIRQOnly(){
	REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
}

//Slower

static inline void sendByteIPCNOIRQ(uint8 inByte){
	REG_IPC_SYNC = (((REG_IPC_SYNC&0xfffff0ff) | (inByte<<8)) & ~(IPC_SYNC_IRQ_REQUEST));
}

static inline void sendByteIPC(uint8 inByte){
	REG_IPC_SYNC = ((REG_IPC_SYNC&0xfffff0ff) | (inByte<<8) | IPC_SYNC_IRQ_REQUEST);	// (1<<13) Send IRQ to remote CPU      (0=None, 1=Send IRQ)
}

static inline uint8 receiveByteIPC(){
	return (REG_IPC_SYNC&0xf);
}

#endif

#ifdef __cplusplus
extern "C"{
#endif

//weak symbols : the implementation of these is project-defined, also abstracted from the hardware IPC FIFO Implementation for easier programming.
extern 	void HandleFifoNotEmptyWeakRef(volatile u32 cmd1);
extern 	void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2);

#ifdef ARM7
extern void XYReadScrPos(struct XYTscPos * StouchScrPosInst);	//TSC Impl.
extern struct xyCoord readTSC();
#endif

extern void XYReadScrPosUser(struct XYTscPos * StouchScrPosInst);	//User impl.
extern void HandleFifoNotEmpty();
extern void HandleFifoEmpty();
extern void SendFIFOWords(uint32 data0);

extern void ReadMemoryExt(u32 * srcMemory, u32 * targetMemory, int bytesToRead);
extern void SaveMemoryExt(u32 * srcMemory, u32 * targetMemory, int bytesToRead);
extern void ReadFirmwareARM7Ext(u32 * srcMemory);

extern void Write32bitAddrExtArm(uint32 address, uint32 value);
extern void Write16bitAddrExtArm(uint32 address, uint16 value);
extern void Write8bitAddrExtArm(uint32 address, uint8 value);

//arg 0: channel
//arg 1: arg0: handler, arg1: userdata
extern u32 fifoFunc[FIFO_CHANNELS][2];	//context is only passed on callback prototype stage, because, the channel index generates the callee callback
extern u8 CheckStylus();

#ifdef ARM9
extern u8 ARM7ReadFWVersionFromFlashByFIFOIRQ();
#endif

#ifdef __cplusplus
}
#endif