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

#ifndef __nds_bus_h__
#define __nds_bus_h__

#include "typedefs.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define NDSSLOTBUS_ACCESS_PRIO	(1<<11)	//>1 ARM7, 0 ARM9
#define GBASLOTBUS_ACCESS_PRIO	(1<<7)	//>1 ARM7, 0 ARM9
#define NDSSLOT_ARM9BUS	0
#define NDSSLOT_ARM7BUS	NDSSLOTBUS_ACCESS_PRIO
#define GBASLOT_ARM9BUS	0
#define GBASLOT_ARM7BUS	GBASLOTBUS_ACCESS_PRIO


#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void setCpuNdsBusAccessPrio(uint16 bus_assignment);
extern void setCpuGbaBusAccessPrio(uint16 bus_assignment);

#ifdef __cplusplus
}
#endif