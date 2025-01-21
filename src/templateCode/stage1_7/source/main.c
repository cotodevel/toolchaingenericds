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

#include "main.h"
#include "biosTGDS.h"
#include "loader.h"
#include "spifwTGDS.h"
#include "posixHandleTGDS.h"
#include "wifi_arm7.h"
#include "dldi.h"
#include "ipcfifoTGDSUser.h"
#include "TGDS_threads.h"

//TGDS-MB v3 bootloader
void bootfile(){

}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char **argv)  {
//---------------------------------------------------------------------------------
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	//installWifiFIFO(); //causes issues if enabled + removing -DIGNORELIBS from ToolchainGenericDS\src\templateCode\arm7bootldr\Makefile
	while(!(*(u8*)0x04000240 & 2) ){} //wait for VRAM_D block
	ARM7InitDLDI(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, TGDSDLDI_ARM7_ADDRESS);
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	/*			TGDS 1.6 Standard ARM7 Init code end	*/
	
    while (1) {
		int threadsRan = runThreads(TGDSThreads);
	}
	return 0;
}