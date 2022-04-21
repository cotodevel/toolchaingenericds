#include "microphoneShared.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "libutilsShared.h"

/////////////////////////////////////////////////////////////////// ARM7 /////////////////////////////////////////////////////////////////// 

#ifdef ARM7
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
 
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "spiTGDS.h"
#include "spifwTGDS.h"
#include "soundTGDS.h"
#include "timerTGDS.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"

/*
   microphone code based on neimod's microphone example.
   See: http://neimod.com/dstek/ 
   Chris Double (chris.double@double.co.nz)
   http://www.double.co.nz/nintendo_ds
*/
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void PM_SetAmp(u8 control)
{
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz | SIO_CONTINUOUS;
	REG_SPI_DATA = PM_AMP_OFFSET;
	
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz;
	REG_SPI_DATA = control;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TurnOnMicrophone()
{
	PM_SetAmp(PM_AMP_ON);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TurnOffMicrophone()
{
	PM_SetAmp(PM_AMP_OFF);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void PM_SetGain(u8 control)
{
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz | SIO_CONTINUOUS;
	REG_SPI_DATA = PM_GAIN_OFFSET;
	
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz;
	REG_SPI_DATA = control;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u8 MIC_GetData8()
{
	static u16 result, result2;
	
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_TOUCH | SIO_BAUDRATE_2Mhz | SIO_CONTINUOUS;
	REG_SPI_DATA = 0xEC;  // Touchscreen command format for AUX
  
	SPIWAITCNT();
	
	REG_SPI_DATA = 0x00;
	
	SPIWAITCNT();
	
	result = REG_SPI_DATA;
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_TOUCH | SIO_BAUDRATE_2Mhz;
	REG_SPI_DATA = 0x00; 
	
	SPIWAITCNT();
	
	result2 = REG_SPI_DATA;
	
	return (((result & 0x7F) << 1) | ((result2>>7) & 1));
}

// based off of tob's samples
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u16 MIC_ReadData12()
{
	static u16 result, result2;

	SPIWAITCNT();

	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_TOUCH | BIT_SPICLK_2MHZ | BIT_SPICNT_CSHOLDENABLE;
	REG_SPI_DATA = 0xE4;  // Touchscreen command format for AUX, 12bit

	SPIWAITCNT();

	REG_SPI_DATA = 0x00;

	SPIWAITCNT();

	result = REG_SPI_DATA;
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_TOUCH | BIT_SPICLK_2MHZ;
	REG_SPI_DATA = 0x00;

	SPIWAITCNT();

	result2 = REG_SPI_DATA;

	return (((result & 0x7F) << 5) | ((result2>>3)&0x1F));
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void micInterrupt()
{
	SoundRegion * soundPlayerCtx = getSoundIPC();
	struct sIPCSharedTGDS * TGDSIPC = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
#ifdef MIC_8
	u8 *micData = NULL;
	
	switch(sndCursor)
	{
		case 0:
			micData = (u8 *)TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data;
			break;
		case 1:
			micData = (u8 *)TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data;
			break;
		case 2:
			micData = (u8 *)soundPlayerCtx->interlaced;
			break;
	}
	
	int tempX = (MIC_GetData8() - 128) * 4;
	
	if(tempX > 127)
		tempX = 127;
	if(tempX < -128)
		tempX = -128;
	
	tempX += 128;
	
	micData[micBufLoc] = (u8)tempX;
	
	++micBufLoc;
	if(micBufLoc == sampleLen)
	{
		micBufLoc = 0;
		soundPlayerCtx->channels = sndCursor;
		++sndCursor;
		if(sndCursor > 2)
			sndCursor = 0;
		
		SendFIFOWords(TGDS_SAVE_MIC_DATA, 0xFF);	// send command to save the buffer
	}
#endif
#ifdef MIC_16
	s16 *micData = NULL;
	
	switch(sndCursor)
	{
		case 0:
			micData = (s16 *)soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data;
			break;
		case 1:
			micData = (s16 *)soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data;
			break;
		case 2:
			micData = (s16 *)soundPlayerCtx->interlaced;
			break;
	}
	
	int tempX = (MIC_ReadData12() - 2048) << 6;

	if(tempX > 32767)
		tempX = 32767;
	if(tempX < -32768)
		tempX = -32768;
	
	micData[micBufLoc++] = tempX;
	
	if(micBufLoc == sampleLen)
	{
		micBufLoc = 0;
		TGDSIPC->channels = sndCursor;
		++sndCursor;
		if(sndCursor > 2)
			sndCursor = 0;
		
		SendFIFOWords(TGDS_SAVE_MIC_DATA, 0xFF);	// send command to save the buffer
	}
#endif
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void micStartRecording()
{
	micBufLoc = 0;
	sndCursor = 0;
	
	TurnOnMicrophone();
	PM_SetGain(GAIN_160);
	
	irqEnable(IRQ_TIMER2);
	TIMERXDATA(2) = TIMER_FREQ(sndRate);
	TIMERXCNT(2) = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;	// microphone stuff -> micInterrupt(); handler in Interrupts_app7.c: Enabled and setup 
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void micStopRecording()
{
	irqDisable(IRQ_TIMER2); // microphone stuff	
	TIMERXCNT(2) = 0;	// microphone stuff -> micInterrupt();
	TurnOffMicrophone();
}

#endif


/////////////////////////////////////////////////////////////////// ARM9 /////////////////////////////////////////////////////////////////// 
#ifdef ARM9

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
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "fatfslayerTGDS.h"
#include "ipcfifoTGDS.h"
#include "utilsTGDS.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static FILE *recFile = NULL;
bool isRecording = false;
static int recordMode = SR_RECORDING;
static int wavLength = 0;
static u32 ticCount = 0;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void updateRecordingCount()
{
	if(isRecording)
	{
		ticCount++;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int getWavLength(char *fName){
	wavFormatChunk headerChunk;	
	char header[13];
	u32 len = 0;
	
	FILE *fp = fopen(fName, "r");
	if(fp != NULL){
		fread(header, 1, 12, fp);
		
		header[12] = 0;
		header[4] = ' ';
		header[5] = ' ';
		header[6] = ' ';
		header[7] = ' ';
		
		if(strcmp(header, "RIFF    WAVE") != 0)
		{
			// wrong header
			
			fclose(fp);
			return -1;
		}
		
		fread(&headerChunk, 1, sizeof(wavFormatChunk), fp);
		
		if(strncmp(headerChunk.chunkID, "fmt ", 4) != 0)
		{
			// wrong chunk at beginning
			
			fclose(fp);
			return -1;
		}
		
		if(headerChunk.wFormatTag != 1)
		{
			// compression used, hell no to loading this
			
			fclose(fp);
			return -1;
		}
		
		if(headerChunk.wChannels > 2)
		{
			// more than 2 channels.... uh no!
			
			fclose(fp);
			return -1;
		}
		
		//rewind
		int structFD = fileno(fp);
		struct fd * fdinst=getStructFD(structFD);
		fseek(fp, 0, SEEK_SET);
		f_lseek(fdinst->filPtr, 0);
		int wavStartOffset = parseWaveData(fdinst, (u32)(0x64617461));	//Seek for ASCII "data" and return 4 bytes after that: Waveform length (4 bytes), then 
																	//4 bytes after that the raw Waveform
		
		if(wavStartOffset < 0)
		{
			// wav block not found
			fclose(fp);
			return false;
		}
		fseek(fp, wavStartOffset, SEEK_SET);
		u32 len = 0;
		fread(&len, 1, sizeof(len), fp);
		wavStartOffset+=4;
		
		if(headerChunk.wBitsPerSample <= 8)
		{
			headerChunk.wBitsPerSample = 8;
		}
		else if(headerChunk.wBitsPerSample <= 16)
		{
			headerChunk.wBitsPerSample = 16;
		}
		else if(headerChunk.wBitsPerSample <= 24)
		{
			headerChunk.wBitsPerSample = 24;
		}
		else if(headerChunk.wBitsPerSample <= 32)
		{
			headerChunk.wBitsPerSample = 32;
		}
			
		fclose(fp);
		len /= ((headerChunk.wBitsPerSample >> 3) * headerChunk.wChannels * headerChunk.dwSamplesPerSec);
	}	
	
	return (int)len;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void copyChunk()
{
	if(!isRecording)
		return;
	
	SoundRegion * soundPlayerCtx = getSoundIPC();
	struct sIPCSharedTGDS * TGDSIPC = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
	u8 *micData = NULL;
	
	switch(soundPlayerCtx->channels)
	{
		case 0:
			micData = (u8 *)TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data;
			break;
		case 1:
			micData = (u8 *)TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data;
			break;
		case 2:
			micData = (u8 *)soundPlayerCtx->interlaced;
			break;
	}
	
	fwrite(micData, 1, REC_BLOCK_SIZE, recFile);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void startRecording(char * fileName)
{
	if(isRecording)
		return;
	
	SoundRegion * soundPlayerCtx = getSoundIPC();
	struct sIPCSharedTGDS * TGDSIPC = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
	// load the wav file in question
	FILE *fp = fopen(fileName, "r");
	int fileSize = FS_getFileSizeFromOpenHandle(fp);
	if((fp == NULL) || (fileSize == 0))
	{
		// this file is new
		recordMode = SR_RECORDING;
		fclose(fp);
	}
	else
	{
		fclose(fp);
		wavLength = getWavLength(fileName);
		
		if(wavLength == -1)
		{
			recordMode = SR_RECORDING;
		}
		else
		{
			recordMode = SR_PLAYBACK;
		}
	}
	// load the wav file in question end
	
	recFile = fopen(fileName, "w+");
	
	// write header
	fwrite("RIFF    WAVE", 1, 12, recFile);
	
	// write fmt tag
	wavFormatChunk headerChunk;
	
	strncpy(headerChunk.chunkID, "fmt ", 4);
	headerChunk.chunkSize = 16;
	headerChunk.wFormatTag = 1;
	headerChunk.wChannels = 1;
	headerChunk.wBitsPerSample = 8;
	headerChunk.dwSamplesPerSec = REC_FREQ;
	headerChunk.wBlockAlign = 1;
	headerChunk.dwAvgBytesPerSec = REC_FREQ;
	
	fwrite(&headerChunk, 1, sizeof(wavFormatChunk), recFile);
	
	// write data chunk
	fwrite("data    ", 1, 8, recFile);
	
	TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data = (s16 *)malloc(REC_BLOCK_SIZE);
	TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data = (s16 *)malloc(REC_BLOCK_SIZE);
	soundPlayerCtx->interlaced = (s16 *)malloc(REC_BLOCK_SIZE);

	memset(TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data, 0, REC_BLOCK_SIZE);
	memset(TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data, 0, REC_BLOCK_SIZE);
	memset(soundPlayerCtx->interlaced, 0, REC_BLOCK_SIZE);
	
	setSoundFrequency(REC_FREQ);
	setSoundLength(REC_BLOCK_SIZE);
	
	SendFIFOWords(ARM7COMMAND_START_RECORDING, 0xFF);
	
	ticCount = 0;
	isRecording = true;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void endRecording()
{
	if(!isRecording)
		return;
	
	SoundRegion * soundPlayerCtx = getSoundIPC();
	struct sIPCSharedTGDS * TGDSIPC = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
	SendFIFOWords(ARM7COMMAND_STOP_RECORDING, 0xFF);
	isRecording = false;
	
	free(TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data);
	free(TGDSIPC->soundContextShared.soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data);
	free(soundPlayerCtx->interlaced);

	if(recFile != NULL){
		u32 length = FS_getFileSizeFromOpenHandle(recFile) - 8;
		fseek(recFile, 4, SEEK_SET);
		fwrite(&length, 1, sizeof(length), recFile);
		
		length = FS_getFileSizeFromOpenHandle(recFile) - 44;
		fseek(recFile, 40, SEEK_SET);
		fwrite(&length, 1, sizeof(length), recFile);
		
		fclose(recFile);
		recFile = NULL;
	}
}

/*
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void seekRecordSound(int xPos, int yPos)
{
	if(recordMode != SR_PLAYBACK)
		return;
	if(getState() == STATE_STOPPED || getState() == STATE_UNLOADED)
		return;
	
	u32 curSize;
	u32 maxSize;
	getSoundLoc(&curSize, &maxSize);
	
	float percent = (float)xPos / (float)234;
	u32 newLoc = (int)(maxSize * percent);
	
	setSoundLoc(newLoc);
}

char * fileName = "0:/somefilename.wav";
void switchRecordingMode(int x, int y)
{
	if(recordMode == SR_PLAYBACK)
	{
		switch(getState())
		{
			case STATE_UNLOADED:
			case STATE_STOPPED:
				break;
			default:
				closeSound();
		}
		
		recordMode = SR_RECORDING;
	}
	else if(recordMode == SR_RECORDING)
	{	
		if(isRecording)
		{
			endRecording();
		}
		
		recordMode = SR_PLAYBACK;		
		wavLength = getWavLength(fileName);
		
		if(wavLength == -1)
		{
			// Don't allow playback screen if invalid wav file
			recordMode = SR_RECORDING;
		}
	}
}
*/

#endif