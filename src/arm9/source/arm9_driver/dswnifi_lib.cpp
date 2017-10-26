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

#include "common_shared.h"
#include "wifi_shared.h"
#include "clock.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"

#include "bios.h"
#include "InterruptsARMCores_h.h"
#include "dswnifi_lib.h"

#include "timer.h"

#ifdef ARM9

#include "wifi_arm9.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "sgIP_Config.h"

#include "toolchain_utils.h"
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <socket.h>
#include <in.h>
#include <assert.h>

#endif

TdsnwifisrvStr dswifiSrv;

///////////////////////////////////////////////////////////////////////////NIFI Part

//nifi_stat: 0 not ready, 1 act as a host and waiting, 2 act as a guest and waiting, 3 connecting, 4 connected, 5 host ready, 6 guest ready

int nifi_stat = 0;	//start as idle always
int nifi_cmd = 0;
int nifi_keys = 0;		//holds the keys for players. player1 included
int nifi_keys_sync;	//(guestnifikeys & hostnifikeys)

int plykeys1 = 0;		//player1
int plykeys2 = 0;		//player2

int host_vcount = 0;		//host generated REG_VCOUNT
int guest_vcount = 0;		//guest generated REG_VCOUNT

int host_framecount = 0;
int guest_framecount = 0;

//frames
//
//Read-Only
volatile	const uint8 nifitoken[32]		= {0xB2, 0xD1, 'n', 'i', 'f', 'i', 'd', 's'};
volatile 	const uint8 nificonnect[32]	= {0xB2, 0xD1, 'c', 'o', 'n', 'n', 'e', 'c', 't'};

//Read-Write
volatile 	uint8 nificrc[32]				= {0xB2, 0xD1, (uint8)CRC_CRC_STAGE, 0, 0, 0};
volatile 	uint8 data[4096];			//receiver frame, data + frameheader is recv TX'd frame nfdata[128]. Used by NIFI Mode
volatile 	uint8 nfdata[128]			= {0xB2, 0xD1, (uint8)CRC_OK_SAYS_HOST, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};	//sender frame, recv as data[4096], see above. all valid frames have CRC_OK_SAYS_HOST sent.

//all sender frames must be crc'd otherwise the receiver will discard them
//Handler that runs on DSWIFI timings, handle NIFI
//true 	== 	dswnifi frame
//false ==	invalid frame
bool NiFiHandler(int packetID, int readlength, uint8 * data){
	bool validFrame = false;
	
	//decide whether to put data in userbuffer and if frame is valid here
	struct frameBlock * frameHandled = receiveDSWNIFIFrame(data,readlength);
	if(frameHandled != NULL){
		//#1: nifi-handshake  
		//accept a valid NIFI frame (no crc nifi frame will be used)
		if((data[frame_header_size + 0] == nifitoken[0]) && (data[frame_header_size + 1] == nifitoken[1])){
		
			switch(nifi_stat) {
				case 0:{
					return validFrame;
				}
				break;
				case 1:			
					if(strncmp((const char *)(data + frame_header_size + 2), (const char *)(nifitoken + 2), 6) == 0) {	//token "nifids"
						validFrame = true;
						nifi_stat = 3;	
					}
					break;
				case 2:			
					if(strncmp((const char *)(data + frame_header_size + 2), (const char *)(nificonnect + 2), 7) == 0) {	//token "connect"
						validFrame = true;
						nifi_stat = 4;
					}
					break;
				case 3:
					if(data[frame_header_size + 2] == CRC_CRC_STAGE) {		//Check the CRC (cmd from MP). Make sure that both players are using the same game.
						/*
						int remotecrc = (data[frame_header_size + 3] | (data[frame_header_size + 4] << 8));
						if(debuginfo[17] == remotecrc) {	//ok. same game
							valid_nifi_frame = true;
							nifi_stat = 5;
							nifi_cmd |= MP_CONN;
							sendcmd((uint8*)&nfdata[0]);
							hideconsole();
							NES_reset();
							nifi_keys = 0;
							plykeys1 = 0;
							plykeys2 = 0;
							guest_framecount = 0;
							global_playcount = 0;
							joyflags &= ~AUTOFIRE;
							__af_st = __af_start;
							menu_game_reset();	//menu is closed.
						}
						else {		//bad crc. disconnect the comm.
							nifi_stat = 0;
							nifi_cmd &= ~MP_CONN;
							sendcmd((uint8*)&nfdata[0]);
						}
						*/
					}
					break;
				case 4:
					if(data[frame_header_size + 2] == CRC_OK_SAYS_HOST) {
						/*
						if(nifi_cmd & MP_CONN) {	//CRC ok, get ready for multi-play.
							valid_nifi_frame = true;
							nifi_stat = 6;
							hideconsole();
							NES_reset();
							nifi_keys = 0;
							plykeys1 = 0;
							plykeys2 = 0;
							guest_framecount = 0;
							global_playcount = 0;
							joyflags &= ~AUTOFIRE;
							__af_st = __af_start;
							menu_game_reset();	//menu is closed.
						}
						else {					//CRC error, the both sides should choose the some game.
							nifi_stat = 0;
						}
						*/
							
					}
					break;
			}
		}
		
		//#2: crc nifi-frame
		if((nifi_stat == 5) || (nifi_stat == 6)){ //validate frames only if we are past the nifi handshake
			int framesize = 0;
			
			if(nifi_stat == 5) {	//host receives from guest
				framesize = 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
			} 
			else if(nifi_stat == 6){//guest receives from host
				framesize = 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
			}
		}
		
		validFrame = true;
	}
	
	return validFrame;
}


void Handler(int packetID, int readlength)
{
	//coto
	switch(getMULTIMode()){
		case (dswifi_localnifimode):{
			Wifi_RxRawReadPacket(packetID, readlength, (unsigned short *)data);
			NiFiHandler(packetID, readlength, (uint8*)(&data[0]));	//recv packet
		}
		break;
		
		//DSWNIFI uses its own daemon to process input/output frames
		/*
		case (dswifi_udpnifimode):
		case (dswifi_tcpnifimode):
		{
			
		}
		break;
		*/
		
		
	}
}


void Timer_10ms(void) {
	Wifi_Timer(10);
}

void initNiFi()
{
	Wifi_InitDefault(false);
	Wifi_SetPromiscuousMode(1);
	//Wifi_EnableWifi();
	Wifi_RawSetPacketHandler(Handler);
	Wifi_SetChannel(10);

	if(1) {
		//for secial configuration for wifi
		DisableIrq(IRQ_TIMER3);
		//ori:irqSet(IRQ_TIMER3, Timer_10ms); // replace timer IRQ
		//irqSet(IRQ_TIMER3, Timer_50ms); // replace timer IRQ
		//ori: TIMER3_DATA = -(6553 / 5); // 6553.1 * 256 / 5 cycles = ~10ms;
		TIMERXDATA(3) = -6553; // 6553.1 * 256 cycles = ~50ms;
		TIMERXCNT(3) = 0x00C2; // enable, irq, 1/256 clock
		EnableIrq(IRQ_TIMER3);
	}
}




////////////////////////////////////////////////////////////////////////////TCP UDP DSWNIFI Part
client_http_handler client_http_handler_context;

sint8* server_ip = (sint8*)"192.168.43.220";

//these cant be in shared memory, gets stuck
int port = 8888; 	//gdb stub port
//SOCK_STREAM = TCP / SOCK_DGRAM = UDP
struct sockaddr_in stSockAddrServer;

int SocketFDLocal = -1;
int SocketFDServer = -1;

//below calls are internal, used by DSWNIFI library. Not for user code
__attribute__((section(".itcm")))
void sendDSWNIFIFame(uint32 * databuf_src,int sizetoSend)
{
	//coto: generate crc per nifi frame so we dont end up with corrupted data.
	volatile uint16 crc16_frame = swiCRC16	(	0xffff, //uint16 	crc,
		databuf_src,
		(sizetoSend)		//cant have this own crc here
	);
	
	*(uint16*)(databuf_src + sizetoSend)	= crc16_frame;
	sizetoSend = sizetoSend + sizeof(crc16_frame);
	
	switch(getMULTIMode()){
		case (dswifi_localnifimode):{
			Wifi_RawTxFrame_NIFI(sizetoSend , 0x0014, (unsigned short *)databuf_src);
		}
		break;
		case(dswifi_udpnifimode):
		case(dswifi_tcpnifimode):
		{
			Wifi_RawTxFrame_WIFI(sizetoSend , (uint8*)databuf_src);
		}
		break;
	}
}

struct frameBlock FrameSenderBlock;	//used by the user sender process, must be valid so the ToolchainGenericDS library sends proper frame data.
struct frameBlock FrameRecvBlock;	//used by the user receiver process, can be NULL if no data frame was received.

//reads raw packet (and raw read size) and defines if valid frame or not. 
//must be called from either localnifi handler or udp nifi/wifi on-receive-packet handler
__attribute__((section(".itcm")))
struct frameBlock * receiveDSWNIFIFrame(uint8 * databuf_src,int frameSizeRecv)	//the idea is that we append DSWNIFI Frame + extended frame. so copy the extra data to buffer
{
	struct frameBlock * frameRecvInst =  (struct frameBlock *)&FrameRecvBlock;
	
	//1: check the header localnifi appends, udp nifi/wifi does not append this header
	int frame_hdr_size = 0;	//nifi raw only frame has this header
	int framesize = 0;	//calculated frame size , different from frameSizeRecv
	switch(getMULTIMode()){
		case(dswifi_localnifimode):
		{
			frame_hdr_size = frame_header_size;	//localframe has this header
		}
		break;
		
		case(dswifi_udpnifimode):
		case(dswifi_tcpnifimode):
		{
			frame_hdr_size = 0;					//udp nifi frame has not this header
		}
	}
	//2: calculate frame size
	switch(dswifiSrv.dsnwifisrv_stat){
		//#last:connected!
		case(ds_netplay_host):{	//host receives from guest
			framesize = 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
		}break;
		
		case(ds_netplay_guest):{ //guest receives from host
			framesize = 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
		}break;
	}	
	
	//read crc from nifi frame so we dont end up with corrupted data.
	volatile uint16 crc16_recv_frame = (uint16)*(uint16*)(databuf_src + frame_hdr_size + framesize);
	
	//generate crc per nifi frame so we dont end up with corrupted data.
	volatile uint16 crc16_frame_gen = swiCRC16	(	0xffff, //uint16 	crc,
		(uint8*)(databuf_src + frame_hdr_size),
		(framesize)		//cant have this own crc here
		);
	
	//do crc calc here. if valid, set receivedValid = true; and copy contents to recvbufferuser
	
	//data is now nifi frame
	if(crc16_frame_gen == crc16_recv_frame){
		memset ((uint8*)frameRecvInst, 0, sizeof(struct frameBlock));
		frameRecvInst->framebuffer = (uint32*)databuf_src;
		frameRecvInst->frameSize = framesize;
		//valid dswnifi_frame
	}
	else{
		//invalid dswnifi_frame
		frameRecvInst = NULL;
	}
	
	return frameRecvInst;
}


//0 idle, 1 nifi, 2 dswnifi
void switch_dswnifi_mode(sint32 mode){
	
	//idle mode minimal setup
	if (mode == (sint32)dswifi_idlemode){
		dswifiSrv.dsnwifisrv_stat	= ds_multi_notrunning;
		setMULTIMode(dswifi_idlemode);
		dswifiSrv.dswifi_setup = false;
	}
	//Raw Network Packet Nifi
	else if (mode == (sint32)dswifi_localnifimode){
		//nifi
		dswifiSrv.dsnwifisrv_stat	= ds_searching_for_multi_servernotaware;
		setMULTIMode(dswifi_localnifimode);
		dswifiSrv.dswifi_setup = false;
	}
	//UDP Nifi/WIFI
	else if (
		(mode == (sint32)dswifi_udpnifimode)
		||
		(mode == (sint32)dswifi_tcpnifimode)
	){
		dswifiSrv.dsnwifisrv_stat = ds_searching_for_multi_servernotaware;
		setMULTIMode(mode);
		dswifiSrv.dswifi_setup = false;
	}
	
	
	// Idle
	if(getMULTIMode() == dswifi_idlemode){
		dswifiSrv.dswifi_setup = false;
		setConnectionStatus(proc_idle);
	}
	
	//set NIFI mode
	else if((getMULTIMode() == dswifi_localnifimode) && (dswifiSrv.dswifi_setup == false)){
		dswifiSrv.dswifi_setup = true;
		setConnectionStatus(proc_connect);
	}
	
	//set UDP/TCP DSWNIFI
	else if(
		(
		(getMULTIMode() == dswifi_udpnifimode)
		||
		(getMULTIMode() == dswifi_tcpnifimode)
		)
		&& (dswifiSrv.dswifi_setup == false)
	){
		if(Wifi_InitDefault(WFC_CONNECT) == true)
		{
			printf("connected: IP: %s",(char*)print_ip((uint32)Wifi_GetIP()));
			setConnectionStatus(proc_connect);
			dswifiSrv.dswifi_setup = true;
		}
		else{
			//Could not connect
			setConnectionStatus(proc_idle);
		}
	}
	
}

__attribute__((section(".itcm")))
void setMULTIMode(sint32 flag){
	dswifiSrv.dsnwifisrv_mode = (sint32)flag;
}

__attribute__((section(".itcm")))
sint32 getMULTIMode(){
	return (sint32)dswifiSrv.dsnwifisrv_mode;
}

__attribute__((section(".itcm")))
bool getWIFISetup(){
	return (bool)dswifiSrv.dswifi_setup;
}

__attribute__((section(".itcm")))
void setConnectionStatus(sint32 flag){
	dswifiSrv.connectionStatus = (sint32)flag;
}

__attribute__((section(".itcm")))
sint32 getConnectionStatus(){
	return (sint32)dswifiSrv.connectionStatus;
}


struct frameBlock * FrameSenderUser = NULL;

//ret: -1 not connected, 0 UDP NIFI ok, 1 TCP NIFI ok, 2 LOCAL NIFI ok (connect, execute and disconnect)
__attribute__((section(".itcm")))
sint32 doMULTIDaemon(){
	sint32 retDaemonCode = 0;
	
	switch(getConnectionStatus()){
		case (proc_idle):{
			//nothing to do for : LOCAL / UDP NIFI / TCP NIFI
			retDaemonCode = -1;
		}
		break;
		
		case (proc_connect):{
			
			//UDP NIFI
			if(getMULTIMode() == dswifi_udpnifimode){
				
				//DSWNIFI library uses this for UDP handshake for any IP that writes to our own IP @ PORT
				//opens port 32123 @ UDP (any IP inbound from sender)
				client_http_handler_context.socket_id__multi_notconnected=socket(AF_INET,SOCK_DGRAM,0);
				
				int i=1;
				i=ioctl(client_http_handler_context.socket_id__multi_notconnected,FIONBIO,&i); // set non-blocking port
				client_http_handler_context.sain_UDP_PORT.sin_family=AF_INET;
				client_http_handler_context.sain_UDP_PORT.sin_addr.s_addr=0;
				client_http_handler_context.sain_UDP_PORT.sin_port=htons((int)UDP_PORT);
				
				if(bind(client_http_handler_context.socket_id__multi_notconnected,(struct sockaddr *)&client_http_handler_context.sain_UDP_PORT,sizeof(client_http_handler_context.sain_UDP_PORT))) {
					sint8 buf[64];
					sprintf(buf,"binding ERROR ");
					printf((sint8 *)&buf[0]);
					close(client_http_handler_context.socket_id__multi_notconnected);
					retDaemonCode = -1;
					return retDaemonCode;
				}
				else{
					sint8 buf[64];
					sprintf(buf,"binding OK: port %d IP: %s ",(int)UDP_PORT, (sint8*)print_ip((uint32)Wifi_GetIP()));//(sint8*)print_ip((uint32)Wifi_GetIP()));
					printf((sint8 *)&buf[0]);
				}
				
				
				//for server IP / used for sending msges
				memset((sint8 *) &client_http_handler_context.server_addr, 0, sizeof(client_http_handler_context.server_addr));
				client_http_handler_context.server_addr.sin_family = AF_INET;
				client_http_handler_context.server_addr.sin_addr.s_addr = inet_addr(server_ip);//SERV_HOST_ADDR);
				client_http_handler_context.server_addr.sin_port = htons((int)UDP_PORT); //SERVER_PORT_ID);
				
				
				////Default UDP method for connecting to server, DSWNIFI library does not use this.
				/*
				memset(&stSockAddrClient, 0, sizeof(stSockAddrClient));
				SocketFDLocal = socket(AF_INET, SOCK_DGRAM, 0);
				if(-1 == SocketFDLocal)
				{
					return -1;
				}
				
				int enable = 1;
				if (setsockopt(SocketFDLocal, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
					
				}
				
				int i=1;
				i=ioctl(SocketFDLocal,FIONBIO,&i); // set non-blocking port
				
				
				stSockAddrClient.sin_family = AF_INET;
				stSockAddrClient.sin_port = htons(port);
				stSockAddrClient.sin_addr.s_addr = INADDR_ANY;	//local/any ip listen to desired port //inet_addr((sint8*)"192.168.43.220");
				
				if(bind(SocketFDLocal,(struct sockaddr *)&stSockAddrClient, sizeof(stSockAddrClient)))
				{
					return -1;
				}
				
				//Server Setup
				memset((uint8*)&stSockAddrServer, 0, sizeof(stSockAddrServer));
				
				SocketFDServer = socket(PF_INET, SOCK_DGRAM, 0);
				
				stSockAddrServer.sin_family = AF_INET;
				stSockAddrServer.sin_port = htons(port);
				stSockAddrServer.sin_addr.s_addr = inet_addr(server_ip);
				*/
				
				//no binding since we have no control of server port and we should not know it anyway
				setConnectionStatus(proc_execution);
				retDaemonCode = 0;
				return retDaemonCode;
			}
			//TCP NIFI
			if(getMULTIMode() == dswifi_tcpnifimode){
				SocketFDLocal = socket(AF_INET, SOCK_STREAM, 0);
				if(-1 == SocketFDLocal)
				{
					retDaemonCode = -1;
					return retDaemonCode;
				}
				int i=1;
				i=ioctl(SocketFDLocal,FIONBIO,&i); // set non-blocking port (otherwise emulator blocks)
				
				int optval = 1;
				setsockopt(SocketFDLocal, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
				//Server Setup
				memset((uint8*)&stSockAddrServer, 0, sizeof(stSockAddrServer));
				
				/* this is an Internet address */
				stSockAddrServer.sin_family = AF_INET;
				/* let the system figure out our IP address */
				stSockAddrServer.sin_addr.s_addr = INADDR_ANY;
				/* this is the port we will listen on */
				stSockAddrServer.sin_port = htons(port);
				//connect wont work since we want the DS to open TCP at 8888 port
				/*
				if (connect(SocketFDLocal,&stSockAddrServer,sizeof(stSockAddrServer)) < 0){
					printf("ERROR connecting");
					close(SocketFDLocal);
				}
				else{
					printf("OK connecting");
				}
				*/
				if(bind(SocketFDLocal,(struct sockaddr *)&stSockAddrServer, sizeof(stSockAddrServer)))
				{
					retDaemonCode = -1;
					return retDaemonCode;
				}
				
				listen(SocketFDLocal,5);	//DS Acts as server at desired port
				setConnectionStatus(proc_execution);
				
				retDaemonCode = 1;
				return retDaemonCode;
			}
			
			
			//LOCAL NIFI: runs on DSWIFI process
			if(getMULTIMode() == dswifi_localnifimode){
				initNiFi();
				setConnectionStatus(proc_execution);
				retDaemonCode = 2;
				return retDaemonCode;
			}
		}
		break;
		
		case (proc_execution):{
		
			//////////////////////////////////////////Handle Recv
			
			//UDP NIFI
			if(getMULTIMode() == dswifi_udpnifimode){
				
				/* works, default template but dswnifi use its own 
				int clilen = 0;
				int srvlen = 0;
				struct sockaddr_in cli_addr;
				
				clilen = sizeof(cli_addr);
				srvlen = sizeof(stSockAddrServer);
				memset((uint8*)&cli_addr, 0, clilen);
				volatile char incomingbuf[256];
				volatile char sendbuf[256];
				
				int read_size = 0;
				//Receive a message from client
				if( (read_size = recvfrom(SocketFDLocal, (uint8*)incomingbuf, sizeof(incomingbuf), 0, (struct sockaddr*)&cli_addr, (int*)&clilen)) > 0 )
				{
					//Handle Remote Procedure Commands
					sprintf((char*)sendbuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
					
					//Send reply data
					if( sendto(SocketFDLocal,(uint8*)sendbuf, sizeof(sendbuf),0,(struct sockaddr *)&stSockAddrServer,srvlen) < 0)
					{
						//printf("Send failed");
					}
					else{
						//printf("Send ok");
					}
					
					//only after we accepted we can close Server socket.
					//setConnectionStatus(proc_shutdown);
				}
				*/
				
				//UDP: (execute RPC from server and process frames)
				switch(dswifiSrv.dsnwifisrv_stat){
					//#1 DS is not connected, serve any upcoming commands related to non connected to multi
					//ds_searching_for_multi_servernotaware -> ds_wait_for_multi here
					case(ds_searching_for_multi_servernotaware):{
						
						//Server UDP Handler listener
						unsigned long available_ds_server = 0;
						ioctl (client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds_server);
						char incomingbuf[256] = {0};
						int datalen = 0;
						int sain_len = 0;
						struct sockaddr_in sender_server;
						sain_len=sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sain_len);
						char cmd[12] = {0};	//srv->ds command handler
						if(available_ds_server > 0){
							if( (datalen = recvfrom(client_http_handler_context.socket_id__multi_notconnected, (uint8*)incomingbuf, sizeof(incomingbuf), 0, (struct sockaddr *)&sender_server,(int*)&sain_len)) > 0 ){
								if(datalen>0) {
									//incomingbuf[datalen]=0;
									memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
								}
							}
						}
				
						//add frame receive here and detect if valid frame, if not, run the below
						
						//Server aware
						if(strncmp((const char *)cmd, (const char *)"srvaware", 8) == 0){
							
							//server send other NDS ip format: cmd-ip-multi_mode- (last "-" goes as well!)
							char **tokens;
							int count, i;
							//const char *str = "JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC";
							volatile char str[256] = {0};
							memcpy ( (uint8*)str, (uint8*)incomingbuf, 256);

							count = split ((const char*)str, '-', &tokens);
							for (i = 0; i < count; i++) {
								//then send back "dsaware"
								//char outgoingbuf[64];
								//sprintf(outgoingbuf,"%s",tokens[i]);
								//sendto(client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
							}
							
							//tokens[0];	//cmd
							//tokens[1];	//external NDS ip to connect
							//tokens[2];	//host or guest
							
							int host_mode = strncmp((const char*)tokens[2], (const char *)"host", 4); //host == 0
							int guest_mode = strncmp((const char*)tokens[2], (const char *)"guest", 5); //guest == 0
							
							client_http_handler_context.socket_multi_listener=socket(AF_INET,SOCK_DGRAM,0);
							int cmd=1;
							cmd=ioctl(client_http_handler_context.socket_multi_listener,FIONBIO,&cmd); // set non-blocking port
							
							client_http_handler_context.socket_multi_sender=socket(AF_INET,SOCK_DGRAM,0);
							
							int optval = 1, len;
							setsockopt(client_http_handler_context.socket_multi_sender, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval));
		
							int LISTENER_PORT 	=	0;
							int SENDER_PORT		=	0;
							if(host_mode == 0){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
								SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_GUEST;
							}
							else if(guest_mode == 0){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
								SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_HOST;
							}
							
							//bind conn to LOCAL ip-port listener
							memset((char *) &client_http_handler_context.sain_listener, 0, sizeof(client_http_handler_context.sain_listener));
							client_http_handler_context.sain_listener.sin_family = AF_INET;
							client_http_handler_context.sain_listener.sin_addr.s_addr=INADDR_ANY;	//local/any ip listen to desired port
							//int atoi ( const char * str );
							//int nds_multi_port = atoi((const char*)tokens[2]);
							client_http_handler_context.sain_listener.sin_port = htons(LISTENER_PORT); //nds_multi_port
							
							
							//NDS MULTI IP: No need to bind / sendto use
							memset((char *) &client_http_handler_context.sain_sender, 0, sizeof(client_http_handler_context.sain_sender));
							client_http_handler_context.sain_sender.sin_family = AF_INET;
							client_http_handler_context.sain_sender.sin_addr.s_addr = INADDR_BROADCAST;//((const char*)"191.161.23.11");// //ip was reversed 
							client_http_handler_context.sain_sender.sin_port = htons(SENDER_PORT); 
							
							struct sockaddr_in *addr_in2= (struct sockaddr_in *)&client_http_handler_context.sain_sender;
							char *IP_string_sender = inet_ntoa(addr_in2->sin_addr);
							
							//bind ThisIP(each DS network hardware) against the current DS UDP port
							if(bind(client_http_handler_context.socket_multi_listener,(struct sockaddr *)&client_http_handler_context.sain_listener,sizeof(client_http_handler_context.sain_listener))) {
								if(host_mode == 0){
									//printf("%s ","[host]binding error");
								}
								else if(guest_mode == 0){
									//printf("%s ","[guest]binding error");
								}
								
								close(client_http_handler_context.socket_multi_listener);
								retDaemonCode = -1;
								return retDaemonCode;
							}
							else{
								char buf[96] = {0};
								char id[16] = {0};
								//read IP from sock interface binded
								struct sockaddr_in *addr_in= (struct sockaddr_in *)&client_http_handler_context.sain_listener;	//0.0.0.0 == (char*)print_ip((uint32)Wifi_GetIP()) 
								char *IP_string = inet_ntoa(addr_in->sin_addr);
								
								if(host_mode == 0){
									sprintf(buf,"[host]binding OK MULTI: port [%d] IP: [%s]  ",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
									sprintf(id,"[host]");
									printf("%s",buf);
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_host_servercheck;
								}
								else if(guest_mode == 0){
									sprintf(buf,"[guest]binding OK MULTI: port [%d] IP: [%s]  ",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
									sprintf(id,"[guest]");
									printf("%s",buf);
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_guest_servercheck;
								}
								
								//note: bind UDPsender?: does not work with UDP Datagram socket format (UDP basically)
							}
							
						}
						
					}
					break;
					
					//servercheck phase acknow
					case(ds_netplay_host_servercheck):case(ds_netplay_guest_servercheck):{
						
						//Server UDP Handler listener
						unsigned long available_ds_server = 0;
						ioctl (client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds_server);
						
						char incomingbuf[256] = {0};
						int datalen;
						int sain_len;
						char cmd[12] = {0};	//srv->ds command handler
						struct sockaddr_in sender_server;
						sain_len=sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sain_len);
						
						if(available_ds_server > 0){
							datalen=recvfrom(client_http_handler_context.socket_id__multi_notconnected,(uint8*)incomingbuf,sizeof(incomingbuf),0,(struct sockaddr *)&sender_server,(int*)&sain_len);
							if(datalen>0) {
								//incomingbuf[datalen]=0;
								memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
							}
						}
						
						if(strncmp((const char *)cmd, (const char *)"dsconnect", 9) == 0){
							
							int LISTENER_PORT 	=	0;
							int SENDER_PORT		=	0;
							if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
								SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_GUEST;
							}
							else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
								SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_HOST;
							}
							
							if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){	
								clrscr();
								printf("//////////DSCONNECTED[HOST]-PORT:%d",LISTENER_PORT);
								dswifiSrv.dsnwifisrv_stat = ds_netplay_host;
								nifi_stat = 5;
							}
							else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
								clrscr();
								printf("//////////DSCONNECTED[GUEST]-PORT:%d",LISTENER_PORT);
								dswifiSrv.dsnwifisrv_stat = ds_netplay_guest;
								nifi_stat = 6;
							}
							
							close(client_http_handler_context.socket_id__multi_notconnected); //closer server socket to prevent problems when udp multiplayer
							
						}
					}
					break;
					
					//#last:connected!
					//logic: recv data(256byte buf) from port
					case(ds_netplay_host):case(ds_netplay_guest):{
						
						//DS-DS UDP Handler listener
						unsigned long available_ds_ds = 0;
						ioctl (client_http_handler_context.socket_multi_listener, FIONREAD, (uint8*)&available_ds_ds);
						
						int datalen2;
						uint8 inputbuf[256] = {0};
						int sain_len2;
						struct sockaddr_in sender_ds;
						sain_len2=sizeof(struct sockaddr_in);
						
						memset((uint8*)&sender_ds, 0, sain_len2);
						
						if(available_ds_ds > 0){
						
							datalen2=recvfrom(client_http_handler_context.socket_multi_listener,(uint8*)inputbuf,sizeof(inputbuf),0,(struct sockaddr *)&sender_ds,(int*)&sain_len2);
							if(datalen2>0) {
								//inputbuf[datalen2-1]=0;
								
								//decide whether to put data in userbuffer and if frame is valid here
								int recSize = 0;
								if (datalen2 > sizeof(inputbuf)){
									recSize = sizeof(inputbuf);
								}
								else{
									recSize = datalen2;
								}
								struct frameBlock * frameHandled = receiveDSWNIFIFrame((uint8 *)&inputbuf[0],recSize);
								if(frameHandled != NULL){
									//trigger the User Recv Process here
									HandleRecvUserspace((uint8*)frameHandled->framebuffer, frameHandled->frameSize);
								}
							}
						}
					}
					break;
				}
				
				//only after we accepted we can close Server socket.
				//setConnectionStatus(proc_shutdown);
				
				retDaemonCode = 0;
			}
			
			//TCP NIFI
			if(getMULTIMode() == dswifi_tcpnifimode){
				
				int connectedSD = -1;
				int read_size = 0;
				struct sockaddr_in stSockAddrClient;
				struct timeval timeout;
				timeout.tv_sec = 0;
				timeout.tv_usec = 0 * 1000;
				int stSockAddrClientSize = sizeof(struct sockaddr_in);
				memset((uint8*)&stSockAddrClient, 0, sizeof(struct sockaddr_in));
				
				volatile uint8 sendbuf[512];
				volatile uint8 recvbuf[512];

				if(!(-1 == SocketFDLocal))
				{
					//TCP
					if ((connectedSD = accept(SocketFDLocal, (struct sockaddr *)(&stSockAddrClient), &stSockAddrClientSize)) == -1)
					{ 
						//perror("accept"); exit(1); 
					}
					else{
						/* Read from the socket */
						//Receive a message from client (only data > 0 ), socket will be closed anyway
						while( (read_size = recv(connectedSD , (uint8*)recvbuf , sizeof(recvbuf) , 0)) > 0 )
						{
							
						}
						recvbuf[read_size] = '\0';
						
						//Handle Remote Procedure Commands
						//sprintf((char*)sendbuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
						
						//Send some data
						if( send(connectedSD , (uint8*)sendbuf , sizeof(sendbuf) , 0) > 0)
						{
							//Send ok
						}
						else{
							//Send error
						}
						
						//only after we accepted we can close Server socket.
						setConnectionStatus(proc_shutdown);
					}
					close(connectedSD);
				}
		
				retDaemonCode = 1;
			}
			
			//Local Nifi: runs from within the DSWIFI frame handler itself so ignored here.
			if(getMULTIMode() == dswifi_localnifimode){
				
				retDaemonCode = 2;
			}
			
			///////////////////////////////////////Handle Send UserCode, if the user used the following code:
			//struct frameBlock * FrameSenderUser = HandleSendUserspace((uint32*)&nfdata[0],sizeof(nfdata));	//use the nfdata as send buffer // struct frameBlock * FrameSenderInst is now used to detect if pending send frame or not
			//then FrameSenderUser should be not NULL, send the packet here now. Packet must be NOT called from a function.
			
			///////////////////////////////////////Handle Send Library
			if(FrameSenderUser){
				sendDSWNIFIFame((uint32*)FrameSenderUser->framebuffer,FrameSenderUser->frameSize);
				FrameSenderUser = NULL;
			}
			
			return retDaemonCode;
		}
		break;
		
		//shutdown
		case (proc_shutdown):{
			close(SocketFDLocal);
			setMULTIMode(proc_connect);
			retDaemonCode = 0;
			return retDaemonCode;
		}
		break;
		
	}
	
}






// datalen = size of packet from beginning of 802.11 header to end, but not including CRC.
__attribute__((section(".itcm")))
int Wifi_RawTxFrame_NIFI(uint16 datalen, uint16 rate, uint16 * data) {
	int base,framelen, hdrlen, writelen;
	int copytotal, copyexpect;
	
	uint16 framehdr[arm9_header_framesize + 2];	//+2 for arm7 header section
	framelen=datalen + 8 + (WifiData->wepmode7 ? 4 : 0);

	if(framelen + 40>Wifi_TxBufferWordsAvailable()*2) { // error, can't send this much!
		SGIP_DEBUG_MESSAGE(("Transmit:err_space"));
		return -1; //?
	}

	framehdr[0]=0;
	framehdr[1]=0;
	framehdr[2]=0;
	framehdr[3]=0;
	framehdr[4]=0; // rate, will be filled in by the arm7.
	hdrlen= arm9_header_framesize; //18;
	framehdr[6]=0x0208;
	framehdr[7]=0;

	// MACs.
	memset((uint8*)(framehdr + 8), 0xFF, 18);

	if(WifiData->wepmode7)
	{
		framehdr[6] |=0x4000;
		hdrlen=20;
	}
	framehdr[17] = 0;
	framehdr[18] = 0; // wep IV, will be filled in if needed on the arm7 side.
	framehdr[19] = 0;

	framehdr[5]=framelen+hdrlen * 2 - 12 + 4;
	copyexpect= ((framelen+hdrlen * 2 - 12 + 4) + 12 - 4 + 1)/2;
	copytotal=0;

	WifiData->stats[WSTAT_TXQUEUEDPACKETS]++;
	WifiData->stats[WSTAT_TXQUEUEDBYTES] += framelen + hdrlen * 2;

	base = WifiData->txbufOut;
	Wifi_TxBufferWrite(base,hdrlen,framehdr);
	base += hdrlen;
	copytotal += hdrlen;
	if(base >= (WIFI_TXBUFFER_SIZE / 2)) base -= WIFI_TXBUFFER_SIZE / 2;

	// add LLC header
	framehdr[0]=0xAAAA;
	framehdr[1]=0x0003;
	framehdr[2]=0x0000;
	unsigned short protocol = 0x08FE;
	framehdr[3] = ((protocol >> 8) & 0xFF) | ((protocol << 8) & 0xFF00);

	Wifi_TxBufferWrite(base, 4, framehdr);
	base += 4;
	copytotal += 4;
	if(base>=(WIFI_TXBUFFER_SIZE/2)) base -= WIFI_TXBUFFER_SIZE/2;

	writelen = datalen;
	if(writelen) {
		Wifi_TxBufferWrite(base,(writelen+1)/2,data);
		base += (writelen + 1) / 2;
		copytotal += (writelen + 1) / 2;
		if(base>=(WIFI_TXBUFFER_SIZE/2)) base -= WIFI_TXBUFFER_SIZE/2;
	}
	if(WifiData->wepmode7)
	{ // add required extra bytes
		base += 2;
		copytotal += 2;
		if(base >= (WIFI_TXBUFFER_SIZE / 2)) base -= WIFI_TXBUFFER_SIZE / 2;
	}
	WifiData->txbufOut = base; // update fifo out pos, done sending packet.

	if(copytotal!=copyexpect)
	{
		SGIP_DEBUG_MESSAGE(("Tx exp:%i que:%i",copyexpect,copytotal));
	}
	if(synchandler) {
		synchandler();
	}
	return 0;
}

//coto: wifi udp netplay code (:
// datalen = size of packet from beginning of 802.11 header to end, but not including CRC.
__attribute__((section(".itcm")))
int Wifi_RawTxFrame_WIFI(uint8 datalen, uint8 * data) {
	
	//sender phase
	{
		
		switch(dswifiSrv.dsnwifisrv_stat){
		
			//#1 DS is not connected, wait until server acknowledges this info
			case(ds_searching_for_multi_servernotaware):{		
				//NDS MAC Address
				//volatile uint8 macbuf[6];
				//Wifi_GetData(WIFIGETDATA_MACADDRESS, sizeof(macbuf), (uint8*)macbuf);
				
				volatile unsigned long available_ds;
				ioctl(client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds);
				
				if(available_ds == 0){
					char outgoingbuf[256];
					sprintf(outgoingbuf,"dsnotaware-NIFINintendoDS-%s-",(char*)print_ip((uint32)Wifi_GetIP()));	//DS udp ports on server inherit the logic from "//Server aware" handler
					sendto(client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
				}
			}
			break;
			
			
			//servercheck phase. DS's are binded each other. safe to send data between DSes
			case(ds_netplay_host_servercheck):case(ds_netplay_guest_servercheck):{
				
				volatile unsigned long available_ds;
				ioctl(client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds);
				
				if(available_ds == 0){
					//check pending receive
					int LISTENER_PORT 	=	0;
					int SENDER_PORT		=	0;
					char status[10];
					if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_GUEST;
						sprintf(status,"host");
					}
					else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_HOST;
						sprintf(status,"guest");
					}
					
					char buf2[64];
					sprintf(buf2,"dsaware-%s-bindOK-%d-%s-",status,LISTENER_PORT,(char*)print_ip((uint32)Wifi_GetIP()));
					//consoletext(64*2-32,(char *)&buf2[0],0);
					sendto(client_http_handler_context.socket_id__multi_notconnected,buf2,sizeof(buf2),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
					
				}
			}
			break;
					
			
			//#last:connected!
			case(ds_netplay_host):case(ds_netplay_guest):{
				//send NIFI Frame here
				sendto(client_http_handler_context.socket_multi_sender,data,datalen,0,(struct sockaddr *)&client_http_handler_context.sain_sender,sizeof(client_http_handler_context.sain_sender));
			}
			break;
		}
		
	}
	return 0;
}