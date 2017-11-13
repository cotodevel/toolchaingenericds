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

//DSWNifi Library 1.2

#include "ipcfifo.h"
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

//DSWNifi Library status:
//If you use: switch_dswnifi_mode(dswifi_idlemode);		//Stable, Release code
//If you use: switch_dswnifi_mode(dswifi_udpnifimode);	//UDP NIFI: Stable Release code
//If you use: switch_dswnifi_mode(dswifi_tcpnifimode);	//TCP NIFI: Disabled, issues so I didn't bother, please use dswifi_udpnifimode (UDP NIFI)
//If you use: switch_dswnifi_mode(dswifi_localnifimode);//Stable, Release code

TdsnwifisrvStr dswifiSrv;

/////////////////////////////////NIFI DSWNIFI Part////////////////////////////////////
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

__attribute__((section(".itcm")))
void Handler(int packetID, int readlength)
{
	switch(getMULTIMode()){
		case (dswifi_localnifimode):{
			Wifi_RxRawReadPacket(packetID, readlength, (unsigned short *)data);
			//decide whether to put data in userbuffer and if frame is valid here
			struct frameBlock * frameHandled = receiveDSWNIFIFrame((uint8*)(data+frame_header_size),readlength);	//sender always cut off 2 bytes for crc16 we use	
			if(frameHandled != NULL){
				//LOCAL:Valid Frame
				//trigger the User Recv Process here
				HandleRecvUserspace(frameHandled);	//Valid Frame
			}
			else{
				//LOCAL:InValid Frame
			}
			
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
	Wifi_RawSetPacketHandler(Handler);
	Wifi_SetChannel(10);
	
	if(1) {
		//for secial configuration for wifi
		DisableIrq(IRQ_TIMER3);
		//irqSet(IRQ_TIMER3, Timer_50ms); // replace timer IRQ
		TIMERXDATA(3) = -6553; // 6553.1 * 256 cycles = ~50ms;
		TIMERXCNT(3) = 0x00C2; // enable, irq, 1/256 clock
		EnableIrq(IRQ_TIMER3);
	}
}

/////////////////////////////////TCP UDP DSWNIFI Part////////////////////////////////////
client_http_handler client_http_handler_context;
sint8* server_ip = (sint8*)"192.168.43.220";
int port = 8888; 	//gdb stub port
//SOCK_STREAM = TCP / SOCK_DGRAM = UDP

/////////////////////////////////internal, used by DSWNIFI library////////////////////////////////////
//true == sends and frees the queued frame / false == didn't send, no frame
__attribute__((section(".itcm")))
bool sendDSWNIFIFame(struct frameBlock * frameInst)
{
	uint8 * databuf_src = 	frameInst->framebuffer;
	sint32 sizetoSend 		= 	frameInst->frameSize;
	
	//coto: generate crc per nifi frame so we dont end up with corrupted data.
	volatile uint16 crc16_frame = swiCRC16	(	0xffff, //uint16 	crc,
		(uint8*)databuf_src,
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
		//case(dswifi_tcpnifimode):
		{
			Wifi_RawTxFrame_WIFI(sizetoSend , databuf_src);
		}
		break;
	}
	
	return true;
}

struct frameBlock FrameSenderBlock;	//used by the user sender process, must be valid so the ToolchainGenericDS library sends proper frame data.
struct frameBlock FrameRecvBlock;	//used by the user receiver process, can be NULL if no data frame was received.

//reads raw packet (and raw read size) and defines if valid frame or not. 
//must be called from either localnifi handler or udp nifi/wifi on-receive-packet handler
__attribute__((section(".itcm")))
struct frameBlock * receiveDSWNIFIFrame(uint8 * databuf_src,int frameSizeRecv)	//the idea is that we append DSWNIFI Frame + extended frame. so copy the extra data to buffer
{
	struct frameBlock * frameRecvInst =  NULL;
	volatile uint16 crc16_recv_frame = 0;
	//1: check the header localnifi appends, udp nifi/wifi does not append this header
	int frame_hdr_size = 0;	//nifi raw only frame has this header
	switch(getMULTIMode()){
		case(dswifi_localnifimode):
		{
			frame_hdr_size = frame_header_size;	//localframe has this header
			frameSizeRecv-=frame_hdr_size;
		}
		break;
		case(dswifi_udpnifimode):
		//case(dswifi_tcpnifimode):
		{
			frame_hdr_size = 0;					//udp nifi frame has not this header
			frameSizeRecv-=frame_hdr_size;
		}
	}
	//read crc from nifi frame so we dont end up with corrupted data.
	crc16_recv_frame = *(uint16*)(databuf_src + frameSizeRecv - (int)sizeof(volatile uint16));
	//generate crc per nifi frame so we dont end up with corrupted data.
	volatile uint16 crc16_frame_gen = swiCRC16	(	0xffff, //uint16 	crc,
		(databuf_src),
		(frameSizeRecv - (int)sizeof(volatile uint16))		//cant have this own crc here
		);
	
	//do crc calc here. if valid, set receivedValid = true; and copy contents to recvbufferuser
	//data is now nifi frame
	if(crc16_frame_gen == crc16_recv_frame){
		frameRecvInst = (struct frameBlock *)&FrameRecvBlock;
		frameRecvInst->framebuffer = databuf_src;
		frameRecvInst->frameSize = frameSizeRecv;
		//valid dswnifi_frame
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
		initNiFi();
		setMULTIMode(dswifi_localnifimode);
		dswifiSrv.dswifi_setup = false;
	}
	//UDP Nifi/WIFI
	else if (
		(mode == (sint32)dswifi_udpnifimode)
		//||
		//(mode == (sint32)dswifi_tcpnifimode)
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
		//||
		//(getMULTIMode() == dswifi_tcpnifimode)
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

//Used for NIFI server not aware req.
bool sentReq = false;
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
				//UDP: DSClient - Server IP / Desktop Server UDP companion Listener.
				client_http_handler_context.socket_id__multi_notconnected=socket(AF_INET,SOCK_DGRAM,0);	
				int i=1;
				i=ioctl(client_http_handler_context.socket_id__multi_notconnected,FIONBIO,&i); // set non-blocking port
				memset((uint8*)&client_http_handler_context.sain_UDP_PORT, 0, sizeof(client_http_handler_context.sain_UDP_PORT));
				client_http_handler_context.sain_UDP_PORT.sin_family=AF_INET;
				client_http_handler_context.sain_UDP_PORT.sin_addr.s_addr=0;
				client_http_handler_context.sain_UDP_PORT.sin_port=htons((int)UDP_PORT);
				if(bind(client_http_handler_context.socket_id__multi_notconnected,(struct sockaddr *)&client_http_handler_context.sain_UDP_PORT,sizeof(client_http_handler_context.sain_UDP_PORT))) {
					//binding ERROR
					close(client_http_handler_context.socket_id__multi_notconnected);
					retDaemonCode = -1;
					return retDaemonCode;
				}
				else{
					//binding OK
				}
				//UDP: DSClient - Server IP / Desktop Server UDP companion Sender
				memset((uint8*)&client_http_handler_context.server_addr, 0, sizeof(client_http_handler_context.server_addr));
				client_http_handler_context.server_addr.sin_family = AF_INET;
				client_http_handler_context.server_addr.sin_addr.s_addr = inet_addr(server_ip);
				client_http_handler_context.server_addr.sin_port = htons((int)UDP_PORT);
				//no binding since we have no control of server port and we should not know it anyway, DGRAM specific.
				
				setConnectionStatus(proc_execution);
				retDaemonCode = 0;
				return retDaemonCode;
			}
			
			//TCP NIFI Disabled
			/*
			if(getMULTIMode() == dswifi_tcpnifimode){
				//client_http_handler_context.socket_id__multi_notconnected --> UDP : Used by connect to acknowledge the server, the DS ip.												
				//client_http_handler_context.socket_multi_listener			-->	TCP : Used as DS Server listening port, for handshake with server so both DS can connect each other.	
				//UDP: DSClient - Server IP / Desktop Server UDP companion Listener.
				client_http_handler_context.socket_id__multi_notconnected=socket(AF_INET,SOCK_DGRAM,0);
				int i=1;
				i=ioctl(client_http_handler_context.socket_id__multi_notconnected,FIONBIO,&i); // set non-blocking port
				memset((uint8*)&client_http_handler_context.sain_UDP_PORT, 0, sizeof(client_http_handler_context.sain_UDP_PORT));
				client_http_handler_context.sain_UDP_PORT.sin_family=AF_INET;
				client_http_handler_context.sain_UDP_PORT.sin_addr.s_addr=0;
				client_http_handler_context.sain_UDP_PORT.sin_port=htons((int)UDP_PORT);
				if(bind(client_http_handler_context.socket_id__multi_notconnected,(struct sockaddr *)&client_http_handler_context.sain_UDP_PORT,sizeof(client_http_handler_context.sain_UDP_PORT))) {
					//binding ERROR
					close(client_http_handler_context.socket_id__multi_notconnected);
					retDaemonCode = -1;
					return retDaemonCode;
				}
				else{
					//binding OK
				}
				//UDP: DSClient - Server IP / Desktop Server UDP companion Sender
				memset((uint8*)&client_http_handler_context.server_addr, 0, sizeof(client_http_handler_context.server_addr));
				client_http_handler_context.server_addr.sin_family = AF_INET;
				client_http_handler_context.server_addr.sin_addr.s_addr = inet_addr(server_ip);
				client_http_handler_context.server_addr.sin_port = htons((int)UDP_PORT);
				//no binding since we have no control of server port and we should not know it anyway, DGRAM specific.
				
				//////////////////////////////////////////////////TCP DS Server////////////////////////////////////////////////////////
				
				//Set TCP 7777 Companion Listener
				client_http_handler_context.socket_multi_listener = socket(AF_INET, SOCK_STREAM, 0);	//UDP: unused,	TCP: Used as DS Server listening port, for handshake with server so both DS can connect each other.
				if(-1 == client_http_handler_context.socket_multi_listener)
				{
					retDaemonCode = -1;
					return retDaemonCode;
				}
				i=1;
				i=ioctl(client_http_handler_context.socket_multi_listener,FIONBIO,&i); // TCP DS Server: set non-blocking port (otherwise emulator blocks)
				
				//Server Companion Listen Socket: TCP 7777
				memset((uint8*)&client_http_handler_context.sain_listener, 0, sizeof(client_http_handler_context.sain_listener));
				client_http_handler_context.sain_listener.sin_family = AF_INET;			// Internet/IP 
				client_http_handler_context.sain_listener.sin_addr.s_addr = INADDR_ANY;	// let the system figure out our IP address 
				client_http_handler_context.sain_listener.sin_port = htons((int)TCP_PORT);	// this is the port we will listen on 
				if(bind(client_http_handler_context.socket_multi_listener,(struct sockaddr *)&client_http_handler_context.sain_listener, sizeof(client_http_handler_context.sain_listener)))
				{
					retDaemonCode = -1;
					return retDaemonCode;
				}
				listen(client_http_handler_context.socket_multi_listener,5);	//DS Acts as server at desired port
				
				//This NDS IP MULTI: PORT
				client_http_handler_context.socket_multi_sender = socket(AF_INET, SOCK_STREAM, 0);	//UDP: unused,	TCP: Used as DS Server listening port, for handshake with server so both DS can connect each other.
				if(-1 == client_http_handler_context.socket_multi_sender)
				{
					clrscr();
					printf("cannot open DS EXT socket.");
					while(1==1){}
					
					retDaemonCode = -1;
					return retDaemonCode;
				}
				i=1;
				i=ioctl(client_http_handler_context.socket_multi_sender,FIONBIO,&i); // TCP DS Server: set non-blocking port (otherwise emulator blocks)
				
				//Set TCP MULTI: incoming DS
				client_http_handler_context.socket_multi_listenerNetplay = socket(AF_INET, SOCK_STREAM, 0);	//UDP: unused,	TCP: Used as DS Server listening port, for handshake with server so both DS can connect each other.
				if(-1 == client_http_handler_context.socket_multi_listenerNetplay)
				{
					clrscr();
					printf("cannot open DS Server socket.");
					while(1==1){}
					retDaemonCode = -1;
					return retDaemonCode;
				}
				i=1;
				i=ioctl(client_http_handler_context.socket_multi_listenerNetplay,FIONBIO,&i); // TCP DS Server: set non-blocking port (otherwise emulator blocks)
				
				setConnectionStatus(proc_execution);
				retDaemonCode = 1;
				return retDaemonCode;
			}
			*/
			//LOCAL NIFI: runs on DSWIFI process
			if(getMULTIMode() == dswifi_localnifimode){
				Wifi_EnableWifi();
				setConnectionStatus(proc_execution);
				retDaemonCode = 2;
				return retDaemonCode;
			}
		}
		break;
		
		case (proc_execution):{
			//////////////////////////////////////////Handle Internal Send/Recv
			//Server UDP Handler listener
			//unsigned long available_ds_server = 0;
			//ioctl (client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds_server);
			volatile uint8 incomingbuf[frameDSsize] = {0};
			int datalen = 0;
			struct sockaddr_in sender_server;
			int sain_len = 0;
			//UDP NIFI
			if(getMULTIMode() == dswifi_udpnifimode){
				//UDP: (execute RPC from server and process frames)
				switch(dswifiSrv.dsnwifisrv_stat){
					//#1 DS is not connected, serve any upcoming commands related to non connected to multi
					//ds_searching_for_multi_servernotaware -> ds_wait_for_multi here
					case(ds_searching_for_multi_servernotaware):{
						sain_len = sizeof(sender_server);
						memset((uint8*)&sender_server, 0, sizeof(sender_server));
						volatile uint8 cmd[12] = {0};	//srv->ds command handler
						if(sentReq == false){
							char outgoingbuf[frameDSsize] = {0};
							sprintf(outgoingbuf,"dsnotaware-NIFINintendoDS-%s-",(char*)print_ip((uint32)Wifi_GetIP()));	//DS udp ports on server inherit the logic from "//Server aware" handler
							sendto(client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
							sentReq = true;	//acknowledge DS send
						}
						if( (datalen = recvfrom(client_http_handler_context.socket_id__multi_notconnected, (uint8*)incomingbuf, sizeof(incomingbuf), 0, (struct sockaddr *)&sender_server,(int*)&sain_len)) > 0 ){
							if(datalen>0) {
								//incomingbuf[datalen]=0;
								memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
								cmd[sizeof(cmd)-1] = '\0';
							}
						}
						//add frame receive here and detect if valid frame, if not, run the below
						
						//Server aware
						if(strncmp((const char *)cmd, (const char *)"srvaware", 8) == 0){
							
							//server send other NDS ip format: cmd-ip-multi_mode- (last "-" goes as well!)
							char **tokens;
							int count, i;
							//const char *str = "JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC";
							volatile char str[frameDSsize] = {0};
							memcpy ( (uint8*)str, (uint8*)incomingbuf, sizeof(str));

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
							client_http_handler_context.sain_listener.sin_addr.s_addr = htonl(INADDR_ANY);	//local/any ip listen to desired port
							//int atoi ( const char * str );
							//int nds_multi_port = atoi((const char*)tokens[2]);
							client_http_handler_context.sain_listener.sin_port = htons(LISTENER_PORT); //nds_multi_port
							
							
							//printf("IPToConnect:%s",tokens[1]);	//ok EXT DS
							//NDS MULTI IP: No need to bind / connect / sendto use
							memset((char *) &client_http_handler_context.sain_sender, 0, sizeof(client_http_handler_context.sain_sender));
							client_http_handler_context.sain_sender.sin_family = AF_INET;
							client_http_handler_context.sain_sender.sin_addr.s_addr = inet_addr((char*)tokens[1]);//INADDR_BROADCAST;//((const char*)"191.161.23.11");// //ip was reversed 
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
									//sprintf(buf,"[host]binding OK MULTI: port [%d] IP: [%s]  ",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
									//sprintf(id,"[host]");
									//printf("%s",buf);
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_host_servercheck;
								}
								else if(guest_mode == 0){
									//sprintf(buf,"[guest]binding OK MULTI: port [%d] IP: [%s]  ",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
									//sprintf(id,"[guest]");
									//printf("%s",buf);
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_guest_servercheck;
								}
								
								sentReq = false;	//prepare next issuer msg
								//note: bind UDPsender?: does not work with UDP Datagram socket format (UDP basically)
							}	
						}
					}
					break;
					
					//servercheck phase acknow
					case(ds_netplay_host_servercheck):case(ds_netplay_guest_servercheck):{
						if(sentReq == false){
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
							
							char buf2[frameDSsize] = {0};
							sprintf(buf2,"dsaware-%s-bindOK-%d-%s-",status,LISTENER_PORT,(char*)print_ip((uint32)Wifi_GetIP()));
							//consoletext(64*2-32,(char *)&buf2[0],0);
							sendto(client_http_handler_context.socket_id__multi_notconnected,buf2,sizeof(buf2),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
							sentReq = true;	//acknowledge DS send
						}
						volatile uint8 cmd[12] = {0};	//srv->ds command handler
						sain_len = sizeof(sender_server);
						memset((uint8*)&sender_server, 0, sizeof(sender_server));
						datalen=recvfrom(client_http_handler_context.socket_id__multi_notconnected,(uint8*)incomingbuf,sizeof(incomingbuf),0,(struct sockaddr *)&sender_server,(int*)&sain_len);
						if(datalen>0) {
							//incomingbuf[datalen]=0;
							memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
							cmd[sizeof(cmd)-1] = '\0';
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
					//logic: recv data: frameDSsize from port
					case(ds_netplay_host):case(ds_netplay_guest):{						
						sain_len = sizeof(sender_server);
						memset((uint8*)&sender_server, 0, sizeof(sender_server));
						int datalen2=recvfrom(client_http_handler_context.socket_multi_listener,(uint8*)incomingbuf,sizeof(incomingbuf),0,(struct sockaddr *)&sender_server,(int*)&sain_len);
						if(datalen2>0) {
							//decide whether to put data in userbuffer and if frame is valid here
							struct frameBlock * frameHandled = receiveDSWNIFIFrame((uint8*)incomingbuf,datalen2);
							if(frameHandled != NULL){
								//trigger the User Recv Process here
								HandleRecvUserspace(frameHandled);	//Valid Frame
							}
							else{
								//Invalid Frame
							}
						}
					}
					break;
				}
				retDaemonCode = 0;
			}
			
			//TCP NIFI Disabled
			/*
			if(getMULTIMode() == dswifi_tcpnifimode){
				int connectedSD = -1;
				int read_size = 0;
				struct sockaddr_in stSockAddrClient;
				struct timeval timeout;
				timeout.tv_sec = 0;
				timeout.tv_usec = 0 * 1000;
				int stSockAddrClientSize = sizeof(stSockAddrClient);
				memset((uint8*)&stSockAddrClient, 0, sizeof(stSockAddrClient));
				
				volatile uint8 incomingbuf[frameDSsize] = {0};
				volatile char cmd[12] = {0};	//srv->ds command handler
				
				switch(dswifiSrv.dsnwifisrv_stat){
					//#1 DS is not connected, serve any upcoming commands related to non connected to multi
					//ds_searching_for_multi_servernotaware -> ds_wait_for_multi here
					case(ds_searching_for_multi_servernotaware):{
						if(!(-1 == client_http_handler_context.socket_multi_listener))
						{
							//TCP @ PORT 7777 
							if ((connectedSD = accept(client_http_handler_context.socket_multi_listener, (struct sockaddr *)(&stSockAddrClient), &stSockAddrClientSize)) == -1)
							{ 
								
							}
							else{
								// Read from the socket 
								//Receive a message from client (only data > 0 ), socket will be closed anyway
								while( (read_size = recv(connectedSD , (uint8*)incomingbuf , sizeof(incomingbuf) , 0)) > 0 )
								{
									
								}
								//recvbuf[read_size] = '\0';
								memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
								
								//clrscr();
								//printf("recvpacketdata!");
								//Server aware
								if(strncmp((const char *)cmd, (const char *)"srvaware", 8) == 0){
									
									//server send other NDS ip format: cmd-ip-multi_mode- (last "-" goes as well!)
									char **tokens;
									int count, i;
									volatile char str[frameDSsize] = {0};
									memcpy ( (uint8*)str, (uint8*)incomingbuf, sizeof(str));

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
									
									int LISTENER_PORT 	=	0;
									int SENDER_PORT		=	0;
									if(host_mode == 0){
										LISTENER_PORT 	= 	(int)NDSMULTI_TCP_PORT_HOST;
										SENDER_PORT		=	(int)NDSMULTI_TCP_PORT_GUEST;
									}
									else if(guest_mode == 0){
										LISTENER_PORT 	= 	(int)NDSMULTI_TCP_PORT_GUEST;
										SENDER_PORT		=	(int)NDSMULTI_TCP_PORT_HOST;
									}
									
									//TCP Setup:
									//1# This NDS IP sockaddr_in: client_http_handler_context.sain_listener DS Server for Server Companion (port TCP 7777) runs in background
									
									
									//2# This NDS IP MULTI:
									memset((uint8*)&client_http_handler_context.sain_listenerNetplay, 0, sizeof(client_http_handler_context.sain_listenerNetplay));
									client_http_handler_context.sain_listenerNetplay.sin_family = AF_INET;			// Internet/IP 
									client_http_handler_context.sain_listenerNetplay.sin_addr.s_addr = INADDR_ANY;	// let the system figure out our IP address 
									client_http_handler_context.sain_listenerNetplay.sin_port = htons((int)LISTENER_PORT);	// this is the port we will listen on 
									
									if(bind(client_http_handler_context.socket_multi_listenerNetplay,(struct sockaddr *)&client_http_handler_context.sain_listenerNetplay, sizeof(client_http_handler_context.sain_listenerNetplay)))
									{
										clrscr();
										printf("cannot bind DS Server socket.");
										while(1==1){}
										
										retDaemonCode = -1;
										return retDaemonCode;
									}
									
									listen(client_http_handler_context.socket_multi_listenerNetplay,5);	//DS Acts as server at desired port
									
									//printf("this IP Address:%s",(char*)print_ip((uint32)Wifi_GetIP()));
									//printf("destination IP Address:%s",(char*)tokens[1]);
									//while(1==1){}
									
									//3# External NDS IP MULTI: sockaddr_in: sain_sender and use Connect(); !
									
									memset((uint8*)&client_http_handler_context.sain_sender, 0, sizeof(client_http_handler_context.sain_sender));
									client_http_handler_context.sain_sender.sin_family = AF_INET;
									client_http_handler_context.sain_sender.sin_addr.s_addr = inet_addr((char*)tokens[1]);	//external IP to connect to.
									client_http_handler_context.sain_sender.sin_port = htons((int)SENDER_PORT); 
									
									//if connect fails:
									if (connect(client_http_handler_context.socket_multi_sender,(struct sockaddr *)&client_http_handler_context.sain_sender,sizeof(client_http_handler_context.sain_sender)) < 0){
										printf("ERROR connecting");
										while(1==1){}
									}
									//if connect success:
									else{
										clrscr();
										printf("connect OK");
										//if host
										if(host_mode == 0){
											dswifiSrv.dsnwifisrv_stat = ds_netplay_host;
											//nifi_stat = 5;
											printf("DSCONNECTED:HOST!");
										}
										//if guest
										else if(guest_mode == 0){
											dswifiSrv.dsnwifisrv_stat = ds_netplay_guest;
											//nifi_stat = 6;
											printf("DSCONNECTED:GUEST!");
										}
									}
									
									//SenderFrame uses: client_http_handler_context.socket_multi_sender
									//RecvFrameHandler receives msges in: client_http_handler_context.socket_multi_listenerNetplay
								}
							}
							close(connectedSD);
						}
						
						//TCP (use UDP):Internal Sender Handshake. Does run during the DS - DS handshake, when DS are connected through UDP NetPlay, they this does not run anymore.
						volatile unsigned long available_ds = 0;
						ioctl(client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds);
						
						if(available_ds == 0){
							char outgoingbuf[frameDSsize] = {0};
							sprintf(outgoingbuf,"dsnotaware-NIFINintendoDS-%s-TCP",(char*)print_ip((uint32)Wifi_GetIP()));	//DS udp ports on server inherit the logic from "//Server aware" handler
							sendto(client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
						}
					}
					break;
					
					//#last:connected!
					//TCP
					//logic: recv data: frameDSsize from port
					case(ds_netplay_host):case(ds_netplay_guest):{
						
						//DS-DS TCP Handler listener
						if(!(-1 == client_http_handler_context.socket_multi_listenerNetplay))
						{
							//TCP
							if ((connectedSD = accept(client_http_handler_context.socket_multi_listenerNetplay, (struct sockaddr *)(&stSockAddrClient), &stSockAddrClientSize)) == -1)
							{ 
								//perror("accept"); exit(1); 
							}
							else{
								// Read from the socket 
								//Receive a message from client (only data > 0 ), socket will be closed anyway
								while( (read_size = recv(connectedSD , (uint8*)incomingbuf , sizeof(incomingbuf) , 0)) > 0 )
								{
									
								}
								//decide whether to put data in userbuffer and if frame is valid here
								struct frameBlock * frameHandled = receiveDSWNIFIFrame((uint8*)incomingbuf,read_size);
								if(frameHandled != NULL){
									clrscr();
									printf("TCP: Frame recv OK");
									//trigger the User Recv Process here
									HandleRecvUserspace(frameHandled);	//Valid Frame
								}
								else{
									clrscr();
									printf("TCP: Frame recv ERROR");
									//Invalid Frame
								}
							}
							close(connectedSD);
						}
					}
					break;
				}
				retDaemonCode = 1;
			}
			*/
			//Local Nifi: runs from within the DSWIFI frame handler itself so ignored here.
			if(getMULTIMode() == dswifi_localnifimode){
				retDaemonCode = 2;
			}
			
			///////////////////////////////////////Handle Send UserCode, if the user used the following code:
			//struct frameBlock * FrameSenderUser = HandleSendUserspace((uint8*)&nfdata[0],sizeof(nfdata));	//use the nfdata as send buffer // struct frameBlock * FrameSenderInst is now used to detect if pending send frame or not
			//then FrameSenderUser should be not NULL, send the packet here now. Packet must be NOT called from a function.
			
			///////////////////////////////////////Handle Send Library
			// Send Frame UserCore
			if(FrameSenderUser){
				sendDSWNIFIFame(FrameSenderUser);
				FrameSenderUser = NULL;
			}
			return retDaemonCode;
		}
		break;
		//shutdown
		case (proc_shutdown):{
			//close(client_http_handler_context.socket_multi_listener);	//todo
			setMULTIMode(proc_connect);
			retDaemonCode = 0;
			return retDaemonCode;
		}
		break;
		
	}
}

// datalen = size of packet from beginning of 802.11 header to end, but not including CRC.
__attribute__((section(".itcm")))
int Wifi_RawTxFrame_NIFI(sint32 datalen, uint16 rate, uint16 * data) {
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
int Wifi_RawTxFrame_WIFI(sint32 datalen, uint8 * data) {
	//sender phase: only send packets if we are connected!
	switch(dswifiSrv.dsnwifisrv_stat){
		//#last:connected!
		case(ds_netplay_host):case(ds_netplay_guest):{
			//TCP Removed
			//if(getMULTIMode() == dswifi_tcpnifimode){
			//	sendto(client_http_handler_context.socket_multi_sender,data,datalen,0,(struct sockaddr *)&client_http_handler_context.sain_sender,sizeof(client_http_handler_context.sain_sender));
			//}
			//Default to UDP
			//if(getMULTIMode() == dswifi_udpnifimode){
				//send NIFI Frame here
				sendto(client_http_handler_context.socket_multi_sender,data,datalen,0,(struct sockaddr *)&client_http_handler_context.sain_sender,sizeof(client_http_handler_context.sain_sender));
			//}
		}
		break;
	}
	return 0;
}

//UserCodeHandlers that belong to TGDS
struct frameBlock * HandleSendUserspace(uint8 * databuf_src, int bufsize){
	if(!databuf_src){
		return NULL;
	}
	//make room for upcoming crc16 frame, we need at least that
	if((bufsize - sizeof(volatile uint16)) <= 0){
		return NULL;
	}
	bufsize -= sizeof(volatile uint16);
	struct frameBlock * frameSenderInst =  (struct frameBlock *)&FrameSenderBlock;
	frameSenderInst->framebuffer = databuf_src;
	frameSenderInst->frameSize = bufsize;
	return frameSenderInst;
}
#endif

//ARM7 Code here