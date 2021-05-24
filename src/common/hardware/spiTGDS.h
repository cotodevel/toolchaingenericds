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
#define CARD_1BA       (*(vu16*)0x040001BA)
#define SPI_IRQ     (1<<14)	

#define SPI_ENABLE  (1<<15)	
#define BIT_SPICNT_BUSY (1<<7) 
#define SPI_BUSY 	(1<<7) 	

#define BIT_SPICLK_4MHZ		0 	
#define SPI_BAUD_4MHz    0
#define BIT_SPICLK_2MHZ		1 	
#define SPI_BAUD_2MHz    1
#define BIT_SPICLK_1MHZ		2	
#define SPI_BAUD_1MHz    2
#define BIT_SPICLK_512KHZ		3	
#define SPI_BAUD_512KHz  3

#define BIT_SPICNT_BYTETRANSFER   (0<<10)	
#define SPI_BYTE_MODE   (0<<10)
#define BIT_SPICNT_HALFWORDTRANSFER   (1<<10)	
#define SPI_HWORD_MODE  (1<<10)

//8-9   Device Select       (0=Powerman., 1=Firmware, 2=Touchscr, 3=Reserved)
#define BIT_SPICNT_POWERCNT		(0 << 8)	
#define SPI_DEVICE_POWER      (0 << 8)
#define BIT_SPICNT_FWCNT		(1 << 8)	
#define SPI_DEVICE_FIRMWARE   (1 << 8)
#define BIT_SPICNT_NVRAMCNT		(1 << 8)	
#define SPI_DEVICE_NVRAM      (1 << 8)
#define BIT_SPICNT_TSCCNT		(2 << 8)	
#define SPI_DEVICE_TOUCH      (2 << 8)
#define BIT_SPICNT_MICCNT		(2 << 8)	
#define SPI_DEVICE_MICROPHONE (2 << 8)
 
// When used, the /CS line will stay low after the transfer ends
// i.e. when we're part of a continuous transfer
#define BIT_SPICNT_CSHOLDENABLE		(1 << 11)	
#define SPI_CONTINUOUS       (1<<11)

#define		REG_SPI_CR		(*((uint16 volatile *) 0x040001C0))	
#define 	REG_SPICNT   (*(vuint16*)0x040001C0)
#define		REG_SPI_DATA	(*((uint16 volatile *) 0x040001C2))	
#define 	REG_SPIDATA   (*(vuint16*)0x040001C2)
#define		BITMASK_SPI_DATA	0xff

//Card SPI
#define CARD_COMMAND   ((vu8*)0x040001A8)
#define REG_ROMCTRL		(*(vu32*)0x040001A4)
#define REG_CARD_COMMAND	((vu8*)0x040001A8)
#define	REG_CARD_DATA_RD	(*(vu32*)0x04100010)
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
#define CARD_ACTIVATE     (1<<31)           // when writing, get the ball rolling
#define CARD_nRESET       (1<<29)           // value on the /reset pin (1 = high out, not a reset state, 0 = low out = in reset)

#define CARD_SEC_LARGE    (1<<28)           // Use "other" secure area mode, which tranfers blocks of 0x1000 bytes at a time
#define CARD_CLK_SLOW     (1<<27)           // Transfer clock rate (0 = 6.7MHz, 1 = 4.2MHz)
#define CARD_BLK_SIZE(n)  (((n)&0x7)<<24)   // Transfer block size, (0 = None, 1..6 = (0x100 << n) bytes, 7 = 4 bytes)
#define CARD_SEC_CMD      (1<<22)           // The command transfer will be hardware encrypted (KEY2)
#define CARD_DELAY2(n)    (((n)&0x3F)<<16)  // Transfer delay length part 2
#define CARD_SEC_SEED     (1<<15)           // Apply encryption (KEY2) seed to hardware registers
#define CARD_SEC_EN       (1<<14)           // Security enable
#define CARD_SEC_DAT      (1<<13)           // The data transfer will be hardware encrypted (KEY2)
#define CARD_DELAY1(n)    ((n)&0x1FFF)      // Transfer delay length part 1

// Card commands
#define CARD_CMD_DUMMY          0x9F
#define CARD_CMD_HEADER_READ    0x00
#define CARD_CMD_HEADER_CHIPID  0x90
#define CARD_CMD_ACTIVATE_BF    0x3C  // Go into blowfish (KEY1) encryption mode
#define CARD_CMD_ACTIVATE_SEC   0x40  // Go into hardware (KEY2) encryption mode
#define CARD_CMD_SECURE_CHIPID  0x10
#define CARD_CMD_SECURE_READ    0x20
#define CARD_CMD_DISABLE_SEC    0x60  // Leave hardware (KEY2) encryption mode
#define CARD_CMD_DATA_MODE      0xA0
#define CARD_CMD_DATA_READ      0xB7
#define CARD_CMD_DATA_CHIPID    0xB8

// SPI EEPROM COMMANDS
#define SPI_EEPROM_WRSR   0x01
#define SPI_EEPROM_PP     0x02	// Page Program
#define SPI_EEPROM_READ   0x03
#define SPI_EEPROM_WRDI   0x04  // Write disable
#define SPI_EEPROM_RDSR   0x05  // Read status register
#define SPI_EEPROM_WREN   0x06  // Write enable
#define SPI_EEPROM_PW     0x0a	// Page Write
#define SPI_EEPROM_FAST   0x0b	// Fast Read
#define SPI_EEPROM_RDID   0x9f
#define SPI_EEPROM_RDP    0xab	// Release from deep power down
#define SPI_EEPROM_DPD    0xb9  // Deep power down

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