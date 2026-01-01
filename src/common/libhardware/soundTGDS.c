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

#ifdef ARM7
s16 *strpcmL0 = NULL;
s16 *strpcmL1 = NULL;
s16 *strpcmR0 = NULL;
s16 *strpcmR1 = NULL;
int lastL = 0;
int lastR = 0;
int multRate = 1;
u32 sndCursor = 0;
u32 micBufLoc = 0;
u32 sampleLen = 0;
int sndRate = 0;
#endif

#ifdef ARM9
bool updateRequested = false;

__attribute__((section(".dtcm")))
struct soundPlayerContext soundData;

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

//Volume control
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int getVolume() 
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 
	coherent_user_range_by_size((uint32)&TGDSIPC->soundIPC, sizeof(TGDSIPC->soundIPC));
	return TGDSIPC->soundIPC.volume;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setVolume(int volume) 
{
	if(volume > SoundSampleContext_max_volume){
		volume = SoundSampleContext_max_volume;
	}
	if(volume < SoundSampleContext_min_volume){
		volume = SoundSampleContext_min_volume;
	}
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
	coherent_user_range_by_size((uint32)&TGDSIPC->soundIPC, sizeof(TGDSIPC->soundIPC));
	TGDSIPC->soundIPC.volume = volume;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void volumeUp() 
{
	setVolume(getVolume() + 1);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void volumeDown() 
{
	setVolume(getVolume() - 1);
}
#endif