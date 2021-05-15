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
#include "libndsFIFO.h"
#include "global_settings.h"
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "utilsTGDS.h"
#include "timerTGDS.h"
#include "dmaTGDS.h"
#include "eventsTGDS.h"
#include "posixHandleTGDS.h"
#include "biosTGDS.h"

//TGDS -> Libnds FIFO compatibility API. Ensures the behaviour of the FIFO messaging system works.

//ARM9 initializes FIFO
bool fifoInit(){
	#ifdef ARM9
	//Invalidate Cache here
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	memset((u8*)&TGDSIPC->libndsFIFO, 0, sizeof(TGDSIPC->libndsFIFO));
	int i = 0;
	for(i = 0; i < FIFO_CHANNELS; i++){
		TGDSIPC->libndsFIFO.channelBufferSize[i] = 0;
		fifoFunc[i][0] = (u32)0;
		fifoFunc[i][1] = (u32)0;
		memset((u8*)&TGDSIPC->libndsFIFO.channelBuffer[i*FIFO_MAX_DATA_BYTES], 0, FIFO_MAX_DATA_BYTES);
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBufferSize[i], sizeof(TGDSIPC->libndsFIFO.channelBufferSize[i]));
		coherent_user_range_by_size((u32)&TGDSIPC->libndsFIFO.channelBuffer[i*FIFO_MAX_DATA_BYTES], FIFO_MAX_DATA_BYTES);
	}
	#endif
	
	return true;
}

//Format: fifoSendAddress(int channel, void *data) --> [ARM core external installed user receive handler: void *data = fifoGetAddress(int channel)]
bool fifoSendAddress(int channel, void *address){
	u8 buf[4];	//4 == sizeof(address)
	*(u32*)&buf[0] = (u32)address;
	return fifoSendDatamsg(channel, sizeof(address), (u8 *)&buf[0]);
}

//Format: fifoSendValue32(int channel, u32 value32) --> [ARM core external installed user receive handler: u32 value32 = fifoGetValue32(int channel)]
bool fifoSendValue32(int channel, u32 value32){
	return fifoSendAddress(channel, (void*)value32);
}

//Format: fifoSendDatamsg(int channel, int num_bytes, u8 * data_array) --> [ARM core external installed user receive handler: int buffersize = fifoGetDatamsg(int channel, int buffersize, u8 * destbuffer)]
bool fifoSendDatamsg(int channel, int num_bytes, u8 * data_array){
	if((channel >= 0) && (channel < FIFO_CHANNELS) && (num_bytes < FIFO_MAX_DATA_BYTES)){
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		#ifdef ARM9
		//Invalidate to read status
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBufferSize[channel], sizeof(TGDSIPC->libndsFIFO.channelBufferSize[channel]));
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBuffer[channel*FIFO_MAX_DATA_BYTES], FIFO_MAX_DATA_BYTES);
		coherent_user_range_by_size((uint32)data_array, num_bytes);
		#endif
		TGDSIPC->libndsFIFO.channelBufferSize[channel] = num_bytes;
		memcpy((u8*)&TGDSIPC->libndsFIFO.channelBuffer[channel*FIFO_MAX_DATA_BYTES], data_array, num_bytes);
		#ifdef ARM9
		//Invalidate to write changes
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBufferSize[channel], sizeof(TGDSIPC->libndsFIFO.channelBufferSize[channel]));
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBuffer[channel*FIFO_MAX_DATA_BYTES], FIFO_MAX_DATA_BYTES);
		#endif
		
		sendByteIPCNOIRQ((uint8)(channel&0xFF));
		SendFIFOWords(TGDS_LIBNDSFIFO_COMMAND); //Signal IRQ FIFO context. Action may, or not, be taken by the current external ARM core handler if assigned.
		return true;
		
	}
	return false;
}

bool fifoCheckAddress(int channel){
	if(fifoFunc[channel][0] == 0){
		return fifoCheckDatamsg(channel);
	}
	return false;
}

bool fifoCheckValue32(int channel){
	if(fifoFunc[channel][0] == 0){
		return fifoCheckDatamsg(channel);
	}
	return false;
}

bool fifoCheckDatamsg(int channel){
	if( (fifoCheckDatamsgLength(channel) > 0) && (fifoFunc[channel][0] == 0) ){
		return true;
	}
	return false;
}

void * fifoGetAddress(int channel){
	u8 buf[4];	//4 == sizeof(buf)
	if(fifoGetDatamsg(channel, sizeof(buf), (u8 *)&buf[0]) > 0){
		return (void *)*(u32*)&buf[0];
	}
	return NULL;
}

u32 fifoGetValue32(int channel){
	return (u32)fifoGetAddress(channel);
}

int fifoGetDatamsg(int channel, int buffersize, u8 * destbuffer){
	if( (channel >= 0) && (channel < FIFO_CHANNELS) ){
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		#ifdef ARM9
		//Invalidate to read status
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBufferSize[channel], sizeof(TGDSIPC->libndsFIFO.channelBufferSize[channel]));
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBuffer[channel*FIFO_MAX_DATA_BYTES], FIFO_MAX_DATA_BYTES);
		coherent_user_range_by_size((uint32)destbuffer, buffersize);	//make destination buffer coherent regardless
		#endif
		//int ChannelBufSize = TGDSIPC->libndsFIFO.channelBufferSize[channel];
		memcpy(destbuffer, (u8*)&TGDSIPC->libndsFIFO.channelBuffer[channel*FIFO_MAX_DATA_BYTES], buffersize);
		return buffersize;
	}
	return -1;
}

//Handlers: IRQ event based. 
//When a message is RECEIVED from the external ARM core channel, the provided user callback will be called.
//It is MANDATORY, to use the fifoGetXXXX and fifoCheckXXXX methods in the provided user callback, to read the message contents.
bool fifoSetAddressHandler(int channel, FifoAddressHandlerFunc newhandler, void * userdata){
	if( (channel >= 0) && (channel < FIFO_CHANNELS) ){
		#ifdef ARM9
		//Invalidate to read status
		coherent_user_range_by_size((uint32)&fifoFunc[channel][0], sizeof(fifoFunc[channel]));
		#endif
		
		fifoFunc[channel][0] = (u32)newhandler;
		fifoFunc[channel][1] = (u32)userdata;
		return true;	
	}
	return false;
}

bool fifoSetValue32Handler(int channel, FifoValue32HandlerFunc newhandler, void * userdata){
	return fifoSetAddressHandler(channel, (FifoAddressHandlerFunc)newhandler, userdata);
}

bool fifoSetDatamsgHandler(int channel, FifoDatamsgHandlerFunc newhandler, void * userdata){
	return fifoSetAddressHandler(channel, (FifoAddressHandlerFunc)newhandler, userdata);
}