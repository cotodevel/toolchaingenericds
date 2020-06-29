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

#ifndef __spicnt_h
#define __spicnt_h

#include "typedefsTGDS.h"
#include "dsregs.h"

//40001C0: 

#define BIT_SPICNT_IRQ (1<<14) //#define SPI_IRQ     (1<<14)	

#define BIT_SPICNT_ENABLE (1<<15) //#define SPI_ENABLE  (1<<15)	
#define BIT_SPICNT_BUSY (1<<7) //#define SPI_BUSY 	(1<<7) 	

#define BIT_SPICLK_4MHZ		0 	//#define SPI_BAUD_4MHz    0
#define BIT_SPICLK_2MHZ		1 	//#define SPI_BAUD_2MHz    1
#define BIT_SPICLK_1MHZ		2	//#define SPI_BAUD_1MHz    2
#define BIT_SPICLK_512KHZ		3	//#define SPI_BAUD_512KHz  3

#define BIT_SPICNT_BYTETRANSFER   (0<<10)	//#define SPI_BYTE_MODE   (0<<10)
#define BIT_SPICNT_HALFWORDTRANSFER   (1<<10)	//#define SPI_HWORD_MODE  (1<<10)

//8-9   Device Select       (0=Powerman., 1=Firmware, 2=Touchscr, 3=Reserved)
#define BIT_SPICNT_POWERCNT		(0 << 8)	//#define SPI_DEVICE_POWER      (0 << 8)
#define BIT_SPICNT_FWCNT		(1 << 8)	//#define SPI_DEVICE_FIRMWARE   (1 << 8)
#define BIT_SPICNT_NVRAMCNT		(1 << 8)	//#define SPI_DEVICE_NVRAM      (1 << 8)
#define BIT_SPICNT_TSCCNT		(2 << 8)	//#define SPI_DEVICE_TOUCH      (2 << 8)
#define BIT_SPICNT_MICCNT		(2 << 8)	//#define SPI_DEVICE_MICROPHONE (2 << 8)
 
// When used, the /CS line will stay low after the transfer ends
// i.e. when we're part of a continuous transfer
#define BIT_SPICNT_CSHOLDENABLE		(1 << 11)	//#define SPI_CONTINUOUS       (1<<11)

#define		REG_SPI_CR		(*((uint16 volatile *) 0x040001C0))	//#define 	REG_SPICNT   (*(vuint16*)0x040001C0)
#define		REG_SPI_DATA	(*((uint16 volatile *) 0x040001C2))	//#define 	REG_SPIDATA   (*(vuint16*)0x040001C2)

#define		BITMASK_SPI_DATA	0xff

//Card SPI
#define CARD_COMMAND   ((vu8*)0x040001A8)
#define REG_ROMCTRL		(*(vu32*)0x040001A4)
#define REG_AUXSPICNT	(*(vu16*)0x040001A0)
#define REG_AUXSPICNTH	(*(vu8*)0x040001A1)
#define REG_AUXSPIDATA	(*(vu8*)0x040001A2)

#define CARD_DATA_RD   (*(vu32*)0x04100010)

#define CARD_1B0       (*(vu32*)0x040001B0)
#define CARD_1B4       (*(vu32*)0x040001B4)
#define CARD_1B8       (*(vu16*)0x040001B8)
#define CARD_1BA       (*(vu16*)0x040001BA)


#define CARD_CR1_ENABLE  0x80  // in byte 1, i.e. 0x8000
#define CARD_CR1_IRQ     0x40  // in byte 1, i.e. 0x4000

// 3 bits in b10..b8 indicate something
// read bits
#define CARD_BUSY         (1<<31)           // when reading, still expecting incomming data?
#define CARD_DATA_READY   (1<<23)           // when reading, CARD_DATA_RD or CARD_DATA has another word of data and is good to go

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void SPIWAITCNT();
extern uint8 RWSPICNT(uint8 val);
extern void SPICSHIGH();
#ifdef __cplusplus
}
#endif