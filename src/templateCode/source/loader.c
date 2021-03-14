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


//[Blocking]: Local ARM Core waits until External action takes place, waits while resolving internal NDS hardware wait states.
void waitWhileNotSetStatus(u32 status){
	while(NDS_LOADER_IPC_CTX_UNCACHED->ndsloaderInitStatus != status){
		swiDelay(111);	
	}
}

void setNDSLoaderInitStatus(int ndsloaderStatus){
	NDS_LOADER_IPC_CTX_UNCACHED->ndsloaderInitStatus = ndsloaderStatus;
}

#ifdef ARM9
void ARM7JumpTo(u32 ARM7Entrypoint){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[64], (uint32)ARM7Entrypoint);
	SendFIFOWords(ARM7COMMAND_RELOADARM7);
}
#endif