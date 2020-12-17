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

#include "typedefsTGDS.h"
#include "dsregs.h"

#ifdef ARM9
#include "fatfslayerTGDS.h"
#endif

//IPC bits
#define REG_IPC_FIFO_TX		(*(vuint32*)0x4000188)
#define REG_IPC_FIFO_RX		(*(vuint32*)0x4100000)
#define REG_IPC_FIFO_CR		(*(vuint16*)0x4000184)

#define REG_IPC_SYNC	(*(vuint16*)0x04000180)
#define IPC_SYNC_IRQ_ENABLE		(uint16)(1<<14)
#define IPC_SYNC_IRQ_REQUEST	(uint16)(1<<13)
#define IPC_FIFO_SEND_EMPTY		(uint16)(1<<0)
#define IPC_FIFO_SEND_FULL		(uint16)(1<<1)
#define IPC_FIFO_SEND_IRQ		(uint16)(1<<2)
#define IPC_FIFO_SEND_CLEAR		(uint16)(1<<3)
#define IPC_FIFO_RECV_EMPTY		(uint16)(1<<8)
#define IPC_FIFO_RECV_FULL		(uint16)(1<<9)
#define IPC_FIFO_RECV_IRQ		(uint16)(1<<10)
#define IPC_FIFO_ERROR			(uint16)(1<<14)
#define IPC_FIFO_ENABLE			(uint16)(1<<15)

#define ARM9COMMAND_UPDATE_BUFFER (uint32)(0xFFFFFF02)
#define ARM7COMMAND_START_SOUND (uint32)(0xFFFFFF10)
#define ARM7COMMAND_STOP_SOUND (uint32)(0xFFFFFF11)
#define ARM7COMMAND_SOUND_SETMULT (uint32)(0xFFFFFF12)
#define ARM7COMMAND_SOUND_SETRATE (uint32)(0xFFFFFF13)
#define ARM7COMMAND_SOUND_SETLEN (uint32)(0xFFFFFF14)
#define ARM7COMMAND_SOUND_COPY (uint32)(0xFFFFFF15)
#define ARM7COMMAND_SOUND_DEINTERLACE (uint32)(0xFFFFFF16)
#define ARM7COMMAND_PSG_COMMAND (uint32)(0xFFFFFF23)

//Linear sound sample playback: Sound Sample Context cmds (ARM9 -> ARM7)
#define FIFO_PLAYSOUND	(uint32)(0xffff0203)
#define FIFO_INITSOUND	(uint32)(0xffff0204)
#define FIFO_FLUSHSOUNDCONTEXT	(uint32)(0xffff0210)


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

//Hardware Channels used by soundStream playback
#define SOUNDSTREAM_L_BUFFER (int)(0)
#define SOUNDSTREAM_R_BUFFER (int)(1)
//Auxiliary Hardware Channels used by soundStream playback
#define SOUNDSTREAM_L_BUFFER_AUX (int)(0+2)
#define SOUNDSTREAM_R_BUFFER_AUX (int)(1+2)

//Free Hardware channels for either sampleStream or soundStream
#define SOUNDSTREAM_FREE_CHANNEL (int)(SOUNDSTREAM_R_BUFFER_AUX + 1)

#define SRC_NONE	(int)(0)
#define SRC_WAV		(int)(1)
#define SRC_MIKMOD	(int)(2)
#define SRC_MP3		(int)(3)
#define SRC_OGG		(int)(4)
#define SRC_AAC		(int)(5)
#define SRC_FLAC	(int)(6)
#define SRC_SID		(int)(7)
#define SRC_NSF		(int)(8)
#define SRC_SPC		(int)(9)
#define SRC_SNDH	(int)(10)
#define SRC_GBS		(int)(11)
#define SRC_STREAM_MP3		(int)(12)
#define SRC_STREAM_OGG		(int)(13)
#define SRC_STREAM_AAC		(int)(14)
#define SRC_WAVADPCM		(int)(15)
#define WAV_READ_SIZE 4096

#define VRAM_D		((s16*)0x06000000)
#define SIWRAM		((s16*)0x037F8000)

#define REG_SIWRAMCNT (*(vu8*)0x04000247)
#define SIWRAM0 ((s16 *)0x037F8000)
#define SIWRAM1 ((s16 *)0x037FC000)

//IMA-ADPCM
struct dvi_adpcmblockheader_tag {
	int iSamp0;
	char bStepTableIndex;
	char bReserved;
};

//WAV Descriptor
typedef struct 
{
	char chunkID[4];
	long chunkSize;
	
	//-- WAVEFORMAT --//
	short wFormatTag;
	unsigned short wChannels;
	unsigned long dwSamplesPerSec;
	unsigned long dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
	
	//-- WAVEFORMATEX --//
	unsigned short cbSize;	//bytes 16-17  cbSize: Size, in bytes, of extra format information appended to the end of the WAVEFORMATEX structure
	short wSamplesPerBlock;	//bytes 18..   extradata [Extended WAV impl]
	
} wavFormatChunk __attribute__((aligned (4)));

//Extended Header WAV 
#define WAVE_FORMAT_RAW_PCM		(short)(0x0001)
#define WAVE_FORMAT_DVI_ADPCM	(short)(0x0011)					/* Intel Corporation */
#define WAVE_FORMAT_IMA_ADPCM	(short)(WAVE_FORMAT_DVI_ADPCM)	/* Intel Corporation */

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

//Soundstream
#define BIT(n) (1 << (n))
typedef struct
{
	s16 *arm9L;
	s16 *arm9R;
	
	s16 *interlaced;
	int channels;
	u8 volume;
	
	u32 tX;
	u32 tY;
	
	int psgChannel;
	u32 cr;
	u32 timer;
} SoundRegion;

//ARM7: PTR to EWRAM
//ARM9: EWRAM (can't be Shared Memory cause sound clicks)
//SoundStream;TGDS format: Runs synchronously in ARM9 decoder thread
struct soundPlayerContext{
	//Decoder properties
	struct soundSampleContext soundSampleCxt[SoundSampleContextChannels];	//Each channel used as sample stream context, which means a single channel here can fill n hardware channels
	
	#ifdef ARM9
	bool soundStreamPause;	//Indicates wether a WAV / IMA-ADPCM / Other stream format is playing
	int sourceFmt;
	int bufLoc;
	int channels;
	FILE *filePointer;
	struct fd* filePointerStructFD;	//TGDS FS handle of the above FILE* . a.k.a: GlobalSoundStreamStructFD
	int bits;
	u32 len;
	u32 loc;
	u32 dataOffset;
	u32 dataLen;
	int mp3SampleRate;
	#endif
	
} __attribute__((aligned (4)));

#ifdef ARM7

extern s16 *strpcmL0;
extern s16 *strpcmL1;
extern s16 *strpcmR0;
extern s16 *strpcmR1;
extern int lastL;
extern int lastR;
extern int multRate;
extern int pollCount; //start with a read
extern u32 sndCursor;
extern u32 micBufLoc;
extern u32 sampleLen;
extern int sndRate;
extern void freeData();
extern void setSwapChannel();

#ifdef __cplusplus
extern "C"{
#endif

extern void setupSound();
extern void SendFIFOWords(uint32 data0, uint32 data1);

#ifdef __cplusplus
}
#endif

static inline void TIMER1Handler()
{	
	setSwapChannel();
	SendFIFOWords(ARM9COMMAND_UPDATE_BUFFER, 0);
}

#endif

#ifdef __cplusplus
extern "C"{
#endif

//Sound Sample Context: Plays raw sound samples at VBLANK intervals
extern void startSoundSample(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format);
extern void initSound();
extern void stopSound();
extern void mallocData(int size);

#ifdef ARM7
extern int soundSampleContextCurrentMode;
extern void initSoundSampleContext();
extern void initSoundStream(u32 srcFmt);
extern int SoundTGDSCurChannel;
#endif

#ifdef ARM9
//weak symbols : the implementation of these is project-defined, also abstracted from the hardware IPC FIFO Implementation for easier programming.
extern __attribute__((weak))    void updateStreamCustomDecoder(u32 srcFrmt);
extern __attribute__((weak))    void freeSoundCustomDecoder(u32 srcFrmt);
extern void flushSoundContext(int soundContextIndex);
extern __attribute__((weak))    void closeSoundUser();

//Stream Sound controls
extern void closeSound();
extern void initComplexSound();
extern void setSoundInterrupt();
extern void setSoundFrequency(u32 freq);
extern void setSoundLength(u32 len);
extern u8 getVolume();
extern void setVolume(u8 volume);
extern void volumeUp(int x, int y);
extern void volumeDown(int x, int y);
extern bool soundLoaded;
extern char *strlwr(char *str);
extern void swapAndSend(u32 type);
extern int getSoundLength();

#endif

extern struct soundSampleContext * getsoundSampleContextByIndex(int index);
extern bool freesoundSampleContext(struct soundSampleContext * sampleInst);	//free up a given soundSampleContext
extern struct soundSampleContext * getFreeSoundSampleContext();				//obtains a free soundSampleContext, if any

#ifdef ARM9
extern int initSoundStream(char * audioStreamFilename);
extern int initSoundStreamFromStructFD(struct fd * _FileHandleAudio, char * ext);
extern bool setSoundSampleContext(int sampleRate, u32 * data, u32 bytes, u8 channel, u8 vol, u8 pan, u8 format);
extern void setWavDecodeCallback(void (*cb)());

extern int parseWaveData(struct fd * fdinst, u32 u32chunkToSeek);
extern void setSoundLength(u32 len);
extern void setSoundFrequency(u32 freq);
extern void setSoundInterpolation(u32 mult);
extern void setSoundFrequency(u32 freq);

extern bool updateRequested;
extern struct soundPlayerContext soundData;
extern bool soundLoaded;
extern bool canSend;

extern int bufCursor;
extern int bytesLeft;
extern s16 *bytesLeftBuf;
extern int maxBytes;

extern bool cutOff;
extern bool sndPaused;
extern bool playing;
extern bool seekSpecial;
extern bool updateRequested;
extern int sndLen;
extern int seekUpdate;

// sound out
extern s16 *lBuffer;
extern s16 *rBuffer;

// wav
extern bool memoryLoad;
extern char *memoryContents;
extern u32 memoryPos;
extern u32 memorySize;
extern void (*wavDecode)();
// alternate malloc stuff
extern int m_SIWRAM;
extern int m_size;

extern void wavDecode8Bit();
extern void wavDecode16Bit();
extern void wavDecode24Bit();
extern void wavDecode32Bit();
extern void updateStream();
extern void freeSound();
extern void setWavDecodeCallback(void (*cb)());
extern void startSound9();

//Usercode: Opens a .WAV or IMA-ADPCM (Intel) file and begins to stream it.
//Returns: the stream format.
extern int playSoundStream(char * audioStreamFilename);

//Usercode: Stops an audiostream playback.
//Returns: true if successfully halted, false if no audiostream available.
extern bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType);

#endif

extern void EnableSoundSampleContext(int SndSamplemode);
extern void DisableSoundSampleContext();

#ifdef __cplusplus
}
#endif

#ifdef ARM7
static inline s32 getFreeSoundChannel(){
	int i;
	for (i=SOUNDSTREAM_FREE_CHANNEL;i<16;++i){
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

//ARM7: Sample Playback handler
static inline void updateSoundContextSamplePlayback(){
	bool playSample = false;
	switch(getSoundSampleContextEnabledStatus()){
		//Samples
		case(SOUNDSAMPLECONTEXT_SOUND_SAMPLEPLAYBACK):
		{
			playSample = true;
		}
		break;
		
		case(SOUNDSAMPLECONTEXT_SOUND_STREAMPLAYBACK):{
			//Only play sound samples when SRC_WAV or SRC_WAVADPCM streams
			/*	//todo
			if(soundData.sourceFmt == SRC_WAV){
				playSample = true;
			}
			*/
		}
		break;
	}
	
	if(playSample == true){
		//VBLANK intervals: Look out for assigned channels, playing.
		struct soundSampleContext * curSoundSampleContext = getsoundSampleContextByIndex(SoundTGDSCurChannel);
		int thisChannel = curSoundSampleContext->channel;
		
		//Returns -1 if channel is busy, or channel if idle
		if( (isFreeSoundChannel(thisChannel) == thisChannel) && (curSoundSampleContext->status == SOUNDSAMPLECONTEXT_PENDING) ){	//Play sample?
			startSoundSample(curSoundSampleContext->sampleRate, curSoundSampleContext->arm9data, curSoundSampleContext->bytes, thisChannel, curSoundSampleContext->vol,  curSoundSampleContext->pan, curSoundSampleContext->format);
			curSoundSampleContext->status = SOUNDSAMPLECONTEXT_PLAYING;
		}
		
		//Returns -1 if channel is busy, or channel if idle
		if( (isFreeSoundChannel(thisChannel) == thisChannel) && (curSoundSampleContext->status == SOUNDSAMPLECONTEXT_PLAYING) ){	//Idle? free context
			freesoundSampleContext(curSoundSampleContext);
			SendFIFOWords(FIFO_FLUSHSOUNDCONTEXT, thisChannel);
		}
		
		if(SoundTGDSCurChannel > SoundSampleContextChannels){
			SoundTGDSCurChannel = 0;
		}
		else{
			SoundTGDSCurChannel++;
		}
	}
}

#endif

#endif
