/*

			Copyright (C) 2021  Coto
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

#ifndef __libndsFIFO_h__
#define __libndsFIFO_h__

#include "ipcfifoTGDS.h"
#include "biosTGDS.h"

#ifdef __cplusplus
extern "C"{
#endif

/*!
	\brief fifo callback function pointer with the sent address and the callback's user data.

	The handler is called when new data arrives.

	\note callback functions are called from interrupt level, but are well secured. not too much caution is necessary,
			but don't call alloc, free or printf from within them, just to be safe.
*/
typedef void (*FifoAddressHandlerFunc)(void * address, void * userdata);


/*!
	\brief fifo callback function pointer with the sent value and the callback's user data.

	The handler is called when new data arrives.

	\note callback functions are called from interrupt level, but are well secured. not too much caution is necessary,
			but don't call alloc, free or printf from within them, just to be safe.
*/
typedef void (*FifoValue32HandlerFunc)(u32 value32, void * userdata);


/*!
	\brief fifo callback function pointer with the number of bytes sent and the callback's user data

	The handler is called when new data arrives.
	This callback must call fifoGetData to actually retrieve the data. If it doesn't, the data will be destroyed on return.

	\note callback functions are called from interrupt level, but are well secured. not too much caution is necessary,
			but don't call alloc, free or printf from within them, just to be safe.
*/
typedef void (*FifoDatamsgHandlerFunc)(int num_bytes, void * userdata);


/*!
	\brief Initializes the fifo system.
	Attempts to sync with the other CPU, if it fails, fifo services won't be provided.
	\note call irqInit() before calling this function.
	\return true if syncing worked, false if something went wrong.
*/
extern bool fifoInit();
	
/*!
	\brief Send an address to an channel.

	Transmits an address in the range 0x02000000-0x023FFFFF to the other CPU.

	\param channel channel number to send to.
	\param address address to send.

	\return true if the address has been send, false if something went wrong.
*/
extern bool fifoSendAddress(int channel, void *address);


/*!
	\brief Send a 32bit value.

	Transmits a 32bit value to the other CPU.

	\param channel channel number to send to
	\param value32 32bit value to send

	\return true if the value has been send, false if something went wrong.

	\note Transfer is more efficient if the top 8 bits are zero. So sending smaller values or bitmasks that don't include the top bits is preferred.

*/
extern bool fifoSendValue32(int channel, u32 value32);


/*!
	\brief Send a sequence of bytes to the other CPU.

	num_bytes can be between 0 and FIFO_MAX_DATA_BYTES - sending 0 bytes can be useful sometimes...

	\param channel channel number to send to
	\param num_bytes number of bytes to send
	\param data_array pointer to data array

	\return true if the data message has been send, false if something went wrong.
*/
extern bool fifoSendDatamsg(int channel, int num_bytes, u8 * data_array);


/*!
	\brief Set user address message callback.

	Set a callback to receive incoming address messages on a specific channel.

	\param channel channel number to send to.
	\param newhandler a function pointer to the new handler function.
	\param userdata a pointer that will be passed on to the handler when it will be called.

	\return true if the handler has been set, false if something went wrong.

	\note Setting the handler for a channel feeds the queue of buffered messages to the new handler, if there are any unread messages.
*/
extern bool fifoSetAddressHandler(int channel, FifoAddressHandlerFunc newhandler, void * userdata);


/*!
	\brief Set user value32 message callback.

	Set a callback to receive incoming value32 messages on a specific channel.

	\param channel channel number to send to.
	\param newhandler a function pointer to the new handler function.
	\param userdata a pointer that will be passed on to the handler when it will be called.

	\return true if the handler has been set, false if something went wrong.

	\note Setting the handler for a channel feeds the queue of buffered messages to the new handler, if there are any unread messages.
*/
extern bool fifoSetValue32Handler(int channel, FifoValue32HandlerFunc newhandler, void * userdata);


/*!
	\brief Set user data message callback.

	Set a callback to receive incoming data messages on a specific channel.

	\param channel channel number to send to.
	\param newhandler a function pointer to the new handler function.
	\param userdata a pointer that will be passed on to the handler when it will be called.

	\return true if the handler has been set, false if something went wrong.

	\note Setting the handler for a channel feeds the queue of buffered messages to the new handler, if there are any unread messages.
*/
extern bool fifoSetDatamsgHandler(int channel, FifoDatamsgHandlerFunc newhandler, void * userdata);



/*!
	\brief checks if there is any addresses in the fifo queue.

	\param channel the channel to check.

	\return true if there is any addresses in the queue and if there isn't an address handler in place for the channel.
*/
extern bool fifoCheckAddress(int channel);


/*!
	\brief checks if there is any values in the fifo queue.

	\param channel the channel to check.

	\return true if there is any values in the queue and if there isn't a value handler in place for the channel.
*/
extern bool fifoCheckValue32(int channel);


/*!
	\brief checks if there is any data messages in the fifo queue.

	\param channel the channel to check.

	\return true if there is any data messages in the queue and if there isn't a data message handler in place for the channel.
*/
extern bool fifoCheckDatamsg(int channel);


/*!
	\brief gets the number of bytes in the queue for the first data entry.

	\param channel the channel to check.

	\return the number of bytes in the queue for the first data entry, or -1 if there are no entries.
*/

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
static inline int fifoCheckDatamsgLength(int channel) {
	if( (channel >= 0) && (channel < FIFO_CHANNELS) ){
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		#ifdef ARM9
		//Invalidate to read status
		coherent_user_range_by_size((uint32)&TGDSIPC->libndsFIFO.channelBufferSize[channel], sizeof(TGDSIPC->libndsFIFO.channelBufferSize[channel]));
		#endif
		return TGDSIPC->libndsFIFO.channelBufferSize[channel];
	}
	return -1;
}

static inline void fifoWaitValue32(int channel) {
	while(!fifoCheckValue32(channel)) {
		swiDelay(1);
	}

}

/*!
	\brief Get the first address in queue for a specific channel.

	\param channel the channel to check.

	\return the first address in queue, or NULL if there is none.
*/
extern void * fifoGetAddress(int channel);


/*!
	\brief Get the first value32 in queue for a specific channel.

	\param channel the channel to check.

	\return the first value32 in queue, or 0 if there is no message.
*/
extern u32 fifoGetValue32(int channel);


/*!
	\brief Reads a data message in a given buffer and returns the number of bytes written.

	\param channel the channel to check.
	\param buffersize the size of the buffer where the message will be copied to.
	\param destbuffer a pointer to the buffer where the message will be copied to.

	\return the number of bytes written, or -1 if there is no message.

	\warning If your buffer is not big enough, you may lose data! Check the data length first if you're not sure what the size is.
*/
extern int fifoGetDatamsg(int channel, int buffersize, u8 * destbuffer);

#ifdef __cplusplus
}
#endif

#endif