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

#ifndef __soundTGDS_h__
#define __soundTGDS_h__

#ifdef ARM9
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

#define MAX_SNDBUF_SIZE (int)(128*1024)
#define SOUNDCONTEXTCAPACITY (int)(16)

#define SOUNDSAMPLECONTEXT_IDLE (u32)(0xffff1110)
#define SOUNDSAMPLECONTEXT_PENDING (u32)(0xffff1111)	//play now!
#define SOUNDSAMPLECONTEXT_PLAYING (u32)(0xffff1112)	//if SOUNDSAMPLECONTEXT_PLAYING && hw channel == disabled, -> SOUNDSAMPLECONTEXT_IDLE

//WAV Descriptor
typedef struct 
{
	char chunkID[4];
	long chunkSize;

	short wFormatTag;
	unsigned short wChannels;
	unsigned long dwSamplesPerSec;
	unsigned long dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
} wavFormatChunk;


//ARM7 will send a FIFO IRQ to ARM9 to force update a given soundSampleContext by index

//Sound Context
struct soundSampleContext{
	int sampleRate;
	u32 * data;
	u32 bytes;
	u8 channel;
	u8 vol;
	u8 pan;
	u32 status;
	u8 format;
};

struct soundSampleContextList{
	bool soundSampleContextEnabled;
	struct soundSampleContext soundSampleCxt[SOUNDCONTEXTCAPACITY];
};

struct soundPlayerChannelContext{
	u8 *chData;
	int channelIndex;
	int sampleSize;
};

struct soundPlayerContext{
	int fileSize;
	int fileOffset;
	//XX decoders:
	
	//Wav
	wavFormatChunk wavDescriptor;
	
	//Decoded XX buffers
	struct soundPlayerChannelContext soundPlayerContextCh[2];
	
	#ifdef ARM9
	FILE * fh;
	#endif
};

#ifdef ARM7
static inline s32 getFreeSoundChannel(){
	int i;
	for (i=0;i<16;++i){
		if (!(SCHANNEL_CR(i) & SCHANNEL_ENABLE)) return i;
	}
	return -1;
}

//returns -1 if channel is busy, or channel if idle
static inline s32 isFreeSoundChannel(u8 chan){
	if (!(SCHANNEL_CR(chan) & SCHANNEL_ENABLE)){
		return chan;
	}
	return -1;
}

#endif

#endif

#ifdef __cplusplus
extern "C"{
#endif

//Sound Sample Context: Plays raw sound samples at VBLANK intervals
extern void startSound(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format);
extern void initSound();

#ifdef ARM7
extern void updateSoundContext();
extern void initSoundSampleContext();
#endif

#ifdef ARM9
extern void flushSoundContext(int soundContextIndex);
#endif

extern struct soundSampleContext * getsoundSampleContextByIndex(int index);
extern bool freesoundSampleContext(struct soundSampleContext * sampleInst);	//free up a given soundSampleContext
extern struct soundSampleContext * getFreeSoundSampleContext();				//obtains a free soundSampleContext, if any

#ifdef ARM9
extern bool setSoundSampleContext(int sampleRate, u32 * data, u32 bytes, u8 channel, u8 vol, u8 pan, u8 format);
#endif

extern void EnableSoundSampleContext();
extern void DisableSoundSampleContext();
extern bool getSoundSampleContextEnabledStatus();

#ifdef __cplusplus
}
#endif