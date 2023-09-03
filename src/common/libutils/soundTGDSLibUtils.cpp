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
#include "posixHandleTGDS.h"
#include "InterruptsARMCores_h.h"
#include "debugNocash.h"

#ifdef ARM9
#include "utilsTGDS.h"
#include "fatfslayerTGDS.h"
#include "videoTGDS.h"
#include "ima_adpcm.h"
#include "biosTGDS.h"
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
void mallocData(int size) 
{
    // this no longer uses malloc due to using vram bank d.
	strpcmL0 = VRAM_D;
	strpcmL1 = strpcmL0 + (size >> 1);
	strpcmR0 = strpcmL1 + (size >> 1);
	strpcmR1 = strpcmR0 + (size >> 1);
	
	// clear vram d bank to not have sound leftover
	int i = 0;
	
	for(i=0;i<(size);++i)
	{
		strpcmL0[i] = 0;
	}
	
	for(i=0;i<(size);++i)
	{
		strpcmR0[i] = 0;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void freeData()  
{	
	//free(strpcmR0);
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
void setupSound() {
	//Init SoundSampleContext
	initSound();

	sndCursor = 0;
	if(multRate != 1 && multRate != 2 && multRate != 4){
		multRate = 1;
	}
	
	mallocData(sampleLen * 2 * multRate);
    
	TIMERXDATA(0) = SOUND_FREQ((sndRate * multRate));
	TIMERXCNT(0) = TIMER_DIV_1 | TIMER_ENABLE;
  
	TIMERXDATA(1) = 0x10000 - (sampleLen * 2 * multRate);
	TIMERXCNT(1) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
	
	//irqSet(IRQ_TIMER1, TIMER1Handler);
	int ch;
	
	for(ch=0;ch<4;++ch)
	{
		SCHANNEL_CR(ch) = 0;
		SCHANNEL_TIMER(ch) = SOUND_FREQ((sndRate * multRate));
		SCHANNEL_LENGTH(ch) = (sampleLen * multRate) >> 1;
		SCHANNEL_REPEAT_POINT(ch) = 0;
	}

	//irqSet(IRQ_VBLANK, 0);
	//irqDisable(IRQ_VBLANK);
	REG_IE&=~IRQ_VBLANK;
	REG_IE |= IRQ_TIMER1;
	
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
	//irqSet(IRQ_TIMER1, 0);
	TIMERXCNT(0) = 0;
	TIMERXCNT(1) = 0;
	
	SCHANNEL_CR(0) = 0;
	SCHANNEL_CR(1) = 0;
	SCHANNEL_CR(2) = 0;
	SCHANNEL_CR(3) = 0;
	
	freeData();
	//irqSet(IRQ_VBLANK, VblankHandler);
	//irqEnable(IRQ_VBLANK);
	REG_IE|=IRQ_VBLANK;
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
bool canSend = false;

__attribute__((section(".dtcm")))
int bufCursor;

__attribute__((section(".dtcm")))
int bytesLeft = 0;

__attribute__((section(".dtcm")))
s16 *bytesLeftBuf = NULL;

__attribute__((section(".dtcm")))
int maxBytes = 0;

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

// wav

__attribute__((section(".dtcm")))
char *memoryContents = NULL;

__attribute__((section(".dtcm")))
u32 memoryPos = 0;

__attribute__((section(".dtcm")))
u32 memorySize = 0;

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

void freeData()
{
	lBuffer = NULL;
	rBuffer = NULL;
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
	if(!updateRequested)
	{
		// exit if nothing is needed
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
	
	//checkKeys();
	
	switch(soundData.sourceFmt)
	{
		case SRC_WAV:
		{
			swapAndSend(ARM7COMMAND_SOUND_COPY);
			wavDecode();
			
			soundData.loc = ftell(soundData.filePointer) - soundData.dataOffset;
		}
		break;
		default:{
			updateStreamCustomDecoder(soundData.sourceFmt);	//Custom decoder
		}
		break;
	}
}

void setSoundInterrupt()
{
	//irqSet(IRQ_FIFO_NOT_EMPTY, FIFO_Receive);
	//irqEnable(IRQ_FIFO_NOT_EMPTY);

	//REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR | IPC_FIFO_RECV_IRQ;
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
	setVolume(4);	//Default volume
}

void setSoundFrequency(u32 freq)
{
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[60], (uint32)freq);
	SendFIFOWords(ARM7COMMAND_SOUND_SETRATE, 0xFF);
}

void setSoundInterpolation(u32 mult)
{
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[62], (uint32)mult);
	SendFIFOWords(ARM7COMMAND_SOUND_SETMULT, 0xFF);
}

void setSoundLength(u32 len)
{
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[61], (uint32)len);
	SendFIFOWords(ARM7COMMAND_SOUND_SETLEN, 0xFF);
	sndLen = len;
}

int getSoundLength()
{
	return sndLen;
}

void startSound9()
{	
	if(!playing)
		SendFIFOWords(ARM7COMMAND_START_SOUND, 0xFF);
	playing = true;
}

void stopSound()
{
	if(playing)
		SendFIFOWords(ARM7COMMAND_STOP_SOUND, 0xFF);
	playing = false;
}

void closeSound(){
	freeSound();
	closeSoundUser();
	soundLoaded = false;
}

void freeSound()
{
	switch(internalCodecType)
	{
		case SRC_WAV:{
			
		}
		break;
		case SRC_WAVADPCM:{
			player.stop();
		}
		break;
		
		default:{
			freeSoundCustomDecoder(soundData.sourceFmt);
		}
		break;
	}
	
	stopSound();
	freeData();
	
	if(soundData.filePointer != NULL){
		fclose(soundData.filePointer);
	}
	
	soundData.filePointer = NULL;
	soundData.sourceFmt = SRC_NONE;	
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int getVolume() 
{
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 
	coherent_user_range_by_size((uint32)&TGDSIPC->soundIPC, sizeof(TGDSIPC->soundIPC));
	return TGDSIPC->soundIPC.volume;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setVolume(int volume) 
{
	if(volume > 8){
		volume = 8;
	}
	if(volume < 1){
		volume = 1;
	}
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 	
	coherent_user_range_by_size((uint32)&TGDSIPC->soundIPC, sizeof(TGDSIPC->soundIPC));
	TGDSIPC->soundIPC.volume = volume;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void volumeUp(int x, int y) 
{
	setVolume(getVolume() + 1);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void volumeDown(int x, int y) 
{
	setVolume(getVolume() - 1);
}

void setWavDecodeCallback(void (*cb)()){
	wavDecode = cb;
}

u32 getWavData(void *outLoc, int amount, FILE *fh)
{
	return fread(outLoc, 1, amount, fh);
}

void wavDecode8Bit()
{
	// 8bit wav file
	
	u8 *s8Data = (u8 *)TGDSARM9Malloc(WAV_READ_SIZE * soundData.channels);
	int rSize = getWavData(s8Data, (WAV_READ_SIZE * soundData.channels), soundData.filePointer);
	
	if(rSize < (WAV_READ_SIZE * soundData.channels))
	{
		cutOff = true;
		memset(s8Data + rSize, 0, (WAV_READ_SIZE * soundData.channels) - rSize);
	}
	
	//checkKeys();
	
	u8 *lData = (u8 *)lBuffer + 1;
	u8 *rData = (u8 *)rBuffer + 1;
	
	if(soundData.channels == 2)
	{
		uint i=0;
		for(i=0;i<(WAV_READ_SIZE << 1);i+=2)
		{
			lData[i] = (s8Data[i] - 128);
			rData[i] = (s8Data[i + 1] - 128);
		}
	}
	else
	{
		uint i=0,j=0;
		for(i=0,j=0;i<(WAV_READ_SIZE << 1);i+=2,++j)
		{
			lData[i] = (s8Data[j] - 128);
			rData[i] = lData[i];
		}
	}
	
	//checkKeys();
	
	TGDSARM9Free(s8Data);
}

void wavDecode16Bit()
{
	// 16bit wav file
	
	s16 *tmpData = (s16 *)TGDSARM9Malloc(WAV_READ_SIZE * 2 * soundData.channels);
	int rSize = getWavData(tmpData, (WAV_READ_SIZE * soundData.channels * 2), soundData.filePointer);
	
	if(rSize < (WAV_READ_SIZE * soundData.channels * 2))
	{
		cutOff = true;
		memset(tmpData + (rSize >> 1), 0, (WAV_READ_SIZE * soundData.channels * 2) - rSize);
	}
	
	//checkKeys();
	
	if(soundData.channels == 2)
	{
		uint i=0;
		for(i=0;i<WAV_READ_SIZE;++i)
		{					
			lBuffer[i] = tmpData[i << 1];
			rBuffer[i] = tmpData[(i << 1) | 1];
		}
	}
	else
	{
		uint i=0;
		for(i=0;i<WAV_READ_SIZE;++i)
		{
			lBuffer[i] = tmpData[i];
			rBuffer[i] = tmpData[i];
		}
	}
	
	//checkKeys();
	
	TGDSARM9Free(tmpData);
}

void wavDecode24Bit()
{
	// 24bit wav file
	
	u8 *tmpData = (u8 *)TGDSARM9Malloc(WAV_READ_SIZE * soundData.channels * 3);
	int rSize = getWavData(tmpData, (WAV_READ_SIZE * soundData.channels * 3), soundData.filePointer);	
	
	if(rSize < (WAV_READ_SIZE * soundData.channels * 3))
	{
		cutOff = true;
		memset(tmpData + rSize, 0, (WAV_READ_SIZE * soundData.channels * 3) - rSize);
	}
	
	//checkKeys();
	
	void *oldPointer = tmpData;
	
	u16 tmpVal;
	s16 *tmpSound = (s16 *)&tmpVal;
	
	if(soundData.channels == 2)
	{
		uint i=0;
		for(i=0;i<WAV_READ_SIZE;++i)
		{					
			tmpVal = (tmpData[1]) | ((tmpData[2]) << 8);
			lBuffer[i] = *tmpSound;
			tmpData+=3;
			
			tmpVal = (tmpData[1]) | ((tmpData[2]) << 8);
			rBuffer[i] = *tmpSound;
			tmpData+=3;
		}
	}
	else
	{
		uint i=0;
		for(i=0;i<WAV_READ_SIZE;++i)
		{	
			tmpVal = (tmpData[1]) | ((tmpData[2]) << 8);
			
			lBuffer[i] = *tmpSound;
			rBuffer[i] = *tmpSound;
			
			tmpData+=3;
		}
	}
	
	//checkKeys();
	
	TGDSARM9Free(oldPointer);
}

void wavDecode32Bit()
{
	// 32bit wav file
	
	s16 *tmpData = (s16 *)TGDSARM9Malloc(WAV_READ_SIZE * 4 * soundData.channels);	
	int rSize = getWavData(tmpData, (WAV_READ_SIZE * soundData.channels * 4), soundData.filePointer);
	
	if(rSize < (WAV_READ_SIZE * soundData.channels * 4))
	{
		cutOff = true;
		memset(tmpData + (rSize >> 1), 0, (WAV_READ_SIZE * soundData.channels * 4) - rSize);
	}
	
	//checkKeys();
	
	if(soundData.channels == 2)
	{
		uint i=0;
		for(i=0;i<WAV_READ_SIZE;++i)
		{					
			lBuffer[i] = tmpData[(i << 2) + 1];
			rBuffer[i] = tmpData[(i << 2) + 3];
		}
	}
	else
	{
		uint i=0;
		for(i=0;i<WAV_READ_SIZE;++i)
		{
			lBuffer[i] = tmpData[(i << 1) + 1];
			rBuffer[i] = lBuffer[i];
		}
	}
	
	//checkKeys();
	
	TGDSARM9Free(tmpData);
}

//WAV Header: Searches for u32chunkToSeek and returns file offset if found.
int parseWaveData(struct fd * fdinst, u32 u32chunkToSeek){
    u32 bytes[4];
	int fileOffset = 0;
	
	// Read first 4 bytes.
	// (Should be RIFF descriptor.)
	//if (fread((u8*)&bytes[0], 1, 4, fh) < 0) {
	//	return -1;
	//}
	int read = fatfs_read(fdinst->cur_entry.d_ino, (u8*)&bytes[0], 4); //fread(&len, 1, sizeof(len), fp);
	if(read < 0){
		return -1;
	}
	fdinst->loc += (read); fatfs_lseek(fdinst->cur_entry.d_ino, fdinst->loc, SEEK_SET);
	
	fileOffset+=4;
	
	//If we looking for "data" skip other headers
	if((u32)u32chunkToSeek == (u32)0x64617461){
		// First subchunk will always be at byte 12.
		// (There is no other dependable constant.)
		fdinst->loc += (12); fatfs_lseek(fdinst->cur_entry.d_ino, fdinst->loc, SEEK_SET); //fseek(fh, 12, SEEK_CUR);
		fileOffset+=12;
	}
	
	for (;;) {
		// Read chunk length.
		//if (fread((u8*)&bytes[0], 1, sizeof(bytes), fh) < 0) {
		//	return -1;
		//}
		read = fatfs_read(fdinst->cur_entry.d_ino, (u8*)&bytes[0], sizeof(bytes)); //fread(&len, 1, sizeof(len), fp);
		if(read < 0){
			return -1;
		}
		fdinst->loc += (read); fatfs_lseek(fdinst->cur_entry.d_ino, fdinst->loc, SEEK_SET);
		
		//Way over the header area, exit
		if(fileOffset > 96){
			return -2;
		}
		
		u8 bytesNonAligned[16];
		memcpy(bytesNonAligned, (u8*)&bytes[0], sizeof(bytesNonAligned));
		
		u8 lastVar0 = 0;
		u8 lastVar1 = 0;
		u8 lastVar2 = 0;
		u8 lastVar3 = 0;
		//int match = 0;
		int j = 0;
		for(j=0; j < sizeof(bytesNonAligned); j++){
			if(j>0){
				lastVar0 = bytesNonAligned[j-1];
			}
			if(j>1){
				lastVar1 = bytesNonAligned[j-2];
			}
			if(j>2){
				lastVar2 = bytesNonAligned[j-3];
			}
			if(j>3){
				lastVar3 = bytesNonAligned[j-4];
			}
			u32 read = (u32)((lastVar3<<24) | (lastVar2<<16) | (lastVar1<<8) | (lastVar0<<0));
			if(read == u32chunkToSeek){
				return (fileOffset+j);
			}
		}
		fileOffset+=16;
	}
	return -1;
}

//Usercode: Opens a .WAV or IMA-ADPCM (Intel) file and begins to stream it. Copies the file handles 
//Returns: the stream format.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int playSoundStream(char * audioStreamFilename, struct fd * _FileHandleVideo, struct fd * _FileHandleAudio) {
	
	//If trying to play SoundStream code while Shared RAM is mapped @ ARM7, throw error
	if((WRAM_CR & WRAM_0KARM9_32KARM7) == WRAM_0KARM9_32KARM7){
		loggerARM9LibUtilsCallback("TGDS:playSoundStream(): Trying to play a Sound Stream ");
		loggerARM9LibUtilsCallback("but SharedWRAM is used by ARM7. Aborting.");
		return SRC_NONE;
	}
	
	int physFh1 = -1;
	int physFh2 = -1;
	if(openDualTGDSFileHandleFromFile(audioStreamFilename, &physFh1, &physFh2) == true){
		//OK
		_FileHandleVideo = getStructFD(physFh1);
		_FileHandleAudio = getStructFD(physFh2);
		int intCodecType = initSoundStreamFromStructFD(_FileHandleAudio, (char*)".wav");
		if(intCodecType == SRC_WAVADPCM){
			bool loop_audio = false;
			bool automatic_updates = false;
			if(player.play(getPosixFileHandleByStructFD(_FileHandleAudio, "r"), loop_audio, automatic_updates, ADPCM_SIZE, stopSoundStreamUser) == 0){
				//ADPCM Playback!
			}
		}
		else if(intCodecType == SRC_WAV){
			//WAV Playback!
		}
		return intCodecType;
	}
	return SRC_NONE;
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
	closeDualTGDSFileHandleFromFile(tgdsStructFD1, tgdsStructFD2);
	swiDelay(1);
	return true;
}

//Internal: Opens a .WAV file (or returns the detected header), otherwise returns SRC_NONE
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int initSoundStream(char * audioStreamFilename) {
	char tmpName[256];
	char ext[256];
	
	strcpy(tmpName, audioStreamFilename);	
	separateExtension(tmpName, ext);
	strlwr(ext);
	
	freeSound();	
	cutOff = false;
	sndPaused = false;
	playing = false;
	seekUpdate = -1;
	
	FILE *fp = fopen(audioStreamFilename, "r");
	int StructFD = fileno(fp);
	struct fd *tgdsfd = getStructFD(StructFD);
	return initSoundStreamFromStructFD(tgdsfd, ext);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int initSoundStreamFromStructFD(struct fd * _FileHandleAudio, char * ext) {	//ARM9 Impl.
	// try this first to prevent false positives with other streams
	/*
	if(isURL(fName))
	{	
		//it's a url
		if(!urlFromString(fName, &curSite))
		{
			return false;
		}
		
		strcpy(tmpName, curSite.remotePath);
		separateExtension(tmpName, ext);
		strlwr(ext);
		
		if(strcmp(ext, ".ogg") == 0)
			soundData.sourceFmt = SRC_STREAM_OGG;
		else if(strcmp(ext, ".aac") == 0)
			soundData.sourceFmt = SRC_STREAM_AAC;		
		else
			soundData.sourceFmt = SRC_STREAM_MP3;
		
		soundData.bufLoc = 0;
		
		if(isWIFIConnected())
		{
			disconnectWifi();
		}
		
		streamMode = STREAM_DISCONNECTED;
		streamOpened = false;
		mp3Buf = NULL;
		
		freeStreamBuffer();
		
		tmpMeta = (char *)trackMalloc(4081, "metadata");
		memset(&curIcy, 0, sizeof(ICY_HEADER));	
		memset(tmpMeta, 0, 4081);
		
		return true;
	}
	*/
	
	initComplexSound(); // initialize sound variables
	soundData.filePointerStructFD = _FileHandleAudio;	//Global StructDS handle -> WAV stream
	if(strcmp(ext,".wav") == 0){
		// wav file!
		soundData.sourceFmt = SRC_WAV;
		soundData.bufLoc = 0;
		
		wavFormatChunk headerChunk;
		char header[13];
		soundData.filePointerStructFD->loc = 0; fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
		int read = fatfs_read(soundData.filePointerStructFD->cur_entry.d_ino, (u8*)&header[0], 12);
		soundData.filePointerStructFD->loc += (read); fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
		
		header[12] = 0;
		header[4] = ' ';
		header[5] = ' ';
		header[6] = ' ';
		header[7] = ' ';
		
		if(strcmp(header, "RIFF    WAVE") != 0)
		{
			// Wrong header
			fatfs_close(soundData.filePointerStructFD->cur_entry.d_ino);
			return SRC_NONE;
		}
		
		//fread((char*)&headerChunk, 1, sizeof(wavFormatChunk), fp);
		read = fatfs_read(soundData.filePointerStructFD->cur_entry.d_ino, (u8*)&headerChunk, sizeof(wavFormatChunk)); 
		soundData.filePointerStructFD->loc += (read); fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
		
		if(strncmp((char*)&headerChunk.chunkID[0], "fmt ", 4) != 0)	
		{
			// Wrong chunk at beginning
			fatfs_close(soundData.filePointerStructFD->cur_entry.d_ino);
			return SRC_NONE;
		}
		
		//RAW PCM
		if(headerChunk.wFormatTag == WAVE_FORMAT_RAW_PCM)
		{
			if(headerChunk.wChannels > 2)
			{
				// more than 2 channels.... uh no!
				fatfs_close(soundData.filePointerStructFD->cur_entry.d_ino);
				return SRC_NONE;
			}
			else
			{
				soundData.channels = headerChunk.wChannels;
			}
			
			if(headerChunk.wBitsPerSample <= 8)
			{
				soundData.bits = 8;
				wavDecode = wavDecode8Bit;
			}
			else if(headerChunk.wBitsPerSample <= 16)
			{
				soundData.bits = 16;
				wavDecode = wavDecode16Bit;
			}
			else if(headerChunk.wBitsPerSample <= 24)
			{
				soundData.bits = 24;
				wavDecode = wavDecode24Bit;
			}
			else if(headerChunk.wBitsPerSample <= 32)
			{
				soundData.bits = 32;
				wavDecode = wavDecode32Bit;
			}
			else
			{
				// more than 32bit sound, not supported
				fatfs_close(soundData.filePointerStructFD->cur_entry.d_ino);
				return SRC_NONE;	
			}
			
			//rewind
			soundData.filePointerStructFD->loc = 0; fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
			int wavStartOffset = parseWaveData(soundData.filePointerStructFD, (u32)(0x64617461));	//Seek for ASCII "data" and return 4 bytes after that: Waveform length (4 bytes), then 
																		//4 bytes after that the raw Waveform
			
			if(wavStartOffset == -1)
			{
				// wav block not found
				fatfs_close(soundData.filePointerStructFD->cur_entry.d_ino);
				return SRC_NONE;
			}
			
			soundData.filePointerStructFD->loc = 0;
			
			u32 len = 0;
			//data section not found, use filesize as size...
			if(wavStartOffset == -2)
			{
				len = FS_getFileSizeFromOpenStructFD(soundData.filePointerStructFD);
				wavStartOffset = 96;	//Assume header size: 96 bytes
			}
			else{
				soundData.filePointerStructFD->loc = wavStartOffset; fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
				read = fatfs_read(soundData.filePointerStructFD->cur_entry.d_ino, (u8*)&len, sizeof(len)); //fread(&len, 1, sizeof(len), fp);
				soundData.filePointerStructFD->loc += (read); fatfs_lseek(soundData.filePointerStructFD->cur_entry.d_ino, soundData.filePointerStructFD->loc, SEEK_SET);
				wavStartOffset+=4;
			}
			
			soundData.filePointerStructFD->loc = wavStartOffset;	//not really used, but kept anyway. WAV decoding reads from soundData.loc
			soundData.len = len;
			soundData.loc = 0;
			soundData.dataOffset = wavStartOffset;	
			soundData.filePointer = getPosixFileHandleByStructFD(soundData.filePointerStructFD, "r");
			bufCursor = 0;
			
			setSoundInterpolation(1);
			setSoundFrequency(headerChunk.dwSamplesPerSec);
			
			setSoundLength(WAV_READ_SIZE);		
			mallocData(WAV_READ_SIZE);
			
			memoryContents = NULL;
			wavDecode();
			startSound9();
			internalCodecType = SRC_WAV;
			return SRC_WAV;
		}
		if(headerChunk.wFormatTag == WAVE_FORMAT_IMA_ADPCM){
		
			//If ADPCM do not close the file handle because it was initialized earlier
			return SRC_WAVADPCM;
		}	
		fatfs_close(soundData.filePointerStructFD->cur_entry.d_ino);
	}
	
	return SRC_NONE;
}

#endif