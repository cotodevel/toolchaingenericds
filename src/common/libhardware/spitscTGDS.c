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

#include "spitscTGDS.h"
#include <stdbool.h>
#include "ipcfifoTGDS.h"
#include "utils.twl.h"

#ifdef ARM7

#ifdef TWLMODE
#include "codec.h"
#include "utils.twl.h"
#endif

#include "biosTGDS.h"
#include "spiTGDS.h"
#include "clockTGDS.h"

static inline void SerialWaitBusy() { while (REG_SPICNT & SPI_BUSY) swiDelay(1); }

//!	User's DS settings.
/*!	\struct tPERSONAL_DATA

	Defines the structure the DS firmware uses for transfer
	of the user's settings to the booted program.
*/
typedef struct tPERSONAL_DATA {
  u8 RESERVED0[2];			//!<	??? (0x05 0x00).

  u8 theme;					//!<	The user's theme color (0-15).
  u8 birthMonth;			//!<	The user's birth month (1-12).
  u8 birthDay;				//!<	The user's birth day (1-31).

  u8 RESERVED1[1];			//!<	???

  s16 name[10];				//!<	The user's name in UTF-16 format.
  u16 nameLen;				//!<	The length of the user's name in characters.

  s16 message[26];			//!<	The user's message.
  u16 messageLen;			//!<	The length of the user's message in characters.

  u8 alarmHour;				//!<	What hour the alarm clock is set to (0-23).
  u8 alarmMinute;			//!<	What minute the alarm clock is set to (0-59).
            //0x027FFCD3  alarm minute

  u8 RESERVED2[4];			//!<	???
           //0x027FFCD4  ??

  u16 calX1;				//!<	Touchscreen calibration: first X touch
  u16 calY1;				//!<	Touchscreen calibration: first Y touch
  u8 calX1px;				//!<	Touchscreen calibration: first X touch pixel
  u8 calY1px;				//!<	Touchscreen calibration: first X touch pixel

  u16 calX2;				//!<	Touchscreen calibration: second X touch
  u16 calY2;				//!<	Touchscreen calibration: second Y touch
  u8 calX2px;				//!<	Touchscreen calibration: second X touch pixel
  u8 calY2px;				//!<	Touchscreen calibration: second Y touch pixel

  struct __attribute__ ((packed)) {
    unsigned language    : 3;	//!<	User's language.
    unsigned gbaScreen   : 1;	//!<	GBA screen selection (lower screen if set, otherwise upper screen).
    unsigned RESERVED3   : 2;	//!<	???
    unsigned autoMode    : 1;	//!<	The DS should boot from the DS cart or GBA cart automatically if one is inserted.
    unsigned RESERVED4   : 1;	//!<	???
  };
} PERSONAL_DATA ;

//!	Default location for the user's personal data (see %PERSONAL_DATA).
#define PersonalData ((PERSONAL_DATA*)&((struct sIPCSharedTGDS *)TGDSIPCStartAddress)->DSFWSETTINGSInst)


static u8 last_time_touched = 0;

static u8 range_counter_1 = 0;
static u8 range_counter_2 = 0;
static u8 range = 20;
static u8 min_range = 20;

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u8 CheckStylus(){
//---------------------------------------------------------------------------------

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS; //0x8A01;
	REG_SPIDATA = TSC_MEASURE_TEMP1;

	SerialWaitBusy();

	REG_SPIDATA = 0;

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH;// 0x8201;
	REG_SPIDATA = 0;

	SerialWaitBusy();

	if(last_time_touched == 1){
		if( !(REG_KEYXY & 0x40) )
			return 1;
		else{
			REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
			REG_SPIDATA = TSC_MEASURE_TEMP1;

			SerialWaitBusy();

			REG_SPIDATA = 0;

			SerialWaitBusy();

			REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH;
			REG_SPIDATA = 0;

			SerialWaitBusy();

			return !(REG_KEYXY & 0x40) ? 2 : 0;
		}
	}else{
		return !(REG_KEYXY & 0x40) ? 1 : 0;
	}
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint16 touchRead(uint32 command) {
//---------------------------------------------------------------------------------
	uint16 result, result2;

	//uint32 oldIME = REG_IME;

	//REG_IME = 0;
	
	SerialWaitBusy();

	// Write the command and wait for it to complete
	REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS; //0x8A01;
	REG_SPIDATA = command;
	SerialWaitBusy();

	// Write the second command and clock in part of the data
	REG_SPIDATA = 0;
	SerialWaitBusy();
	result = REG_SPIDATA;

	// Clock in the rest of the data (last transfer)
	REG_SPICNT = SPI_ENABLE | 0x201;
	REG_SPIDATA = 0;
	SerialWaitBusy();

	result2 = REG_SPIDATA >>3;

	//REG_IME = oldIME;

	// Return the result
	return ((result & 0x7F) << 5) | result2;
}


//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint32 touchReadTemperature(int * t1, int * t2) {
//---------------------------------------------------------------------------------
	*t1 = touchRead(TSC_MEASURE_TEMP1);
	*t2 = touchRead(TSC_MEASURE_TEMP2);
	return 8490 * (*t2 - *t1) - 273*4096;
}


bool touchInit = false;
s32 xscaleTGDSScr, yscaleTGDSScr;
s32 xoffset, yoffset;

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
sint16 readTouchValue(uint32 command, sint16 *dist_max, u8 *err){
//---------------------------------------------------------------------------------
	sint16 values[5];
	sint32 aux1, aux2, aux3, dist, dist2, result = 0;
	u8 i, j, k;

	*err = 1;

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
	REG_SPIDATA = command;

	SerialWaitBusy();

	for(i=0; i<5; i++){
		REG_SPIDATA = 0;
		SerialWaitBusy();

		aux1 = REG_SPIDATA;
		aux1 = aux1 & 0xFF;
		aux1 = aux1 << 16;
		aux1 = aux1 >> 8;

		values[4-i] = aux1;

		REG_SPIDATA = command;
		SerialWaitBusy();

		aux1 = REG_SPIDATA;
		aux1 = aux1 & 0xFF;
		aux1 = aux1 << 16;

		aux1 = values[4-i] | (aux1 >> 16);
		values[4-i] = ((aux1 & 0x7FF8) >> 3);
	}

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH;
	REG_SPIDATA = 0;
	SerialWaitBusy();

	dist = 0;
	for(i=0; i<4; i++){
		aux1 = values[i];

		for(j=i+1; j<5; j++){
			aux2 = values[j];
			aux2 = abs(aux1 - aux2);
			if(aux2>dist) dist = aux2;
		}
	}

	*dist_max = dist;

	for(i=0; i<3; i++){
		aux1 = values[i];

		for(j=i+1; j<4; j++){
			aux2 = values[j];
			dist = abs(aux1 - aux2);

			if( dist <= range ){
				for(k=j+1; k<5; k++){
					aux3 = values[k];
					dist2 = abs(aux1 - aux3);

					if( dist2 <= range ){
						result = aux2 + (aux1 << 1);
						result = result + aux3;
						result = result >> 2;
						result = result & (~7);

						*err = 0;

						break;
					}
				}
			}
		}
	}

	if((*err) == 1){
		result = values[0] + values[4];
		result = result >> 1;
		result = result & (~7);
	}

	return (result & 0xFFF);
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void UpdateRange(uint8 *this_range, sint16 last_dist_max, u8 data_error, u8 tsc_touched){
//---------------------------------------------------------------------------------
	//range_counter_1 = counter_0x380A98C
	//range_counter_2 = counter_0x380A990
	//Initial values:
	// range = 20
	// min_range = 20

	if(tsc_touched != 0){
		if( data_error == 0){
			range_counter_2 = 0;

			if( last_dist_max >= ((*this_range) >> 1)){
				range_counter_1 = 0;
			}else{
				range_counter_1++;

				if(range_counter_1 >= 4){
					range_counter_1 = 0;

					if((*this_range) > min_range){
						(*this_range)--;
						range_counter_2 = 3;
					}
				}
			}
		}else{
			range_counter_1 = 0;
			range_counter_2++;

			if(range_counter_2 >= 4){

				range_counter_2 = 0;

				if((*this_range) < 35){  //0x23 = 35
					*this_range = (*this_range) + 1;
				}
			}
		}
	}else{
		range_counter_2 = 0;
		range_counter_1 = 0;
	}
}

//---------------------------------------------------------------------------------
bool touchPenDown() {
//---------------------------------------------------------------------------------
	bool down;
	//int oldIME = enterCriticalSection();
	#ifdef TWLMODE
	if (cdcIsAvailable() && (useTWLTSC == true)) {
		down = cdcTouchPenDown();
	} 
	else {
		down = !(REG_KEYXY & (1<<6));
	}
	#endif
	#ifdef NTRMODE
	down = !(REG_KEYXY & (1<<6));
	#endif
	//leaveCriticalSection(oldIME);
	return down;
}

bool useTWLTSC = false;

//---------------------------------------------------------------------------------
// reading pixel position:
//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void touchReadXY(struct touchPosition *touchPos) {
//---------------------------------------------------------------------------------	
	if ( !touchInit ) {
		xscaleTGDSScr = ((PersonalData->calX2px - PersonalData->calX1px) << 19) / ((PersonalData->calX2) - (PersonalData->calX1));
		yscaleTGDSScr = ((PersonalData->calY2px - PersonalData->calY1px) << 19) / ((PersonalData->calY2) - (PersonalData->calY1));
		xoffset = ((PersonalData->calX1 + PersonalData->calX2) * xscaleTGDSScr  - ((PersonalData->calX1px + PersonalData->calX2px) << 19) ) / 2;
		yoffset = ((PersonalData->calY1 + PersonalData->calY2) * yscaleTGDSScr  - ((PersonalData->calY1px + PersonalData->calY2px) << 19) ) / 2;
		
		#ifdef TWLMODE
		if (cdcIsAvailable() && (useTWLTSC == true)) {
			//int oldIME = enterCriticalSection();
			cdcTouchInit();
			//leaveCriticalSection(oldIME);
		}
		#endif
		
		touchInit = true;
	}
	
	sint16 dist_max_y=0, dist_max_x=0, dist_max=0;
	u8 error=0, error_where=0, first_check=0, i=0;
	
	if (cdcIsAvailable() && (useTWLTSC == true)) {	//TWL Mode
		#ifdef TWLMODE
		cdcTouchRead(touchPos);	
		s16 px = ( touchPos->rawx * xscaleTGDSScr - xoffset + xscaleTGDSScr/2 ) >>19;
		s16 py = ( touchPos->rawy * yscaleTGDSScr - yoffset + yscaleTGDSScr/2 ) >>19;
		if ( px < 0) px = 0;
		if ( py < 0) py = 0;
		if ( px > (SCREEN_WIDTH -1)) px = SCREEN_WIDTH -1;
		if ( py > (SCREEN_HEIGHT -1)) py = SCREEN_HEIGHT -1;
		touchPos->px = px;
		touchPos->py = py;
		#endif
	} 
	else {	//(NTR) DS Mode Start
		//uint32 oldIME = REG_IME;
		//REG_IME = 0;
		first_check = CheckStylus();
		if(first_check != 0){
			error_where = 0;
			touchPos->z1 =  readTouchValue(TSC_MEASURE_Z1 | 1, &dist_max, &error);
			touchPos->z2 =  readTouchValue(TSC_MEASURE_Z2 | 1, &dist_max, &error);
			touchPos->rawx = readTouchValue(TSC_MEASURE_X | 1, &dist_max_x, &error);
			if(error==1) error_where += 1;

			touchPos->rawy = readTouchValue(TSC_MEASURE_Y | 1, &dist_max_y, &error);
			if(error==1) error_where += 2;

			REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS;
			for(i=0; i<12; i++){
				REG_SPIDATA = 0;
				SerialWaitBusy();
			}

			REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH;
			REG_SPIDATA = 0;
			SerialWaitBusy();
			if(first_check == 2) error_where = 3;

			switch( CheckStylus() ){
				case 0:
					last_time_touched = 0;
					break;
				case 1:
					last_time_touched = 1;

					if(dist_max_x > dist_max_y)
						dist_max = dist_max_x;
					else
						dist_max = dist_max_y;

					break;
				case 2:
					last_time_touched = 0;
					error_where = 3;

					break;
			}
			s16 px = ( touchPos->rawx * xscaleTGDSScr - xoffset + xscaleTGDSScr/2 ) >>19;
			s16 py = ( touchPos->rawy * yscaleTGDSScr - yoffset + yscaleTGDSScr/2 ) >>19;
			if ( px < 0) px = 0;
			if ( py < 0) py = 0;
			if ( px > (SCREEN_WIDTH -1)) px = SCREEN_WIDTH -1;
			if ( py > (SCREEN_HEIGHT -1)) py = SCREEN_HEIGHT -1;
			touchPos->px = px;
			touchPos->py = py;
		}else{
			error_where = 3;
			touchPos->rawx = 0;
			touchPos->rawy = 0;
			last_time_touched = 0;
		}
	}
	
	
	UpdateRange(&range, dist_max, error_where, last_time_touched);
	//REG_IME = oldIME;
}

//Source http://problemkaputt.de/gbatek.htm & devkitARM
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void doSPIARM7IO(){
	#ifdef ARM7
	//struct sIPCSharedTGDS * sIPCSharedTGDSInst = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
	//Handle Clock (should this one run on IRQs instead?)
	//sIPCSharedTGDSInst->ndsRTCSeconds = nds_get_time7(); //disable clock by default, re-enable it if TGDS Project needs it
	#endif
}
#endif

bool penIRQread(){

	#ifdef ARM7
	if(!(REG_KEYXY & KEY_PENIRQARM7)){
		return true;
	}
	else{
		return false;
	}
	#endif
	
	#ifdef ARM9
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	return (bool)(TGDSIPC->EXTKEYINInst.PenDown);
	#endif
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TWLSetTouchscreenTWLMode(){
	#ifdef TWLMODE
	#ifdef ARM7
	touchInit = false;
	useTWLTSC = true; //force TWL TSC readings
	
	//Extended SPI Clock (8MHz)     (0=NITRO, 1=Extended) (40001C0h)
	u32 SFGEXT7 = *(u32*)0x04004008;
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 9)) | (0x1 << 9); //TWL / Extended
	*(u32*)0x04004008 = SFGEXT7;
	#endif
	#endif
	
	#ifdef ARM9
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[7], (u32)TGDS_ARM7_TWL_SET_TSC_TWLMODE);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
	#endif
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TWLSetTouchscreenNTRMode(){
	#ifdef TWLMODE
	#ifdef ARM7
	touchInit = false;
	useTWLTSC = false; //force NTR TSC readings
	
	//Extended SPI Clock (8MHz)     (0=NITRO, 1=Extended) (40001C0h)
	u32 SFGEXT7 = *(u32*)0x04004008;
	SFGEXT7 = (SFGEXT7 & ~(0x1 << 9)) | (0x0 << 9); //NTR / NITRO
	*(u32*)0x04004008 = SFGEXT7;
	
	#endif
	#endif
	
	#ifdef ARM9
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	setValueSafe(&fifomsg[7], (u32)TGDS_ARM7_TWL_SET_TSC_NTRMODE);
	SendFIFOWords(FIFO_SEND_TGDS_CMD, 0xFF);
	while( ( ((uint32)getValueSafe(&fifomsg[7])) != ((uint32)0)) ){
		swiDelay(1);
	}
	#endif
}

#ifdef ARM7

static bool penDown = false;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void taskARM7TouchScreen(u32 * args){
	struct sIPCSharedTGDS * sIPCSharedTGDSInst = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
	struct touchPosition * sTouchPosition = (struct touchPosition *)&sIPCSharedTGDSInst->tscIPC;
	
	//ARM7 Keypad has access to X/Y/Hinge/Pen down bits
	sIPCSharedTGDSInst->KEYINPUT7 = (uint16)REG_KEYINPUT;
	
	u16 keys= REG_KEYXY;	
	#ifdef TWLMODE
	keys |= (1 << 6);
	if(touchPenDown() == true){
		keys &= ~(1 << 6);
	}
	#endif
	/*
	4000136h - NDS7 - EXTKEYIN - Key X/Y Input (R)
	0      Button X     (0=Pressed, 1=Released)
	1      Button Y     (0=Pressed, 1=Released)
	3      DEBUG button (0=Pressed, 1=Released/None such)
	6      Pen down     (0=Pressed, 1=Released/Disabled) (always 0 in DSi mode)
	7      Hinge/folded (0=Open, 1=Closed)
	2,4,5  Unknown / set
	8..15  Unknown / zero
	*/
	if(keys & KEY_TOUCH){
		penDown = false;
	}
	else{	
		//reset state
		sTouchPosition->rawy    = 0;
		sTouchPosition->py = 0;
		sTouchPosition->rawx    = 0;
		sTouchPosition->px = 0;
		sTouchPosition->z1 = 0;
		sTouchPosition->z2 = 0;
		
		if(penDown){
			keys |= KEY_TOUCH;	//tsc event must be before coord handling to give priority over touch events
			
			touchPosition tempPos = {0};
			touchReadXY(&tempPos);
			
			if(tempPos.rawx && tempPos.rawy){
				sTouchPosition->rawy    = tempPos.rawy;
				sTouchPosition->py = tempPos.py;
				sTouchPosition->rawx    = tempPos.rawx;
				sTouchPosition->px = tempPos.px;
				sTouchPosition->z1 = tempPos.z1;
				sTouchPosition->z2 = tempPos.z2;
			}
			else{
				penDown = false;
			}
			
		}
		else{
			penDown = true;
		}
		
		//handle re-click
		
		#ifdef NTRMODE
		if( !(((uint16)REG_KEYINPUT) & KEY_TOUCH) ){
			penDown = true;
		}
		#endif
		
		#ifdef TWLMODE
		if(touchPenDown() == false){
			penDown = true;
		}
		#endif
	}
	
	sIPCSharedTGDSInst->buttons7	= keys;
}

bool ARM7TouchScreenEnabled = false;

void enableARM7TouchScreen(){
	ARM7TouchScreenEnabled = true;
}

void disableARM7TouchScreen(){
	ARM7TouchScreenEnabled = false;
}

#endif