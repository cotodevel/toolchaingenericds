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

#ifndef __nds_ipc_h__
#define __nds_ipc_h__

#include "typedefs.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define SEND_FIFO_IPC_EMPTY	(1<<0)	
#define SEND_FIFO_IPC_FULL	(1<<1)	
#define SEND_FIFO_IPC_IRQ	(1<<2)		
#define SEND_FIFO_IPC_CLEAR	(1<<3)	
#define RECV_FIFO_IPC_EMPTY	(1<<8)	
#define RECV_FIFO_IPC_FULL	(1<<9)	
#define RECV_FIFO_IPC_IRQ	(1<<10)	

#define FIFO_IPC_ERROR	(1<<14)	
#define FIFO_IPC_ENABLE	(1<<15)

//fifo 
#define REG_IPC_FIFO_TX		(*(vuint32*)0x4000188)
#define REG_IPC_FIFO_RX		(*(vuint32*)0x4100000)
#define REG_IPC_FIFO_CR		(*(vuint16*)0x4000184)

//ipc fifo sync
#define REG_IPC_SYNC	(*(vuint16*)0x04000180)


#endif

#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
}
#endif