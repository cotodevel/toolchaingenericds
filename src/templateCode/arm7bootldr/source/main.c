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


////////////////////////////////TGDS-MB v3 VRAM Bootcode start////////////////////////////////
#include "main.h"
#include "biosTGDS.h"
#include "spifwTGDS.h"
#include "posixHandleTGDS.h"
#include "pff.h"
#include "ipcfifoTGDSUser.h"
#include "loader.h"
#include "dldi.h"
#include "exceptionTGDS.h"
#include "dmaTGDS.h"
#include "TGDS_threads.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FATFS fileHandle;					// Petit-FatFs work area 
char fname[256];
u8 NDSHeaderStruct[4096];
char debugBuf7[256];
struct addrList addresses[TGDS_MB_V3_ADDR_COUNT];

// Comparison function
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int compare(const void* a, const void* b) {
    return ( ((struct addrList*)a)->armRamAddress > ((struct addrList*)b)->armRamAddress);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int isNTROrTWLBinaryTGDSMB7(FATFS * currentFH, u8 * NDSHeaderStructInst, int NDSHeaderStructSize, u32 * ARM7i_HEADER_SCFG_EXT7Inst, bool * inIsTGDSTWLHomebrew){
	uint8_t fresult;
	int headerSize = NDSHeaderStructSize;
	u8 passmeRead[24];
	memset(passmeRead, 0, sizeof(passmeRead));
	memset(NDSHeaderStructInst, 0, headerSize);
	pf_lseek(0, currentFH);
	UINT nbytes_read;
	pf_read((u8*)NDSHeaderStructInst, headerSize, &nbytes_read, currentFH);
	if(nbytes_read != headerSize){
		return notTWLOrNTRBinary;
	}
	else{
		pf_lseek(0xA0, currentFH);
		pf_read((u8*)&passmeRead[0], sizeof(passmeRead), &nbytes_read, currentFH);
	}
	return isNTROrTWLBinaryTGDSShared(NDSHeaderStructInst, (u8*)&passmeRead[0], ARM7i_HEADER_SCFG_EXT7Inst, inIsTGDSTWLHomebrew);
}


#include <stdio.h>

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void strrev(char *arr, int start, int end)
{
    char temp;

    if (start >= end)
        return;

    temp = *(arr + start);
    *(arr + start) = *(arr + end);
    *(arr + end) = temp;

    start++;
    end--;
    strrev(arr, start, end);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
char *itoa(int number, char *arr, int base)
{
    int i = 0, r, negative = 0;

    if (number == 0)
    {
        arr[i] = '0';
        arr[i + 1] = '\0';
        return arr;
    }

    if (number < 0 && base == 10)
    {
        number *= -1;
        negative = 1;
    }

    while (number != 0)
    {
        r = number % base;
        arr[i] = (r > 9) ? (r - 10) + 'a' : r + '0';
        i++;
        number /= base;
    }

    if (negative)
    {
        arr[i] = '-';
        i++;
    }

    strrev(arr, 0, i - 1);

    arr[i] = '\0';

    return arr;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u32 getEntryPointByType(u32 inType){
	int count = sizeof(addresses) / sizeof(struct addrList);
	int i = 0;
	for(i = 0; i < count; i++){
		if(addresses[i].type == inType){
			return addresses[i].armEntryAddress;
		}
	}
	return -1;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void bootfile(){
	int stage = 10;
	//Wait for 96K ARM7 mapped @ 0x037f8000 ~ 0x03810000 & Clean IWRAM
	while( !(WRAM_CR & WRAM_0KARM9_32KARM7) ){
		swiDelay(1);
	}
	dmaFillHalfWord(0, 0, ((uint32)0x037f8000), (uint32)(96*1024));
	bool isTGDSTWLHomebrew = false;
	uint8_t fresult;
	FATFS * currentFH = &fileHandle;
	char * filename = (char*)(TGDS_MB_V3_BOOTSTUB_FILENAME);
	memset(&addresses, 0, sizeof(addresses));
	strcpy((char*)fname, &filename[2]);
	
	REG_IME = 1;
	fresult = pf_mount(currentFH);
	if (fresult != FR_OK) {
		setValueSafe((u32*)0x02FFFE24, (u32)0xFFFFFFFE); //ARM9 go handle error
		while(1==1){}
	}
	else{
		fresult = pf_open(fname, currentFH);
		if (fresult != FR_OK) {
			strcpy(debugBuf7, "TGDS-MB: arm7bootldr/bootfile():[");
			strcat(debugBuf7, filename);
			strcat(debugBuf7, "] Open FAIL! Halting.");
			handleDSInitOutputMessage((char*)&debugBuf7[0]);
			handleDSInitError7(stage, (u32)savedDSHardware);
		}
		
		pf_lseek(0, currentFH);
		int isNTRTWLBinary = isNTROrTWLBinaryTGDSMB7(currentFH, &NDSHeaderStruct, sizeof(NDSHeaderStruct), ARM7i_HEADER_SCFG_EXT7, &isTGDSTWLHomebrew);

		if(
			(isNTRTWLBinary == isNDSBinaryV1) || (isNTRTWLBinary == isNDSBinaryV2) || (isNTRTWLBinary == isNDSBinaryV3) || (isNTRTWLBinary == isNDSBinaryV1Slot2)
			||
			(isNTRTWLBinary == isTWLBinary)
		){
			///////////////////////////////////////////////////ARM7 Loader start ///////////////////////////////////////////////////////
			
			//Todo: Support isNDSBinaryV1Slot2 binary (Slot2 Passme v1 .ds.gba homebrew)
			if(isNTRTWLBinary == isNDSBinaryV1Slot2){
				setValueSafe((u32*)0x02FFFE24, (u32)0xFFFFFFFE); //ARM9 go handle error
			}
			
			setupDisabledExceptionHandler();
			
			UINT nbytes_read;
			pf_lseek(0, currentFH);
			memset(&NDSHeaderStruct, 0, sizeof(NDSHeaderStruct));
			pf_read((u8*)&NDSHeaderStruct, sizeof(NDSHeaderStruct), &nbytes_read, currentFH);
			struct sDSCARTHEADER * NDSHdr = (struct sDSCARTHEADER *)&NDSHeaderStruct;
			//- ARM9 passes the filename to ARM7
			//- ARM9 waits in ITCM code until ARM7 reloads
			//- ARM7 handles/reloads everything in memory
			//- ARM7 gives permission to ARM9 to reload and ARM7 reloads as well
			//ARM7 reloads here
			int arm7BootCodeSize = (int)((NDSHdr->arm7size + 1) & ~1); //2 byte alignment (thumb / ARM code)
			u32 arm7BootCodeOffsetInFile = NDSHdr->arm7romoffset;
			u32 arm7EntryAddress = NDSHdr->arm7entryaddress;	
			u32 arm7ramaddress = NDSHdr->arm7ramaddress;
			
			//Incoming address list: ARM7 [arm7.bin]
			addresses[0].armRamAddress = arm7ramaddress;
			addresses[0].armEntryAddress = arm7EntryAddress;
			addresses[0].armBootCodeOffsetInFile = arm7BootCodeOffsetInFile;
			addresses[0].armBootCodeSize = arm7BootCodeSize;
			addresses[0].type = TGDS_MB_V3_TYPE_ENTRYPOINT_ARM7;
			
			//Clear arm7 ram
			dmaFillHalfWord(0, 0, ((uint32)arm7ramaddress), ((uint32)arm7BootCodeSize) );
			
			//ARM9
			//Wait until ARM9 arrives at TGDS_MB_payload first
			while( getValueSafe((u32*)0x02FFFE24) != ((u32)0)){
				
			}
			
			int arm9BootCodeSize = (int)((NDSHdr->arm9size + 1) & ~1); //2 byte alignment (thumb / ARM code)
			u32 arm9BootCodeOffsetInFile = NDSHdr->arm9romoffset;
			u32 arm9EntryAddress = NDSHdr->arm9entryaddress;
			u32 arm9ramaddress = NDSHdr->arm9ramaddress;
			
			//Clear arm9 ram
			dmaFillHalfWord(0, 0, ((uint32)arm9ramaddress), ((uint32)arm9BootCodeSize) );
			
			//Incoming address list: ARM9 [arm9.bin]
			addresses[1].armRamAddress = arm9ramaddress;
			addresses[1].armEntryAddress = arm9EntryAddress;
			addresses[1].armBootCodeOffsetInFile = arm9BootCodeOffsetInFile;
			addresses[1].armBootCodeSize = arm9BootCodeSize;
			addresses[1].type = TGDS_MB_V3_TYPE_ENTRYPOINT_ARM9;
			
			//Turn off IRQs right now because an interrupt calling to ARM7 exception handler (through bios) crashes ARM7 
			REG_IME = 0;
			REG_IE = 0;
			REG_IF = ~0;
			
			if(
				(__dsimode == true)
				&&
				(isNTRTWLBinary == isTWLBinary)
			){
				initMBK();
			}
			
			//Bios can now be changed @ ARM9 (from ARM7, keep reading)
			//BIOS NTR/TWL switch (backwards compatibility mode): ARM9 has 4004000h - DSi9 - SCFG_A9ROM - ROM Status (R) [0000h] bits as read-only. 
			//Needs to be set on ARM7 side
			if (*((vu32*)(arm9EntryAddress + 0xEC)) == 0xEAFFFFFE){ // b #0; //loop
				*((vu32*)(arm9EntryAddress + 0xEC)) = 0xE3A00000; // mov r0, #0
			}
			
			//Bios can now be changed @ ARM7
			if(__dsimode == true){
				//- DSi7 - SCFG_ROM - ROM Control (R/W, Set-Once)
				u16 * SCFG_ROM = 0x04004000;
				//4004000h - DSi7 - SCFG_ROM - ROM Control (R/W, Set-Once)
				//  0     ARM9 BIOS Upper 32Kbyte of DSi BIOS (0=Enabled, 1=Disabled) (FFFF8xxxh)
				//  1     ARM9 BIOS for NDS Mode              (0=DSi BIOS, 1=NDS BIOS)(FFFF0xxxh)
				//  2-7   Unused (0)
				//  8     ARM7 BIOS Upper 32Kbyte of DSi BIOS (0=Enabled, 1=Disabled)  (0008xxxh)
				//  9     ARM7 BIOS for NDS Mode              (0=DSi BIOS, 1=NDS BIOS) (0000xxxh)
				//  10    Access to Console ID registers      (0=Enabled, 1=Disabled)  (4004Dxxh)
				//  11-15 Unused (0)
				//  16    Unknown, used by bootrom, set to 0  (0=Maybe start ARM9 ?)
				//  17-31 Unused (0)
				//Bits in this register can be set once (cannot be changed back from 1 to 0).
				//Don't change bit1 while executing IRQs or SWI functions on ARM9 side.
				if( 
					(isNTRTWLBinary == isNDSBinaryV1Slot2)
					||
					(isNTRTWLBinary == isNDSBinaryV1)
					||
					(isNTRTWLBinary == isNDSBinaryV2)
					||
					(isNTRTWLBinary == isNDSBinaryV3)
				){
					//Use NTR Bios
					*SCFG_ROM = 0x0703;
				}
				else if(isNTRTWLBinary == isTWLBinary){
					//Use TWL Bios
					*SCFG_ROM = 0x0101;
				}
				else{
					//Throw exception
					strcpy(debugBuf7, "TGDS-MB: arm7bootldr/bootfile():[");
					strcat(debugBuf7, filename);
					strcat(debugBuf7, "] Unknown Mode BIOS! ");
					handleDSInitOutputMessage((char*)&debugBuf7[0]);
					handleDSInitError7(stage, (u32)savedDSHardware);
				}
				
				
				//Set SCFG_EXT7
				//0     Revised ARM7 DMA Circuit       (0=NITRO, 1=Revised) = NTR
				//1     Revised Sound DMA              (0=NITRO, 1=Revised) = NTR
				//2     Revised Sound                  (0=NITRO, 1=Revised) = NTR
				//3-6   Unused (0)
				//7     Revised Card Interface Circuit (0=NITRO, 1=Revised) (set via ARM9) (R)
				//8     Extended ARM7 Interrupts      (0=NITRO, 1=Extended) (4000218h) = TWL
				//9     Extended SPI Clock (8MHz)     (0=NITRO, 1=Extended) (40001C0h) = TGDS Project decides it
				//10    Extended Sound DMA        ?   (0=NITRO, 1=Extended) (?) = NTR
				u32 SCFG_EXT7 = *ARM7i_HEADER_SCFG_EXT7;
				
				//16    Access to New DMA Controller  (0=Disable, 1=Enable) (40041xxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 16));
				
				//17    Access to AES Unit            (0=Disable, 1=Enable) (40044xxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 17));
				
				//18    Access to SD/MMC registers    (0=Disable, 1=Enable) (40048xxh-40049xxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 18));
				
				//19    Access to SDIO Wifi registers (0=Disable, 1=Enable) (4004Axxh-4004Bxxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 19));
				
				//20    Access to Microphone regs     (0=Disable, 1=Enable) (40046xxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 20));
				
				//21    Access to SNDEXCNT register   (0=Disable, 1=Enable) (40047xxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 21));
				
				//22    Access to I2C registers       (0=Disable, 1=Enable) (40045xxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 22));
				
				//23    Access to GPIO registers      (0=Disable, 1=Enable) (4004Cxxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 23));
				
				//24    Access to 2nd NDS Cart Slot   (0=Disable, 1=Enable) (40021xxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 24));
				
				//25    Access to New Shared WRAM     (0=Disable, 1=Enable) (3xxxxxxh)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 25));
				
				//26-27 Unused (0)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 26) | (1 << 27));
				
				//28    Undocumented/Unknown          (0=???, 1=Normal)     (?)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 28));
				
				//29-30 Unused (0)
				
				//31    Access to SCFG/MBK registers  (0=Disable, 1=Enable) (4004000h-4004063h)
				SCFG_EXT7 = (SCFG_EXT7 | (1 << 31));
				*(u32*)0x04004008 = SCFG_EXT7;
				
				//Copy ARM7i/ARM9i sections since new TWL memory has been correctly set up.
				u32 arm7iBootCodeOffsetInFile = *(u32*)&NDSHeaderStruct[0x1D0];	//0x1D0 DSi7 ROM offset
				u32 arm7iRamAddress = *(u32*)&NDSHeaderStruct[0x1D8];	//0x1D8   DSi7 RAM address
				int arm7iBootCodeSize = *(u32*)&NDSHeaderStruct[0x1DC];	//0x1DC   DSi7 code size
				
				//Incoming address list: TWL [arm7i.bin]
				addresses[2].armRamAddress = arm7iRamAddress;
				addresses[2].armEntryAddress = 0;
				addresses[2].armBootCodeOffsetInFile = arm7iBootCodeOffsetInFile;
				addresses[2].armBootCodeSize = arm7iBootCodeSize;
				addresses[2].type = TGDS_MB_V3_TYPE_DEFAULT_VALUE;
				
				u32 arm9iBootCodeOffsetInFile = *(u32*)&NDSHeaderStruct[0x1C0];	//0x1C0   DSi9 ROM offset
				u32 arm9iRamAddress = *(u32*)&NDSHeaderStruct[0x1C8];	//0x1C8   DSi9 RAM address
				int arm9iBootCodeSize = *(u32*)&NDSHeaderStruct[0x1CC];	//0x1CC   DSi9 code size
				
				//Incoming address list: TWL [arm9i.bin]
				addresses[3].armRamAddress = arm9iRamAddress;
				addresses[3].armEntryAddress = 0;
				addresses[3].armBootCodeOffsetInFile = arm9iBootCodeOffsetInFile;
				addresses[3].armBootCodeSize = arm9iBootCodeSize;
				addresses[3].type = TGDS_MB_V3_TYPE_DEFAULT_VALUE;
				
				setValueSafe((u32*)ARM7i_RAM_ADDRESS, (u32)arm7iRamAddress);
				setValueSafe((u32*)ARM7i_BOOT_SIZE, (u32)arm7iBootCodeSize);
				setValueSafe((u32*)ARM9i_RAM_ADDRESS, (u32)arm9iRamAddress);
				setValueSafe((u32*)ARM9i_BOOT_SIZE, (u32)arm9iBootCodeSize);
			}
			//NTR hardware trying to boot TWL binaries? 
			else if( (__dsimode == false) && (isNTRTWLBinary == isTWLBinary) ){
				strcpy(debugBuf7, "TGDS-MB: arm7bootldr/bootfile():[");
				strcat(debugBuf7, filename);
				strcat(debugBuf7, "] NTR Unit trying TWL/Hybrid TWL Binary.");
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
			}
			
			//Copy addresses in lowest to highest order to prevent sections overlapping each other
			int count = sizeof(addresses) / sizeof(struct addrList);
			qsort(addresses, count, sizeof(struct addrList), compare);
			int i = 0;
			for(i = 0; i < count; i++){
				if( addresses[i].armBootCodeSize > 0 ){
					pf_lseek(addresses[i].armBootCodeOffsetInFile, currentFH);
					pf_read((u8*)addresses[i].armRamAddress, addresses[i].armBootCodeSize, &nbytes_read, currentFH);
					if( ((int)nbytes_read) != ((int)addresses[i].armBootCodeSize) ){
						setValueSafe((u32*)0x02FFFE24, (u32)0xFFFFFFFE); //ARM9 go handle error
						int stage = 10;
						strcpy(debugBuf7, "TGDS-MB: arm7bootldr/bootfile():[");
						strcat(debugBuf7, filename);
						strcat(debugBuf7, "] ARM payload write FAIL...");
						handleDSInitOutputMessage((char*)&debugBuf7[0]);
						handleDSInitError7(stage, (u32)savedDSHardware);
					}
				}
			}
			
			int wtf = 0;
			(void)wtf;

			//SM64 DSi port (DKARM): (https://github.com/Hydr8gon/sm64/commit/7d50caa3856be22dd167f1bfa874f8e5f3ad2b0e) 
			//has a libnds bug in its initialization routines (segfaults). Since TGDS don't use them, stub them.
			if(__dsimode == true){
				if(
					(isNTRTWLBinary == isTWLBinary)
					&&
					(isTGDSTWLHomebrew == false)
					&&
					((uint32)(u32)swiCRC16( 0xffff, (uint8*)arm9ramaddress, 512*1024) == ((uint32)0x000021DC))
				){
					*(u16*)0x020D5340 = 0x2500;
				}

				//Sound fix on DKARM homebrew
				if(
					(isNTRTWLBinary == isTWLBinary)
					&&
					(isTGDSTWLHomebrew == false)
				){
					*(u32*)0x04004008 = *ARM7i_HEADER_SCFG_EXT7;
				}
			}
			initSound();
			if(__dsimode == true){
				nocashMessage(" TGDS-MB v3 TWL");
			}
			else{
				nocashMessage(" TGDS-MB v3 NTR");
			}
			
			REG_IME = 0;
			dldi_handler_deinit();
			
			setValueSafe((u32*)TGDS_IS_TGDS_HOMEBREW, (u32)isTGDSTWLHomebrew);
			setValueSafe((u32*)ARM9_TWLORNTRPAYLOAD_MODE, (u32)isNTRTWLBinary);
			setValueSafe((u32*)ARM9_BOOT_SIZE, (u32)arm9BootCodeSize);
			setValueSafe((u32*)ARM7_BOOT_SIZE, (u32)arm7BootCodeSize);
			setValueSafe((u32*)ARM7_BOOTCODE_OFST, (u32)arm7BootCodeOffsetInFile);
			setValueSafe((u32*)ARM9_BOOTCODE_OFST, (u32)arm9BootCodeOffsetInFile);
			setValueSafe((u32*)0x02FFFE34, (u32)arm7EntryAddress);
			setValueSafe((u32*)0x02FFFE24, (u32)arm9EntryAddress); //ARM9 go (skip NTR v3/TWL ARM9(i) secure section)
			
			//Reload ARM7 core. //Note: swiSoftReset(); can't be used here because ARM Core needs to switch to Thumb1 v4t or ARM v4t now
			typedef void (*t_bootAddr)();
			t_bootAddr bootARMPayload = (t_bootAddr)arm7EntryAddress;
			bootARMPayload();
			
			///////////////////////////////////////////////////ARM7 Loader end ///////////////////////////////////////////////////////
			
			//Should never read this
			strcpy(debugBuf7, "TGDS-MB: arm7bootldr/bootfile():[");
			strcat(debugBuf7, filename);
			strcat(debugBuf7, "] NTR or TWL Boot failed! Halting. ");
			handleDSInitOutputMessage((char*)&debugBuf7[0]);
			handleDSInitError7(stage, (u32)savedDSHardware);
			
		}
		else{
			strcpy(debugBuf7, "TGDS-MB: arm7bootldr/bootfile():[");
			strcat(debugBuf7, filename);
			strcat(debugBuf7, "] NOT NTR or TWL Binary! Halting.");
			handleDSInitOutputMessage((char*)&debugBuf7[0]);
			handleDSInitError7(stage, (u32)savedDSHardware);
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	installWifiFIFO();
	while(!(*(u8*)0x04000240 & 2) ){} //wait for VRAM_D block
	ARM7InitDLDI(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, TGDSDLDI_ARM7_ADDRESS);
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	/*			TGDS 1.6 Standard ARM7 Init code end	*/
	
	while (1) {
		bool waitForVblank = false;
		int threadsRan = runThreads(TGDSThreads, waitForVblank);
	}
	return 0;
}

////////////////////////////////TGDS-MB v3 VRAM Bootcode end////////////////////////////////