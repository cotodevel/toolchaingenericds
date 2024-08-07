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

#include "typedefsTGDS.h"
#include "soundTGDS.h"
#include "ipcfifoTGDS.h"
#include "utilsTGDS.h"
#include "biosTGDS.h"

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
//Writes to ARM7 Sound Channel through direct interrupt, PSG mode.
void writeARM7SoundChannel(int channel, u32 cnt, u16 freq){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	TGDSIPC->soundIPC.psgChannel = channel;
	TGDSIPC->soundIPC.cr = cnt;
	TGDSIPC->soundIPC.timer = SOUND_FREQ(freq);
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[7], (u32)ARM7COMMAND_PSG_COMMAND);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
//Writes to ARM7 Sound Channel through direct interrupt, dataSrc must be in EWRAM.
void writeARM7SoundChannelFromSource(int channel, u32 cnt, u16 freq, u32 dataSrc, u32 dataSize){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	TGDSIPC->soundIPC.psgChannel = channel;
	TGDSIPC->soundIPC.cr = cnt;
	TGDSIPC->soundIPC.timer = SOUND_FREQ(freq);
	TGDSIPC->soundIPC.arm9L = (void*)dataSrc;
	TGDSIPC->soundIPC.volume = (int)dataSize; //volume == size
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[7], (u32)ARM7COMMAND_SND_COMMAND);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
}

#endif