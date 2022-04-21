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

#ifndef __microphoneShared_h__
#define __microphoneShared_h__

#include "typedefsTGDS.h"
#include "dsregs.h"

#ifdef ARM7
#include "dsregs.h"
#include "ipcfifoTGDS.h"
#include "soundTGDS.h"

#define SIO_DEVICE_POWER  (0 << 8)
#define SIO_DEVICE_TOUCH  (2 << 8)
#define SIO_BAUDRATE_2Mhz 1
#define SIO_BAUDRATE_1Mhz 2
#define SIO_CONTINUOUS    (1<<11)
//#define PM_AMP_GAIN       3	//PM_GAIN_OFFSET
//#define PM_AMP_OFFSET     2 //PM_AMP_OFFSET
#define GAIN_20           0
#define GAIN_40           1
#define GAIN_80           2
#define GAIN_160          3
#endif

#ifdef ARM9
#define SR_RECORDING 0
#define SR_PLAYBACK 1

#define REC_FREQ 11025
#define REC_BLOCK_SIZE 1024
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM7

extern void TurnOnMicrophone();
extern void TurnOffMicrophone();
extern u8 MIC_GetData8();
extern u16 MIC_ReadData12();
extern void PM_SetGain(u8 control);
extern void micInterrupt();
extern void micStartRecording();
extern void micStopRecording();

#endif

#ifdef ARM9

extern void updateRecordingCount();
extern int getWavLength(char *fName);
extern void copyChunk();
extern void startRecording(char * fileName);
extern void endRecording();
extern bool isRecording;

#endif

#ifdef __cplusplus
}
#endif