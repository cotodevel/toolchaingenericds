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
#include "dsregs.h"
#include "dsregs_asm.h"
#include <unistd.h>
#include <math.h>
#include "audioTGDS.h"
#include "ipcfifoTGDS.h"
#include "timerTGDS.h"
#include "utilsTGDS.h"
#ifdef ARM9
#include "nds_cp15_misc.h"
#endif

#ifdef ARM7
int currentChannelSeek = 0;
#endif

//formula to calculate sample fragment in time: (int)BUS_CLOCK/(1/60)

//this runs in ARM7/ARM9 but task are separated
#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
void updateSound(){
	#ifdef ARM7
	//double cyclesPerVblank = (int)BUS_CLOCK * (int)round(1/60);	// (33554432) * 0.017 = 570425.344 cycles each vblank interrupt
	//round(570425 / 44100) = at least 13 samples running @ 44100 in VBLANK, that's our top decoded samples per channel
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	//sound system Init works...
	if(sIPCSharedTGDSAudioGlobalInst->audioSystemEnabled == AUDIOSYSENABLED){
		struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel = (struct sIPCSharedTGDSAudioChannel*)&sIPCSharedTGDSAudioGlobalInst->sIPCSharedTGDSAudioChannelInst[currentChannelSeek];
		if(CurrentAudioChannel->ChannelEnabledState == audiostateEnabled){
			//Channel being played by NDS hardware
			if(CurrentAudioChannel->pendingPlayStatusHW == sampleStatusIdle){	//decoder sets this channel to: sampleStatusPlay
				//if samples are pending, play
				PlayChannel(CurrentAudioChannel->ChannelIndex);
			}		
			if(CurrentAudioChannel->pendingPlayStatusHW == sampleStatusPlaying){	//logic should detect if sample offset does not reach end of file yet, if so, set sampleStatusSeeking to repeat decoder step
				//read current NDS hardware channel assigned, and stop if playback ended
				if(!(SCHANNEL_CR(CurrentAudioChannel->ChannelIndex) & SCHANNEL_ENABLE)){
					StopChannel(CurrentAudioChannel->ChannelIndex);
				}
			}
		}
	}
	if(currentChannelSeek < (MaxChannels - 1) ){
		currentChannelSeek++;
	}
	else{
		currentChannelSeek = 0;
	}
	#endif
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
struct sIPCSharedTGDSAudioGlobal * getsIPCSharedTGDSAudioGlobal(){
	#ifdef ARM9
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)&getsIPCSharedTGDS()->audioglobal, sizeof(getsIPCSharedTGDS()->audioglobal));
	#endif
	return (struct sIPCSharedTGDSAudioGlobal *)&getsIPCSharedTGDS()->audioglobal;
}

#ifdef ARM9
void setAudioGlobalInst(){
	
}
#endif

#ifdef ARM7
void initTGDSAudioSystem(){
	memset((uint32*)&getsIPCSharedTGDS()->audioglobal,0, sizeof(getsIPCSharedTGDS()->audioglobal));
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	int i = 0;
	for(i = 0; i < (int)MaxChannels; i++){
		StopChannel(i);
	}
	enableAUDIOSys();
}

//double getTopSamplesPerVblank(){
//	return (double)((int)BUS_CLOCK * (int)round(1/60));
//}

int PlayChannel(int channel){
	if((channel >= 0) && (channel <= (MaxChannels-1)) ){
		struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
		struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel = (struct sIPCSharedTGDSAudioChannel*)&sIPCSharedTGDSAudioGlobalInst->sIPCSharedTGDSAudioChannelInst[channel];
		//double cyclesPerVblank = getTopSamplesPerVblank();
		//int TopSamplesPerVblankSound = round( (cyclesPerVblank * CurrentAudioChannel->ChannelPendingSamples)/CurrentAudioChannel->ChannelFrequency);
		//havent started the stream yet?
		//SCHANNEL_CR(CurrentAudioChannel->ChannelIndexL) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(0x7F) | SOUND_PAN(0x0) | ((CurrentAudioChannel->Channels == stereoChannel) ? SOUND_16BIT : SOUND_8BIT);
		//SCHANNEL_CR(CurrentAudioChannel->ChannelIndexR) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | ((CurrentAudioChannel->Channels == stereoChannel) ? SOUND_16BIT : SOUND_8BIT);			
		SCHANNEL_TIMER(CurrentAudioChannel->ChannelIndex) = CurrentAudioChannel->ChannelFrequency;
		SCHANNEL_LENGTH(CurrentAudioChannel->ChannelIndex) = (3 * CurrentAudioChannel->ChannelSampleSize*CurrentAudioChannel->ChannelPendingSamples) >> 2;
		SCHANNEL_SOURCE(CurrentAudioChannel->ChannelIndex) = (u32)CurrentAudioChannel->ChannelSampleSource;				
		SCHANNEL_CR(CurrentAudioChannel->ChannelIndex) = CurrentAudioChannel->ChannelControl;
		CurrentAudioChannel->pendingPlayStatusHW = sampleStatusPlaying;
		//char * printfBuf7 = (char*)getPrintfBuffer();	
		//sprintf(printfBuf7,"PlayChannel:channel:%d",CurrentAudioChannel->ChannelIndex);
		//SendMultipleWordACK(FIFO_PRINTF_7, 0, 0, NULL);
		
		//pcmL == 16K
		//pcmR == 16K
		//todo: decoder should tell which sample is L / R
		//memcpy ((uint32*)pcmL, (uint32*)CurrentAudioChannel->ChannelSampleSource, (CurrentAudioChannel->ChannelSampleSize*CurrentAudioChannel->ChannelPendingSamples));
		return audioChannelOK;
	}
	return audioChannelError;
}

int StopChannel(int channel){
	if((channel >= 0) && (channel <= (MaxChannels-1)) ){
		//disable hw channel
		SCHANNEL_CR(channel) = 0;
		SCHANNEL_REPEAT_POINT(channel) = 0;
		SCHANNEL_TIMER(channel) = 0;
		SCHANNEL_LENGTH(channel) = 0;
		
		//free channel here
		FreeSoundChannel(channel);
		
		//char * printfBuf7 = (char*)getPrintfBuffer();
		//sprintf(printfBuf7,"StopChannel:channel:%d",channel);
		//SendMultipleWordACK(FIFO_PRINTF_7, 0, 0, NULL);
		
		return audioChannelOK;
	}
	return audioChannelError;
}
#endif

void enableAUDIOSys(){
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	sIPCSharedTGDSAudioGlobalInst->audioSystemEnabled = AUDIOSYSENABLED;
}

void disableAUDIOSys(){
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	sIPCSharedTGDSAudioGlobalInst->audioSystemEnabled = AUDIOSYSDISABLED;
	//todo: ds hardware de-init
}


sint32 getSoundFreeChannel(){
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	int i = 0;
	for(i = 0; i < MaxChannels; i++){
		struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel = (struct sIPCSharedTGDSAudioChannel*)&sIPCSharedTGDSAudioGlobalInst->sIPCSharedTGDSAudioChannelInst[i];
		#ifdef ARM9
		//clrscr();
		//printf("CurrentAudioChannel->ChannelEnabledState:%x",CurrentAudioChannel->ChannelEnabledState);
		#endif
		if(CurrentAudioChannel->ChannelEnabledState == audiostateDisabled){
			return i;
		}
	}
	return audioChannelError;
}

void FreeSoundChannel(int channel){
	if(channel > (MaxChannels-1) ){
		return;
	}
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel = (struct sIPCSharedTGDSAudioChannel*)&sIPCSharedTGDSAudioGlobalInst->sIPCSharedTGDSAudioChannelInst[channel];
	CurrentAudioChannel->ChannelEnabledState = audiostateDisabled;
	CurrentAudioChannel->ChannelFrequency =	sampleStubFrequency;
	CurrentAudioChannel->ChannelSampleSource =	sampleStubSampleSource;
	CurrentAudioChannel->ChannelSampleSize = sampleStubSampleSize;
}