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
#include "powerTGDS.h"
#include "timerTGDS.h"
#include "dmaTGDS.h"
#include "posixHandleTGDS.h"
#include "InterruptsARMCores_h.h"
#include "debugNocash.h"
#include "biosTGDS.h"

#ifdef ARM9
#include "utilsTGDS.h"
#include "fatfslayerTGDS.h"
#include "videoTGDS.h"
#include "ima_adpcm.h"
#endif

//Sound stream code taken from DSOrganize because it works 100% through interrupts and that's wonderful
/***************************************************************************
 *                                                                         *
 *  This file is part of DSOrganize.                                       *
 *                                                                         *
 *  DSOrganize is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  DSOrganize is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with DSOrganize.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                         *
 ***************************************************************************/

#ifdef ARM7

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mallocDataARM7(int size, uint16* sourceBuf)
{
    // this no longer uses malloc due to using dynamic memory.
	strpcmL0 = (s16*)sourceBuf;
	strpcmL1 = strpcmL0 + (size >> 1);
	strpcmR0 = strpcmL1 + (size >> 1);
	strpcmR1 = strpcmR0 + (size >> 1);
	
	// clear memory to not have sound leftover
	dmaFillHalfWord(0, 0, (uint32)strpcmL0, (uint32)((size + 3) & ~3) );
	dmaFillHalfWord(0, 0, (uint32)strpcmR0, (uint32)((size + 3) & ~3) );
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setSwapChannel() 
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setupSound(uint32 sourceBuf) {
	//Init SoundSampleContext
	initSound();

	sndCursor = 0;
	if(multRate != 1 && multRate != 2 && multRate != 4){
		multRate = 1;
	}
	
	mallocDataARM7(sampleLen * 2 * multRate, (uint16*)sourceBuf);
    
	TIMERXDATA(0) = SOUND_FREQ((sndRate * multRate));
	TIMERXCNT(0) = TIMER_DIV_1 | TIMER_ENABLE;
  
	TIMERXDATA(1) = 0x10000 - (sampleLen * 2 * multRate);
	TIMERXCNT(1) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
	
	int ch;
	for(ch=0;ch<4;++ch)
	{
		SCHANNEL_CR(ch) = 0;
		SCHANNEL_TIMER(ch) = SOUND_FREQ(sndRate * multRate);
		SCHANNEL_LENGTH(ch) = (sampleLen * multRate) >> 1;
		SCHANNEL_REPEAT_POINT(ch) = 0;
	}

	REG_IE&=~IRQ_VBLANK; //Not sure this is needed, could be removed in the future.
	REG_IE |= IRQ_TIMER1;
	
	// prevent accidentally reading garbage from buffer 0, by waiting for buffer 1 instead
	swiDelay((0x10000 - (sampleLen * multRate)) >> 1);
	
	lastL = 0;
	lastR = 0;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void stopSound() {
	TIMERXCNT(0) = 0;
	TIMERXCNT(1) = 0;
	
	SCHANNEL_CR(0) = 0;
	SCHANNEL_CR(1) = 0;
	SCHANNEL_CR(2) = 0;
	SCHANNEL_CR(3) = 0;
	
	REG_IE|=IRQ_VBLANK;	//Not sure this is needed, could be removed in the future.
	REG_IE &= ~IRQ_TIMER1;
}

void TIMER1Handler(){	
	setSwapChannel();
	SendFIFOWords(ARM9COMMAND_UPDATE_BUFFER, 0xFF);
}

#endif 
 
#ifdef ARM9
//Handles current file playback status
__attribute__((section(".dtcm")))
bool soundLoaded = false;

__attribute__((section(".dtcm")))
bool cutOff = false;	//used to detect if audio stream has ended

__attribute__((section(".dtcm")))
bool sndPaused = false;

__attribute__((section(".dtcm")))
bool playing = false;

__attribute__((section(".dtcm")))
bool seekSpecial = false;

__attribute__((section(".dtcm")))
int sndLen = 0;

__attribute__((section(".dtcm")))
int seekUpdate = -1;

// sound out
__attribute__((section(".dtcm")))
s16 *lBuffer = NULL;

__attribute__((section(".dtcm")))
s16 *rBuffer = NULL;

__attribute__((section(".dtcm")))
void (*wavDecode)() = NULL;

// alternate malloc stuff
__attribute__((section(".dtcm")))
int m_SIWRAM = 0;

__attribute__((section(".dtcm")))
int m_size = 0;

void mallocData(int size)
{
	m_SIWRAM = 0;
	REG_SIWRAMCNT = 0; // arm9 owns both
	
	u32 *siTemp = (u32 *)SIWRAM0;
	int i=0;
	for(i=0;i<0x2000;++i)
		siTemp[i] = 0;
	
	m_size = size;
	
	lBuffer = SIWRAM0;
	rBuffer = SIWRAM0 + m_size;
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
	TGDSIPC->soundIPC.arm9L = NULL; // temporary
	TGDSIPC->soundIPC.arm9R = NULL; // temporary
	TGDSIPC->soundIPC.interlaced = NULL;
}

__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void swapData() 
{
	m_SIWRAM = 1 - m_SIWRAM;
	
	switch(m_SIWRAM)
	{
		case 0:{
			lBuffer = SIWRAM0;
			rBuffer = SIWRAM0 + m_size;
			
			REG_SIWRAMCNT = 2; // bank 0 to arm9, bank 1 to arm7
			struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
			TGDSIPC->soundIPC.arm9L = SIWRAM1;
			TGDSIPC->soundIPC.arm9R = SIWRAM1 + m_size;
			TGDSIPC->soundIPC.interlaced = SIWRAM1;
		}
		break;
		case 1:{
			lBuffer = SIWRAM1;
			rBuffer = SIWRAM1 + m_size;
			
			REG_SIWRAMCNT = 1; // bank 0 to arm7, bank 1 to arm9
			struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
			TGDSIPC->soundIPC.arm9L = SIWRAM0;
			TGDSIPC->soundIPC.arm9R = SIWRAM0 + m_size;
			TGDSIPC->soundIPC.interlaced = SIWRAM0;
		}	
		break;
	}
}

__attribute__((section(".itcm")))
void swapAndSend(u32 type)
{
	swapData();
	SendFIFOWords(type, 0xFF);
}

// update function
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void updateStream() 
{	
	if((!updateRequested) || (soundData.sourceFmt == SRC_SPC))
	{
		// exit if nothing is needed or if SPC as it's handled by ARM7 SPC
		return;
	}
	
	// clear flag for update
	updateRequested = false;
	
	if(lBuffer == NULL || rBuffer == NULL)
	{
		// file is done
		stopSound();
		sndPaused = false;
		soundLoaded = false;
		return;
	}
	
	if(sndPaused || seekSpecial)
	{
		memset(lBuffer, 0, m_size * 2);
		memset(rBuffer, 0, m_size * 2);
		
		swapAndSend(ARM7COMMAND_SOUND_COPY);
		return;
	}
	
	if(cutOff)
	{
		// file is done
		
		stopSound();
		sndPaused = false;
		soundLoaded = false;
		return;
	}
	
	switch(soundData.sourceFmt)
	{
		case SRC_WAV:
		case SRC_WAVADPCM:
		{
			swapAndSend(ARM7COMMAND_SOUND_COPY);
			wavDecode();
		}
		break;
		default:{
			updateStreamCustomDecoder(soundData.sourceFmt);	//Custom decoder
		}
		break;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void initComplexSound() {
	soundData.sourceFmt = SRC_NONE;
	soundData.filePointer = NULL;
}

void setSoundFrequency(u32 freq)
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (uint32)freq);
	setValueSafe(&fifomsg[7], (u32)ARM7COMMAND_SOUND_SETRATE);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
}

void setSoundInterpolation(u32 mult)
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (uint32)mult);
	setValueSafe(&fifomsg[7], (u32)ARM7COMMAND_SOUND_SETMULT);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}	
}

void setSoundLength(u32 len)
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (uint32)len);
	setValueSafe(&fifomsg[7], (u32)ARM7COMMAND_SOUND_SETLEN);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
	sndLen = len;
}

int getSoundLength()
{
	return sndLen;
}

void startSound9(uint32 sourceBuf)
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[0], (uint32)sourceBuf);
	setValueSafe(&fifomsg[7], (u32)ARM7COMMAND_START_SOUND);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
	playing = true;
}

void stopSound()
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[7], (u32)ARM7COMMAND_STOP_SOUND);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
	playing = false;
}

void closeSound(){
	switch(internalCodecType)
	{
		case (SRC_WAVADPCM):
		case (SRC_WAV):
		{
			player.stop();
		}
		break;
		
		default:{
			freeSoundCustomDecoder(soundData.sourceFmt);
		}
		break;
	}
	
	stopSound();
	lBuffer = NULL;
	rBuffer = NULL;
	
	if(soundData.filePointer != NULL){
		fclose(soundData.filePointer);
	}
	
	soundData.filePointer = NULL;
	soundData.sourceFmt = SRC_NONE;	

	closeSoundUser();
	soundLoaded = false;	
}

void setWavDecodeCallback(void (*cb)()){
	wavDecode = cb;
}

//Usercode: Opens a .WAV or IMA-ADPCM (Intel) file and begins to stream it. Copies the file handles 
//Returns: the stream format.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int playSoundStream(char * audioStreamFilename, struct fd * _FileHandleVideo, struct fd * _FileHandleAudio, uint32 sourceBuf) {
	//If trying to play SoundStream code while Shared RAM is mapped @ ARM7, throw error
	if((WRAM_CR & WRAM_0KARM9_32KARM7) == WRAM_0KARM9_32KARM7){
		nocashMessage("TGDS:playSoundStream(): Trying to play a Sound Stream ");
		nocashMessage("but SharedWRAM is used by ARM7. Aborting.");
		return SRC_NONE;
	}
	
	int physFh1 = TGDSFSUserfatfs_open_file((const sint8 *)audioStreamFilename, (char *)"r");
	if(physFh1 != -1){ //struct fd index open ok?
		_FileHandleAudio = getStructFD(physFh1); //assign struct fd
		soundData.sourceFmt = initSoundStreamFromStructFD(_FileHandleAudio, sourceBuf);
		if(
			(soundData.sourceFmt == SRC_WAV)
			||
			(soundData.sourceFmt == SRC_WAVADPCM)
		){
			bool loop_audio = false;
			bool automatic_updates = false;
			if(player.play(getPosixFileHandleByStructFD(_FileHandleAudio, "r"), loop_audio, automatic_updates, ADPCM_SIZE, stopSoundStreamUser, sourceBuf) == 0){
				//ADPCM Playback!
			}
		}
		else{
			//Custom codecs require the file handle to be freed. TGDS FS does not support multiple file handles from the same file.
			TGDSFSUserfatfs_close(_FileHandleAudio);
		}
	}
	return soundData.sourceFmt;
}

//Checkout a Specific TGDS Project implementing a default close audio stream (supporting audio stream that is)
//bool stopSoundStreamUser(){
	//...
//}

//Internal: Stops an audiostream playback.
//Returns: true if successfully halted, false if no audiostream available.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType) {
	closeSound(); 
	*internalCodecType = SRC_NONE;
	swiDelay(888);
	TGDSFSUserfatfs_close(tgdsStructFD2);
	swiDelay(1);
	return true;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int initSoundStreamFromStructFD(struct fd * _FileHandleAudio, uint32 sourceBuf) {	//ARM9 Impl.
	initComplexSound(); // initialize sound variables
	soundData.filePointerStructFD = _FileHandleAudio;	//Global StructDS handle -> WAV stream
	
	// is it wav file?
	soundData.filePointerStructFD->loc = soundData.bufLoc = 0;
	char header[13];
	fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
	int read = fatfs_read(soundData.filePointerStructFD->cur_entry.d_ino, (u8*)&header[0], 12);
	soundData.filePointerStructFD->loc += (read); 
	fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
	
	header[12] = 0;
	header[4] = ' ';
	header[5] = ' ';
	header[6] = ' ';
	header[7] = ' ';
	
	if(strcmp(header, "RIFF    WAVE") == 0){
		soundData.sourceFmt = SRC_WAV;

		//ok both .wav / .ima detected properly here
	}
	
	//fatfs_close(soundData.filePointerStructFD->cur_entry.d_ino); //can't close yet, because the decoder may rely on an existing struct fd* audio stream context, or not. This is decided later.
	return soundData.sourceFmt;
}

#endif