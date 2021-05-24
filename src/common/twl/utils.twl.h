#ifdef TWLMODE

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
	IRQ_I2C	=	(1 << 6),	/*!< I2C interrupt mask (DSi ARM7)*/
	IRQ_SDMMC = 	(1 << 8)  /*!< Sdmmc interrupt mask (DSi ARM7)*/
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

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern bool isDSiMode();

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

#endif