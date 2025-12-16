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
#include "dmaTGDS.h"
#include "debugNocash.h"
#include "dldi.h"
#include "exceptionTGDS.h"

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int isNTROrTWLBinaryTGDSShared(u8 * NDSHeaderStructInst, u8 * passmeRead, u32 * ARM7i_HEADER_SCFG_EXT7Inst, bool * inIsTGDSTWLHomebrew) {
	int mode = notTWLOrNTRBinary;
	struct sDSCARTHEADER * NDSHdr = (struct sDSCARTHEADER *)NDSHeaderStructInst;
	u32 arm9EntryAddress = NDSHdr->arm9entryaddress;
	u32 arm7EntryAddress = NDSHdr->arm7entryaddress;
	u32 arm9BootCodeOffsetInFile = NDSHdr->arm9romoffset;
	int arm7BootCodeSize = NDSHdr->arm7size;
	int arm9BootCodeSize = NDSHdr->arm9size;
	int checkCounter = 0;
	int i = 0;
	for(i = 0; i < sizeof(NDSHdr->reserved1); i++){
		checkCounter += NDSHdr->reserved1[i];
	}
	checkCounter += NDSHdr->reserved2;
	if(
		//(gotDLDISection == false) && //pre-DLDI era could be confused with no filesystem binaries, so skipped.
		(
			//Slot 2 passme v1 (pre 2008 NTR homebrew)
			(0x66 == passmeRead[0x0])
			&&	(0x72 == passmeRead[0x1])
			&&	(0x61 == passmeRead[0x2])
			&&	(0x6D == passmeRead[0x3])
			&&	(0x65 == passmeRead[0x4])
			&&	(0x62 == passmeRead[0x5])
			&&	(0x75 == passmeRead[0x6])
			&&	(0x66 == passmeRead[0x7])
			&&	(0x66 == passmeRead[0x8])
			&&	(0x65 == passmeRead[0x9])
			&&	(0x72 == passmeRead[0xA])
			&&	(0x5F == passmeRead[0xB])
			&&	(0x50 == passmeRead[0xC])
			&&	(0x41 == passmeRead[0xD])
			&&	(0x53 == passmeRead[0xE])
			&&	(0x53 == passmeRead[0xF])
			&&	(0x44 == passmeRead[0x10])
			&&	(0x46 == passmeRead[0x11])
			&&	(0x96 == passmeRead[0x12])
			&&	(0x00 == passmeRead[0x13])
		)
	){
		mode = isNDSBinaryV1Slot2;
	}

	else if(
		(checkCounter == 0) &&
		(arm9EntryAddress >= 0x02000000) &&
		//(gotDLDISection == false) && //pre-DLDI era could be confused with no filesystem binaries, so skipped.
		(
			//Slot 1 passme v1 (pre 2008 NTR homebrew)
			(0x00 == passmeRead[0x0])
			&&	(0x00 == passmeRead[0x1])
			&&	(0x00 == passmeRead[0x2])
			&&	(0x00 == passmeRead[0x3])
			&&	(0x00 == passmeRead[0x4])
			&&	(0x00 == passmeRead[0x5])
			&&	(0x00 == passmeRead[0x6])
			&&	(0x00 == passmeRead[0x7])
			&&	(0x00 == passmeRead[0x8])
			&&	(0x00 == passmeRead[0x9])
			&&	(0x00 == passmeRead[0xA])
			&&	(0x00 == passmeRead[0xB])
			&&	(0x50 == passmeRead[0xC])
			&&	(0x41 == passmeRead[0xD])
			&&	(0x53 == passmeRead[0xE])
			&&	(0x53 == passmeRead[0xF])
			&&	(0x00 == passmeRead[0x10])
			&&	(0x00 == passmeRead[0x11])
			&&	(0x00 == passmeRead[0x12])
			&&	(0x00 == passmeRead[0x13])
		)
	){
		mode = isNDSBinaryV1;
	}
	else if(
		(checkCounter == 0) &&
		(arm9EntryAddress >= 0x02000000) &&
		(
			//Slot 1 passme v2 (pre 2008 NTR homebrew)
			(0x00 == passmeRead[0x0])
			&&	(0x00 == passmeRead[0x1])
			&&	(0x00 == passmeRead[0x2])
			&&	(0x00 == passmeRead[0x3])
			&&	(0x00 == passmeRead[0x4])
			&&	(0x00 == passmeRead[0x5])
			&&	(0x00 == passmeRead[0x6])
			&&	(0x00 == passmeRead[0x7])
			&&	(0x00 == passmeRead[0x8])
			&&	(0x00 == passmeRead[0x9])
			&&	(0x00 == passmeRead[0xA])
			&&	(0x00 == passmeRead[0xB])
			&&	(0x50 == passmeRead[0xC])
			&&	(0x41 == passmeRead[0xD])
			&&	(0x53 == passmeRead[0xE])
			&&	(0x53 == passmeRead[0xF])
			&&	(0x30 == passmeRead[0x10])
			&&	(0x31 == passmeRead[0x11])
			&&	(0x96 == passmeRead[0x12])
			&&	(0x00 == passmeRead[0x13])
		)
	){
		mode = isNDSBinaryV2;
	}
	else if(
		(checkCounter == 0) &&
		(arm9EntryAddress >= 0x02000000) &&
		//(gotDLDISection == true) && //some v2+ homebrew may have the DLDI section stripped (such as barebones demos without filesystem at the time the translation unit built the ARM9 payload)
			//Slot 1 passme v3 (2009+ NTR homebrew)
			(0x53 == passmeRead[0x0])
			&&	(0x52 == passmeRead[0x1])
			&&	(0x41 == passmeRead[0x2])
			&&	(0x4D == passmeRead[0x3])
			&&	(0x5F == passmeRead[0x4])
			&&	(0x56 == passmeRead[0x5])
			&&	(0x31 == passmeRead[0x6])
			&&	(0x31 == passmeRead[0x7])
			&&	(0x30 == passmeRead[0x8])
			&&	(0x00 == passmeRead[0x9])
			&&	(0x00 == passmeRead[0xA])
			&&	(0x00 == passmeRead[0xB])
			&&	(0x50 == passmeRead[0xC])
			&&	(0x41 == passmeRead[0xD])
			&&	(0x53 == passmeRead[0xE])
			&&	(0x53 == passmeRead[0xF])
			&&	(0x30 == passmeRead[0x10])
			&&	(0x31 == passmeRead[0x11])
			&&	(0x96 == passmeRead[0x12])
			&&	(0x00 == passmeRead[0x13])
	){
		mode = isNDSBinaryV3;
	}
	
	//TWL Slot 1 / Internal SD mode: (2009+ TWL homebrew)
	else if( 
		(checkCounter > 0) && (arm9EntryAddress >= 0x02000000) && (arm9EntryAddress <= 0x02FFFFFF) &&
		(
			((arm7EntryAddress >= 0x02000000) && (arm7EntryAddress <= 0x02FFFFFF))
			||
			((arm7EntryAddress >= 0x037F8000) && (arm7EntryAddress <= 0x03810000))
		)
		//&&
		//(gotDLDISection == true) //some homebrew may have the DLDI section stripped (such as barebones demos without filesystem at the time the translation unit built the ARM9 payload)
	){
		mode = isTWLBinary;
		*ARM7i_HEADER_SCFG_EXT7Inst = *(u32*)NDSHeaderStructInst[0x1B8];	//0x1B8h 4    ARM7 SCFG_EXT7 setting (bit0,1,2,10,18,31)
	}
	
	//Check for Headerless NTR binary (2004 homebrew on custom devkits, or custom devkits overall)
	if( 
		(arm7EntryAddress >= 0x02000000)
		&&
		(arm9EntryAddress >= 0x02000000)
		&&
		(arm7BootCodeSize > 0)
		&&
		(arm9BootCodeSize > 0)
		&&
		(arm9BootCodeOffsetInFile > 0) //even headerless Passme NTR binaries reserve the NTR header section of 0x200 bytes
		&&
		(mode == notTWLOrNTRBinary)
		&&
		(checkCounter == 0)
	){
		mode = isNDSBinaryV1;
	}
	
	u32 * TGDSHdr = (u32*)&NDSHeaderStructInst[0];
	u16 * TGDSHdr2 = (u16*)&TGDSHdr[4];
	//"NDS.TinyFB..TGDSNN" TGDS-TWL homebrew
	if(
		(TGDSHdr[0] == (u32)0x2E53444E)
		&&
		(TGDSHdr[1] == (u32)0x796E6954)
		&&
		(TGDSHdr[2] == (u32)0x00004246)
		&&
		(TGDSHdr[3] == (u32)0x53444754)
		&&
		(TGDSHdr2[0] == (u16)0x4E4E)
		&&
		(mode == isTWLBinary)
	){
		*inIsTGDSTWLHomebrew = true;
	}
	else{
		*inIsTGDSTWLHomebrew = false;
	}
	
	return mode;
}


#ifdef ARM9
#include "videoTGDS.h"
#include "consoleTGDS.h"
#include "soundTGDS.h"
#include "fatfslayerTGDS.h"

//ToolchainGenericDS-multiboot NDS Binary loader: Requires TGDS_MB_V3_NTR_PAYLOAD_FILENAME & TGDS_MB_V3_TWL_PAYLOAD_FILENAME (TGDS-multiboot Project) in SD root.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool TGDSMultibootRunNDSPayload(char * filename, u8 * tgdsMbv3ARM7Bootldr, int argc, char *argv) {
	char * TGDSMBPAYLOAD = NULL;
	bool isTGDSTWLHomebrew = false;
	register int isNTRTWLBinary = isNTROrTWLBinary(filename, &isTGDSTWLHomebrew);
	memset(msgDebugException, 0, MAX_TGDSFILENAME_LENGTH);
	//NTR/TWL mode? Can only boot valid NTR/TWL binaries, the rest is skipped.
	if(!(isNTRTWLBinary == isNDSBinaryV1Slot2) && !(isNTRTWLBinary == isNDSBinaryV1) && !(isNTRTWLBinary == isNDSBinaryV2) && !(isNTRTWLBinary == isNDSBinaryV3) && !(isNTRTWLBinary == isTWLBinary) ){
		return false;
	}
	else{
		addARGV(argc, argv);
		//Run payload depending on current NTR/TWL Hardware supported.
		if (__dsimode == true){
			TGDSMBPAYLOAD = getfatfsPath((sint8 *)TGDS_MB_V3_TWL_PAYLOAD_FILENAME);	//TGDS TWL SDK (ARM9i binaries) emits TGDSMultibootRunNDSPayload() which reloads into TWL TGDS-MB Reload payload
		}
		else {
			TGDSMBPAYLOAD = getfatfsPath((sint8 *)TGDS_MB_V3_NTR_PAYLOAD_FILENAME);	//TGDS NTR SDK (ARM9 binaries) emits TGDSMultibootRunNDSPayload() which reloads into NTR TGDS-MB Reload payload
		}
		
		//Save ARGV-CMD line
		memcpy((char*)TGDS_ARGV_BUFFER, (void *)__system_argv, 256);
		coherent_user_range_by_size((uint32)TGDS_ARGV_BUFFER, (int)256);

		//NTR / TWL RAM Setup
		stopSoundStreamUser();	//Prevents TGDS ARM7 Cores from segfaulting if trying to boot NTR/TWL homebrew after playing a sound stream
		
		if(
			(__dsimode == true)
			&&
			(
				(
				(isNTRTWLBinary == isNDSBinaryV1Slot2)
				||
				(isNTRTWLBinary == isNDSBinaryV1)
				||
				(isNTRTWLBinary == isNDSBinaryV2)
				||
				(isNTRTWLBinary == isNDSBinaryV3)
				)
			)
		){
			//Enable 4M EWRAM (TWL)
			u32 SFGEXT9 = *(u32*)0x04004008;
			//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
			SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x0 << 14);
			*(u32*)0x04004008 = SFGEXT9;
		}
		//DKARM TWL mode homebrew
		else if (		
			(isTGDSTWLHomebrew == false)
			&&
			(__dsimode == true)
			&&
			(isNTRTWLBinary == isTWLBinary)
		){
			//Enable 16M EWRAM (TWL)
			u32 SFGEXT9 = *(u32*)0x04004008;
			//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
			SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x2 << 14);
			*(u32*)0x04004008 = SFGEXT9;
		}
		//TGDS TWL mode homebrew self reload fix
		else if (
			(isTGDSTWLHomebrew == true)
			&&
			(__dsimode == true)
			&&
			(isNTRTWLBinary == isTWLBinary)
		){
			//Enable 4M EWRAM (TWL)
			u32 SFGEXT9 = *(u32*)0x04004008;
			//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
			SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x0 << 14);
			*(u32*)0x04004008 = SFGEXT9;
		}
		
		//NTR TGDS-MB v3 compatibility
		if(__dsimode == false){
			//Execute Stage 1: IWRAM ARM7 payload: NTR/TWL (0x03800000)
			executeARM7Payload((u32)0x02380000, 96*1024, (u32 *)TGDS_MB_V3_ARM7_STAGE1_ADDR);
		}

		//Save Stage 2: VRAM ARM7 payload: NTR/TWL (0x06000000). To be ran on the upcoming TGDS-MB v3 ARM9 bootstrap core.
		memcpy((u32 *)TGDS_MB_V3_ARM7_STAGE1_ADDR, tgdsMbv3ARM7Bootldr, 96*1024);

		//rudimentary debugger
		//initFBModeMainEngine0x06000000();
		//dmaFillHalfWord(0, 0xF800, (uint32)0x06000000, (uint32)(128*1024)); //blue
		//dmaFillHalfWord(0, 0x87c0, (uint32)0x06000000, (uint32)(128*1024)); //green
		//dmaFillHalfWord(0, 0x801f, (uint32)0x06000000, (uint32)(128*1024)); //red
		
		remove(TGDS_MB_V3_BOOTSTUB_FILENAME);
		
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
			printf("%s: read (1)", TGDSMBPAYLOAD);
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
			printf("%s: read (3)", TGDSMBPAYLOAD);
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
				if(__dsimode == true){
					printf("%s: write (4)", TGDSMBPAYLOAD);
					printf("payload fail [%s]", tempFile);
					while(1==1){}
				}
			}
			f_sync(&fPagingFDRead);
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
				printf("%s: read (2)", TGDSMBPAYLOAD);
				printf("payload fail [%s]", filename);
				while(1==1){}
			}
			coherent_user_range_by_size((uint32)workBuffer, (int)blocksToRead);
			
			int writtenSize=0;
			result = f_write(&fPagingFDWrite, workBuffer, (int)blocksToRead, (UINT*)&writtenSize); //workbuffer is already coherent
			if (result != FR_OK){
				if(__dsimode == true){
					printf("%s: write (4)", TGDSMBPAYLOAD);
					printf("payload fail [%s]", tempFile);
					while(1==1){}
				}
			}
			f_sync(&fPagingFDWrite); //make persistent file in filesystem coherent
		}
		
		//Free
		f_close(&fPagingFDRead);
		f_close(&fPagingFDWrite);
		TGDSARM9Free(workBuffer);
		
		setValueSafe((u32*)ARM9_TWLORNTRPAYLOAD_MODE, (u32)isNTRTWLBinary); 
		setValueSafe((u32*)ARM7_ARM9_SAVED_DSFIRMWARE, savedDSHardware); //required by TGDS-multiboot's tgds_multiboot_payload.bin
		setValueSafe((u32*)TGDS_IS_TGDS_TWL_HOMEBREW, (u32)isTGDSTWLHomebrew); //is TGDS TWL homebrew? Uses special map
		
		FILE * tgdsPayloadFh = fopen(TGDSMBPAYLOAD, "r");
		if(tgdsPayloadFh != NULL){
			fseek(tgdsPayloadFh, 0, SEEK_SET);
			int	tgds_multiboot_payload_size = FS_getFileSizeFromOpenHandle(tgdsPayloadFh);
			u8 * TGDSMBPAYLOADLZSSCompressed = TGDSARM9Malloc(tgds_multiboot_payload_size);
			fread((u32*)TGDSMBPAYLOADLZSSCompressed, 1, tgds_multiboot_payload_size, tgdsPayloadFh); //NTR = read into uncached mirror / TWL = read into uncached memory
			coherent_user_range_by_size((uint32)TGDSMBPAYLOADLZSSCompressed, (int)tgds_multiboot_payload_size);
			fclose(tgdsPayloadFh);
			int decompressed_tgds_multiboot_payload_size = *(unsigned int *)(TGDSMBPAYLOADLZSSCompressed) >> 8;
			swiDecompressLZSSWram((u8*)TGDSMBPAYLOADLZSSCompressed, (u8*)TGDS_MB_V3_PAYLOAD_ADDR_TWL);
			coherent_user_range_by_size((uint32)TGDS_MB_V3_PAYLOAD_ADDR_TWL, (int)decompressed_tgds_multiboot_payload_size);
			TGDSARM9Free(TGDSMBPAYLOADLZSSCompressed);
			FS_deinit();
			
			bool stat = dldiPatchLoader((data_t *)TGDS_MB_V3_PAYLOAD_ADDR_TWL, (int)tgds_multiboot_payload_size, (u32)&_io_dldi_stub);
			if(stat == false){
				sprintf(msgDebugException, "%s%s", "TGDSMultibootRunNDSPayload(): DLDI Patch failed. NTR/TWL binary missing DLDI section.", "");
				nocashMessage((char*)&msgDebugException[0]);
			}
			else{
				sprintf(msgDebugException, "%s", "TGDSMultibootRunNDSPayload(): DLDI Patch OK.");
				nocashMessage((char*)&msgDebugException[0]);
			}
				
			dmaTransferWord(0, (u32)TGDS_MB_V3_PAYLOAD_ADDR_TWL, (u32)TGDS_MB_V3_PAYLOAD_ADDR, (uint32)tgds_multiboot_payload_size);
			coherent_user_range_by_size((uint32)TGDS_MB_V3_PAYLOAD_ADDR, tgds_multiboot_payload_size); //Make ARM9
	
			setValueSafe((u32*)TGDS_MB_V3_PAYLOAD_SIZE, (u32)tgds_multiboot_payload_size);
			typedef void (*t_bootAddr)();
			t_bootAddr bootARM9Payload = (t_bootAddr)TGDS_MB_V3_PAYLOAD_ADDR_TWL;
			
			//Restore ARGV-CMD line
			memcpy((char*)__system_argv, (void *)TGDS_ARGV_BUFFER, 256);
			coherent_user_range_by_size((uint32)__system_argv, (int)256);

			REG_IME = 0;
			REG_IE = 0;
			
			//Disable mpu
			CP15ControlRegisterDisable(CR_M);
			
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
	REG_IME = 1;
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
	TWLModeInternalSDAccess = getValueSafe((u32*)0x02FFDFE8); //ARM7DLDI mode: TWL SD @ ARM7 or DLDI SD @ ARM7: @ARM7_ARM9_DLDI_STATUS
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//Clear FIFO messages to prevent further ones being blocked
}
#endif



#ifdef ARM7

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void initMBK(void) {
	// This function has no effect with ARM7 SCFG locked

	// ARM7 is master of WRAM-A, arm9 of WRAM-B & C
	REG_MBK9 = 0x3000000F;

	// WRAM-A fully mapped to ARM7
	*(vu32*)REG_MBK1 = 0x8185898D; // Same as DSiWare

	// WRAM-B fully mapped to ARM7 // inverted order
	*(vu32*)REG_MBK2 = 0x9195999D;
	*(vu32*)REG_MBK3 = 0x8185898D;

	// WRAM-C fully mapped to arm7 // inverted order
	*(vu32*)REG_MBK4 = 0x9195999D;
	*(vu32*)REG_MBK5 = 0x8185898D;

	// WRAM mapped to the 0x3700000 - 0x37FFFFF area 
	// WRAM-A mapped to the 0x3000000 - 0x303FFFF area : 256k
	REG_MBK6=0x00403000; // same as dsi-enhanced and certain dsiware
	// WRAM-B mapped to the 0x3740000 - 0x37BFFFF area : 512k // why? only 256k real memory is there
	REG_MBK7=0x07C03740; // same as dsiware
	// WRAM-C mapped to the 0x3700000 - 0x373FFFF area : 256k
	REG_MBK8=0x07403700; // same as dsiware
}

#endif


#ifdef ARM9

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void initMBKARM9(void) {
	// Default DSiWare settings

	// WRAM-B fully mapped to arm7 // inverted order
	*(vu32*)REG_MBK2 = 0x9195999D;
	*(vu32*)REG_MBK3 = 0x8185898D;

	// WRAM-C fully mapped to arm7 // inverted order
	*(vu32*)REG_MBK4 = 0x9195999D;
	*(vu32*)REG_MBK5 = 0x8185898D;

	// WRAM-A not mapped (reserved to arm7)
	REG_MBK6=0x00000000;
	// WRAM-B mapped to the 0x3740000 - 0x37BFFFF area : 512k // why? only 256k real memory is there
	REG_MBK7=0x07C03740; // same as dsiware
	// WRAM-C mapped to the 0x3700000 - 0x373FFFF area : 256k
	REG_MBK8=0x07403700; // same as dsiware
}

#endif