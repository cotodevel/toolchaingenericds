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

#define ARM9COMMAND_UPDATE_BUFFER (uint32)(0xFEFEFF02)
#define ARM7COMMAND_START_SOUND (uint32)(0xFEFEFF10)
#define ARM7COMMAND_STOP_SOUND (uint32)(0xFEFEFF11)
#define ARM7COMMAND_SOUND_SETMULT (uint32)(0xFEFEFF12)
#define ARM7COMMAND_SOUND_SETRATE (uint32)(0xFEFEFF13)
#define ARM7COMMAND_SOUND_SETLEN (uint32)(0xFEFEFF14)
#define ARM7COMMAND_SOUND_COPY (uint32)(0xFEFEFF15)
#define ARM7COMMAND_SOUND_DEINTERLACE (uint32)(0xFEFEFF16)
#define ARM7COMMAND_PSG_COMMAND (uint32)(0xF0F0FF22)
#define ARM7COMMAND_SND_COMMAND (uint32)(0xF0F0FF23)

#define FIFO_INITSOUND	(uint32)(0xffff0204)

#ifdef ARM9
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

#define SoundSampleContextChannels (int)(16)

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
#define SRC_VGM				(int)(16)

#define WAV_READ_SIZE 4096

static inline void SendFIFOWords(uint32 data0, uint32 data1){	//format: arg0: cmd, arg1: value
	REG_IPC_FIFO_TX = (uint32)data1;
	REG_IPC_FIFO_TX = (uint32)data0;
}

#ifdef ARM7
#define VRAM_D		((s16*)0x06000000)	//ARM7 128K 
#endif

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
	int volume;
	
	u32 tX;
	u32 tY;
	
	//PSG
	int psgChannel;
	u32 cr;
	u16 timer;
} SoundRegion __attribute__((aligned (4)));

//ARM7: PTR to EWRAM
//ARM9: EWRAM (can't be Shared Memory cause sound clicks)
//SoundStream;TGDS format: Runs synchronously in ARM9 decoder thread
struct soundPlayerContext{
	//Decoder properties
	struct soundSampleContext soundSampleCxt[SoundSampleContextChannels];	//Each channel used as sample stream context, which means a single channel here can fill n hardware channels
	
	bool soundStreamPause;	//Indicates wether a WAV / IMA-ADPCM / Other stream format is playing
	int sourceFmt;
	int bufLoc;
	int channels;
	int bits;
	u32 len;
	u32 loc;
	u32 dataOffset;
	u32 dataLen;
	#ifdef ARM9
	FILE *filePointer;
	struct fd* filePointerStructFD;	//TGDS FS handle of the above FILE* . a.k.a: GlobalSoundStreamStructFD
	int mp3SampleRate;
	#endif
	
} __attribute__((aligned (4)));

#ifdef __cplusplus
extern "C"{
#endif

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
extern void setSwapChannel();
extern void setupSound(uint32 sourceBuf);
extern void mallocDataARM7(int size, uint16* sourceBuf);
#endif

extern void initSound();
extern void stopSound();

#ifdef ARM9
extern bool soundLoaded;
extern void mallocData(int size);

//weak symbols : the implementation of these is project-defined, also abstracted from the hardware IPC FIFO Implementation for easier programming.
extern     void updateStreamCustomDecoder(u32 srcFrmt);
extern     void freeSoundCustomDecoder(u32 srcFrmt);
extern     void closeSoundUser();
extern     bool stopSoundStreamUser();	//abstracts an user class when closing an WAV/IMAADPCM stream (because the audio stream context is TGDS Project specific)

//Stream Sound controls
extern void closeSound();
extern void initComplexSound();
extern void setSoundFrequency(u32 freq);
extern void setSoundLength(u32 len);
extern int getVolume();
extern void setVolume(int volume);
extern void volumeUp(int x, int y);
extern void volumeDown(int x, int y);
extern char *strlwr(char *str);
extern void swapAndSend(u32 type);
extern int getSoundLength();

#endif

extern void TIMER1Handler();

#ifdef ARM9
extern int initSoundStreamFromStructFD(struct fd * _FileHandleAudio, uint32 sourceBuf);
extern void setWavDecodeCallback(void (*cb)());

extern int parseWaveData(struct fd * fdinst, u32 u32chunkToSeek);
extern void setSoundLength(u32 len);
extern void setSoundFrequency(u32 freq);
extern void setSoundInterpolation(u32 mult);
extern void setSoundFrequency(u32 freq);
extern bool updateRequested;
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
extern char *memoryContents;
extern u32 memoryPos;
extern u32 memorySize;
extern void (*wavDecode)();
// alternate malloc stuff
extern int m_SIWRAM;
extern int m_size;

extern void updateStream();
extern void startSound9(uint32 sourceBuf);

//Usercode: Opens a .WAV or IMA-ADPCM (Intel) file and begins to stream it.
//Returns: the stream format.
extern int playSoundStream(char * audioStreamFilename, struct fd * _FileHandleVideo, struct fd * _FileHandleAudio, uint32 sourceBuf);

//Usercode: Stops an audiostream playback.
//Returns: true if successfully halted, false if no audiostream available.
extern bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType);

extern int internalCodecType;	//defines either AD-PCM, WAV or other formats

#endif

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

#endif

#ifdef ARM9
extern void writeARM7SoundChannel(int channel, u32 cnt, u16 freq);
extern void writeARM7SoundChannelFromSource(int channel, u32 cnt, u16 freq, u32 dataSrc, u32 dataSize);
#endif

#endif
