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

//DSWNIFI Library revision: 1.x

#ifndef __dswnifi_lib_h__
#define __dswnifi_lib_h__

#include "common_shared.h"
#include "wifi_shared.h"
#include "clock.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"

#include "bios.h"
#include "InterruptsARMCores_h.h"
#include "dswnifi_lib.h"
#include "netdb.h"

#ifdef ARM9

#include "wifi_arm9.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "toolchain_utils.h"
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
#define frame_header_size 		(arm9_header_framesize + arm7_header_framesize)	

#define CRC_CRC_STAGE 		0x81
#define CRC_OK_SAYS_HOST	0x88

//WIFI UDP defs
#define UDP_PORT 8888				//used for UDP Server - NDS Companion connecting
//used for UDP transfers between NDS Multi mode
#define NDSMULTI_UDP_PORT_HOST 8889			//host 	listener - listener is local - sender is multi IP NDS
#define NDSMULTI_UDP_PORT_GUEST 8890		//guest listener - 

//WIFI TCP defs:	7777 to 7788, 8080, 8777, 9777, 27900, 42292
#define TCP_PORT 7777		//used for TCP Server - NDS Companion connecting
#define NDSMULTI_TCP_PORT_HOST 7778			//host 	listener - listener is local - sender is multi IP NDS
#define NDSMULTI_TCP_PORT_GUEST 7779		//guest listener - 

//process status
#define proc_idle (sint32)(0)
#define proc_connect (sint32)(1)
#define proc_execution (sint32)(2)
#define proc_shutdown (sint32)(3)

//coto: nifi & wifi support. todo: test in order
#define dswifi_tcpnifimode (sint32)(4)	//TCP
#define dswifi_udpnifimode (sint32)(5)	//UDP Nifi
#define dswifi_localnifimode (sint32)(6)	//Raw Network Packet Nifi
#define dswifi_idlemode (sint32)(7)	//Idle

//special udp nifi/wifi mode
#define ds_multi_notrunning (sint32)(8)
#define ds_searching_for_multi_servernotaware (sint32)(9)	//"srvaware" sends back other NDS info (IP,PORT), then we open PORT socket, and bind it to the new IP, then set flag to ds_wait_for_multi, we listen to new port and close the old one here

#define ds_netplay_host_servercheck (sint32)(10)		//ds are binded at this point but we need to know if both are connected each other
#define ds_netplay_guest_servercheck (sint32)(11)

#define ds_netplay_host (sint32)(12)
#define ds_netplay_guest (sint32)(13)

//struct that spinlocks a current request
typedef struct {
    
	//dswifi socket @ PORT 8888
    struct sockaddr_in sain_UDP_PORT;	//UDP: Listener sockaddr_in
	struct sockaddr_in server_addr;		//UDP Sender sockaddr_in: Desktop Server UDP companion Sender
    int socket_id__multi_notconnected;	//UDP/TCP: Listener FD. For handshake with server so both DS can connect each other.
    
	//DS is server here (this DS)
	struct sockaddr_in sain_listener;	//UDP: Unused, TCP: DS sockaddr_in server structure
	int socket_multi_listener;			//TCP: DS Server Listener. For handshake with server so both DS can connect each other/ DS-DS Multiplay
    
	//The other DS this DS is connected to
	struct sockaddr_in sain_sender;	//UDP/TCP: Client DS sockaddr_in assigned by the server
	int socket_multi_sender;		//UDP/TCP: Client DS File Descriptor assigned by the server to send messages to.
    
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
typedef struct dsnwifisrvStr {
//---------------------------------------------------------------------------------
	sint32 dsnwifisrv_mode;	//dswifi_idlemode / dswifi_localnifimode / dswifi_udpnifimode / dswifi_tcpnifimode				//used by setMULTIMode() getMULTIMode()
	
	sint32	connectionStatus;	//proc_idle / proc_connect / proc_execution / proc_shutdown	//used by getConnectionStatus() setConnectionStatus()
	sint32 	dsnwifisrv_stat;	//MULTI: inter DS Connect status: ds_multi_notrunning / ds_searching_for_multi / (ds_multiplay): ds_netplay_host ds_netplay_guest
	
	bool dswifi_setup;	//false: not setup / true: setup already
	
	bool incoming_packet;	//when any of the above methods received a packet == true / no == false
}TdsnwifisrvStr;

extern TdsnwifisrvStr dswifiSrv;
extern client_http_handler client_http_handler_context;

#endif


#ifdef __cplusplus
extern "C"{
#endif

//These calls are implemented in TGDS layer

//NIFI Part
//DSWNIFI: NIFI
extern void Handler(int packetID, int readlength);
extern bool NiFiHandler(int packetID, int readlength, uint8 * data);
extern void initNiFi();
extern void Timer_10ms(void);

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
extern int Wifi_RawTxFrame_WIFI(uint8 datalen, uint8 * data);
extern int Wifi_RawTxFrame_NIFI(uint16 datalen, uint16 rate, uint16 * data);

extern void switch_dswnifi_mode(sint32 mode);
extern void setMULTIMode(sint32 flag);	//idle(dswifi_idlemode) / raw packet(dswifi_localnifimode) / UDP nifi(dswifi_udpnifimode) / TCP wifi(dswifi_wifimode)
extern sint32 getMULTIMode();			//idle(dswifi_idlemode) / raw packet(dswifi_localnifimode) / UDP nifi(dswifi_udpnifimode) / TCP wifi(dswifi_wifimode)
extern bool getWIFISetup();
extern void setConnectionStatus(sint32 flag);
extern void getConnectionStatus(sint32 flag);
extern struct frameBlock * FrameSenderUser;	//if !NULL, then must sendFrame. HandleSendUserspace(); generates this one
//the process that runs on vblank and ensures DS - DS Comms
extern sint32 doMULTIDaemon();
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

#ifdef __cplusplus
}
#endif
