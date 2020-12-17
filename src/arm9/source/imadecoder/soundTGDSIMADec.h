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

#ifndef __soundTGDSUser_h__
#define __soundTGDSUser_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "ipcfifoTGDS.h"
#include "soundTGDS.h"
#include "ima_adpcm.h"

#endif


#ifdef __cplusplus
extern IMA_Adpcm_Player player;
extern "C" {
#endif

extern void SendArm7Command(u32 command, u32 data);
extern void swapData();

extern bool player_loop;
extern void soundPauseStart();

extern void IMAADPCMDecode();

#ifdef __cplusplus
}
#endif

