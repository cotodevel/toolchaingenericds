// DS Wifi interface code
// Copyright (C) 2005-2006 Stephen Stair - sgstair@akkit.org - http://www.akkit.org
// wifi_arm9.c - arm9 wifi support header
/****************************************************************************** 
DSWifi Lib and test materials are licenced under the MIT open source licence:
Copyright (c) 2005-2006 Stephen Stair

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/


#ifndef WIFI_ARM9_H
#define WIFI_ARM9_H

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "wifi_shared.h"
#include "sgIP_Hub.h"



// default option is to use 128k heap
#define WIFIINIT_OPTION_USEHEAP_64     (int)(0x00000000)
#define WIFIINIT_OPTION_USEHEAP_96     (int)(0x10000000)
#define WIFIINIT_OPTION_USEHEAP_128    (int)(0x20000000)
#define WIFIINIT_OPTION_USEHEAP_256    (int)(0x30000000)
#define WIFIINIT_OPTION_USEHEAP_512    (int)(0x40000000)
#define WIFIINIT_OPTION_USECUSTOMALLOC (int)(0x50000000)
#define WIFIINIT_OPTION_HEAPMASK       (int)(0xF0000000)

#ifdef WIFI_USE_TCP_SGIP

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
};
#endif


#endif

// user code uses members of the WIFIGETDATA structure in calling Wifi_GetData to retreive miscellaneous odd information
enum WIFIGETDATA {
	WIFIGETDATA_MACADDRESS,			// MACADDRESS: returns data in the buffer, requires at least 6 bytes
	WIFIGETDATA_NUMWFCAPS,			// NUM WFC APS: returns number between 0 and 3, doesn't use buffer.

	MAX_WIFIGETDATA
};


// Wifi Packet Handler function: (int packetID, int packetlength) - packetID is only valid while the called function is executing.
// call Wifi_RxRawReadPacket while in the packet handler function, to retreive the data to a local buffer.
typedef void (*WifiPacketHandler)(int, int);

// Wifi Sync Handler function: Callback function that is called when the arm7 needs to be told to synchronize with new fifo data.
// If this callback is used (see Wifi_SetSyncHandler()), it should send a message via the fifo to the arm7, which will call Wifi_Sync() on arm7.
typedef void (*WifiSyncHandler)();


#ifdef __cplusplus
extern "C" {
#endif

extern volatile Wifi_MainStruct * WifiData;

extern WifiSyncHandler synchandler;
extern void Wifi_CopyMacAddr(volatile void * dest, volatile void * src);
extern int Wifi_CmpMacAddr(volatile void * mac1, volatile void * mac2);

extern unsigned long Wifi_Init(int initflags);
extern bool Wifi_InitDefault(bool useFirmwareSettings);
extern int Wifi_CheckInit();

extern int Wifi_RawTxFrame(u16 datalen, u16 rate, u16 * data);
extern void Wifi_SetSyncHandler(WifiSyncHandler wshfunc);
extern void Wifi_RawSetPacketHandler(WifiPacketHandler wphfunc);
extern int Wifi_RxRawReadPacket(s32 packetID, s32 readlength, u16 * data);

extern void Wifi_DisableWifi();
extern void Wifi_EnableWifi();
extern void Wifi_SetPromiscuousMode(int enable);
extern void Wifi_ScanMode();
extern void Wifi_SetChannel(int channel);

extern int Wifi_GetNumAP();

// Wifi_GetAPData: Grabs data from internal structures for user code (always succeeds)
//  int apnum:					the 0-based index of the access point record to fetch
//  Wifi_AccessPoint * apdata:	Pointer to the location where the retrieved data should be stored
extern int Wifi_GetAPData(int apnum, Wifi_AccessPoint * apdata);

// Wifi_FindMatchingAP: determines whether various APs exist in the local area. You provide a
//   list of APs, and it will return the index of the first one in the list that can be found
//   in the internal list of APs that are being tracked
//  int numaps:					number of records in the list
//  Wifi_AccessPoint * apdata:	pointer to an array of structures with information about the APs to find
//  Wifi_AccessPoint * match_dest:	OPTIONAL pointer to a record to receive the matching AP record.
//  Returns:					-1 for none found, or a positive/zero integer index into the array
extern int Wifi_FindMatchingAP(int numaps, Wifi_AccessPoint * apdata, Wifi_AccessPoint * match_dest);

// Wifi_ConnectAP: Connect to an access point
//  Wifi_AccessPoint * apdata:	basic data on the AP
//  int wepmode:				indicates whether wep is used, and what kind
//  int wepkeyid:				indicates which wep key ID to use for transmitting
//  unsigned char * wepkey:		the wep key, to be used in all 4 key slots (should make this more flexible in the future)
//  Returns:					0 for ok, -1 for error with input data
extern int Wifi_ConnectAP(Wifi_AccessPoint * apdata, int wepmode, int wepkeyid, unsigned char * wepkey);

extern void Wifi_AutoConnect();

extern int Wifi_AssocStatus();
extern int Wifi_DisconnectAP();
extern int Wifi_GetData(int datatype, int bufferlen, unsigned char * buffer);


extern void Wifi_Update();
extern void Wifi_Sync();


#ifdef WIFI_USE_TCP_SGIP
extern void Wifi_Timer(int num_ms);
extern void Wifi_SetIP(u32 IPaddr, u32 gateway, u32 subnetmask, u32 dns1, u32 dns2);
extern u32 Wifi_GetIP();

#endif

extern uint32 Wifi_TxBufferWordsAvailable();
extern void Wifi_TxBufferWrite(sint32 start, sint32 len, uint16 * data);

extern void Timer_50ms(void);
extern sgIP_Hub_HWInterface * wifi_hw;
extern bool WNifi_InitSafeDefault(int DSWNIFI_MODE);

extern u32 getRandomSeed();

extern Wifi_AccessPoint * wifi_connect_point;

extern Wifi_MainStruct wifiSharedContext;
extern Wifi_AccessPoint wifiAPContext;

#ifdef __cplusplus
}
#endif


#endif
