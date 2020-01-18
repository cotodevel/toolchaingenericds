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
#include "microphone9.h"
#include "soundplayerShared.h"
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

void updateRecordingCount()
{
	if(isRecording)
	{
		ticCount++;
	}
}

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
		
		fseek(fp, (headerChunk.chunkSize + 8) + 12, SEEK_SET); // seek to next chunk
		
		char fmtHeader[5];
		
		fread(fmtHeader, 1, 4, fp);
		fmtHeader[4] = 0;
		
		if(strcmp(fmtHeader, "data") != 0)
		{
			// wrong chunk next, sorry, doing strict only
			
			fclose(fp);
			return -1;
		}
		
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
		
		fread(&len, 1, 4, fp);	
		fclose(fp);
		len /= ((headerChunk.wBitsPerSample >> 3) * headerChunk.wChannels * headerChunk.dwSamplesPerSec);
	}	
	
	return (int)len;
}

void copyChunk()
{
	if(!isRecording)
		return;
	
	struct sSoundPlayerStruct * sndPlayerCtx = soundIPC();
	u8 *micData = NULL;
	
	switch(sndPlayerCtx->channels)
	{
		case 0:
			micData = (u8 *)sndPlayerCtx->arm9L;
			break;
		case 1:
			micData = (u8 *)sndPlayerCtx->arm9R;
			break;
		case 2:
			micData = (u8 *)sndPlayerCtx->interlaced;
			break;
	}
	
	fwrite(micData, 1, REC_BLOCK_SIZE, recFile);
}

void startRecording(char * fileName)
{
	if(isRecording)
		return;
	
	struct sSoundPlayerStruct * sndPlayerCtx = soundIPC();
	
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
	
	sndPlayerCtx->arm9L = (s16 *)malloc(REC_BLOCK_SIZE);
	sndPlayerCtx->arm9R = (s16 *)malloc(REC_BLOCK_SIZE);
	sndPlayerCtx->interlaced = (s16 *)malloc(REC_BLOCK_SIZE);

	memset(sndPlayerCtx->arm9L, 0, REC_BLOCK_SIZE);
	memset(sndPlayerCtx->arm9R, 0, REC_BLOCK_SIZE);
	memset(sndPlayerCtx->interlaced, 0, REC_BLOCK_SIZE);
	
	setSoundFrequency(REC_FREQ);	
	setSoundLength(REC_BLOCK_SIZE);
	SendFIFOWords(ARM7COMMAND_START_RECORDING, 0);
	
	ticCount = 0;
	isRecording = true;
}

void endRecording()
{
	if(!isRecording)
		return;
	
	struct sSoundPlayerStruct * sndPlayerCtx = soundIPC();
	SendFIFOWords(ARM7COMMAND_STOP_RECORDING, 0);
	isRecording = false;
	
	free(sndPlayerCtx->arm9L);
	free(sndPlayerCtx->arm9R);
	free(sndPlayerCtx->interlaced);

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