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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "soundTGDS.h"
#include "consoleTGDS.h"
#include "soundTGDSIMADec.h"
#include "ima_adpcm.h"

__attribute__((section(".itcm")))
void IMAADPCMDecode(){
	s16 *tmpData = (s16 *)TGDSARM9Malloc(ADPCMchunksize * 2 * ADPCMchannels);	//ADPCM uses 1 src as decoding frame, and 2nd src as scratchpad
	if(active_player->i_stream_request(ADPCMchunksize, tmpData, WAV_FORMAT_IMA_ADPCM) != 1){
		coherent_user_range((uint32)tmpData, (uint32)(ADPCMchunksize* 2 * ADPCMchannels));
		if(soundData.channels == 2)
		{
			uint i=0;
			for(i=0;i<(ADPCMchunksize * 2);++i)
			{					
				lBuffer[i] = tmpData[i << 1];
				rBuffer[i] = tmpData[(i << 1) | 1];
			}
		}
		else
		{
			uint i=0;
			for(i=0;i<(ADPCMchunksize * 2);++i)
			{
				lBuffer[i] = tmpData[i];
				rBuffer[i] = tmpData[i];
			}
		}
	}	
	TGDSARM9Free(tmpData);
}
