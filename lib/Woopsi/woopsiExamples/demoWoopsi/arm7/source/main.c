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

#include <string.h>
#include "main.h"
#include "InterruptsARMCores_h.h"
#include "interrupts.h"

#include "ipcfifoTGDSUser.h"
#include "wifi_arm7.h"
#include "usrsettingsTGDS.h"
#include "timerTGDS.h"
#include "biosTGDS.h"

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	
	//wait for VRAM D to be assigned from ARM9->ARM7 (ARM7 has load/store on byte/half/words on VRAM)
	while (!(*((vuint8*)0x04000240) & 0x2));
	
	installWifiFIFO();		
	
	/*			TGDS 1.6 Standard ARM7 Init code end	*/
	
    while (1) {
		handleARM7SVC();	/* Do not remove, handles TGDS services */
		IRQWait(0, IRQ_VBLANK | IRQ_VCOUNT | IRQ_IPCSYNC | IRQ_RECVFIFO_NOT_EMPTY | IRQ_SCREENLID);
	}
	return 0;
}
