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

#ifdef ARM7

#include "typedefsTGDS.h"
#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDS.h"
#include "linkerTGDS.h"
#include "CPUARMTGDS.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint32 globalVRAMStackStartPtr;
uint32 allocVRAMStacks(){
	globalVRAMStackStartPtr = (uint32)malloc(1024 * 8);	//(uint32)vramAlloc(vramBlockA,(uint32)0x06000000 + (32*1024) + (256),(1024 * 8));	//keep 0x06000000 ~ 32K region used by block D. Just use block A
	return globalVRAMStackStartPtr;
}


void deallocVRAMStacks(){
	free((uint32*)globalVRAMStackStartPtr);
}

#endif
