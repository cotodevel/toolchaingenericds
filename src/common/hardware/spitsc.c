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

#include <stdbool.h>
#include "common_shared.h"
#include "spitsc.h"
#include "common_shared.h"

#ifdef ARM7
#include "spi.h"
#include "clock.h"

//Source http://problemkaputt.de/gbatek.htm
void doSPIARM7IO(){
	MyIPC->EXTKEYINInst.PenDown = penIRQread();
	
	//external X Y Struct
	uint8 readEXTKEYIN = (uint8)REG_KEYXY;
	if(!(readEXTKEYIN & KEY_XARM7)){
		MyIPC->EXTKEYINInst.butX = true;
	}
	else{
		MyIPC->EXTKEYINInst.butX = false;
	}
	
	if(!(readEXTKEYIN & KEY_YARM7)){
		MyIPC->EXTKEYINInst.butY = true;
	}
	else{
		MyIPC->EXTKEYINInst.butY = false;
	}
	
	//Folding
	if(!(readEXTKEYIN & KEY_HINGE)){
		MyIPC->EXTKEYINInst.hinge_folded = true;
	}
	else{
		MyIPC->EXTKEYINInst.hinge_folded = false;
	}
	
	//NDS Format ARM7 XY/PENIRQ/HINGE buttons
	MyIPC->buttons7 = REG_KEYXY;
	
	//do touchscreen process
	XYReadPos();
	
	//read clock
	nds_get_time7();
}
#endif


bool penIRQread(){

	#ifdef ARM7
	//Set Chip Select LOW to invoke the command & Transmit the instruction byte
	REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_TSCCNT | BIT_SPICLK_2MHZ;
	RWSPICNT(BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL);
	//Set Chip Select HIGH to finish the command
	SPICSHIGH();
	
	if(!(REG_KEYXY & KEY_PENIRQARM7)){
		return true;
	}
	else{
		return false;
	}
	#endif
	
	#ifdef ARM9
	if(MyIPC->EXTKEYINInst.PenDown == 1){
		return true;
	}
	
	return false;
	#endif
}

#ifdef ARM7
//Internal
void XYReadPos(){
	
	//Update touchscreen only when tsc is being pressed
	if(penIRQread() == true){
		
		uint32 OLD_IME = REG_IME;
		REG_IME = 0;
		
		//Set Chip Select LOW to invoke the command & Transmit the instruction byte: TSC CNT Differential Mode: X Raw TSC 
		REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_TSCCNT | BIT_SPICLK_2MHZ;
		RWSPICNT(BIT_TSCCNT_START_CTRL|BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL| BIT_TSCCNT_REFSEL_DIFFERENTIAL | BIT_TSCCNT_CONVMODE_12bit | BIT_TSCCNT_TOUCHXPOS);
		uint8 resultx1 =RWSPICNT(0);
		uint8 resultx2 = RWSPICNT(0) >>3;
		uint16 read_raw_x = ((resultx1 & 0x7F) << 5) | resultx2;
		
		//required
		SPICSHIGH();
		SPIWAITCNT();
		
		//Set Chip Select LOW to invoke the command & Transmit the instruction byte: TSC CNT Differential Mode: Y Raw TSC 
		REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_TSCCNT | BIT_SPICLK_2MHZ;
		RWSPICNT(BIT_TSCCNT_START_CTRL|BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL| BIT_TSCCNT_REFSEL_DIFFERENTIAL | BIT_TSCCNT_CONVMODE_12bit | BIT_TSCCNT_TOUCHYPOS);
		uint8 resulty1 =RWSPICNT(0);
		uint8 resulty2 = RWSPICNT(0) >>3;
		uint16 read_raw_y = ((resulty1 & 0x7F) << 5) | resulty2;
		
		//raw x/y
		MyIPC->touchX    = read_raw_x;
		MyIPC->touchY    = read_raw_y;
		
		//required
		SPICSHIGH();
		SPIWAITCNT();
		
		REG_IME = OLD_IME;
		
		//Touchscreen Position (pixel TFT X Y Coordinates conversion)
		//Read the X and Y positions in 12bit differential mode, then convert the touchscreen values (adc) to screen/pixel positions (scr), as such:
		//scr.x = (adc.x-adc.x1) * (scr.x2-scr.x1) / (adc.x2-adc.x1) + (scr.x1-1)
		//scr.y = (adc.y-adc.y1) * (scr.y2-scr.y1) / (adc.y2-adc.y1) + (scr.y1-1)
		
		uint16 adc_x1 = (MyIPC->DSFWSETTINGSInst.tsc_adcposx1y112bit[1]<<8) | MyIPC->DSFWSETTINGSInst.tsc_adcposx1y112bit[0];
		uint16 adc_y1 = (MyIPC->DSFWSETTINGSInst.tsc_adcposx1y112bit[3]<<8) | MyIPC->DSFWSETTINGSInst.tsc_adcposx1y112bit[2];
		
		uint8 scr_x1  = (MyIPC->DSFWSETTINGSInst.tsc_tsccalx1y18bit[0]);
		uint8 scr_y1  = (MyIPC->DSFWSETTINGSInst.tsc_tsccalx1y18bit[1]);
		
		uint16 adc_x2 = (MyIPC->DSFWSETTINGSInst.tsc_adcposx2y212bit[1]<<8) | MyIPC->DSFWSETTINGSInst.tsc_adcposx2y212bit[0];
		uint16 adc_y2 = (MyIPC->DSFWSETTINGSInst.tsc_adcposx2y212bit[3]<<8) | MyIPC->DSFWSETTINGSInst.tsc_adcposx2y212bit[2];
		
		uint8 scr_x2  = (MyIPC->DSFWSETTINGSInst.tsc_tsccalx2y28bit[0]);
		uint8 scr_y2  = (MyIPC->DSFWSETTINGSInst.tsc_tsccalx2y28bit[1]);
		
		sint32 scrx = (read_raw_x-adc_x1) * (scr_x2-scr_x1) / (adc_x2-adc_x1) + (scr_x1-1);
		sint32 scry = (read_raw_y-adc_y1) * (scr_y2-scr_y1) / (adc_y2-adc_y1) + (scr_y1-1);
		
		//MyIPC->DSFWSETTINGSInst.tsc_adcposx1y112bit	//4 bytes, 2 adc.x1, 2 adc.y1 //12 bit
		//MyIPC->DSFWSETTINGSInst.tsc_tsccalx1y18bit	//2 bytes, 1 scr.x1, 1 scr.y1 //8 bit
		//MyIPC->DSFWSETTINGSInst.tsc_adcposx2y212bit	//4 bytes, 2 adc.x2, 2 adc.y2 //12bit
		//MyIPC->DSFWSETTINGSInst.tsc_tsccalx2y28bit	//2 bytes, 1 scr.x2, 1 scr.y2	//8bit
		
		MyIPC->touchXpx = scrx;
		MyIPC->touchYpx = scry;
		
	}
	
}

#endif
//External 
//relies on doSPIARM7IO() XY Readings
void XYReadScrPos(XYTscPos * StouchScrPosInst){
    
    StouchScrPosInst->rawx =   MyIPC->touchX;
    StouchScrPosInst->rawy =   MyIPC->touchY;
    
    //TFT x/y pixel
    StouchScrPosInst->px   =   MyIPC->touchXpx;
    StouchScrPosInst->py   =   MyIPC->touchYpx;
    
    StouchScrPosInst->z1   =   MyIPC->touchZ1;
    StouchScrPosInst->z2   =   MyIPC->touchZ2;
	
}