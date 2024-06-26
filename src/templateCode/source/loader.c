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

#include "loader.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "biosTGDS.h"
#include "debugNocash.h"
#include "dldi.h"
#include "exceptionTGDS.h"

#ifdef ARM9
#include "videoTGDS.h"

//ToolchainGenericDS-multiboot NDS Binary loader: Requires tgds_multiboot_payload_ntr.bin / tgds_multiboot_payload_twl.bin (TGDS-multiboot Project) in SD root.
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool TGDSMultibootRunNDSPayload(char * filename, u8 * tgdsMbv3ARM7Bootldr) {
	char * TGDSMBPAYLOAD = NULL;
	register int isNTRTWLBinary = isNTROrTWLBinary(filename);
	memset(msgDebugException, 0, MAX_TGDSFILENAME_LENGTH);
	//NTR mode? Can only boot valid NTR binaries, the rest is skipped.
	if((__dsimode == false) && !(isNTRTWLBinary == isNDSBinaryV1Slot2) && !(isNTRTWLBinary == isNDSBinaryV1) && !(isNTRTWLBinary == isNDSBinaryV2) && !(isNTRTWLBinary == isNDSBinaryV3) ){
		return false;
	}
	//TWL mode? Can only boot valid NTR and TWL binaries, the rest is skipped.
	else if((__dsimode == true) && !(isNTRTWLBinary == isNDSBinaryV1Slot2) && !(isNTRTWLBinary == isNDSBinaryV1) && !(isNTRTWLBinary == isNDSBinaryV2) && !(isNTRTWLBinary == isNDSBinaryV3) && !(isNTRTWLBinary == isTWLBinary) ){
		return false;
	}
	else{
		//Run payload depending on current NTR/TWL Hardware supported.
		if (__dsimode == true){
			TGDSMBPAYLOAD = "0:/tgds_multiboot_payload_twl.bin";	//TGDS TWL SDK (ARM9i binaries) emits TGDSMultibootRunNDSPayload() which reloads into TWL TGDS-MB Reload payload
		}
		else {
			TGDSMBPAYLOAD = "0:/tgds_multiboot_payload_ntr.bin";	//TGDS NTR SDK (ARM9 binaries) emits TGDSMultibootRunNDSPayload() which reloads into NTR TGDS-MB Reload payload
		}
		
		//Execute Stage 1: IWRAM ARM7 payload: NTR/TWL (0x03800000)
		executeARM7Payload((u32)0x02380000, 96*1024, (u32*)TGDS_MB_V3_ARM7_STAGE1_ADDR);
			
		//Execute Stage 2: VRAM ARM7 payload: NTR/TWL (0x06000000)
		executeARM7Payload((u32)0x02380000, 96*1024, tgdsMbv3ARM7Bootldr);
		
		//Copy the file into non-case sensitive "tgdsboot.bin" into ARM7,
		//since PetitFS only understands 8.3 DOS format filenames.
		//File copy in chunks, will be slower to boot TWL homebrew, but will handle them properly regardless of its binary size.
		FIL fPagingFDRead;
		FIL fPagingFDWrite;
		UINT ret;
		
		//Homebrew source
		int flags = charPosixToFlagPosix("r");
		BYTE mode = posixToFatfsAttrib(flags);
		FRESULT result = f_open(&fPagingFDRead, (const TCHAR*)filename, mode);
		if(result != FR_OK){
			printf("tgds_multiboot_payload.bin: read (1)");
			printf("payload fail [%s]", filename);
			while(1==1){}
		}
		int payloadSize = (int)f_size(&fPagingFDRead);
		u8* workBuffer = (u8*)TGDSARM9Malloc(TGDS_MB_V3_WORKBUFFER_SIZE);
		
		//Homebrew target
		char * tempFile = TGDS_MB_V3_BOOTSTUB_FILENAME;
		flags = charPosixToFlagPosix("w+");
		mode = posixToFatfsAttrib(flags);
		result = f_open(&fPagingFDWrite, (const TCHAR*)tempFile, mode);
		if(result != FR_OK){
			printf("tgds_multiboot_payload.bin: read (3)");
			printf("payload fail [%s]", tempFile);
			while(1==1){}
		}
		f_lseek (
				&fPagingFDWrite,
				(DWORD)0       
			);
			
		int blocksToRead = (payloadSize / TGDS_MB_V3_WORKBUFFER_SIZE);
		int blocksWritten = 0;
		
		while(blocksToRead > blocksWritten){
			f_lseek (
				&fPagingFDRead,
				(DWORD) (blocksWritten * TGDS_MB_V3_WORKBUFFER_SIZE)
			);
			
			result = f_read(&fPagingFDRead, workBuffer, (int)TGDS_MB_V3_WORKBUFFER_SIZE, (UINT*)&ret);
			coherent_user_range_by_size((uint32)workBuffer, (int)TGDS_MB_V3_WORKBUFFER_SIZE);
			
			int writtenSize=0;
			result = f_write(&fPagingFDWrite, workBuffer, (int)TGDS_MB_V3_WORKBUFFER_SIZE, (UINT*)&writtenSize); //workbuffer is already coherent
			if (result != FR_OK){
				printf("tgds_multiboot_payload.bin: write (4)");
				printf("payload fail [%s]", tempFile);
				while(1==1){}
			}
			f_sync(&fPagingFDWrite); //make persistent file in filesystem coherent
			
			blocksWritten++;
		}
		
		//write last part not considered?
		blocksToRead = (payloadSize % TGDS_MB_V3_WORKBUFFER_SIZE);
		if(blocksToRead > 0){
			f_lseek (
				&fPagingFDRead,
				(DWORD) (blocksWritten * TGDS_MB_V3_WORKBUFFER_SIZE)
			);
			
			result = f_read(&fPagingFDRead, workBuffer, (int)blocksToRead, (UINT*)&ret);
			if(ret != blocksToRead){
				printf("tgds_multiboot_payload.bin: read (2)");
				printf("payload fail [%s]", filename);
				while(1==1){}
			}
			coherent_user_range_by_size((uint32)workBuffer, (int)blocksToRead);
			
			int writtenSize=0;
			result = f_write(&fPagingFDWrite, workBuffer, (int)blocksToRead, (UINT*)&writtenSize); //workbuffer is already coherent
			if (result != FR_OK){
				printf("tgds_multiboot_payload.bin: write (4)");
				printf("payload fail [%s]", tempFile);
				while(1==1){}
			}
			f_sync(&fPagingFDWrite); //make persistent file in filesystem coherent
		}
		
		//Free
		f_close(&fPagingFDRead);
		f_close(&fPagingFDWrite);
		TGDSARM9Free(workBuffer);
		
		setValueSafe((u32*)ARM9_TWLORNTRPAYLOAD_MODE, (u32)isNTRTWLBinary); 
		setValueSafe((u32*)ARM7_ARM9_SAVED_DSFIRMWARE, savedDSHardware); //required by TGDS-multiboot's tgds_multiboot_payload.bin
		FILE * tgdsPayloadFh = fopen(TGDSMBPAYLOAD, "r");
		if(tgdsPayloadFh != NULL){
			fseek(tgdsPayloadFh, 0, SEEK_SET);
			int	tgds_multiboot_payload_size = FS_getFileSizeFromOpenHandle(tgdsPayloadFh);
			fread((u32*)TGDS_MB_V3_PAYLOAD_ADDR, 1, tgds_multiboot_payload_size, tgdsPayloadFh);
			coherent_user_range_by_size(TGDS_MB_V3_PAYLOAD_ADDR, (int)tgds_multiboot_payload_size);
			fclose(tgdsPayloadFh);
			FS_deinit();
			bool stat = dldiPatchLoader((data_t *)TGDS_MB_V3_PAYLOAD_ADDR, (u32)tgds_multiboot_payload_size, (u32)&_io_dldi_stub);
			if(stat == false){
				sprintf(msgDebugException, "%s%s", "TGDSMultibootRunNDSPayload(): DLDI Patch failed. NTR/TWL binary missing DLDI section.", "");
				nocashMessage((char*)&msgDebugException[0]);
			}
			else{
				sprintf(msgDebugException, "%s", "TGDSMultibootRunNDSPayload(): DLDI Patch OK.");
				nocashMessage((char*)&msgDebugException[0]);
			}
			REG_IME = 0;
			
			typedef void (*t_bootAddr)();
			t_bootAddr bootARM9Payload = (t_bootAddr)TGDS_MB_V3_PAYLOAD_ADDR_TWL;
			
			//TWL mode? then: 
			//1) Upcoming bootloader will run from mirror #2 of @0x2328000 (NTR 4MB EWRAM) or @0x2F28000 (TWL 16MB EWRAM by copying bootloader section)
			//2) Go back to NTR 4MB EWRAM, in order to read binary context mirrored properly on NTR/TWL hardware. 
			if (__dsimode == true){
				//Enable 16M EWRAM (TWL)
				u32 SFGEXT9 = *(u32*)0x04004008;
				//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
				SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x2 << 14);
				*(u32*)0x04004008 = SFGEXT9;
				
				memcpy((void *)TGDS_MB_V3_PAYLOAD_ADDR_TWL, (const void *)TGDS_MB_V3_PAYLOAD_ADDR, tgds_multiboot_payload_size);
				coherent_user_range_by_size((uint32)TGDS_MB_V3_PAYLOAD_ADDR_TWL, (int)tgds_multiboot_payload_size);
				
				//Enable 4M EWRAM (TWL)
				SFGEXT9 = *(u32*)0x04004008;
				//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
				SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x0 << 14);
				*(u32*)0x04004008 = SFGEXT9;				
			}
			
			bootARM9Payload();
			return true; //should never jump here
		}
		else{
			strcpy(msgDebugException, TGDSMBPAYLOAD);
			u8 fwNo = *(u8*)ARM7_ARM9_SAVED_DSFIRMWARE;
			int stage = 3;
			handleDSInitError(stage, (u32)fwNo);	
		}
	}
	return false;
}


//ARM9: Executes ARM7 payload at runtime
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void executeARM7Payload(u32 arm7entryaddress, int arm7BootCodeSize, u32 * payload){
	//1) Give VRAM_D to ARM7 @0x06000000
	*(u8*)0x04000243 = (VRAM_D_0x06000000_ARM7 | VRAM_ENABLE);
	//2) Initialize ARM7DLDI: ARM9 passes its DLDI section to ARM7
	setValueSafe(ARM7_ARM9_DLDI_STATUS, (u32)&_io_dldi_stub);
	if(payload != ((u32*)0)){
		coherent_user_range_by_size((u32)payload, arm7BootCodeSize);
	}
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (u32)payload);
	setValueSafe(&fifomsg[1], (u32)arm7BootCodeSize);
	setValueSafe(&fifomsg[2], (u32)arm7entryaddress);
	setValueSafe(&fifomsg[7], (u32)FIFO_ARM7_RELOAD);	//ARM9: Execute ARM7 payload-> ARM7
	int TGDSInitLoopCount = 0;
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF); 
	while( getValueSafe(ARM7_ARM9_DLDI_STATUS) == ((u32)&_io_dldi_stub) ){
		if(TGDSInitLoopCount > 1048576){
			u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
			int stage = 9;
			handleDSInitError(stage, (u32)fwNo);			
		}
		TGDSInitLoopCount++;
		swiDelay(1);
	}
}
#endif
