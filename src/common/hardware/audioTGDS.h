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

#ifndef __nds_audiotgds_h__
#define __nds_audiotgds_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <unistd.h>

#define SOUND_16BIT (1<<29)
#define SOUND_8BIT (0)

//NDS Audio channels
#define MaxChannels	(sint32)(16)

//used by: CurrentAudioChannel->ChannelHWIndexL
#define	HWchannelDisabled	(sint32)(17)

//used by: ChannelEnabledState
#define audiostateDisabled	(uint32)(0xffff1001)
#define audiostateEnabled	(uint32)(0xffff1000)

//used by: pendingPlayStatusHW
#define	sampleStatusIdle	(uint32)(0xffff1101)
#define	sampleStatusPlaying	(uint32)(0xffff1102)	//hw play


//used by : pendingPlayStatusSW. on fetch/decode should be set to Play
#define	sampleStatusPlay	(uint32)(0xffff1100)		//actual playback
#define	sampleStatusSeeking	(uint32)(0xffff1103)	//used by fetch/decoder -> and arm7: sample was played already
#define	sampleStatusStop	(uint32)(0xffff1104)

#define	sampleStubFrequency	(uint32)(0)
#define	sampleStubSampleSource	(uint32)(0)
#define	sampleStubSampleSize	(sint32)(0)

//uint32 audioSystemEnabled;
#define AUDIOSYSENABLED (uint32)(0)
#define AUDIOSYSDISABLED (uint32)(1)

//Channels
#define stereoChannel (sint32)(1)
#define monoChannel (sint32)(2)
#define audioChannelError	(sint32)(-1)
#define audioChannelOK	(sint32)(0)

struct sIPCSharedTGDSAudioChannel {
	uint32 ChannelEnabledState;
	uint32	ChannelFrequency;
	uint32	ChannelSampleSource;	//buffer that has the fetch/decoded sample(s): buffer is at least ChannelSampleSize*ChannelPendingSamples
	sint32	ChannelSampleSize;		//each sample size
	sint32	ChannelPendingSamples;	//number of ChannelSampleSize to play 
	uint32 pendingPlayStatusHW;
	sint32 ChannelIndex;				//channel index alloced by: getSoundFreeChannel(), takes a physical NDS channel
	uint32 ChannelControl;				//this decides wether it's left or right speaker
};

struct sIPCSharedTGDSAudioGlobal {
	struct sIPCSharedTGDSAudioChannel	sIPCSharedTGDSAudioChannelInst[MaxChannels];
	sint32	Channels;				//stereoChannel	/ monoChannel
	uint32 audioSystemEnabled;
	uint32 pendingPlayStatusSW;		//1 audio : N hardware channels assigned
	sint32	channelIndexL;
	sint32	channelIndexR;
	
};


#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void updateSound();

extern struct sIPCSharedTGDSAudioGlobal * getsIPCSharedTGDSAudioGlobal();

#ifdef ARM7
extern int currentChannelSeek;
#endif

#ifdef ARM7
extern 	void initTGDSAudioSystem();
extern 	int PlayChannel(int channel);
extern	int StopChannel(int channel);
#endif

extern void enableAUDIOSys();
extern void disableAUDIOSys();
extern sint32 getSoundFreeChannel();
extern void FreeSoundChannel(int channel);

#ifdef __cplusplus
}
#endif