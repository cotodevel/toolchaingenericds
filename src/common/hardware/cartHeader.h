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

#ifndef __cartheader_h
#define __cartheader_h

//NTR Header
typedef struct sDSCARTHEADER {
	uint8 	gametitle[12];	//000h    12    Game Title  (Uppercase ASCII, padded with 00h)
	uint32	gamecode;	//00Ch    4     Gamecode    (Uppercase ASCII, NTR-<code>)        (0=homebrew)
	uint16	makercode;	//010h    2     Makercode   (Uppercase ASCII, eg. "01"=Nintendo) (0=homebrew)
	uint8	unitcode;	//012h    1     Unitcode    (00h=NDS, 02h=NDS+DSi, 03h=DSi) (bit1=DSi)
  uint8 encriptionseedsel;//013h    1     Encryption Seed Select (00..07h, usually 00h)
  uint8 devicecapacity;	//014h    1     Devicecapacity         (Chipsize = 128KB SHL nn) (eg. 7 = 16MB)
  uint8	reserved1[7];	//015h    7     Reserved    (zero filled)
  uint8	reserved2;	//01Ch    1     Reserved    (zero)                      (except, used on DSi)
  uint8	ndsregion;	//01Dh    1     NDS Region  (00h=Normal, 80h=China, 40h=Korea) (other on DSi)
  uint8	romversion;	//01Eh    1     ROM Version (usually 00h)
  uint8	autostart;	//01Fh    1     Autostart (Bit2: Skip "Press Button" after Health and Safety) (Also skips bootmenu, even in Manual mode & even Start pressed)
  uint32	arm9romoffset;	//020h    4     ARM9 rom_offset    (4000h and up, align 1000h)
  uint32	arm9entryaddress;	//024h    4     ARM9 entry_address (2000000h..23BFE00h)
  uint32	arm9ramaddress;	//028h    4     ARM9 ram_address   (2000000h..23BFE00h)
  uint32	arm9size;	//02Ch    4     ARM9 size          (max 3BFE00h) (3839.5KB)
  uint32	arm7romoffset;	//030h    4     ARM7 rom_offset    (8000h and up)
  uint32	arm7entryaddress;	//034h    4     ARM7 entry_address (2000000h..23BFE00h, or 37F8000h..3807E00h)
  uint32 	arm7ramaddress;	//038h    4     ARM7 ram_address   (2000000h..23BFE00h, or 37F8000h..3807E00h)
  uint32	arm7size;	//03Ch    4     ARM7 size          (max 3BFE00h, or FE00h) (3839.5KB, 63.5KB)
  //...rest don't care
}tDSCARTHEADER;

//Libnds ARGV impl.
#ifdef ARM9

	// argv struct magic number
	#define ARGV_MAGIC 0x5f617267

	//!	argv structure
	/*!	\struct __argv

		structure used to set up argc/argv on the DS

	*/
	struct __argv {
		int argvMagic;		//!< argv magic number, set to 0x5f617267 ('_arg') if valid 
		char *commandLine;	//!< base address of command line, set of null terminated strings
		int length;		//!< total length of command line
		int argc;		//!< internal use, number of arguments
		char **argv;		//!< internal use, argv pointer
	};

	//!	Default location for the libnds argv structure.
	#define __system_argv		((struct __argv *)0x02FFFE70)
	#define cmdline 			((char*)(int)0x02FFFE70 + sizeof(struct __argv))

	/* 

	Usage:	
		char thisArgv[3][MAX_TGDSFILENAME_LENGTH];
		memset(thisArgv, 0, sizeof(thisArgv));
		strcpy(&thisArgv[0][0], "toolchaingenericds-template.nds");	//Arg0
		strcpy(&thisArgv[1][0], "fat:/dir1/dir2/file2.zip");		//Arg1 
		strcpy(&thisArgv[2][0], "fat:/dir1/dir2/file3.txt");		//Arg2 
		addARGV(3, (char*)&thisArgv);

	Note:
		In order to keep compatibility between TGDS and devkitPro environment, 
		devkitPro argv format is untouched and is used by default. 
		TGDS binaries parse it into TGDS format internally.

	Implementation:
		https://bitbucket.org/Coto88/toolchaingenericds-argvtest/
	*/

	static inline void addARGV(int argc, char *argv){
		if((argc <= 0) || (argv == NULL)){
			return;
		}
		memset(cmdline, 0, 512);
		int cmdlineLen = 0;
		int i= 0;
		for(i = 0; i < argc; i++){
			
			//Libnds compatibility: If (send) addARGV 0:/ change to fat:/
			char thisARGV[MAX_TGDSFILENAME_LENGTH+1];
			memset(thisARGV, 0, sizeof(thisARGV));
			strcpy(thisARGV, &argv[i*MAX_TGDSFILENAME_LENGTH]);
			
			if(
				(thisARGV[0] == '0')
				&&
				(thisARGV[1] == ':')
				&&
				(thisARGV[2] == '/')
				){
				char thisARGV2[MAX_TGDSFILENAME_LENGTH+1];
				memset(thisARGV2, 0, sizeof(thisARGV2));
				strcpy(thisARGV2, "fat:/");
				strcat(thisARGV2, &thisARGV[3]);	//1+last
				
				//copy back
				memset(&argv[i*MAX_TGDSFILENAME_LENGTH], 0, 256);
				strcpy(&argv[i*MAX_TGDSFILENAME_LENGTH], thisARGV2);
			}
			
			strcpy(cmdline + cmdlineLen, &argv[i*MAX_TGDSFILENAME_LENGTH]);
			cmdlineLen+= strlen(&argv[i*MAX_TGDSFILENAME_LENGTH]) + 1;
		}

		__system_argv->argvMagic = ARGV_MAGIC;
		__system_argv->commandLine = cmdline;
		__system_argv->length = cmdlineLen;
		
		// reserved when ARGV receives into main()
		//__system_argv->argc = ;
		//__system_argv->argv = ;
	}
	#endif

#endif
