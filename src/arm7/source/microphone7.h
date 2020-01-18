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
 
#ifndef __microphone7_h__
#define __microphone7_h__

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

#ifdef __cplusplus
extern "C" {
#endif

extern void TurnOnMicrophone();
extern void TurnOffMicrophone();
extern u8 MIC_GetData8();
extern u16 MIC_ReadData12();
extern void PM_SetGain(u8 control);
extern void micInterrupt();
extern void micStartRecording();
extern void micStopRecording();

extern u32 micBufLoc;
extern u32 sndCursor;
extern u32 sampleLen;
extern int multRate;
extern int sndRate;

#ifdef __cplusplus
}
#endif
