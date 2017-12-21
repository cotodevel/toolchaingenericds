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
#include "audioTGDS.h"
#include "ipcfifoTGDS.h"

#ifdef ARM7
int currentChannelSeek = 0;
#endif

//this runs in ARM7/ARM9 but task are separated
#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
void updateSound(){
	
	#ifdef ARM7	
	//struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	//struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel = (struct sIPCSharedTGDSAudioChannel*)&sIPCSharedTGDSAudioGlobalInst->sIPCSharedTGDSAudioChannelInst[currentChannelSeek];
	
	//if samples are pending, play
	if(currentChannelSeek < (MaxChannels-1) ){
		currentChannelSeek++;
	}
	else{
		currentChannelSeek = 0;
	}
	#endif
	
	#ifdef ARM9

	#endif
	
}


struct sIPCSharedTGDSAudioGlobal * getsIPCSharedTGDSAudioGlobal(){
	struct sIPCSharedTGDS* sIPCSharedTGDSInst = getsIPCSharedTGDS();
	return (struct sIPCSharedTGDSAudioGlobal *)&sIPCSharedTGDSInst->sIPCSharedTGDSAudioGlobalInst;
}

sint32 getSoundFreeChannel(){
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	int i = 0;
	for(i = 0; i < MaxChannels; i++){
		struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel = (struct sIPCSharedTGDSAudioChannel*)&sIPCSharedTGDSAudioGlobalInst->sIPCSharedTGDSAudioChannelInst[i];
		if(CurrentAudioChannel->ChannelEnabledState == audiostateDisabled){
			return i;
		}
	}
	return audioChannelError;
}

void initTGDSAudioSystem(){
	struct sIPCSharedTGDSAudioGlobal * sIPCSharedTGDSAudioGlobalInst = getsIPCSharedTGDSAudioGlobal();
	int i = 0;
	for(i = 0; i < MaxChannels; i++){
		struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel = (struct sIPCSharedTGDSAudioChannel*)&sIPCSharedTGDSAudioGlobalInst->sIPCSharedTGDSAudioChannelInst[i];
		CurrentAudioChannel->ChannelEnabledState = audiostateDisabled;
		CurrentAudioChannel->ChannelFrequency =	sampleStubFrequency;
		CurrentAudioChannel->ChannelSampleSource =	sampleStubSampleSource;
		CurrentAudioChannel->ChannelSampleSize = sampleStubSampleSize;
		CurrentAudioChannel->ChannelController = sampleStubSampleController;
		CurrentAudioChannel->pendingPlayStatus = sampleStatusStop;
	}
}

int PlaySample(struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel){
	if(CurrentAudioChannel){
		
	}
	
	return audioChannelError;
}

int StopSample(struct sIPCSharedTGDSAudioChannel* CurrentAudioChannel){
	if(CurrentAudioChannel){
		
	}
	
	return audioChannelError;
}