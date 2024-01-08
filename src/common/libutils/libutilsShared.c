#include "libutilsShared.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "spifwTGDS.h"
#include "biosTGDS.h"
#include "limitsTGDS.h"
#include "dldi.h"
#include "debugNocash.h"
#include "libndsFIFO.h"
#include "wifi_shared.h"
#include "microphoneShared.h"

#ifdef ARM7
#include "wifi_arm7.h"
#endif

#ifdef ARM9
#include "wifi_arm9.h"
#endif

SoundRegion * getSoundIPC(){
	struct sIPCSharedTGDS * TGDSIPC = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
	return &TGDSIPC->soundIPC;
}

#ifdef ARM9

/*
* Copyright (C) 1996-2001  Internet Software Consortium.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
* DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
* INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
* FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
* WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "socket.h"
#include "utilsTGDS.h"
#include "fatfslayerTGDS.h"
#include "videoTGDS.h"
#include "dmaTGDS.h"
#include "tgds_ramdisk_dldi.h"

#define NS_INT16SZ       2
#define NS_INADDRSZ      4
#define NS_IN6ADDRSZ    16

/* int
* inet_pton4(src, dst)
*      like inet_aton() but without all the hexadecimal and shorthand.
* return:
*      1 if `src' is a valid dotted quad, else 0.
* notice:
*      does not touch `dst' unless it's returning 1.
* author:
*      Paul Vixie, 1996.
*/
static int inet_pton4(src, dst)
const char *src;
unsigned char *dst;
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[NS_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;
		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int new = *tp * 10 + (pch - digits);
			if (new > 255)
				return (0);
			*tp = new;
			if (! saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} 
		else
			return (0);
	}
	if (octets < 4)
		return (0);
	memcpy(dst, tmp, NS_INADDRSZ);
	return (1);
}

/* int
* isc_net_pton(af, src, dst)
*      convert from presentation format (which usually means ASCII printable)
*      to network format (which is usually some kind of binary format).
* return:
*      1 if the address was valid for the specified address family
*      0 if the address wasn't valid (`dst' is untouched in this case)
*      -1 if some other error occurred (`dst' is untouched in this case, too)
* author:
*      Paul Vixie, 1996.
*/
int inet_pton(int af, const char *src, void *dst){
	switch(af){
		case AF_INET:{
			return (inet_pton4(src, dst));
		}
		break;
		default:{
			errno = EAFNOSUPPORT;
		}
		break;
	}
	return (-1);
}

#endif

void libUtilsFIFONotEmpty(u32 cmd1, u32 cmd2){	
	//Execute ToolchainGenericDS FIFO commands
	switch (cmd1) {
		#ifdef ARM7
		case ARM7COMMAND_START_RECORDING:{
			micStartRecording();
		}
		break;				
		case ARM7COMMAND_STOP_RECORDING:{
			micStopRecording();
		}
		break;
		
		//arm9 wants to send a WIFI context block address / userdata is always zero here
		case((uint32)WIFI_INIT):{
			uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
			//	wifiAddressHandler( void * address, void * userdata )
			wifiAddressHandler((Wifi_MainStruct *)fifomsg[60], 0);
		}
		break;
		// Deinit WIFI
		case((uint32)WIFI_DEINIT):{
			DeInitWIFI();
		}
		break;
		#endif
		
		#ifdef ARM9
		case ((uint32)TGDS_SAVE_MIC_DATA):{
			copyChunk();
		}
		break;
		#endif
		
		//Shared
		//Handle Libnds FIFO receive handlers
		case(TGDS_LIBNDSFIFO_COMMAND):{
			int channel = (int)receiveByteIPC();
			sendByteIPCNOIRQ((uint8)0);	//clean
			//Run: FifoDatamsgHandlerFunc newhandler -> arg: void * userdata by a given channel				
			//arg 0: channel
			//arg 1: arg0: handler, arg1: userdata
			FifoHandlerFunc fn = (FifoHandlerFunc)fifoFunc[channel][0];
			if((int)fn != 0){
				fn(fifoCheckDatamsgLength(channel), fifoFunc[channel][1]);
			}
		}
		break;
		
		case((uint32)WIFI_SYNC):{
			Wifi_Sync();
		}
		break;
	}
}
