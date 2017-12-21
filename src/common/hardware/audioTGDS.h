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

#include "typedefs.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <unistd.h>

//NDS Audio channels
#define MaxChannels	(sint32)(16)

//used by: ChannelEnabledState
#define audiostateDisabled	(bool)(false)
#define audiostateEnabled	(bool)(true)

//used by : pendingPlayStatus
#define	sampleStatusPlay	(uint32)(0xffff1100)
#define	sampleStatusStopping	(uint32)(0xffff1101)
#define	sampleStatusStarting	(uint32)(0xffff1102)
#define	sampleStatusSeeking	(uint32)(0xffff1103)
#define	sampleStatusStop	(uint32)(0xffff1104)

#define	sampleStubFrequency	(uint32)(0)
#define	sampleStubSampleSource	(uint32)(0)
#define	sampleStubSampleSize	(sint32)(0)
#define	sampleStubSampleController	(uint32)(0)

#define audioChannelError	(sint32)(-1)
struct sIPCSharedTGDSAudioChannel {
	bool ChannelEnabledState;
	uint32	ChannelFrequency;
	uint32	ChannelSampleSource;
	sint32	ChannelSampleSize;
	uint32	ChannelController;
	uint32 pendingPlayStatus;
};

struct sIPCSharedTGDSAudioGlobal {
	struct sIPCSharedTGDSAudioChannel	sIPCSharedTGDSAudioChannelInst[MaxChannels];
};


#endif

#ifdef __cplusplus
extern "C"{
#endif

extern struct sIPCSharedTGDSAudioGlobal * getsIPCSharedTGDSAudioGlobal();
extern void updateSound();

#ifdef ARM7
extern int currentChannelSeek;
#endif

extern sint32 getSoundFreeChannel();
extern 	void initTGDSAudioSystem();
extern 	int PlaySample(struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel);
extern	int StopSample(struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel);

#ifdef __cplusplus
}
#endif