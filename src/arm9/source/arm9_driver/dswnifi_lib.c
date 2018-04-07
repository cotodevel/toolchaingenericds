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

//DSWNifi Library 1.3 (update: 7/04/2018)	(dd/mm/yyyy)

#include "ipcfifoTGDS.h"
#include "wifi_shared.h"
#include "clockTGDS.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"

#include "biosTGDS.h"
#include "InterruptsARMCores_h.h"
#include "dswnifi_lib.h"

#include "timerTGDS.h"

#include <string.h>
#include <unistd.h>
#include <socket.h>

#ifdef ARM9

#include "wifi_arm9.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "sgIP_Config.h"
#include "utilsTGDS.h"
#include "memoryHandleTGDS.h"

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
//If you use: switch_dswnifi_mode(dswifi_localnifimode);//Stable, Release code
//If you use: switch_dswnifi_mode(dswifi_gdbstubmode);	//Stable, Release code

struct dsnwifisrvStr dswifiSrv;

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
	
	if(1){
		DisableIrq(IRQ_TIMER3);
		TIMERXDATA(3) = -6553; // 6553.1 * 256 cycles = ~50ms;
		TIMERXCNT(3) = 0x00C2; // enable, irq, 1/256 clock
		EnableIrq(IRQ_TIMER3);
	}
	Wifi_EnableWifi();	//required by localplay, but must not be called when WIFI runs (not localplay)
}

/////////////////////////////////TCP UDP DSWNIFI Part////////////////////////////////////
client_http_handler client_http_handler_context;
sint8* server_ip = (sint8*)"192.168.43.221";
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

//if IDLE: returns true always, 
//if UDPNIFI connect OK: true, otherwise false. 
//if NIFI: returns true always
//if GDBSTUB: connect OK: true, otherwise false.
bool switch_dswnifi_mode(sint32 mode){
	//idle mode minimal setup
	if (mode == (sint32)dswifi_idlemode){
		dswifiSrv.dsnwifisrv_stat	= ds_multi_notrunning;
		setMULTIMode(mode);
		setWIFISetup(false);
		setConnectionStatus(proc_shutdown);
		DeInitWIFI();
		return true;
	}
	//Raw Network Packet Nifi
	else if (mode == (sint32)dswifi_localnifimode){
		bool useWIFIAP = false;
		if(connectDSWIFIAP(false,useWIFIAP) == true){	//setWIFISetup set inside
			dswifiSrv.dsnwifisrv_stat	= ds_searching_for_multi_servernotaware;
			setMULTIMode(mode);
			setConnectionStatus(proc_connect);
			return true;
		}
		else{
			//Could not connect
			switch_dswnifi_mode(dswifi_idlemode);
			return false;
		}
	}
	//UDP Nifi (wifi AP)
	else if (mode == (sint32)dswifi_udpnifimode){
		dswifiSrv.dsnwifisrv_stat = ds_searching_for_multi_servernotaware;
		bool useWIFIAP = true;
		if(connectDSWIFIAP(WFC_CONNECT,useWIFIAP) == true){	//setWIFISetup set inside
			setConnectionStatus(proc_connect);
			setMULTIMode(mode);
			return true;
		}
		else{
			//Could not connect
			switch_dswnifi_mode(dswifi_idlemode);
			return false;
		}
	}
	
	//GDBStub mode
	else if (mode == (sint32)dswifi_gdbstubmode){
		dswifiSrv.dsnwifisrv_stat	= ds_multi_notrunning;
		if (gdbNdsStart() == true){	//setWIFISetup set inside
			setMULTIMode(mode);
			return true;
		}
		else{
			//Could not connect
			switch_dswnifi_mode(dswifi_idlemode);
			return false;
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
bool setWIFISetup(bool flag){
	dswifiSrv.dswifi_setup = (bool)flag;
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

//Performs connect, execute and disconnect phases of a local/udp session between DS's (for now it's 2 DS)
//ret: dswifi_idlemode (not connected) / dswifi_udpnifimode (UDP NIFI) / dswifi_localnifimode (LOCAL NIFI)
__attribute__((section(".itcm")))
sint32 doMULTIDaemon(){
	sint32 retDaemonCode = 0;
	switch(getConnectionStatus()){
		case (proc_idle):{
			//nothing to do for : LOCAL / UDP NIFI
			retDaemonCode = dswifi_idlemode;
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
					retDaemonCode = dswifi_udpnifimodeFailConnectStage;
				}
				else{
					//binding OK
				}
				//UDP: DSClient - Server IP / Desktop Server UDP companion Sender
				memset((uint8*)&client_http_handler_context.server_addr, 0, sizeof(struct sockaddr_in));
				client_http_handler_context.server_addr.sin_family = AF_INET;
				client_http_handler_context.server_addr.sin_addr.s_addr = inet_addr(server_ip);
				client_http_handler_context.server_addr.sin_port = htons((int)UDP_PORT);
				//no binding since we have no control of server port and we should not know it anyway, DGRAM specific.
				
				setConnectionStatus(proc_execution);
				retDaemonCode = dswifi_udpnifimode;
			}
			//LOCAL NIFI: runs on DSWIFI process
			if(getMULTIMode() == dswifi_localnifimode){
				setConnectionStatus(proc_execution);
				retDaemonCode = dswifi_localnifimode;
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
						sain_len = sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sizeof(struct sockaddr_in));
						uint8 cmd[12] = {0};	//srv->ds command handler
						if(sentReq == false){
							
							int k = 0;
							for(k = 0; k < 4; k++){
								char outgoingbuf[frameDSsize] = {0};
								sprintf(outgoingbuf,"dsnotaware-NIFINintendoDS-%s-",(char*)print_ip((uint32)Wifi_GetIP()));	//DS udp ports on server inherit the logic from "//Server aware" handler
								sendto(client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(struct sockaddr_in));
								swiDelay(8888);
							}
							sentReq = true;	//acknowledge DS send
						}
						if( (datalen = recvfrom(client_http_handler_context.socket_id__multi_notconnected, (uint8*)incomingbuf, sizeof(incomingbuf), 0, (struct sockaddr *)&sender_server,(int*)&sain_len)) > 0 ){
							if(datalen>0) {
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
							char str[frameDSsize] = {0};
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
							memset((char *)&client_http_handler_context.sain_listener, 0, sizeof(struct sockaddr_in));
							client_http_handler_context.sain_listener.sin_family = AF_INET;
							client_http_handler_context.sain_listener.sin_addr.s_addr = htonl(INADDR_ANY);	//local/any ip listen to desired port
							//int atoi ( const char * str );
							//int nds_multi_port = atoi((const char*)tokens[2]);
							client_http_handler_context.sain_listener.sin_port = htons(LISTENER_PORT); //nds_multi_port
							
							
							////printf("IPToConnect:%s",tokens[1]);	//ok EXT DS
							//NDS MULTI IP: No need to bind / connect / sendto use
							memset((char *) &client_http_handler_context.sain_sender, 0, sizeof(struct sockaddr_in));
							client_http_handler_context.sain_sender.sin_family = AF_INET;
							client_http_handler_context.sain_sender.sin_addr.s_addr = inet_addr((char*)tokens[1]);//INADDR_BROADCAST;//((const char*)"191.161.23.11");// //ip was reversed 
							client_http_handler_context.sain_sender.sin_port = htons(SENDER_PORT); 
							
							struct sockaddr_in *addr_in2= (struct sockaddr_in *)&client_http_handler_context.sain_sender;
							char *IP_string_sender = inet_ntoa(addr_in2->sin_addr);
							
							//bind ThisIP(each DS network hardware) against the current DS UDP port
							if(bind(client_http_handler_context.socket_multi_listener,(struct sockaddr *)&client_http_handler_context.sain_listener,sizeof(struct sockaddr_in))) {
								if(host_mode == 0){
									////printf("%s ","[host]binding error");
								}
								else if(guest_mode == 0){
									////printf("%s ","[guest]binding error");
								}
								
								close(client_http_handler_context.socket_multi_listener);
								retDaemonCode = dswifi_udpnifimodeFailExecutionStage;
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
									////printf("%s",buf);
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_host_servercheck;
								}
								else if(guest_mode == 0){
									//sprintf(buf,"[guest]binding OK MULTI: port [%d] IP: [%s]  ",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
									//sprintf(id,"[guest]");
									////printf("%s",buf);
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
							char status[10] = {0};
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
							sendto(client_http_handler_context.socket_id__multi_notconnected,buf2,sizeof(buf2),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(struct sockaddr_in));
							sentReq = true;	//acknowledge DS send
						}
						uint8 cmd[12] = {0};	//srv->ds command handler
						sain_len = sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sizeof(struct sockaddr_in));
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
								//printf("//////////DSCONNECTED[HOST]-PORT:%d",LISTENER_PORT);
								dswifiSrv.dsnwifisrv_stat = ds_netplay_host;
								nifi_stat = 5;
							}
							else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
								clrscr();
								//printf("//////////DSCONNECTED[GUEST]-PORT:%d",LISTENER_PORT);
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
						sain_len = sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sizeof(struct sockaddr_in));
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
				retDaemonCode = dswifi_udpnifimode;
			}
			
			//Local Nifi: runs from within the DSWIFI frame handler itself so ignored here.
			if(getMULTIMode() == dswifi_localnifimode){
				retDaemonCode = dswifi_localnifimode;
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
		}
		break;
		//shutdown
		case (proc_shutdown):{
			close(client_http_handler_context.socket_id__multi_notconnected);
			sentReq = false;
			setConnectionStatus(proc_idle);
			retDaemonCode = dswifi_idlemode;
		}
		break;
	}
	return retDaemonCode;
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
			if(getMULTIMode() == dswifi_udpnifimode){
				//send NIFI Frame here
				sendto(client_http_handler_context.socket_multi_sender,data,datalen,0,(struct sockaddr *)&client_http_handler_context.sain_sender,sizeof(client_http_handler_context.sain_sender));
			}
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

struct dsnwifisrvStr * getDSWNIFIStr(){
	return (struct dsnwifisrvStr *)&dswifiSrv;
}

#endif


// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

//GDB Stub implementation

int (*remoteSendFnc)(char *, int) = NULL;
int (*remoteRecvFnc)(char *, int) = NULL;
bool (*remoteInitFnc)() = NULL;
void (*remoteCleanUpFnc)() = NULL;

int remotePort = 55555;
int remoteSignal = 5;
int remoteSocket = -1;
int remoteListenSocket = -1;
bool remoteConnected = false;
bool remoteResumed = false;
int reconnectCount = 0;

//this one performs the actual connection and toggles the connected status.
bool connectDSWIFIAP(bool WFC_CONNECTION,bool usewifiAP){
	if(getWIFISetup() == false){
		if(WNifi_InitSafeDefault(WFC_CONNECTION,usewifiAP) == true){
			setWIFISetup(true);
			return true;
		}
		else{
			setWIFISetup(false);
			return false;
		}
	}
	return getWIFISetup();
}

bool gdbNdsStart(){
	dswifiSrv.GDBStubEnable = false;
	bool useWIFIAP = true;
	if(connectDSWIFIAP(WFC_CONNECT,useWIFIAP) == true){
		dswifiSrv.GDBStubEnable = true;
		return true;
	}
	return false;
}

int remoteTcpSend(char *data, int len)
{
	return send(remoteSocket, data, len, 0);
}

int remoteTcpRecv(char *data, int len)
{
	return recv(remoteSocket, data, len, 0);
}

bool remoteTcpInit()
{

if(remoteSocket == -1) {
    int s = socket(PF_INET, SOCK_STREAM, 0);
    remoteListenSocket = s;
	if(s < 0) {
		printf("Error opening socket ");
		while(1==1){}
	}
    int tmp = 1;
    setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp, sizeof (tmp));
	
    //    char hostname[256];
    //    gethostname(hostname, 256);

    //    hostent *ent = gethostbyname(hostname);
    //    unsigned long a = *((unsigned long *)ent->h_addr);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remotePort);
    addr.sin_addr.s_addr = htonl(0);
    int count = 0;
    while(count < 3) {
      if(bind(s, (struct sockaddr *)&addr, sizeof(addr))) {
        addr.sin_port = htons(ntohs(addr.sin_port)+1);
      } else
        break;
    }
    if(count == 3) {
		//printf("Error binding ");
		while(1==1){}
    }

    //printf("Listening for a connection at port %d ",ntohs(addr.sin_port));
    if(listen(s, 1)) {
		printf("Error listening ");
		while(1==1){}
    }
    int len = sizeof(addr);	//socklen_t

    int s2 = accept(s, (struct sockaddr *)&addr, &len);
    if(s2 > 0) {
		//printf("Got a connection from %s %d ",inet_ntoa((in_addr)addr.sin_addr),ntohs(addr.sin_port));
    }
	
    char dummy;
    recv(s2, &dummy, 1, 0);
    if(dummy != '+') {
		//GDB Server: ACK NOT received
	}else{
		//GDB Server: ACK RECEIVED
	}
	
    remoteSocket = s2;
    //    close(s);
  }
  return true;
}

void remoteTcpCleanUp()
{
	if(remoteSocket > 0) {
		//Closing remote socket
		close(remoteSocket);
		remoteSocket = -1;
	}
	if(remoteListenSocket > 0) {
		//Closing listen socket
		close(remoteListenSocket);
		remoteListenSocket = -1;
	}
}

int remotePipeSend(char *data, int len)
{
	int res = write(1, data, len);
	return res;
}

int remotePipeRecv(char *data, int len)
{
	int res = read(0, data, len);
	return res;
}

bool remotePipeInit()
{
	char dummy;
	read(0, &dummy, 1);
	if(dummy != '+') {
		//GDB Server: ACK not received
		while(1==1);
	}
	return true;
}

void remotePipeCleanUp()
{
}

void remoteSetPort(int port)
{
	remotePort = port;
}

void remoteSetProtocol(int p)
{
  if(p == 0) {
    remoteSendFnc = remoteTcpSend;
    remoteRecvFnc = remoteTcpRecv;
    remoteInitFnc = remoteTcpInit;
    remoteCleanUpFnc = remoteTcpCleanUp;
  } else {
    remoteSendFnc = remotePipeSend;
    remoteRecvFnc = remotePipeRecv;
    remoteInitFnc = remotePipeInit;
    remoteCleanUpFnc = remotePipeCleanUp;
  }
}

void remoteInit()
{
	remoteSetProtocol(0);	//use TCP
	if(remoteInitFnc){
		remoteInitFnc();
	}
}

void remotePutPacket(char *packet)
{
  char *hex = "0123456789abcdef";
  char * buffer = (char *)malloc(1024);
	
  int count = strlen(packet);
  unsigned char csum = 0;
  char *p = buffer;
  *p++ = '$';

  for(int i = 0 ;i < count; i++) {
    csum += packet[i];
    *p++ = packet[i];
  }
  *p++ = '#';
  *p++ = hex[csum>>4];
  *p++ = hex[csum & 15];
  *p++ = 0;
  //  //printf("Sending %s\n", buffer);
  remoteSendFnc(buffer, count + 4);

  char c = 0;
  remoteRecvFnc(&c, 1);
  /*
  if(c == '+')
    //printf("ACK\n");
  else if(c=='-')
    //printf("NACK\n");
  */
  free(buffer);
}



void remoteOutput(char *s, u32 addr)
{
  char * buffer = (char *)malloc(16384);
  char *d = buffer;
  *d++ = 'O';

  if(s) {
    char c = *s++;
    while(c) {
      sprintf(d, "%02x", c);
      d += 2;
      c = *s++;
    }
  } else {
    char c= debuggerReadByte(addr);
    addr++;
    while(c) {
      sprintf(d, "%02x", c);
      d += 2;
      c = debuggerReadByte(addr);
      addr++;
    }
  }
  remotePutPacket(buffer);
  free(buffer);
}

void remoteSendSignal()
{
    char * buffer = (char *)malloc(1024);
	sprintf(buffer, "S%02x", remoteSignal);
	remotePutPacket(buffer);
	free(buffer);
}

void remoteSendStatus()
{
  char * buffer = (char *)malloc(1024);
  sprintf(buffer, "T%02x", remoteSignal);
  char *s = buffer;
  s += 3;
  for(int i = 0; i < 15; i++) {
    u32 v = i+(0x08000000);	//mockup ARM Core Registers
    sprintf(s, "%02x:%02x%02x%02x%02x;",i,
            (v & 255),
            (v >> 8) & 255,
            (v >> 16) & 255,
            (v >> 24) & 255);
    s += 12;
  }
  u32 v = (0x08000000);	//Next ARM PC
  sprintf(s, "0f:%02x%02x%02x%02x;", (v & 255),
          (v >> 8) & 255,
          (v >> 16) & 255,
          (v >> 24) & 255);
  s += 12;
  //CPUUpdateCPSR();
  v = 0;	//reg[16].I;	//read CPSR
  sprintf(s, "19:%02x%02x%02x%02x;", (v & 255),
          (v >> 8) & 255,
          (v >> 16) & 255,
          (v >> 24) & 255);
  s += 12;
  *s = 0;
  //  //printf("Sending %s\n", buffer);
  remotePutPacket(buffer);
  free(buffer);
}

void remoteBinaryWrite(char *p)
{
  u32 address = 0;
  int count = 0;
  sscanf(p,"%x,%d:", &address, &count);
  //  //printf("Binary write for %08x %d\n", address, count);

  p = strchr(p, ':');
  p++;
  for(int i = 0; i < count; i++) {
    u8 b = *p++;
    switch(b) {
    case 0x7d:
      b = *p++;
      debuggerWriteByte(address, (b^0x20));
      address++;
      break;
    default:
      debuggerWriteByte(address, b);
      address++;
      break;
    }
  }
  //  //printf("ROM is %08x\n", debuggerReadMemory(0x8000254));
  remotePutPacket("OK");
}

void remoteMemoryWrite(char *p)
{
  u32 address = 0;
  int count = 0;
  sscanf(p,"%x,%d:", &address, &count);
  //  //printf("Memory write for %08x %d\n", address, count);

  p = strchr(p, ':');
  p++;
  for(int i = 0; i < count; i++) {
    u8 v = 0;
    char c = *p++;
    if(c <= '9')
      v = (c - '0') << 4;
    else
      v = (c + 10 - 'a') << 4;
    c = *p++;
    if(c <= '9')
      v += (c - '0');
    else
      v += (c + 10 - 'a');
    debuggerWriteByte(address, v);
    address++;
  }
  //  //printf("ROM is %08x\n", debuggerReadMemory(0x8000254));
  remotePutPacket("OK");
}

void remoteMemoryRead(char *p)
{
  u32 address = 0;
  int count = 0;
  sscanf(p,"%x,%d:", &address, &count);
  
  char * buffer = (char *)malloc(1024);
  char *s = buffer;
  for(int i = 0; i < count; i++) {
    u8 b = debuggerReadByte(address);
    sprintf(s, "%02x", b);
    address++;
    s += 2;
  }
  *s = 0;
  remotePutPacket(buffer);
  free(buffer);
}

void remoteStepOverRange(char *p)
{
  u32 address = 0;
  u32 final = 0;
  sscanf(p, "%x,%x", &address, &final);
  remotePutPacket("OK");
  remoteResumed = true;
  
  /*
  do {
    CPULoop(1);
    if(debugger)
      break;
  } while(armNextPC >= address && armNextPC < final);
	*/
  remoteResumed = false;
  remoteSendStatus();
}

void remoteWriteWatch(char *p, bool active)
{
  u32 address = 0;
  int count = 0;
  sscanf(p, ",%x,%d#", &address, &count);

  //printf("Write watch for %08x %d ", address, count);

  if(address < 0x2000000 || address > 0x3007fff) {
    remotePutPacket("E01");
    return;
  }

  if(address > 0x203ffff && address < 0x3000000) {
    remotePutPacket("E01");
    return;
  }

  u32 final = address + count;

  if(address < 0x2040000 && final > 0x2040000) {
    remotePutPacket("E01");
    return;
  } else if(address < 0x3008000 && final > 0x3008000) {
    remotePutPacket("E01");
    return;
  }

  for(int i = 0; i < count; i++) {
    if((address >> 24) == 2)
      WorkRAM[address & 0x3ffff] = active;
    else
      InternalRAM[address & 0x7fff] = active;
    address++;
  }

  remotePutPacket("OK");
}

void remoteReadRegisters(char *p)
{
  char * buffer = (char *)malloc(1024);
  char *s = buffer;
  int i;
  // regular registers
  for(i = 0; i < 15; i++) {
    u32 v = i + (0x08000000);//reg[i].I;
    sprintf(s, "%02x%02x%02x%02x",  v & 255, (v >> 8) & 255,
            (v >> 16) & 255, (v >> 24) & 255);
    s += 8;
  }
  // PC
  u32 pc = i;	//armNextPC;
  sprintf(s, "%02x%02x%02x%02x", pc & 255, (pc >> 8) & 255,
          (pc >> 16) & 255, (pc >> 24) & 255);
  s += 8;

  // floating point registers (24-bit)
  for(i = 0; i < 8; i++) {
    sprintf(s, "000000000000000000000000");
    s += 24;
  }

  // FP status register
  sprintf(s, "00000000");
  s += 8;
  // CPSR
  //CPUUpdateCPSR();
  u32 v = 0;	//reg[16].I;
  sprintf(s, "%02x%02x%02x%02x",  v & 255, (v >> 8) & 255,
          (v >> 16) & 255, (v >> 24) & 255);
  s += 8;
  *s = 0;
  remotePutPacket(buffer);
  free(buffer);
}

void remoteWriteRegister(char *p)
{
  int r = 0;
  sscanf(p, "%d=", &r);
  p = strchr(p, '=');
  p++;
  char c = *p++;
  u32 v = 0;
  u8 data[4] = {0,0,0,0};

  int i = 0;
  while(c != '#') {
    u8 b = 0;
    if(c <= '9')
      b = (c - '0') << 4;
    else
      b = (c + 10 - 'a') << 4;
    c = *p++;
    if(c <= '9')
      b += (c - '0');
    else
      b += (c + 10 - 'a');
    data[i++] = b;
    c = *p++;
  }

  v = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

  //  //printf("Write register %d=%08x\n", r, v);
  
  //todo
  /*
  reg[r].I = v;
  if(r == 15) {
    armNextPC = v;
    if(armState)
      reg[15].I = v + 4;
    else
      reg[15].I = v + 2;
  }
  */
  remotePutPacket("OK");
}

sint32 remoteStubMain()
{
	if( (getMULTIMode() == dswifi_gdbstubmode) && (getWIFISetup() == true) && (dswifiSrv.GDBStubEnable == true) ){
		if(remoteResumed) {
			remoteSendStatus();
			remoteResumed = false;
		}
		char * buffer = (char *)malloc(1024);
		int res = remoteRecvFnc(buffer, 1024);
		//if DSWIFI connection is lost, re-connect and init GDB Server if external logic says so.
		if(res == -1) {
			remoteCleanUp();
			return remoteStubMainWIFIConnectedGDBDisconnected;
		}
		else{
			char *p = buffer;
			char c = *p++;
			char pp = '+';
			remoteSendFnc(&pp, 1);
			if(c != '$'){
				//try next time
			}
			else{
				c= *p++;
				switch(c) {
				case '?':
				  remoteSendSignal();
				  break;
				case 'D':
				  remotePutPacket("OK");
				  remoteResumed = true;
				  return;
				case 'e':
				  remoteStepOverRange(p);
				  break;
				case 'k':
				  remotePutPacket("OK");
				  return;
				case 'C':
				  remoteResumed = true;
				  return;
				case 'c':
				  remoteResumed = true;
				  return;
				case 's':
				  remoteResumed = true;
				  remoteSignal = 5;
				  //CPULoop(1);
				  if(remoteResumed) {
					remoteResumed = false;
					remoteSendStatus();
				  }
				  break;
				case 'g':
				  remoteReadRegisters(p);
				  break;
				case 'P':
				  remoteWriteRegister(p);
				  break;
				case 'M':
				  remoteMemoryWrite(p);
				  break;
				case 'm':
				  remoteMemoryRead(p);
				  break;
				case 'X':
				  remoteBinaryWrite(p);
				  break;
				case 'H':
				  remotePutPacket("OK");
				  break;
				case 'q':
				  remotePutPacket("");
				  break;
				case 'Z':
				  if(*p++ == '2') {
					remoteWriteWatch(p, true);
				  } else
					remotePutPacket("");
				  break;
				case 'z':
				  if(*p++ == '2') {
				remoteWriteWatch(p, false);
				  } else
				remotePutPacket("");
				  break;
				default:
				  {
					*(strchr(p, '#') + 3) = 0;
					//printf("Unknown packet %s ", --p);
					remotePutPacket("");
				  }
				  break;
				}
			}
		}
		free(buffer);
		return remoteStubMainWIFIConnectedGDBRunning;	//WIFI connected and GDB running
	}
	
	else{
		if(getWIFISetup() == true){
			return remoteStubMainWIFIConnectedNoGDB;	//gdb not running, WIFI connected	(retry connection here)
		}
	}
	
	return remoteStubMainWIFINotConnected;				//gdb not running, WIFI not connected (first time connection)
}

void remoteStubSignal(int sig, int number)
{
  remoteSignal = sig;
  remoteResumed = false;
  remoteSendStatus();
}

void remoteCleanUp()
{
  if(remoteCleanUpFnc)
    remoteCleanUpFnc();
	dswifiSrv.GDBStubEnable = false;
}

u32 debuggerReadMemory(u32 addr){
	if(isValidMap(addr) == true){
		return (*(u32*)addr);
	}
	else{
		return (u32)(0xffffffff);
	}
}

u16 debuggerReadHalfWord(u32 addr){
	if(isValidMap(addr) == true){
		return (*(u16*)addr);
	}
	else{
		return (u16)(0xffff);
	}
}

u8 debuggerReadByte(u32 addr){
	if(isValidMap(addr) == true){
		return (*(u8*)addr);
	}
	else{
		return (u8)(0xff);
	}
}