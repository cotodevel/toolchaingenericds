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


#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "timerTGDS.h"

///////////////////////////////////Usercode Timer///////////////////////////////////
//Note: IRQ Timer 3 is used. So Wifi can't be used at the same time when counting up

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
unsigned int timerTicks = 0;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
unsigned int timerUnitsPerTick = 0;

/* Example:
//Timer start: Measured in milliseconds
	startTimerCounter(tUnitsMilliseconds, 1); //tUnitsMilliseconds equals 1 millisecond/unit. A single unit (1) is the default value for normal timer count-up scenarios. 
	
	//do stuff
	int j = 0;
	for(j = 0; j < 100; j++){
		int a = 0;
		int b = a;
		int c = b-a;
		printf("%d", c);
	}
	
	//Timer stop
	stopTimerCounter();
	
	printf("timer took: %d ms", getTimerCounter()); 
*/

#ifdef ARM9
__attribute__((section(".itcm")))
void startTimerCounter(enum timerUnits units, int timerUnitsPERTick){
	timerTicks = 0;
	timerUnitsPerTick = timerUnitsPERTick;
	TIMERXDATA(3) = TIMER_FREQ((int)units);
	TIMERXCNT(3) = TIMER_DIV_1 | TIMER_IRQ_REQ | TIMER_ENABLE;
	irqEnable(IRQ_TIMER3);
	
}

__attribute__((section(".itcm")))
unsigned int getTimerCounter(){
	return timerTicks;
}

__attribute__((section(".itcm")))
void stopTimerCounter(){
	irqDisable(IRQ_TIMER3);
}
#endif