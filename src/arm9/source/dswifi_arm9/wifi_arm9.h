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
#define WIFIINIT_OPTION_USEHEAP_128    0x0000
#define WIFIINIT_OPTION_USEHEAP_64     0x1000
#define WIFIINIT_OPTION_USEHEAP_256    0x2000
#define WIFIINIT_OPTION_USEHEAP_512    0x3000
#define WIFIINIT_OPTION_USECUSTOMALLOC 0x4000
#define WIFIINIT_OPTION_HEAPMASK       0xF000


#ifdef WIFI_USE_TCP_SGIP

// Wifi Packet Handler function: (int packetID, int packetlength) - packetID is only valid while the called function is executing.
// call Wifi_RxRawReadPacket while in the packet handler function, to retreive the data to a local buffer.
typedef void (*WifiPacketHandler)(int, int);

// Wifi Sync Handler function: Callback function that is called when the arm7 needs to be told to synchronize with new fifo data.
// If this callback is used (see Wifi_SetSyncHandler()), it should send a message via the fifo to the arm7, which will call Wifi_Sync() on arm7.
typedef void (*WifiSyncHandler)();
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern Wifi_MainStruct Wifi_Data_Struct;
extern volatile Wifi_MainStruct * WifiData;

extern WifiSyncHandler synchandler;

extern void Wifi_CopyMacAddr(volatile void * dest, volatile void * src);
extern int Wifi_CmpMacAddr(volatile void * mac1, volatile void * mac2);

extern unsigned long Wifi_Init(int initflags);
extern bool Wifi_InitDefault(bool useFirmwareSettings);
extern int Wifi_CheckInit();

extern int Wifi_RawTxFrame(uint16 datalen, uint16 rate, uint16 * data);
extern void Wifi_SetSyncHandler(WifiSyncHandler wshfunc);
extern void Wifi_RawSetPacketHandler(WifiPacketHandler wphfunc);
extern int Wifi_RxRawReadPacket(sint32 packetID, sint32 readlength, uint16 * data);

extern void Wifi_DisableWifi();
extern void Wifi_EnableWifi();
extern void Wifi_SetPromiscuousMode(int enable);
extern void Wifi_ScanMode();
extern void Wifi_SetChannel(int channel);

extern int Wifi_GetNumAP();
extern int Wifi_GetAPData(int apnum, Wifi_AccessPoint * apdata);
extern int Wifi_FindMatchingAP(int numaps, Wifi_AccessPoint * apdata, Wifi_AccessPoint * match_dest);
extern int Wifi_ConnectAP(Wifi_AccessPoint * apdata, int wepmode, int wepkeyid, uint8 * wepkey);
extern void Wifi_AutoConnect();

extern int Wifi_AssocStatus();
extern int Wifi_DisconnectAP();
extern int Wifi_GetData(int datatype, int bufferlen, uint8 * buffer);


extern void Wifi_Update();
extern void Wifi_Sync();


#ifdef WIFI_USE_TCP_SGIP
extern void Wifi_Timer(int num_ms);
extern void Wifi_SetIP(uint32 IPaddr, uint32 gateway, uint32 subnetmask, uint32 dns1, uint32 dns2);
extern uint32 Wifi_GetIP();
#endif

extern uint32 Wifi_TxBufferWordsAvailable();
extern void Wifi_TxBufferWrite(sint32 start, sint32 len, uint16 * data);

extern void Timer_50ms(void);
extern sgIP_Hub_HWInterface * wifi_hw;
extern bool WNifi_InitSafeDefault(int DSWNIFI_MODE);
#ifdef __cplusplus
}
#endif

#endif