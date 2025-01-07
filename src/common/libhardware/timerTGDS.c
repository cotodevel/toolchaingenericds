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
#include "InterruptsARMCores_h.h"

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

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 timerIRQDest = -1;

/* Example:
//Timer start: Measured in milliseconds
	startTimerCounter(tUnitsMilliseconds, 1, IRQ_TIMER2); //tUnitsMilliseconds equals 1 millisecond/unit. A single unit (1) is the default value for normal timer count-up scenarios. Use timer 2 to perform the count.
	
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
void startTimerCounter(enum timerUnits units, int timerUnitsPERTick, u32 timerIRQ){
	timerTicks = 0;
	timerUnitsPerTick = timerUnitsPERTick;
	int timerDest = -1;
	timerIRQDest = timerIRQ;
	switch(timerIRQDest){
		case(IRQ_TIMER0):{
			timerDest = 0;
		}break;
		case(IRQ_TIMER1):{
			timerDest = 1;
		}break;
		case(IRQ_TIMER2):{
			timerDest = 2;
		}break;
		case(IRQ_TIMER3):{
			timerDest = 3;
		}break;
		default:{
			return;
		}break;
	}
	
	TIMERXDATA(timerDest) = TIMER_FREQ((int)units);
	TIMERXCNT(timerDest) = TIMER_DIV_1 | TIMER_IRQ_REQ | TIMER_ENABLE;
	irqEnable(timerIRQDest);
}

__attribute__((section(".itcm")))
unsigned int getTimerCounter(){
	return timerTicks;
}

__attribute__((section(".itcm")))
void stopTimerCounter(){
	irqDisable(timerIRQDest);
}
#endif