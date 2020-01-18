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
#include "microphone7.h"
#include "soundplayerShared.h"
#include "spiTGDS.h"
#include "spifwTGDS.h"
#include "soundTGDS.h"
#include "timerTGDS.h"
#include "ipcfifoTGDS.h"

/*
   microphone code based on neimod's microphone example.
   See: http://neimod.com/dstek/ 
   Chris Double (chris.double@double.co.nz)
   http://www.double.co.nz/nintendo_ds
*/

u32 micBufLoc = 0;
u32 sndCursor = 0;
u32 sampleLen = 0;
int multRate = 1;
int sndRate = 0;

void PM_SetAmp(u8 control)
{
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz | SIO_CONTINUOUS;
	REG_SPI_DATA = PM_AMP_OFFSET;
	
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz;
	REG_SPI_DATA = control;
}

void TurnOnMicrophone()
{
	PM_SetAmp(PM_AMP_ON);
}

void TurnOffMicrophone()
{
	PM_SetAmp(PM_AMP_OFF);
}

void PM_SetGain(u8 control)
{
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz | SIO_CONTINUOUS;
	REG_SPI_DATA = PM_GAIN_OFFSET;
	
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | SIO_DEVICE_POWER | SIO_BAUDRATE_1Mhz;
	REG_SPI_DATA = control;
}

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

void micInterrupt()
{
	struct sSoundPlayerStruct * sndPlayerCtx = soundIPC();
	
#ifdef MIC_8
	u8 *micData = NULL;
	
	switch(sndCursor)
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
		sndPlayerCtx->channels = sndCursor;
		++sndCursor;
		if(sndCursor > 2)
			sndCursor = 0;
		
		SendFIFOWords(TGDS_SAVE_MIC_DATA, 0);	// send command to save the buffer
	}
#endif
#ifdef MIC_16
	s16 *micData = NULL;
	
	switch(sndCursor)
	{
		case 0:
			micData = (s16 *)sndPlayerCtx->arm9L;
			break;
		case 1:
			micData = (s16 *)sndPlayerCtx->arm9R;
			break;
		case 2:
			micData = (s16 *)sndPlayerCtx->interlaced;
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
		
		SendFIFOWords(TGDS_SAVE_MIC_DATA, 0);	// send command to save the buffer
	}
#endif
}

void micStartRecording()
{
	micBufLoc = 0;
	sndCursor = 0;
	
	TurnOnMicrophone();
	PM_SetGain(GAIN_160);
	
	EnableIrq(IRQ_TIMER2);
	TIMERXDATA(2) = TIMER_FREQ(sndRate);
	TIMERXCNT(2) = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;	// microphone stuff -> micInterrupt(); handler in Interrupts_app7.c: Enabled and setup 
}

void micStopRecording()
{
	DisableIrq(IRQ_TIMER2); // microphone stuff	
	TIMERXCNT(2) = 0;	// microphone stuff -> micInterrupt();
	TurnOffMicrophone();
}