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

//DSWNifi Library 1.3 (update: 15/02/2018)

#ifndef __dswnifi_lib_h__
#define __dswnifi_lib_h__

#include "ipcfifoTGDS.h"
#include "wifi_shared.h"
#include "clockTGDS.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"

#include "biosTGDS.h"
#include "InterruptsARMCores_h.h"
#include "dswnifi_lib.h"
#include "netdb.h"

#ifdef ARM9

#include "wifi_arm9.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "utilsTGDS.h"
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <socket.h>
#include <in.h>
#include <assert.h>

#endif
//NIFI defs
#define arm7_header_framesize 	(12 + 2)									//arm7 
#define arm9_header_framesize	(12 + 6)
#define frame_header_size 		(sint32)(arm9_header_framesize + arm7_header_framesize)		//32

#define CRC_CRC_STAGE 		0x81
#define CRC_OK_SAYS_HOST	0x88

//WIFI UDP defs
#define UDP_PORT (sint32)(8888)				//used for UDP Server - NDS Companion connecting
//used for UDP transfers between NDS Multi mode
#define NDSMULTI_UDP_PORT_HOST (sint32)(8889)			//host 	listener - listener is local - sender is multi IP NDS
#define NDSMULTI_UDP_PORT_GUEST (sint32)(8890)		//guest listener - 

//WIFI TCP defs:	7777 to 7788, 8080, 8777, 9777, 27900, 42292
#define TCP_PORT (sint32)(7777)		//used for TCP Server - NDS Companion connecting
#define NDSMULTI_TCP_PORT_HOST (sint32)(7778)			//host 	listener - listener is local - sender is multi IP NDS
#define NDSMULTI_TCP_PORT_GUEST (sint32)(7779)		//guest listener - 

//process status
#define proc_idle (sint32)(0)
#define proc_connect (sint32)(1)
#define proc_execution (sint32)(2)
#define proc_shutdown (sint32)(3)

//coto: nifi & wifi support.
#define dswifi_udpnifimode (sint32)(5)	//UDP Nifi
#define dswifi_localnifimode (sint32)(6)	//Raw Network Packet Nifi
#define dswifi_idlemode (sint32)(7)	//Idle
#define dswifi_gdbstubmode (sint32)(14)	//GDB Stub mode

//flag extension 
//connect stage
#define dswifi_udpnifimodeFailConnectStage (sint32)(18)
#define dswifi_localnifimodeFailConnectStage (sint32)(19)
//execution stage
#define dswifi_udpnifimodeFailExecutionStage (sint32)(20)
#define dswifi_localnifimodeFailExecutionStage (sint32)(21)


//special udp nifi/wifi mode
#define ds_multi_notrunning (sint32)(8)
#define ds_searching_for_multi_servernotaware (sint32)(9)	//"srvaware" sends back other NDS info (IP,PORT), then we open PORT socket, and bind it to the new IP, then set flag to ds_wait_for_multi, we listen to new port and close the old one here

#define ds_netplay_host_servercheck (sint32)(10)		//ds are binded at this point but we need to know if both are connected each other
#define ds_netplay_guest_servercheck (sint32)(11)

#define ds_netplay_host (sint32)(12)
#define ds_netplay_guest (sint32)(13)

//Standard Frame Size
#define frameDSsize (sint32)((256)+sizeof(volatile uint16))	//256 bytes

//remoteStubMain retcodes
#define remoteStubMainWIFINotConnected (sint32)(-1)	
#define remoteStubMainWIFIConnectedNoGDB (sint32)(16)	//Wifi alone connected, GDB not running
#define remoteStubMainWIFIConnectedGDBRunning (sint32)(17)	//Wifi & GDB running
#define remoteStubMainWIFIConnectedGDBDisconnected (sint32)(18)	//Wifi running, GDB disconnected


typedef struct {
	//dswifi socket @ PORT 8888
    struct sockaddr_in sain_UDP_PORT;	//UDP: Listener sockaddr_in
	struct sockaddr_in server_addr;		//UDP Sender sockaddr_in: Desktop Server UDP companion Sender
    int socket_id__multi_notconnected;	//UDP/TCP: Listener FD. For handshake with server so both DS can connect each other. sendto uses for sending to PC
    
	//(this)DS Server Companion Listener: TCP 7777
	struct sockaddr_in sain_listener;	//UDP: Unused, TCP: DS sockaddr_in Server Companion Listen Socket: TCP 7777
	int socket_multi_listener;			//TCP: DS Server Listener. For handshake with server so both DS can connect each other/ DS-DS Multiplay
    
	//(The other) DS Multi: TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	struct sockaddr_in sain_sender;	//UDP/TCP: Client DS sockaddr_in assigned by the server
	int socket_multi_sender;		//UDP/TCP: Client DS File Descriptor assigned by the server to send messages to.
    
	//(this)DS Server Multi: TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	struct sockaddr_in sain_listenerNetplay;	//UDP: Unused, TCP: (this)DS Server Multi sockaddr_in TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	int socket_multi_listenerNetplay;			//UDP: Unused, TCP: (this)DS Server Multi SocketDescriptor TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	
	//host socket entry
    struct hostent myhost;
    
    bool wifi_enabled;

} client_http_handler;


//returned by HandleSendUserspace. Converts the user buffer and size into a struct the ToolchainGenericDS library understands.
struct frameBlock{
    uint8 * framebuffer;
	sint32	frameSize;
};

//shared memory cant use #ifdef ARMX since corrupts both definition sides for each ARM Core
//---------------------------------------------------------------------------------
struct dsnwifisrvStr {
//---------------------------------------------------------------------------------
	sint32 dsnwifisrv_mode;	//dswifi_idlemode / dswifi_localnifimode / dswifi_udpnifimode / dswifi_gdbstubmode		//used by setMULTIMode() getMULTIMode()
	sint32	connectionStatus;	//proc_idle / proc_connect / proc_execution / proc_shutdown	//used by getConnectionStatus() setConnectionStatus()
	sint32 	dsnwifisrv_stat;	//MULTI: inter DS Connect status: ds_multi_notrunning / ds_searching_for_multi / (ds_multiplay): ds_netplay_host ds_netplay_guest
	bool dswifi_setup;	//false: not setup / true: setup already	//used by getWIFISetup() / setWIFISetup()
	bool incoming_packet;	//when any of the above methods received a packet == true / no == false
	bool GDBStubEnable;	
};

extern struct dsnwifisrvStr dswifiSrv;
extern client_http_handler client_http_handler_context;
  
//GDB Stub part

/*
#define debuggerReadMemory(addr) \
if(	\
		((u32)addr > (u32)0x00000000)	||	\
		((u32)addr < (u32)(0x00000000+16*1024))	||	\
		((u32)addr > (u32)0x01000000)	||	\
		((u32)addr < (u32)(0x01000000+32*1024))	||	\
		((u32)addr > (u32)0x0b000000)	||	\
		((u32)addr < (u32)(0x0b000000+16*1024))	||	\
		((u32)addr > (u32)0x02000000)	||	\
		((u32)addr < (u32)(0x02000000+4*1024*1024))	||	\
		((u32)addr > (u32)0x03000000)	||	\
		((u32)addr < (u32)(0x03000000+32*1024))	||	\
		((u32)addr > (u32)0x04000000)	||	\
		((u32)addr < (u32)(0x04000000+32*1024))	||	\
		((u32)addr > (u32)0x05000000)	||	\
		((u32)addr < (u32)(0x05000000+64*1024))	||	\
		((u32)addr > (u32)0x06000000)	||	\
		((u32)addr < (u32)(0x06000000+512*1024))	||	\
		((u32)addr > (u32)0x08000000)	||	\
		((u32)addr < (u32)(0x08000000+32*1024*1024))\
	){	\
		(*(u32*)addr)	\
	}	\
	else{	\
		(0xff)	\
	}
	
#define debuggerReadHalfWord(addr) \
if(	\
		((u32)addr > (u32)0x00000000)	||	\
		((u32)addr < (u32)(0x00000000+16*1024))	||	\
		((u32)addr > (u32)0x01000000)	||	\
		((u32)addr < (u32)(0x01000000+32*1024))	||	\
		((u32)addr > (u32)0x0b000000)	||	\
		((u32)addr < (u32)(0x0b000000+16*1024))	||	\
		((u32)addr > (u32)0x02000000)	||	\
		((u32)addr < (u32)(0x02000000+4*1024*1024))	||	\
		((u32)addr > (u32)0x03000000)	||	\
		((u32)addr < (u32)(0x03000000+32*1024))	||	\
		((u32)addr > (u32)0x04000000)	||	\
		((u32)addr < (u32)(0x04000000+32*1024))	||	\
		((u32)addr > (u32)0x05000000)	||	\
		((u32)addr < (u32)(0x05000000+64*1024))	||	\
		((u32)addr > (u32)0x06000000)	||	\
		((u32)addr < (u32)(0x06000000+512*1024))	||	\
		((u32)addr > (u32)0x08000000)	||	\
		((u32)addr < (u32)(0x08000000+32*1024*1024))\
	){	\
		(*(u16*)addr)	\
	}	\
	else{	\
		(0xff)	\
	}
	
#define debuggerReadByte(addr)	\
	if(	\
		((u32)addr > (u32)0x00000000)	||	\
		((u32)addr < (u32)(0x00000000+16*1024))	||	\
		((u32)addr > (u32)0x01000000)	||	\
		((u32)addr < (u32)(0x01000000+32*1024))	||	\
		((u32)addr > (u32)0x0b000000)	||	\
		((u32)addr < (u32)(0x0b000000+16*1024))	||	\
		((u32)addr > (u32)0x02000000)	||	\
		((u32)addr < (u32)(0x02000000+4*1024*1024))	||	\
		((u32)addr > (u32)0x03000000)	||	\
		((u32)addr < (u32)(0x03000000+32*1024))	||	\
		((u32)addr > (u32)0x04000000)	||	\
		((u32)addr < (u32)(0x04000000+32*1024))	||	\
		((u32)addr > (u32)0x05000000)	||	\
		((u32)addr < (u32)(0x05000000+64*1024))	||	\
		((u32)addr > (u32)0x06000000)	||	\
		((u32)addr < (u32)(0x06000000+512*1024))	||	\
		((u32)addr > (u32)0x08000000)	||	\
		((u32)addr < (u32)(0x08000000+32*1024*1024))\
	){	\
		(*(u8*)addr)	\
	}	\
	else{	\
		(0xff)	\
	}
*/
#define debuggerWriteMemory(addr, value) \
  *(u32*)addr = (value)

#define debuggerWriteHalfWord(addr, value) \
  *(u16*)addr = (value)

#define debuggerWriteByte(addr, value) \
  *(u8*)addr = (value)

#define InternalRAM ((u8*)0x03000000)
#define WorkRAM ((u8*)0x02000000)


#endif


#ifdef __cplusplus
extern "C"{
#endif

//These calls are implemented in TGDS layer
#ifdef ARM9

//NIFI Part
//DSWNIFI: NIFI
extern void Handler(int packetID, int readlength);
extern void initNiFi();
extern void Timer_10ms(void);

extern bool nifiFrame;

//DSWNIFI: nifi buffer IO
extern volatile uint8	 data[4096];		//data[32] + is recv TX'd frame nfdata[128]
extern volatile uint8	 nfdata[128];	//sender frame, recv as data[4096]

//DSWNIFI: message for nifi beacons
extern volatile const uint8 nifitoken[32];
extern volatile const uint8 nificonnect[32];
extern volatile uint8 nificrc[32];

//DSWNIFI: WIFI specific
extern int nifi_stat;
extern int nifi_cmd;
extern int nifi_keys;		//holds the keys for players.
extern int nifi_keys_sync;	//(guestnifikeys & hostnifikeys)

extern int plykeys1;		//player1
extern int plykeys2;		//player2
extern int guest_framecount;	//used by the guest for syncing.
extern int host_framecount;		//emulator framecount:host
extern int guest_framecount;	//emulator framecount:guest
extern int host_vcount;		//host generated REG_VCOUNT
extern int guest_vcount;		//guest generated REG_VCOUNT
extern int topvalue(int a,int b);
extern int bottomvalue(int a,int b);
extern int getintdiff(int a,int b);

//TCP UDP DSWNIFI Part
extern int Wifi_RawTxFrame_WIFI(sint32 datalen, uint8 * data);
extern int Wifi_RawTxFrame_NIFI(sint32 datalen, uint16 rate, uint16 * data);

extern bool switch_dswnifi_mode(sint32 mode);
extern void setMULTIMode(sint32 flag);	//idle(dswifi_idlemode) / raw packet(dswifi_localnifimode) / UDP nifi(dswifi_udpnifimode) / TCP wifi(dswifi_wifimode)
extern sint32 getMULTIMode();			//idle(dswifi_idlemode) / raw packet(dswifi_localnifimode) / UDP nifi(dswifi_udpnifimode) / TCP wifi(dswifi_wifimode)
extern bool getWIFISetup();
extern bool setWIFISetup(bool flag);
extern void setConnectionStatus(sint32 flag);
extern sint32	getConnectionStatus();
extern struct frameBlock * FrameSenderUser;	//if !NULL, then must sendFrame. HandleSendUserspace(); generates this one

//the process that runs on vblank and ensures DS - DS Comms
//code that runs from ITCM
extern sint32 doMULTIDaemonStage1();
//code can't run from ITCM
extern sint32 doMULTIDaemonStage2(sint32 ThisConnectionStatus);

extern int port;
extern struct frameBlock FrameSenderBlock;	//used by the user sender process, must be valid so the ToolchainGenericDS library sends proper frame data.
extern struct frameBlock FrameRecvBlock;	//used by the user receiver process, can be NULL if no data frame was received.

//frame receiver implementation, has all receiver-like modes here. Returns true if correct frame received from TCP/UDP
extern struct frameBlock * 	receiveDSWNIFIFrame(uint8 * databuf_src,int frameSizeRecv);	//framesize is calculated inside (crc over udp requires framesize previously to here calculated anyway)
extern bool sendDSWNIFIFame(struct frameBlock * frameInst);
extern sint8* server_ip;

//Send a frame to the other connected DS
//example: 
//if(!FrameSenderUser){
//				FrameSenderUser = HandleSendUserspace((uint8*)nfdata,sizeof(nfdata)-sizeof(volatile uint16));	//make room for crc16 frame
//}
extern struct frameBlock * HandleSendUserspace(uint8 * databuf_src, int bufsize);

//userCode must override, provide these functions.

//As long you define this ReceiveHandler, everytime the outter connected DS to this DS sends a packet, it will be received here.
extern __attribute__((weak))	void HandleRecvUserspace(struct frameBlock * frameBlockRecv);	//called by receiveDSWNIFIFrame(); when a frame is valid
//implementation defined. can map a buffer/shared object between DS or keymaps
extern __attribute__((weak))	bool do_multi(struct frameBlock * frameBlockRecv);

extern bool sentReq;
extern sint32 LastDSWnifiMode;
extern struct dsnwifisrvStr * getDSWNIFIStr();
extern bool connectDSWIFIAP(bool WFC_CONNECTION,bool usewifiAP);

//GDB stub
extern bool gdbNdsStart();
extern int remotePort;
extern int remoteSignal;
extern int remoteSocket;
extern int remoteListenSocket;
extern bool remoteConnected;
extern bool remoteResumed;
extern int reconnectCount;

extern int remoteTcpSend(char *data, int len);
extern int remoteTcpRecv(char *data, int len);
extern bool remoteTcpInit();
extern void remoteTcpCleanUp();
extern int remotePipeSend(char *data, int len);
extern int remotePipeRecv(char *data, int len);
extern bool remotePipeInit();
extern void remotePipeCleanUp();
extern void remoteSetPort(int port);
extern void remoteSetProtocol(int p);
extern void remoteInit();
extern void remotePutPacket(char *packet);
extern void remoteOutput(char *s, u32 addr);
extern void remoteSendSignal();
extern void remoteSendStatus();
extern void remoteBinaryWrite(char *p);
extern void remoteMemoryWrite(char *p);
extern void remoteMemoryRead(char *p);
extern void remoteStepOverRange(char *p);
extern void remoteWriteWatch(char *p, bool active);
extern void remoteReadRegisters(char *p);
extern void remoteWriteRegister(char *p);
extern sint32 remoteStubMain();
extern void remoteStubSignal(int sig, int number);
extern void remoteCleanUp();

extern u8 debuggerReadByte(u32 addr);
extern u16 debuggerReadHalfWord(u32 addr);
extern u32 debuggerReadMemory(u32 addr);

#endif

#ifdef __cplusplus
}
#endif
