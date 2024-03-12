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

#ifdef ARM9
__attribute__((section(".dtcm")))
u32 reloadStatus = 0;

//ToolchainGenericDS-multiboot NDS Binary loader: Requires tgds_multiboot_payload_ntr.bin / tgds_multiboot_payload_twl.bin (TGDS-multiboot Project) in SD root.
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool TGDSMultibootRunNDSPayload(char * filename) {
	char * TGDSMBPAYLOAD = NULL;
	int isNTRTWLBinary = isNTROrTWLBinary(filename);
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
		setValueSafe((u32*)ARM7_ARM9_SAVED_DSFIRMWARE, savedDSHardware); //required by TGDS-multiboot's tgds_multiboot_payload.bin
		FILE * tgdsPayloadFh = fopen(TGDSMBPAYLOAD, "r");
		if(tgdsPayloadFh != NULL){
			fseek(tgdsPayloadFh, 0, SEEK_SET);
			int	tgds_multiboot_payload_size = FS_getFileSizeFromOpenHandle(tgdsPayloadFh);
			fread((u32*)TGDS_MB_V3_PAYLOAD_ADDR, 1, tgds_multiboot_payload_size, tgdsPayloadFh);
			coherent_user_range_by_size(TGDS_MB_V3_PAYLOAD_ADDR, (int)tgds_multiboot_payload_size);
			fclose(tgdsPayloadFh);
			FS_deinit();
			//Copy and relocate current TGDS DLDI section into target ARM9 binary
			if(strncmp((char*)&dldiGet()->friendlyName[0], "TGDS RAMDISK", 12) == 0){
				nocashMessage("TGDS DLDI detected. Skipping DLDI patch.");
			}
			else{
				bool stat = dldiPatchLoader((data_t *)TGDS_MB_V3_PAYLOAD_ADDR, (u32)tgds_multiboot_payload_size, (u32)&_io_dldi_stub);
				if(stat == false){
					sprintf(msgDebugException, "%s%s", "TGDSMultibootRunNDSPayload(): DLDI Patch failed. NTR/TWL binary missing DLDI section.", "");
					nocashMessage((char*)&msgDebugException[0]);
				}
				else{
					sprintf(msgDebugException, "%s", "TGDSMultibootRunNDSPayload(): DLDI Patch OK.");
					nocashMessage((char*)&msgDebugException[0]);
				}
			}
			REG_IME = 0;
			typedef void (*t_bootAddr)();
			t_bootAddr bootARM9Payload = (t_bootAddr)TGDS_MB_V3_PAYLOAD_ADDR;
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
#endif
