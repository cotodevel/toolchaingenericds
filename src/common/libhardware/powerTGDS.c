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

#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "utilsTGDS.h"

#ifdef ARM7
#include <string.h>
#include "spifwTGDS.h"
#endif

#ifdef ARM9
#include <stdbool.h>

#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"

#endif

//Turn On the BG/3D engines if ARM9
//Turn On Sound Speakers / Wifi if ARM7
void powerON(uint32 values){
	#ifdef ARM7
	REG_POWERCNT |= values;
	#endif
	
	#ifdef ARM9
	if(!(values & POWERMAN_ARM9)){
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
		setValueSafe(&fifomsg[0], (uint32)values);
		setValueSafe(&fifomsg[7], (u32)FIFO_POWERCNT_ON);
		SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	}
	else{
		REG_POWERCNT |= values;
	}
	#endif
}

//Turn Off the BG/3D engines if ARM9
//Turn Off Sound Speakers / Wifi if ARM7
void powerOFF(uint32 values){
	#ifdef ARM7
	REG_POWERCNT &= ~values;
	#endif
	
	#ifdef ARM9
	if(!(values & POWERMAN_ARM9)){
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
		setValueSafe(&fifomsg[0], (uint32)values);
		setValueSafe(&fifomsg[7], (u32)FIFO_POWERCNT_OFF);
		SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	}
	else{
		REG_POWERCNT &= ~values;
	}
	#endif
}

#ifdef ARM7
void SoundPowerON(u8 volume){
	powerON(POWER_SOUND);
	PowerManagementDeviceWrite(PM_CONTROL_REG, ( PowerManagementDeviceRead(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	REG_SOUNDCNT = SOUND_ENABLE;
	REG_MASTER_VOLUME = volume;
}
#endif