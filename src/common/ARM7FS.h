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

#ifndef __ARM7FS_h__
#define __ARM7FS_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "ipcfifoTGDS.h"

#ifdef ARM9
#include "fatfslayerTGDS.h"
#endif

//TGDS IPC Index commands reserved:	1, 2, 3
#define IR_MP2_Idle (u8)(IPC_NULL_CMD)		//read process
#define IR_MP2_Busy (u8)(4)					//

#define IR_MP2_Init (u8)(5)			//Init MP2 ARM7 FS
#define IR_MP2_Deinit (u8)(6)		//De-init MP2 ARM7 FS

#define IR_ARM7FS_Read (u32)(0xffffaaa0)		
#define IR_ARM7FS_Save (u32)(0xffffaab0)		

static inline bool getARM7FSInitStatus(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->initStatus, sizeof(TGDSIPC->initStatus));
	#endif
	return TGDSIPC->initStatus;
}

static inline void setARM7FSInitStatus(bool ARM7FSInitStatus){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->initStatus, sizeof(TGDSIPC->initStatus));
	#endif
	TGDSIPC->initStatus = ARM7FSInitStatus;
}

//IO
static inline int getARM7FSIOStatus(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSStatus, sizeof(TGDSIPC->ARM7FSStatus));
	#endif
	return TGDSIPC->ARM7FSStatus;
}

static inline void setARM7FSIOStatus(int ARM7FSStatus){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSStatus, sizeof(TGDSIPC->ARM7FSStatus));
	#endif
	TGDSIPC->ARM7FSStatus = ARM7FSStatus;
}

//Global Transaction Status
static inline int getARM7FSTransactionStatus(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSTransactionStatus, sizeof(TGDSIPC->ARM7FSTransactionStatus));
	#endif
	return TGDSIPC->ARM7FSTransactionStatus;
}

static inline void setARM7FSTransactionStatus(int ARM7FSTransactionStatus){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSTransactionStatus, sizeof(TGDSIPC->ARM7FSTransactionStatus));
	#endif
	TGDSIPC->ARM7FSTransactionStatus = ARM7FSTransactionStatus;
}



static inline void ackARM7FSCommand(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->IR, sizeof(TGDSIPC->IR));
	#endif
	TGDSIPC->IR = IR_MP2_Idle;
}

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern u32 handleCommand;	//sadly ARM7FS commands must be issued from mainloop, IRQs seem to cause lockups

#ifdef ARM7
//Internal impl.
extern int FileSys_GetFileSize(void);
extern int ARM7FS_BufferReadByIRQ(void *OutBuffer, int fileOffset, int readBufferSize);
extern int ARM7FS_BufferSaveByIRQ(void *InBuffer, int fileOffset, int writeBufferSize);
extern void initARM7FS(char * ARM7FS_ARM9Filename);	//ARM7

//Test case
extern void performARM7MP2FSTestCase(char * ARM7fsfname, int ARM7BuffSize, u32 * writtenDebug);	//ARM7
#endif

#ifdef ARM9
//Internal impl.
extern bool initARM7FS(char * inFilename, char * outFilename, int splitBufferSize, u32 * debugVar);	//ARM9.
extern void closeARM7FS();	//Must use this!! Instead deinitARM7FS(); directly when closing an ARM7FS session

extern int ARM7FS_ReadBuffer_ARM9Callback(u8 * outBuffer, int fileOffset, FILE * fIn, int bufferSize);
extern int ARM7FS_SaveBuffer_ARM9Callback(u8 * inBuffer, int fileOffset, FILE * fOut, int bufferSize);
extern FILE * ARM7FS_FileHandleWrite;
extern FILE * ARM7FS_FileHandleRead;

//Test Case
extern void performARM7MP2FSTestCase(char * inFilename, char * outFilename, int splitBufferSize, u32 * debugVar);	//ARM9 

#endif

extern void deinitARM7FS();

#ifdef __cplusplus
}
#endif