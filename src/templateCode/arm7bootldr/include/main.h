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

#ifndef __main7_h__
#define __main7_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "pff.h"
#include "soundTGDS.h"
#include "exceptionTGDS.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern int main(int argc, char **argv);
extern FATFS fileHandle;					// Petit-FatFs work area 
extern char fname[256];
extern u8 NDSHeaderStruct[4096];
extern char debugBuf7[256];
extern bool stopSoundStreamUser();
extern void bootfile();
extern int isNTROrTWLBinaryTGDSMB7(FATFS * currentFH);

#ifdef __cplusplus
}
#endif

