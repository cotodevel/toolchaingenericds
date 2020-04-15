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

#include "ARM7FS.h"
#include "ipcfifoTGDS.h"
#include "typedefsTGDS.h"
#include "dmaTGDS.h"
#include "dldi.h"
#include "posixHandleTGDS.h"

#ifdef ARM7
#include "xmem.h"
#endif

#ifdef ARM9
#include "fatfslayerTGDS.h"
#endif

#ifdef ARM7
int FileSys_GetFileSize(void)
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	return(TGDSIPC->IR_filesize);
}

int ARM7FS_BufferReadByIRQ(void *OutBuffer, int fileOffset, int readBufferSize){
	if(readBufferSize == 0){
		return 0;
	}
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	//Index 0 -- 4 used, do not use.
	fifomsg[5] = (uint32)TGDSIPC->IR_readbuf;
	fifomsg[6] = (uint32)readBufferSize;
	fifomsg[7] = (uint32)fileOffset;
	setARM7FSIOStatus(ARM7FS_IOSTATUS_BUSY);
	
	//Wait until ARM9 task done.
	SendFIFOWords(IR_ARM7FS_Read, (u32)fifomsg);
	while(getARM7FSIOStatus() == ARM7FS_IOSTATUS_BUSY){
		swiDelay(1);
	}
	
	dmaTransferWord(0, (u32)((u8*)TGDSIPC->IR_readbuf), (uint32)(u32)OutBuffer, (uint32) readBufferSize);	//dmaTransferHalfWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count)
	return readBufferSize;
}

int ARM7FS_BufferSaveByIRQ(void *InBuffer, int fileOffset, int writeBufferSize){
	if(writeBufferSize == 0){
		return 0;
	}
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	dmaTransferWord(0, (uint32)(u32)InBuffer, (u32)TGDSIPC->IR_readbuf, (uint32)writeBufferSize);	//dmaTransferHalfWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count)
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	//Index 0 -- 4 used, do not use.
	fifomsg[5] = (uint32)TGDSIPC->IR_readbuf;
	fifomsg[6] = (uint32)writeBufferSize;
	fifomsg[7] = (uint32)fileOffset;
	setARM7FSIOStatus(ARM7FS_IOSTATUS_BUSY);
	
	//Wait until ARM9 task done.
	SendFIFOWords(IR_ARM7FS_Save, (u32)fifomsg);
	while(getARM7FSIOStatus() == ARM7FS_IOSTATUS_BUSY){
		swiDelay(1);
	}
	
	return writeBufferSize;
}

//Test case: reads a whole file into ARM9 and dumps it to fat fs
void performARM7MP2FSTestCase(char * ARM7fsfname, int ARM7BuffSize, u32 * writtenDebug){	//ARM7 impl.
	while(getARM7FSInitStatus() == false){	//Wait for ARM7Init
		swiDelay(1);
	}
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	
	//ARM7 MP2 FS test case start
	u8* buffer =(u8*)Xmalloc(ARM7BuffSize);
	TGDSIPC->IR_readbufsize = ARM7BuffSize;
	int filesize = TGDSIPC->IR_filesize;
	
	int i = 0;
	int fileOffst = 0;
	for(i = 0; i < filesize/ARM7BuffSize; i++){
		dmaFillHalfWord(0, 0, (u32)buffer, ARM7BuffSize);	//clear buffer to prevent false positive
		ARM7FS_BufferReadByIRQ(buffer, fileOffst, ARM7BuffSize);
		dmaFillHalfWord(0, 0, (u32)TGDSIPC->IR_readbuf, ARM7BuffSize);	//clear buffer to prevent false positive
		ARM7FS_BufferSaveByIRQ(buffer, fileOffst, ARM7BuffSize);
		fileOffst+= (ARM7BuffSize);
	}
	
	//left?
	int left = 0;
	if(fileOffst < filesize){
		left = filesize - fileOffst;	
		dmaFillHalfWord(0, 0, (u32)buffer, left);	//clear buffer to prevent false positive
		ARM7FS_BufferReadByIRQ(buffer, fileOffst, left);
		dmaFillHalfWord(0, 0, (u32)TGDSIPC->IR_readbuf, left);	//clear buffer to prevent false positive
		ARM7FS_BufferSaveByIRQ(buffer, fileOffst, left);
		fileOffst+=(left);
	}
	
	*writtenDebug = (u32)(TGDSIPC->IR_readbuf);
	Xfree(buffer);
	
	//end testcase
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_IDLE);
}

void initARM7FS(char * ARM7FS_ARM9Filename){	//ARM7 Impl.
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	setARM7FSInitStatus(true); //finish ARM7FS 
	//ARM7 continues with performARM7MP2FSTestCase(testcaseFilename, splitBufferSize)...
}
#endif

void deinitARM7FS(){
	/*
	#ifdef ARM7
	#endif
	
	#ifdef ARM9
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	TGDSIPC->IR_readbufsize=0;
	//todo add: ARM9 updatable malloc
	TGDSIPC->IR_readbuf=0;
	if(fout!= NULL){
		fclose(fout);
	}
	//setup vars
	if(ARM7FS_FileHandleRead !=NULL ){
		fclose(ARM7FS_FileHandleRead);
	}
	TGDSIPC->IR_readbuf=(u8*)TGDSARM9Malloc(splitBufferSize);	//Must be EWRAM because then ARM7 can receive it into ARM7's 0x06000000 through DMA (hardware ring buffer)
	ARM7FS_FileHandleRead = fopen(inFilename, "r");
	ARM7FS_FileHandleWrite = fopen(outFilename, "w+");
	TGDSIPC->IR_ReadOffset = 0;
	TGDSIPC->IR_WrittenOffset = 0;
	TGDSIPC->IR_filesize = FS_getFileSizeFromOpenHandle(ARM7FS_FileHandleRead);
	if(ARM7FS_FileHandleRead != NULL){
		printf("fileopenOK: Size: %d",TGDSIPC->IR_filesize);
	}
	//ARM7 MP2 FS test case start.
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	
	//Wait for ARM7FS init.
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (u32)inFilename;
	fifomsg[1] = (uint32)TGDSIPC->IR_filesize;
	fifomsg[2] = (uint32)splitBufferSize;
	fifomsg[3] = (uint32)IPC_ARM7INIT_ARM7FS;
	fifomsg[4] = (uint32)debugVar;
	fifomsg[5] = (uint32)0xc070c070;	//Test case enable
	
	sendByteIPC(IPC_ARM7INIT_ARM7FS);
	while(fifomsg[3] == IPC_ARM7INIT_ARM7FS){
		swiDelay(1);
	}
	
	//Test Case: file read/writes are async (through interrupts)
	
	//wait until async file writes are done.
	while(getARM7FSTransactionStatus() == ARM7FS_TRANSACTIONSTATUS_BUSY){
		swiDelay(1);
	}
	
	fclose(ARM7FS_FileHandleRead);
	fclose(ARM7FS_FileHandleWrite);
	TGDSARM9Free((u8*)TGDSIPC->IR_readbuf);
	printf("ARM7FS() Test Case: end!");
	#endif
	*/
}

#ifdef ARM9
void initARM7FS(char * inFilename, char * outFilename, FILE * fout, int splitBufferSize, u32 * debugVar){	//ARM9. 
	
}

void performARM7MP2FSTestCase(char * inFilename, char * outFilename, FILE * fout, int splitBufferSize, u32 * debugVar){	//ARM9 Impl.
	printf("ARM7FS() Test Case: start!");
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	TGDSIPC->IR_readbufsize=0;
	TGDSIPC->IR_readbuf=0;
	if(fout!= NULL){
		fclose(fout);
	}
	//setup vars
	if(ARM7FS_FileHandleRead !=NULL ){
		fclose(ARM7FS_FileHandleRead);
	}
	TGDSIPC->IR_readbuf=(u8*)TGDSARM9Malloc(splitBufferSize);	//Must be EWRAM because then ARM7 can receive it into ARM7's 0x06000000 through DMA (hardware ring buffer)
	ARM7FS_FileHandleRead = fopen(inFilename, "r");
	ARM7FS_FileHandleWrite = fopen(outFilename, "w+");
	TGDSIPC->IR_ReadOffset = 0;
	TGDSIPC->IR_WrittenOffset = 0;
	TGDSIPC->IR_filesize = FS_getFileSizeFromOpenHandle(ARM7FS_FileHandleRead);
	if(ARM7FS_FileHandleRead != NULL){
		printf("fileopenOK: Size: %d",TGDSIPC->IR_filesize);
	}
	//ARM7 MP2 FS test case start.
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	
	//Wait for ARM7FS init.
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (u32)inFilename;
	fifomsg[1] = (uint32)TGDSIPC->IR_filesize;
	fifomsg[2] = (uint32)splitBufferSize;
	fifomsg[3] = (uint32)IPC_ARM7INIT_ARM7FS;
	fifomsg[4] = (uint32)debugVar;
	fifomsg[5] = (uint32)0xc070c070;	//Test case enable
	
	sendByteIPC(IPC_ARM7INIT_ARM7FS);
	while(fifomsg[3] == IPC_ARM7INIT_ARM7FS){
		swiDelay(1);
	}
	
	//Test Case: file read/writes are async (through interrupts)
	
	//wait until async file writes are done.
	while(getARM7FSTransactionStatus() == ARM7FS_TRANSACTIONSTATUS_BUSY){
		swiDelay(1);
	}
	
	fclose(ARM7FS_FileHandleRead);
	fclose(ARM7FS_FileHandleWrite);
	TGDSARM9Free((u8*)TGDSIPC->IR_readbuf);
	printf("ARM7FS() Test Case: end!");
}

FILE * ARM7FS_FileHandleWrite = NULL;
FILE * ARM7FS_FileHandleRead = NULL;

//ARM7 FS: Write from ARM7 to ARM9 POSIX filehandle
int ARM7FS_SaveBuffer_ARM9Callback(u8 * inBuffer, int fileOffset, FILE * fOut, int bufferSize){
	UINT written;
	//Get TGDS file handle
	int fdindex = fileno(fOut);
	struct fd * fdinst = getStructFD(fdindex);
	f_write(fdinst->filPtr, inBuffer, bufferSize, &written);
	return (int)(written);
}

//ARM7 FS: Read from ARM9 POSIX filehandle to ARM7
int ARM7FS_ReadBuffer_ARM9Callback(u8 * outBuffer, int fileOffset, FILE * fIn, int bufferSize){
	UINT read;
	//Get TGDS file handle
	int fdindex = fileno(fIn);
	struct fd * fdinst = getStructFD(fdindex);
	f_read(fdinst->filPtr, outBuffer, bufferSize, &read);
	return (int)(read);
}

#endif