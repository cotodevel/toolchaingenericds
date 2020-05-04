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
#include "soundTGDS.h"
#include "ipcfifoTGDS.h"

void initSound(){
	#ifdef ARM7
	SoundPowerON(127);		//volume
	#endif
	
	#ifdef ARM9
	SendFIFOWords(FIFO_INITSOUND, 0);
	#endif
}

//Sound Sample Context: Plays raw sound samples at VBLANK intervals
void startSound(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format)
{
	#ifdef ARM9
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	coherent_user_range_by_size((uint32)fifomsg, sizeof(TGDSIPC->fifoMesaggingQueue));
	coherent_user_range_by_size((uint32)data, bytes);	//coherent sound buffer if within cached EWRAM
	
	fifomsg[0] = (uint32)sampleRate;
	fifomsg[1] = (uint32)data;
	fifomsg[2] = (uint32)bytes;
	
	u32 packedSnd = (u32)( ((channel&0xff) << 24) | ((vol&0xff) << 16) | ((pan&0xff) << 8) | (format&0xff) );
	fifomsg[3] = (uint32)packedSnd;
	SendFIFOWords(FIFO_PLAYSOUND, (uint32)fifomsg);
	#endif
	
	#ifdef ARM7
	SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
	SCHANNEL_SOURCE(channel) = (u32)data;
	SCHANNEL_LENGTH(channel) = bytes >> 2;
	SCHANNEL_CR(channel)     = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(pan) | (format << 29);
	#endif
}


#ifdef ARM7
static int curChannel = 0;
inline void updateSoundContext(){
	if(getSoundSampleContextEnabledStatus() == true){
		//VBLANK intervals: Look out for assigned channels, playing.
		struct soundSampleContext * curSoundSampleContext = getsoundSampleContextByIndex(curChannel);
		int thisChannel = curSoundSampleContext->channel;
		
		//Returns -1 if channel is busy, or channel if idle
		if( (isFreeSoundChannel(thisChannel) == thisChannel) && (curSoundSampleContext->status == SOUNDSAMPLECONTEXT_PENDING) ){	//Play sample?
			startSound(curSoundSampleContext->sampleRate, curSoundSampleContext->data, curSoundSampleContext->bytes, thisChannel, curSoundSampleContext->vol,  curSoundSampleContext->pan, curSoundSampleContext->format);
			curSoundSampleContext->status = SOUNDSAMPLECONTEXT_PLAYING;
		}
		
		//Returns -1 if channel is busy, or channel if idle
		if( (isFreeSoundChannel(thisChannel) == thisChannel) && (curSoundSampleContext->status == SOUNDSAMPLECONTEXT_PLAYING) ){	//Idle? free context
			freesoundSampleContext(curSoundSampleContext);
			SendFIFOWords(FIFO_FLUSHSOUNDCONTEXT, thisChannel);
		}
		
		if(curChannel > SOUNDCONTEXTCAPACITY){
			curChannel = 0;
		}
		else{
			curChannel++;
		}
	}
}

void initSoundSampleContext(){
	int i = 0;
	for(i = 0; i < SOUNDCONTEXTCAPACITY; i++){
		struct soundSampleContext * soundSampleCtx = getsoundSampleContextByIndex(i);
		freesoundSampleContext(soundSampleCtx);
	}
	EnableSoundSampleContext();
}
#endif

struct soundSampleContext * getsoundSampleContextByIndex(int index){
	if((index < 0) || (index > SOUNDCONTEXTCAPACITY)){
		return NULL;
	}
	
	return(struct soundSampleContext *)&TGDSIPC->soundContextShared.soundSampleCxt[index];
}

bool freesoundSampleContext(struct soundSampleContext * sampleInst){
	if(sampleInst != NULL){
		memset(sampleInst, 0, sizeof(struct soundSampleContext));		
		sampleInst->sampleRate = -1;
		sampleInst->data = NULL;
		sampleInst->bytes = -1;
		sampleInst->channel = -1;
		sampleInst->vol = -1;
		sampleInst->pan = -1;
		sampleInst->status = -1;
		sampleInst->format = -1;
		sampleInst->status = SOUNDSAMPLECONTEXT_IDLE;
		return true;
	}
	return false;
}

//Returns any available SoundSampleContext, or NULL
struct soundSampleContext * getFreeSoundSampleContext(){
	int i = 0;
	
	for(i = 0; i < SOUNDCONTEXTCAPACITY; i++){
		struct soundSampleContext * thisSoundSampleCtx = getsoundSampleContextByIndex(i);
		if(thisSoundSampleCtx->status == SOUNDSAMPLECONTEXT_IDLE){
			return thisSoundSampleCtx;
		}
	}
	return NULL;
}


#ifdef ARM9
//Allocates by free sampleContext. Sound Sample Context is queued
//Returns: true if success, false if full samples alloc'ed
bool setSoundSampleContext(int sampleRate, u32 * data, u32 bytes, u8 channel, u8 vol, u8 pan, u8 format){
	struct soundSampleContext * sampleInst = getFreeSoundSampleContext();
	if(sampleInst != NULL){
		sampleInst->sampleRate = sampleRate;
		sampleInst->data = data;
		sampleInst->bytes = bytes;
		sampleInst->channel = channel;
		sampleInst->vol = vol;
		sampleInst->pan = pan;
		sampleInst->format = format;
		sampleInst->status = SOUNDSAMPLECONTEXT_PENDING;
		return true;
	}
	return false;
}

//ARM7->ARM9: Just discarded the curChannelFreed's SoundSampleContext
void flushSoundContext(int soundContextIndex){
	
}

#endif

bool getSoundSampleContextEnabledStatus(){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->soundContextShared, sizeof(TGDSIPC->soundContextShared));
	#endif
	return TGDSIPC->soundContextShared.soundSampleContextEnabled;
}

void EnableSoundSampleContext(){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->soundContextShared, sizeof(TGDSIPC->soundContextShared));
	#endif
	TGDSIPC->soundContextShared.soundSampleContextEnabled = true;
}

void DisableSoundSampleContext(){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->soundContextShared, sizeof(TGDSIPC->soundContextShared));
	#endif
	TGDSIPC->soundContextShared.soundSampleContextEnabled = false;
}
