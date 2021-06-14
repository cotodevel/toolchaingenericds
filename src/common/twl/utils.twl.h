//TWL Header
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

#ifndef __utils_twl_h__
#define __utils_twl_h__

#include "typedefsTGDS.h"
#include "dsregs.h"

//! values allowed for REG_AUXIE and REG_AUXIF
enum IRQ_MASKSAUX {
	GPIO33_2	=	(1 << 6),	/*!< DSi7: GPIO33[2] Powerbutton interrupt (short pulse upon key-down) (DSi ARM7)*/
	IRQ_SDMMC = 	(1 << 8),  /*!< Sdmmc interrupt mask / 8	DSi7: SD/MMC Controller   ;-Onboard eMMC and External SD Slot (DSi ARM7)*/
	IRQ_I2C = 		(1 << 13)	/*!< I2C interrupt mask / 6     */
};


/*! \def REG_IE

    \brief Interrupt Enable Register.

	This is the activation mask for the internal interrupts.  Unless
	the corresponding bit is set, the IRQ will be masked out.
*/
#define REG_AUXIE	(*(vuint32*)0x04000218)

/*! \def REG_IF

    \brief Interrupt Flag Register.

	Since there is only one hardware interrupt vector, the IF register
	contains flags to indicate when a particular of interrupt has occured.
	To acknowledge processing interrupts, set IF to the value of the
	interrupt handled.

*/
#define REG_AUXIF	(*(vuint32*)0x0400021C)

/* internal fifo messages utilized by libnds. */

typedef enum {
	SOUND_PLAY_MESSAGE = 0x1234,
	SOUND_PSG_MESSAGE,
	SOUND_NOISE_MESSAGE,
	MIC_RECORD_MESSAGE,
	MIC_BUFFER_FULL_MESSAGE,
	SYS_INPUT_MESSAGE,
	SDMMC_SD_READ_SECTORS,
	SDMMC_SD_WRITE_SECTORS,
	SDMMC_NAND_READ_SECTORS,
	SDMMC_NAND_WRITE_SECTORS
} FifoMessageType;

typedef struct FifoMessage {
	u16 type;

	union {

		struct {
			const void* data;
			u32 dataSize;
			u16 loopPoint;
			u16 freq;
			u8 volume;
			u8 pan;
			bool loop;
			u8 format;
			u8 channel;
		} SoundPlay;

		struct{
			u16 freq;
			u8 dutyCycle;
			u8 volume;
			u8 pan;
			u8 channel;
		} SoundPsg;

		struct{
			void* buffer;
			u32 bufferLength;
			u16 freq;
			u8 format;
		} MicRecord;

		struct{
			void* buffer;
			u32 length;
		} MicBufferFull;

		struct{
			touchPosition touch;
			u16 keys;
		} SystemInput;
		
		struct{
			void *buffer;
			u32 startsector;
			u32	numsectors;
		} sdParams;

		struct{
			void *buffer;
			u32 address;
			u32	length;
		} blockParams;
	};

} __attribute__((aligned(4))) FifoMessage;

typedef enum {
	SDMMC_HAVE_SD,
	SDMMC_SD_START,
	SDMMC_SD_IS_INSERTED,
	SDMMC_SD_STOP,
	SDMMC_NAND_START,
	SDMMC_NAND_STOP,
	SDMMC_NAND_SIZE
} FifoSdmmcCommands;

typedef enum {
	FW_READ,
	FW_WRITE
} FifoFirmwareCommands;

#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9
/*!
	\brief Sets the ARM9 clock speed, only possible in DSi mode
	\param speed CPU speed (false = 67.03MHz, true = 134.06MHz)
	\return The old CPU speed value
*/
extern bool setCpuClock(bool speed);
#endif


extern void cardReset();
extern void cardReadEeprom(u32 address, u8 *data, u32 length, u32 addrtype);
extern void cardEepromSectorErase(uint32 address);
extern void cardWriteEeprom(uint32 address, uint8 *data, uint32 length, uint32 addrtype);
extern bool cdcIsAvailable(void);

#ifdef __cplusplus
}
#endif