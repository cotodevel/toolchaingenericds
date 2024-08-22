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

//DSWNIFI Library revision: 1.2
#include "ipcfifoTGDSUser.h"
#include "wifi_shared.h"
#include "clockTGDS.h"
#include "ipcfifoTGDS.h"

#ifdef ARM9
#include "dswnifi_lib.h"
#include "dswnifi.h"
#include "wifi_arm9.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "utilsTGDS.h"
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

#ifdef ARM9

//These methods are template you must override (as defined below), to have an easy DS - DS framework running.

//Example Sender Code
//Send This DS Time to External DS through UDP NIFI or Local NIFI:
//volatile uint8 somebuf[256];
//sprintf((char*)somebuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
//if(!FrameSenderUser){
//				FrameSenderUser = HandleSendUserspace((uint8*)somebuf,sizeof(somebuf));	
//}

//Example Receiver Code
__attribute__((section(".itcm")))
bool TGDSRecvHandlerUser(struct frameBlock * frameBlockRecv, int DSWnifiMode){
	//frameBlockRecv->framebuffer	//Pointer to received Frame
	//frameBlockRecv->frameSize		//Size of received Frame
	switch(DSWnifiMode){
		//single player, has no access to shared buffers.
		case(dswifi_idlemode):{
			//DSWNIFIStatus:SinglePlayer
			return false;
		}
		break;
		
		//NIFI local
		case(dswifi_localnifimode):{
			clrscr();
			printf(" ---- ");
			printf(" ---- ");
			printf("DSWNIFIStatus:LocalNifi!");
			return true;
		}
		break;
		
		//UDP NIFI
		case(dswifi_udpnifimode):{
			clrscr();
			printf(" ---- ");
			printf(" ---- ");
			printf("DSWNIFIStatus:UDPNifi!");
			return true;
		}
		break;
		
	}
	return false;
}


//DSWNIFI callbacks. These run when setting DSWNIFI up
void OnDSWIFIlocalnifiEnable(){

}

void OnDSWIFIidlemodeEnable(){

}

void OnDSWIFIudpnifiEnable(){

}

void OnDSWIFIGDBStubEnable(){

}

//UDP Nifi:
//Step 1: TGDS Project is asked for Remote Companion's IP (AKA: WAN Remote TCP/IP)
void ONDSWIFI_UDPNifiInvalidIP(char * targetIP){
	
}

//Step 2: TGDS Project connected successfully to Remote Companion
void ONDSWIFI_UDPNifiRemoteServerConnected(char * targetIP){
	
}

//Step 3: TGDS Project connected successfully to another DS implementing the DSWNFI protocol
void ONDSWIFI_UDPNifiExternalDSConnected(char * externalDSIP){
	
}

//GDBStub Callbacks
void onGDBStubConnect(){

}

void onGDBStubDisconnected(){

}

#endif
