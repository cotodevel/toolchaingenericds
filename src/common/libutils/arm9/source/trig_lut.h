/*---------------------------------------------------------------------------------
	$Id: trig_lut.h,v 1.5 2005-07-29 00:35:52 wntrmute Exp $


	Trig_lut.h provides access to external precompiled trig look up tables


	Copyright (C) 2005

		Michael Noland (joat)

		Jason Rogers (dovoto)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use

		this software in a product, an acknowledgment in the product

		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and

		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source

		distribution.

	


---------------------------------------------------------------------------------*/
#ifndef TRIG_LUT_H
#define TRIG_LUT_H

#ifdef WIN32
#include "TGDSTypes.h"
#endif

#if !defined(_MSC_VER) && defined(ARM9) //TGDS ARM9?
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#endif

#include "SIN_bin.h"
#include "TAN_bin.h"
#include "COS_bin.h"

#define COS ((short*)&COS_bin[0])
#define SIN ((short*)&SIN_bin[0])
#define TAN ((short*)&TAN_bin[0])


#endif 
