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

#define SoundSampleContextChannels (int)(16)
#define SOUNDSAMPLECONTEXT_IDLE (u32)(0xffff1110)
#define SOUNDSAMPLECONTEXT_PENDING (u32)(0xffff1111)	//play now!
#define SOUNDSAMPLECONTEXT_PLAYING (u32)(0xffff1112)	//if SOUNDSAMPLECONTEXT_PLAYING && hw channel == disabled, -> SOUNDSAMPLECONTEXT_IDLE

//soundTGDS playback modes
#define SOUNDSAMPLECONTEXT_SOUND_IDLE (int)(0)
#define SOUNDSAMPLECONTEXT_SOUND_SAMPLEPLAYBACK (int)(1)
#define SOUNDSAMPLECONTEXT_SOUND_STREAMPLAYBACK (int)(2)

#define SOUNDSTREAM_L_BUFFER (int)(0)
#define SOUNDSTREAM_R_BUFFER (int)(1)

#define SRC_NONE	(int)(0)
#define SRC_WAV	(int)(14)

#define WAV_READ_SIZE 2048

#ifdef ARM9
static inline u32 getWavData(void *outLoc, int amount, FILE *fh)
{
	return fread(outLoc, 1, amount, fh);
}
#endif

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
} wavFormatChunk __attribute__((aligned (4)));

//Sound Context
struct soundSampleContext{
	int sampleRate;
	s16 *arm9data;
	s16 *arm9LInterpolated; //auxiliary buffer if interpolation is desired. Needs to be same size as s16 * arm9data
	u32 bytes;
	u8 channel;
	u8 vol;
	u8 pan;
	u32 status;
	u8 format;
}__attribute__((aligned (4)));

//SoundSample;TGDS format: Runs through irqs
struct soundSampleContextList{
	struct soundSampleContext soundSampleCxt[SoundSampleContextChannels];	//Each channel used as a hardware channel
}__attribute__((aligned (4)));

//SoundStream;TGDS format: Runs synchronously in ARM9 decoder thread
struct soundPlayerContext{
	//Decoder properties
	struct soundSampleContext soundSampleCxt[SoundSampleContextChannels];	//Each channel used as sample stream context, which means a single channel here can fill n hardware channels
	
	//FS properties
	int fileSize;	//u32 len;	u32 dataLen;
	int fileOffset;	//u32 loc;	u32 dataOffset;
	
	//Wav
	wavFormatChunk wavDescriptor;
	
	//Playback properties
	u8 volume;
	u32 cr;
	u32 timer;
	int sourceFmt;
	int structFDFH;	//TGDS File Descriptor through TGDS FS API, same as ARM9's FILE * GlobalSoundStreamFile;
} __attribute__((aligned (4)));

#ifdef __cplusplus
extern "C"{
#endif

//Sound Sample Context: Plays raw sound samples at VBLANK intervals
extern void startSound(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format);
extern void initSound();

#ifdef ARM7
extern int soundSampleContextCurrentMode;
extern void initSoundSampleContext();
extern void initSoundStream();

//weak symbols : the implementation of these is project-defined, also abstracted from the hardware IPC FIFO Implementation for easier programming.
extern __attribute__((weak))	void initSoundStreamUser();

//ARM7: Sample Playback handler
extern void updateSoundContextSamplePlayback();

extern s16 *strpcmL0;
extern s16 *strpcmL1;
extern s16 *strpcmR0;
extern s16 *strpcmR1;

extern int lastL;
extern int lastR;

extern void mallocData(int size);
extern void freeData();
extern void SetupSound();
extern void StopSound();

extern u32 sampleLen;
extern int multRate;
extern int sndRate;
extern u32 sndCursor;
#endif

#ifdef ARM9
//ARM9: Stream Playback handler
extern void updateSoundContextStreamPlayback();
extern void flushSoundContext(int soundContextIndex);
#endif

extern struct soundSampleContext * getsoundSampleContextByIndex(int index);
extern bool freesoundSampleContext(struct soundSampleContext * sampleInst);	//free up a given soundSampleContext
extern struct soundSampleContext * getFreeSoundSampleContext();				//obtains a free soundSampleContext, if any

#ifdef ARM9
extern FILE * GlobalSoundStreamFile;
extern bool setSoundSampleContext(int sampleRate, u32 * data, u32 bytes, u8 channel, u8 vol, u8 pan, u8 format);
extern s16 * SharedEWRAM0;	//ptr start = 0
extern s16 * SharedEWRAM1;	//ptr start = 0 + 0x4000

extern void startSound9();
extern void stopSound();

extern int parseWaveData(FILE * fh);
extern void setSoundLength(u32 len);
extern void setSoundFrequency(u32 freq);
extern void setSoundInterpolation(u32 mult);
extern void copyChunk();
extern void setSoundFrequency(u32 freq);
extern void freeSound();
extern void swapAndSend(u32 type);
extern void swapData();
extern void freeData();
extern void mallocData(int size);
#endif

extern void EnableSoundSampleContext(int SndSamplemode);
extern void DisableSoundSampleContext();
extern void closeSoundStream();
extern void startSound9();
extern void stopSound();
extern void updateSoundContextStreamPlayback();
extern void initComplexSound();
extern void pauseSound(bool pause);

#ifdef __cplusplus
}
#endif

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

static inline void setSwapChannel()
{
	s16 *buf;
  
	if(!sndCursor)
		buf = strpcmL0;
	else
		buf = strpcmL1;
    
	// Left channel
	SCHANNEL_SOURCE((sndCursor << 1)) = (uint32)buf;
	SCHANNEL_CR((sndCursor << 1)) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_16BIT;
    	
	if(!sndCursor)
		buf = strpcmR0;
	else
		buf = strpcmR1;
	
	// Right channel
	SCHANNEL_SOURCE((sndCursor << 1) + 1) = (uint32)buf;
	SCHANNEL_CR((sndCursor << 1) + 1) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x3FF) | SOUND_16BIT;
  
	sndCursor = 1 - sndCursor;
}

static inline s16 checkClipping(int data)
{
	if(data > 32767)
		return 32767;
	if(data < -32768)
		return -32768;
	
	return data;
}

static inline int getSoundSampleContextEnabledStatus(){	
	return soundSampleContextCurrentMode;
}

#endif

#endif
