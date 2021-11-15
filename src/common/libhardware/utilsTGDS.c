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

#include "utilsTGDS.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "spifwTGDS.h"
#include "biosTGDS.h"
#include "limitsTGDS.h"

#ifdef ARM7
#include "powerTGDS.h"
#endif

#ifdef TWLMODE

#ifdef ARM7
#include "i2c.h"
#endif
#include "utils.twl.h"
#endif


#ifdef ARM9
#include "fatfslayerTGDS.h"
#include "videoTGDS.h"
#include "dldi.h"
#include "tgds_ramdisk_dldi.h"
#include "cartHeader.h"
#endif

#ifdef ARM7
s16 *strpcmL0 = NULL;
s16 *strpcmL1 = NULL;
s16 *strpcmR0 = NULL;
s16 *strpcmR1 = NULL;

int lastL = 0;
int lastR = 0;

int multRate = 1;
int pollCount = 100; //start with a read

u32 sndCursor = 0;
u32 micBufLoc = 0;
u32 sampleLen = 0;
int sndRate = 0;
#endif

#ifdef ARM9
struct soundPlayerContext soundData;
bool updateRequested = false;
void flushSoundContext(int soundContextIndex){
	
}
#endif

bool __dsimode = false; // set by detecting DS model from firmware

#ifdef NTRMODE
char * TGDSPayloadMode = "NTRModePayload";
#endif

#ifdef TWLMODE
char * TGDSPayloadMode = "TWLModePayload";
#endif

#ifdef ARM9
char bufModeARM7[256];


//ToolchainGenericDS-LinkedModule 
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int getArgcFromTGDSLinkedModule(struct TGDS_Linked_Module * TGDSLMCtx){
	return TGDSLMCtx->argCount;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
char ** getArgvFromTGDSLinkedModule(struct TGDS_Linked_Module * TGDSLMCtx){
	return (char **)&TGDSLMCtx->argvs;
}

//Usage: char * TGDSLinkedModuleFilename = "0:/ToolchainGenericDS-linkedmodule.bin"
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TGDSProjectRunLinkedModule(char * TGDSLinkedModuleFilename, int argc, char **argv, char* TGDSProjectName) {
	
	//switch_dswnifi_mode(dswifi_idlemode); //todo: use callback
	
	FILE * tgdsPayloadFh = fopen(TGDSLinkedModuleFilename, "r");
	if(tgdsPayloadFh != NULL){
		fseek(tgdsPayloadFh, 0, SEEK_SET);
		int	tgds_multiboot_payload_size = FS_getFileSizeFromOpenHandle(tgdsPayloadFh);
		dmaFillHalfWord(0, 0, (uint32)0x02200000, (uint32)(tgds_multiboot_payload_size));
		coherent_user_range_by_size((uint32)0x02200000, tgds_multiboot_payload_size);
		fread((u32*)0x02200000, 1, tgds_multiboot_payload_size, tgdsPayloadFh);
		coherent_user_range_by_size(0x02200000, (int)tgds_multiboot_payload_size);
		fclose(tgdsPayloadFh);
		int ret=FS_deinit();
		
		//Copy and relocate current TGDS DLDI section into target ARM9 binary
		u32 dldiSrc = (u32)&_io_dldi_stub;
		if(strncmp((char*)&dldiGet()->friendlyName[0], "TGDS RAMDISK", 12) == 0){
			dldiSrc = (u32)&tgds_ramdisk_dldi[0];
		}
		bool stat = dldiPatchLoader((data_t *)0x02200000, (u32)tgds_multiboot_payload_size, dldiSrc);
		
		if(stat == false){
			//printf("DLDI Patch failed. APP does not support DLDI format.");
		}
		
		REG_IME = 0;
		
		//Shut down Wifi context so it can re-enabled by upcoming binary.
		//DeInitWIFI(); //todo: use callback
		
		//Generate TGDS-LM context
		struct TGDS_Linked_Module * TGDSLinkedModuleCtx = (struct TGDS_Linked_Module *)((int)0x02200000 - 0x1000);
		memset((u8*)TGDSLinkedModuleCtx, 0, 4096);
		
		TGDSLinkedModuleCtx->TGDS_LM_Size = tgds_multiboot_payload_size;
		TGDSLinkedModuleCtx->TGDS_LM_Entrypoint = 0x02200000;
		//TGDSLinkedModuleCtx->returnAddressTGDSLinkedModule = 0;	//Implemented when TGDS-LM boots
		TGDSLinkedModuleCtx->returnAddressTGDSMainApp = (u32)&TGDSProjectReturnFromLinkedModule;	//Implemented in Parent TGDS App
		//TGDS-LM ARGV
		int i = 0;
		for(i = 0; i < argc; i++){
			strcpy((char*)&TGDSLinkedModuleCtx->args[i][0], (char*)argv[i]);
			TGDSLinkedModuleCtx->argvs[i] = (char*)&TGDSLinkedModuleCtx->args[i][0];
		}
		TGDSLinkedModuleCtx->argCount = argc;
		strcpy((char*)&TGDSLinkedModuleCtx->TGDSMainAppName, TGDSProjectName);
		
		typedef void (*t_bootAddr)();
		t_bootAddr bootARM9Payload = (t_bootAddr)0x02200000;
		bootARM9Payload();
	}
}
#endif

//reportTGDSPayloadMode(&bufModeARM7[0]); //usage

void reportTGDSPayloadMode(u32 bufferSource){
	#ifdef ARM7
	char dbgMsg[64];
	memset(dbgMsg, 0, sizeof(dbgMsg));
	strcpy(dbgMsg, "TGDS ARM7.bin Payload Mode: ");
	strcat(dbgMsg, TGDSPayloadMode);
	strcpy((char*)bufferSource, dbgMsg);
	#endif
	
	#ifdef ARM9
	//send ARM7 signal, wait for it to be ready, then continue
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[45], (u32)0xFFFFFFFF);
	SendFIFOWords(TGDS_ARMCORES_REPORT_PAYLOAD_MODE, (u32)bufferSource);	//ARM7 Setup
	while((u32)getValueSafe(&fifomsg[45]) == (u32)0xFFFFFFFF){
		swiDelay(1);
	}
	printf("TGDS ARM7.bin Payload Mode: %s", (char*)bufferSource);
	printf("TGDS ARM9.bin Payload Mode: %s", (char*)TGDSPayloadMode);
	
	char dbgMsg[64];
	memset(dbgMsg, 0, sizeof(dbgMsg));
	strcpy(dbgMsg, "TGDS ARM9.bin Payload Mode:");
	strcat(dbgMsg, TGDSPayloadMode);
	nocashMessage(dbgMsg);
	#endif
}

//!	Checks whether the application is running in DSi mode.
bool isDSiMode() {
	return __dsimode;
}

size_t ucs2tombs(uint8* dst, const unsigned short* src, size_t len) {
	size_t i=0,j=0;
	for (;src[i];i++){
		if(src[i] <= 0x007f){
			if(!dst)j++;else{
				if(len-j<2)break;
				dst[j++] = ((src[i] & 0x007f)      );
			}
		}else if(src[i] <= 0x07ff){
			if(!dst)j+=2;else{
				if(len-j<3)break;
				dst[j++] = ((src[i] & 0x07c0) >>  6) | 0xc0;
				dst[j++] = ((src[i] & 0x003f)      ) | 0x80;
			}
		}else if((src[i] & 0xdc00) == 0xd800 && (src[i+1] & 0xdc00) == 0xdc00){
			if(!dst)j+=4;else{
				unsigned short z = (src[i]&0x3ff)+0x40;
				if(len-j<5)break;
				dst[j++] = ((z      & 0x0300) >>  8) | 0xf0;   //2
				dst[j++] = ((z      & 0x00fc) >>  2) | 0x80;   //6
				dst[j++] = ((z      & 0x0003) <<  4)           //2
					  | ((src[i+1] & 0x03c0) >>  6) | 0x80; //4
				dst[j++] = ((src[i+1] & 0x003f)      ) | 0x80; //6
			}i++;
		}else{
			if(!dst)j+=3;else{
				if(len-j<4)break;
				dst[j++] = ((src[i] & 0xf000) >> 12) | 0xe0;
				dst[j++] = ((src[i] & 0x0fc0) >>  6) | 0x80;
				dst[j++] = ((src[i] & 0x003f)      ) | 0x80;
			}
		}
	}
	if(dst)dst[j]=0;
	return j;
}


#ifdef ARM9

#include "consoleTGDS.h"
#include "devoptab_devices.h"
#include "fatfslayerTGDS.h"
#include "posixHandleTGDS.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/reent.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <_ansi.h>
#include <reent.h>
#include <sys/lock.h>
#include <fcntl.h>


//Coto: These functions allow to export ARM code to plaintext and backwards, so handlers can be added to config file.
//Note: each handler albeit exported, is ONLY compatible with the current emuCore version + timestamp it was compiled for. Do not try to
//change none of these as it will lead to lockups. Just stick to compile the current emuCore version and let the export function export the handler.

//Reason: the widget plugin for VS (that generates the GUI widget objects), will use the same emuCore version files to manage GUI settings.
//(I mean it's awesome to have a GUI to generate your layouts instead hardcoded... but Archeide STILL did a wonderful job by building a widget system in C)

//reserved for project appVersion
volatile char app_version_static[256];

METHOD_DESCRIPTOR Methods[8];

//returns a signature that holds the function + size
METHOD_DESCRIPTOR * callback_append_signature(uint32 * func_addr, uint32 * func_addr_end, char * methodname,METHOD_DESCRIPTOR * method_inst){
	
	method_inst->cback_address 	= 	func_addr;
	method_inst->cback_size		=	(sint32)((uint8*)(uint32*)func_addr_end - (sint32)(func_addr));
	memcpy ((uint8*)method_inst->methodname, (uint8*)methodname, strlen(methodname));
	
	return (METHOD_DESCRIPTOR *)method_inst;
}

//all handlers will have __attribute__((optimize("O0"))) specified.

__attribute__((optimize("O0")))
__attribute__ ((noinline))
sint32 cback_build(){
	
	__asm__ volatile("bx lr""\n\t");
	return (sint32)0;
}

__attribute__((optimize("O0")))
__attribute__ ((noinline))
void cback_build_end(){
	
}

//export an ARMv5 function to buffer
inline sint32 callback_export_buffer(METHOD_DESCRIPTOR * method_inst, uint8 * buf_out){
	
	if(method_inst->cback_size > 0){
		//void * memcpy ( void * destination, const void * source, size_t num );
		memcpy ((uint32*)(buf_out), (uint32*)method_inst->cback_address, method_inst->cback_size);
		return method_inst->cback_size;
	}
	
	return -1;
}

//export an ARMv5 function to text file
sint32 callback_export_file(char * filename,METHOD_DESCRIPTOR * method_inst){
	return FS_saveFile(filename, (char *)method_inst->cback_address, method_inst->cback_size,true);
}

//Version Handler: Required for config (plaintext) ARM code. We save the timestamp of the emuname.nds and we check it against a text file.
volatile VERSION_DESCRIPTOR Version[1];	//global

//Apps should update this at bootup
sint32 addAppVersiontoCompiledCode(VERSION_DESCRIPTOR * versionInst,char * appVersion,int appVersionCharsize){
	if ((strlen(appVersion) > sizeof(app_version_static)) || (appVersionCharsize > sizeof(app_version_static))){
		return -1;	//error, prevent buffer overflow
	}
	
	memcpy((uint8*)versionInst->app_version,appVersion,appVersionCharsize);
	return 0;
}

//Framework sets this by default. should be re-called right after APP set version
sint32 updateVersionfromCompiledCode(VERSION_DESCRIPTOR * versionInst){
	return -1;
}

//Writes to versionInst the current version TGDS Version
sint32 updateAssemblyParamsConfig(VERSION_DESCRIPTOR * versionInst){
	if(updateVersionfromCompiledCode(versionInst) == -1){
		return -2;
	}
	
	//replace by open source file parser: GUI_setConfigStrUpdateFile((char*)"AssemblyCore", (char*)"appVersion", (char*)versionInst->app_version);	//does update config if zone/tuple was not declared earlier	
	//replace by open source file parser: GUI_setConfigStrUpdateFile((char*)"AssemblyCore", (char*)"ToolchainVersion", (char*)versionInst->framework_version);	//does update config if zone/tuple was not declared earlier
	return 0;
}

sint32 glueARMHandlerConfig(VERSION_DESCRIPTOR * versionInst,METHOD_DESCRIPTOR * method_inst){
	if(updateAssemblyParamsConfig(versionInst) < 0){
		return -1;
	}
	
	char methodfilename[256];
	sprintf(methodfilename,"%s.bin",method_inst->methodname);
	//replace by open source file parser: GUI_setConfigStrUpdateFile((char*)"AssemblyCore", (char*)method_inst->methodname, (char*)methodfilename);
	callback_export_file(methodfilename,method_inst);
	
	return 0;
}

// fork from https://github.com/irl/la-cucina/blob/master/str_replace.c
char * str_replace (char *string, const char *substr, const char *replacement)
{
	char *tok = NULL;
	char *newstr = NULL;
	char *oldstr = NULL;

	/* if either substr or replacement is NULL, duplicate string a let caller handle it */

	if ( substr == NULL || replacement == NULL )
	{
		return strdup((const char *)string);
	}

	newstr = strdup (string);

	while ( ( tok = strstr( newstr, substr ) ) )
	{
		oldstr = newstr;
		newstr = (char*)malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );

		/* If failed to alloc mem, free old string and return NULL */
		if ( newstr == NULL )
		{
			free (oldstr);
			return NULL;
		}

		memcpy ( newstr, oldstr, tok - oldstr );
		memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
		memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
		memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );

		free (oldstr);
	}

	free (string);

	return newstr;
}

int split (const sint8 *txt, sint8 delim, sint8 ***tokens)
{
    int *tklen, *t, count = 1;
    sint8 **arr, *p = (sint8 *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = (int*)calloc (count, sizeof (int));
    for (p = (sint8 *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = (sint8**)malloc (count * sizeof (sint8 *));
    t = tklen;
    p = *arr++ = (sint8*)calloc (*(t++) + 1, sizeof (sint8 *));
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = (sint8*)calloc (*(t++) + 1, sizeof (sint8 *));
            txt++;
        }
        else *p++ = *txt++;
    }
    free (tklen);
    return count;
}

int	FS_loadFile(sint8 *filename, sint8 *buf, int size)
{
	FILE *f;
	int file_size;
	
	f = fopen(filename, "r");	//old: f = fopen(filename, "rb");
	if (f == NULL)
	{
		return -1;
	}
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	if (file_size < size)
	{
		fclose(f);
		return -1;
	}
	
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, size, f);
	
    fclose(f);
	return 0;
}

//force_file_creation == true: create a savefile regardless. false otherwise
int	FS_saveFile(sint8 *filename, sint8 *buf, int size,bool force_file_creation)
{
	FILE * f;
	volatile sint8 var[16];
	
	if(force_file_creation == true){
		sprintf((sint8*)var,"%s","w+");
	}
	else{
		sprintf((sint8*)var,"%s","w");
	}
	
	if((f = fopen(filename, (sint8*)var)) == NULL)
	{
		return -1;
  	}
	
	fwrite(buf, 1, size, f);	//old: fwrite(buf, 1, size, f);
	fclose(f);	//old: fclose(f);	
	return 0;
}

int	FS_getFileSize(sint8 *filename)
{	
	FILE * f = fopen(filename, "r");
	if (f == NULL)
	{
		FS_unlock();
		return -1;
	}
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fclose(f);
	
	return size;
}

int setSoundPower(int flags){
	return 0;
}


void	FS_lock()
{
	
}

void	FS_unlock()
{
	
}

sint8 * print_ip(uint32 ip, char * outBuf)
{
    uint8 bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;	
    sprintf(outBuf,"%d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
	return outBuf;
}

//FileSystem utils
sint8 *_FS_getFileExtension(sint8 *filename)
{
	static sint8 ext[4];
	sint8	*ptr;
	int		i = 0;
	
	ptr = filename;
	do
	{
		ptr = strchr(ptr, '.');
		if (!ptr)
			return NULL;
		ptr++;
	}
	while (strlen(ptr) > 3);
		
	for (i = 0; i < (int)strlen(ptr); i++)
		ext[i] = toupper((int)(ptr[i])); 
	ext[i] = 0;
	return ext;
}

sint8 *FS_getFileName(sint8 *filename)
{
	static sint8 name[100];
	sint8	*ptr;
	int		i;
	
	ptr = filename;
	ptr = strrchr(ptr, '.');
		
	for (i = 0; i < ptr-filename; i++)
		name[i] = filename[i]; 
	name[i] = 0;
	return name;
}

int		FS_chdir(const sint8 *path)
{
	int ret = fatfs_chdir(path);
	return ret;
}

//taken from https://stackoverflow.com/questions/9052490/find-the-count-of-substring-in-string
//modified by Coto
static int indexParse = 0;
int count_substr(const char *str, const char* substr, bool overlap) {
  if (strlen(substr) == 0) return -1; // forbid empty substr

  int count = 0;
  int increment = overlap ? 1 : strlen(substr);
  char* s = NULL;
  for ( s =(char*)str; (s = strstr(s, substr)); s += increment)
    ++count;
  return count;
}

void splitCustom(const char *str, char sep, splitCustom_fn fun, char * outBuf, int indexToLeftOut)
{
    unsigned int start = 0, stop = 0;
    for (stop = 0; str[stop]; stop++) {
        if (str[stop] == sep) {
            fun(str + start, stop - start, outBuf, indexToLeftOut, &sep);
            start = stop + 1;
        }
    }
    fun(str + start, stop - start, outBuf, indexToLeftOut, &sep);
}

//this callback debugs every character separated from splitCustom()
/*
void print(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    if(indexParse != indexToLeftOut){
        char localBuf[MAX_TGDSFILENAME_LENGTH+1];
        snprintf(localBuf,len+1,"%s",str);
        printf(" %d:%s%s:%d\n", (int)len, localBuf, delim, indexParse);
        indexParse++;
    }
}
*/

//this callback builds an output path (outBuf) and filters out the desired index. (used as a trim last directory callback)
void buildPath(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    if(indexParse != indexToLeftOut){
        if(strlen(outBuf) == 0){
            snprintf(outBuf,len+2,"%s%s",str, delim);
        }
        else{
            char localBuf[MAX_TGDSFILENAME_LENGTH+1];
            sprintf(localBuf,"%s",outBuf);
            snprintf(outBuf,strlen(outBuf)+len+2,"%s%s%s",localBuf,str,delim);
        }
        indexParse++;
    }
}


//this callback splits the haystack found in a stream, in the outBuf
void splitCallback(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    snprintf( ((char*)outBuf + (indexParse*256)), len+1, "%s", str);
    indexParse++;
} 

int getLastDirFromPath(char * stream, char * haystack, char * outBuf){
    indexParse = 0;
    //leading / always src stream
    int topval = strlen(stream); 
    if(stream[topval-1] != '/'){
        stream[topval-1] = '/';
    }
    int indexToLeftOut = count_substr(stream, haystack, false);
    int indexToLeftOutCopy = indexToLeftOut;
    if(indexToLeftOutCopy > 1){ //allow index 0 to exist, so it's always left the minimum directory
        indexToLeftOutCopy--;
    }
    splitCustom(stream, (char)*haystack, buildPath, outBuf, indexToLeftOutCopy);
    //remove 0: out stream
    topval = strlen(outBuf) + 1;
    if((outBuf[0] == '0') && (outBuf[1] == ':')){
        char temp[MAX_TGDSFILENAME_LENGTH+1];
        snprintf(temp,topval-2,"%s",(char*)&outBuf[2]);
        sprintf(outBuf,"%s",temp);
    }
	//remove leading / in out stream 
    topval = strlen(outBuf); 
    if(outBuf[topval-1] == '/'){
        outBuf[topval-1] = '\0';
		
		//count how many slashes there are, if zero, force dir to be "/"
		int count = 0;
		int iter = topval-1;
		while(iter >= 0){
			if(outBuf[iter] == '/'){
				count++;
			}
			iter--;
		}
		if(count == 0){
			topval = 1;
			outBuf[topval] = '\0';
		}
	}
	//edge case: the only directory was the leading / and was just removed, if so restore item
    if(topval == 1){
        outBuf[topval-1] = '/';
    }
	//edge case: remove double leading / 
	if((outBuf[0] == '/') && (outBuf[1] == '/')){
		char temp[MAX_TGDSFILENAME_LENGTH+1];
		snprintf(temp,strlen(outBuf)+1-1,"%s",(char*)&outBuf[1]);	//strlen(charBuf) +1 ending char - current offset we start to copy from
		sprintf(outBuf,"%s",temp);
	}
    return indexToLeftOut;
}

int str_split(char * stream, char * haystack, char * outBuf, int itemSize, int blockSize){
	int i = 0;
	for(i = 0; i < itemSize; i++){
		*( outBuf + (i*blockSize) ) = '\0';
	}
	
	indexParse = 0;
    int indexToLeftOut = count_substr(stream, haystack, false);
    splitCustom(stream, (char)*haystack, splitCallback, outBuf, indexToLeftOut);
    return indexToLeftOut;
}


void RenderTGDSLogoMainEngine(u8 * compressedLZSSBMP, int compressedLZSSBMPSize){
	initFBModeMainEngine0x06000000();
	struct LZSSContext LZSSCtx = LZS_DecodeFromBuffer(compressedLZSSBMP, (unsigned int)compressedLZSSBMPSize);
	//These are hardcoded: TGDSLogoLZSSCompressed.bin -> Size: 12.442 / CRC32: e7255f11
	#define TGDSLOGONDSSIZE_SIZE 98304
	#define TGDSLOGONDSSIZE_LENGTH 49152
	#define TGDSLOGONDSSIZE_WIDTH 256
	#define TGDSLOGONDSSIZE_HEIGHT 192
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)LZSSCtx.bufferSource, (int)LZSSCtx.bufferSize);
	renderFBMode3Engine((u16*)LZSSCtx.bufferSource, (u16*)0x06000000, (int)TGDSLOGONDSSIZE_WIDTH,(int)TGDSLOGONDSSIZE_HEIGHT);
	
	//used? discard
	TGDSARM9Free(LZSSCtx.bufferSource);
}

int globalArgc=0; 
char **globalArgv=NULL;

void setGlobalArgc(int argcVal){
	globalArgc = argcVal;
}
int getGlobalArgc(){
	return globalArgc;
}

void setGlobalArgv(char** argvVal){
	globalArgv = argvVal;
}

char** getGlobalArgv(){
	return globalArgv;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
char thisArgv[argvItems][MAX_TGDSFILENAME_LENGTH];

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int thisArgc=0;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleARGV(){
	//Command line vector. Will be reused.
	char** cmdLineVectorPosixCompatible = ((char**)0x02FFFE70);
	
	if(__system_argv->length > 0){
		//get string count (argc) from commandLine
		int argCount=0;
		int internalOffset = 0;
		int i = 0;
		for(i = 0; i < __system_argv->length; i++){
			
			//End of N ARGV? Pass the pointer into the command line vector
			if(__system_argv->commandLine[i] == '\0'){
				thisArgv[argCount][internalOffset] = '\0';
				argCount++;
				internalOffset=0;
			}
			else{
				thisArgv[argCount][internalOffset] = __system_argv->commandLine[i];
				internalOffset++;
			}
		}
		
		//Actually re-count Args, because garbage may be in ARGV code causing false positives.
		//Also it is safe to trash the original __system_argv->commandLine struct
		int argBugged = argCount;
		argCount = 0;
		
		//Reset the command line vector
		memset(cmdLineVectorPosixCompatible, 0, sizeof(struct __argv));
		
		for (i=0; i<argBugged; i++){
			if (thisArgv[i]) {
				if(strlen(thisArgv[i]) > 8){
					//Libnds compatibility: If (recv) mainARGV fat:/ change to 0:/
					char thisARGV[MAX_TGDSFILENAME_LENGTH+1];
					memset(thisARGV, 0, sizeof(thisARGV));
					strcpy(thisARGV, thisArgv[i]);
					
					if(
						(thisARGV[0] == 'f')
						&&
						(thisARGV[1] == 'a')
						&&
						(thisARGV[2] == 't')
						&&
						(thisARGV[3] == ':')
						&&
						(thisARGV[4] == '/')
						){
						char thisARGV2[MAX_TGDSFILENAME_LENGTH+1];
						memset(thisARGV2, 0, sizeof(thisARGV2));
						strcpy(thisARGV2, "0:/");
						strcat(thisARGV2, &thisARGV[5]);
						
						//copy back
						memset(thisArgv[i], 0, 256);
						strcpy(thisArgv[i], thisARGV2);
						
						//build the command line vector
						cmdLineVectorPosixCompatible[i] = (char *)&thisArgv[i];
					}
					
					argCount++;
				}
			}
		}
		
		thisArgc = argCount;
	}
	else{
		thisArgc = 0;
	}
	
	setGlobalArgc(thisArgc);
	setGlobalArgv((char**)cmdLineVectorPosixCompatible);
	//extern int main(int argc, char **argv);
	//main(thisArgc,  (char**)cmdLineVectorPosixCompatible);
}

#endif

//Shuts off the NDS
void shutdownNDSHardware(){
	#ifdef NTRMODE
		#ifdef ARM7
			int PMBitsRead = PowerManagementDeviceRead((int)POWMAN_READ_BIT);
			PMBitsRead |= (int)(POWMAN_SYSTEM_PWR_BIT);
			PowerManagementDeviceWrite(POWMAN_WRITE_BIT, (int)PMBitsRead);
		#endif
		
		#ifdef ARM9
			struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
			uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
			fifomsg[60] = (uint32)FIFO_SHUTDOWN_DS;
			fifomsg[61] = (uint32)0;
			SendFIFOWordsITCM(FIFO_POWERMGMT_WRITE, (uint32)fifomsg);
		#endif
	#endif
	
	#ifdef TWLMODE
		#ifdef ARM7
		i2cWriteRegister(I2C_PM, I2CREGPM_RESETFLAG, 1);
		i2cWriteRegister(I2C_PM, I2CREGPM_PWRCNT, 1);
		#endif
		#ifdef ARM9
		//todo
		#endif
		
	#endif
}

//usage:
//setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);	//both lit screens
//setBacklight(POWMAN_BACKLIGHT_TOP_BIT);								//top lit screen
//setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);							//bottom lit screen
//setBacklight(0);														//non-lit both LCD screens (poweroff)
	
int	setBacklight(int flags){
	#ifdef ARM7
		int PMBitsRead = PowerManagementDeviceRead((int)POWMAN_READ_BIT);
		PMBitsRead &= ~(POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT);
		PMBitsRead |= (int)(flags & (POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT));
		PowerManagementDeviceWrite(POWMAN_WRITE_BIT, (int)PMBitsRead);
	#endif
	
	#ifdef ARM9
		struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
		uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
		fifomsg[60] = (uint32)FIFO_SCREENPOWER_WRITE;
		fifomsg[61] = (uint32)(flags);
		SendFIFOWordsITCM(FIFO_POWERMGMT_WRITE, (uint32)fifomsg);
	#endif
	return 0;
}


#ifdef ARM9
__attribute__((section(".itcm")))
#endif
u32 getValueSafe(u32 * buf) {
	#ifdef ARM9
	coherent_user_range_by_size((uint32)buf, 4);
	#endif
	return (u32)(*buf);
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void setValueSafe(u32 * buf, u32 val) {
	(*buf) = (u32)val;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)buf, 4);
	#endif
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
int getValueSafeInt(u32 * buf) {
	#ifdef ARM9
	coherent_user_range_by_size((uint32)buf, 4);
	#endif
	return (int)(*buf);
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void setValueSafeInt(u32 * buf, int val) {
	*((int*)buf) = (int)val;
	#ifdef ARM9
	coherent_user_range_by_size((uint32)buf, 4);
	#endif
}

#ifdef TWLMODE

#ifdef ARM9

//The rest of SD ARM9 code here

//! Enum values for the fifo system commands.
typedef enum {
	SYS_REQ_TOUCH,
	SYS_REQ_KEYS,
	SYS_REQ_TIME,
	SYS_SET_TIME,
	SDMMC_INSERT,
	SDMMC_REMOVE
} FifoSystemCommands;

static void(*SDcallback)(int)=NULL;

//---------------------------------------------------------------------------------
void setSDcallback(void(*callback)(int)) {
//---------------------------------------------------------------------------------
	SDcallback = callback;
}

//---------------------------------------------------------------------------------
// Handle system requests from the arm7
//---------------------------------------------------------------------------------
void systemValueHandler(u32 value, void* data){
//---------------------------------------------------------------------------------
	switch(value) {
	case SDMMC_INSERT:
		if(SDcallback) SDcallback(1);
		break;
	case SDMMC_REMOVE:
		if(SDcallback) SDcallback(0);
		break;
	}
}


#endif

//---------------------------------------------------------------------------------
void enableSlot1() {
//---------------------------------------------------------------------------------
	#ifdef ARM7
	if(isDSiMode()) twlEnableSlot1();
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_REQ_SLOT1_ENABLE, 0xFF);
	#endif
}

//---------------------------------------------------------------------------------
void disableSlot1() {
//---------------------------------------------------------------------------------
	#ifdef ARM7
	if(isDSiMode()) twlDisableSlot1();
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_REQ_SLOT1_DISABLE, 0xFF);
	#endif
}


//---------------------------------------------------------------------------------
void systemSleep(void) {
//---------------------------------------------------------------------------------
}

//---------------------------------------------------------------------------------
int sleepEnabled(void) {
//---------------------------------------------------------------------------------
	return -1;
}

#endif

int enterCriticalSection() {
	int oldIME = REG_IME;
	REG_IME = 0;
	return oldIME;
}

void leaveCriticalSection(int oldIME) {
	REG_IME = oldIME;
}

#ifdef ARM7

#define		REG_SPI_CR		(*((uint16 volatile *) 0x040001C0))	
#define 	REG_SPICNT   (*(vuint16*)0x040001C0)
#define		REG_SPI_DATA	(*((uint16 volatile *) 0x040001C2))	
#define 	REG_SPIDATA   (*(vuint16*)0x040001C2)
#define		BITMASK_SPI_DATA	0xff

int PowerChip_ReadWrite(int cmd, int data) {
	if(cmd&0x80) data=0;
	while(REG_SPI_CR&0x80);
	REG_SPI_CR=0x8802;
	REG_SPI_DATA=cmd;
	while(REG_SPI_CR&0x80);
	REG_SPI_CR=0x8002;
	REG_SPI_DATA=data;
	while(REG_SPI_CR&0x80);
	data=REG_SPI_DATA;
	REG_SPI_CR=0;
	return data;
}

int led_state=0;
void SetLedState(int state) {
	int i;
	if(state>3 || state<0) return;
	if(state!=led_state) {
		led_state=state;
		i=PowerChip_ReadWrite(0x80,0);
		i=i&0xCF;
		i |= state<<4;
		PowerChip_ReadWrite(0,i);
	}
}
#endif

void TurnOnScreens(){
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);	//both lit screens
	
	#ifdef ARM7
	SetLedState(LED_ON);
	#endif
}

void TurnOffScreens(){
	#ifdef ARM7
	setBacklight(0);
	SetLedState(LED_LONGBLINK);
	#endif
	
	#ifdef ARM9
	setBacklight(0);
	#endif
}

#ifdef ARM9
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
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void addARGV(int argc, char *argv){
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
		if(strlen(thisARGV) <= 3){
			
		}
		else{
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
	}
	
	if(strlen(cmdline) <= 3){
		strcpy(cmdline,"");
		__system_argv->argvMagic = ARGV_MAGIC;
		__system_argv->commandLine = cmdline;
		__system_argv->length = 0;
	}
	else{
		__system_argv->argvMagic = ARGV_MAGIC;
		__system_argv->commandLine = cmdline;
		__system_argv->length = cmdlineLen;
		
		// reserved when ARGV receives into main()
		//__system_argv->argc = ;
		//__system_argv->argv = ;
	}
}
#endif

#ifdef ARM9
__attribute__((section(".dtcm")))
u32 reloadStatus = 0;
#endif

#ifdef ARM9
//ToolchainGenericDS-multiboot NDS Binary loader: Requires tgds_multiboot_payload_ntr.bin / tgds_multiboot_payload_twl.bin (TGDS-multiboot Project) in SD root.
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool TGDSMultibootRunNDSPayload(char * filename) {
	char msgDebug[96];
	memset(msgDebug, 0, sizeof(msgDebug));
	int isNTRTWLBinary = isNTROrTWLBinary(filename);
	//NTR mode? Can only boot valid NTR binaries, the rest is skipped.
	if((__dsimode == false) && !(isNTRTWLBinary == isNDSBinaryV1) && !(isNTRTWLBinary == isNDSBinaryV2) ){
		return false;
	}
	//TWL mode? Can only boot valid NTR and TWL binaries, the rest is skipped.
	else if((__dsimode == true) && !(isNTRTWLBinary == isNDSBinaryV1) && !(isNTRTWLBinary == isNDSBinaryV2) && !(isNTRTWLBinary == isTWLBinary) ){
		return false;
	}
	else{
		strcpy((char*)(0x02280000 - (MAX_TGDSFILENAME_LENGTH+1)), filename);	//Arg0:	
		#ifdef NTRMODE
		char * TGDSMBPAYLOAD = "0:/tgds_multiboot_payload_ntr.bin";	//TGDS NTR SDK (ARM9 binaries) emits TGDSMultibootRunNDSPayload() which reloads into NTR TGDS-MB Reload payload
		#endif
		
		#ifdef TWLMODE
		char * TGDSMBPAYLOAD = "0:/tgds_multiboot_payload_twl.bin";	//TGDS TWL SDK (ARM9i binaries) emits TGDSMultibootRunNDSPayload() which reloads into TWL TGDS-MB Reload payload
		#endif
		
		FILE * tgdsPayloadFh = fopen(TGDSMBPAYLOAD, "r");
		if(tgdsPayloadFh != NULL){
			fseek(tgdsPayloadFh, 0, SEEK_SET);
			int	tgds_multiboot_payload_size = FS_getFileSizeFromOpenHandle(tgdsPayloadFh);
			fread((u32*)0x02280000, 1, tgds_multiboot_payload_size, tgdsPayloadFh);
			coherent_user_range_by_size(0x02280000, (int)tgds_multiboot_payload_size);
			fclose(tgdsPayloadFh);
			int ret=FS_deinit();
			//Copy and relocate current TGDS DLDI section into target ARM9 binary
			if(strncmp((char*)&dldiGet()->friendlyName[0], "TGDS RAMDISK", 12) == 0){
				printf("TGDS DLDI detected. Skipping DLDI patch.");
			}
			else{
				bool stat = dldiPatchLoader((data_t *)0x02280000, (u32)tgds_multiboot_payload_size, (u32)&_io_dldi_stub);
				if(stat == false){
					sprintf(msgDebug, "%s%s", "TGDSMultibootRunNDSPayload():DLDI Patch failed. APP does not support DLDI format.", "");
					nocashMessage((char*)&msgDebug[0]);
				}
				else{
					sprintf(msgDebug, "%s%s", "TGDSMultibootRunNDSPayload():DLDI Patch OK.", "");
					nocashMessage((char*)&msgDebug[0]);
				}
			}
			
			REG_IME = 0;
			typedef void (*t_bootAddr)();
			t_bootAddr bootARM9Payload = (t_bootAddr)0x02280000;
			bootARM9Payload();
			
			return true; //should never jump here
		}
		else{
			sprintf(msgDebug, "%s%s", "TGDSMultibootRunNDSPayload(): Missing Payload:", TGDSMBPAYLOAD);
			nocashMessage((char*)&msgDebug[0]);
			printf((char*)&msgDebug[0]);
		}
	}
	return false;
}
#endif

void initSound(){
	#ifdef ARM7
	SoundPowerON(127);		//volume
	#endif
	
	#ifdef ARM9
	SendFIFOWords(FIFO_INITSOUND, 0xFF);
	#endif
}

//Interfaces / Callbacks to connect to libutils

//FIFO
HandleFifoNotEmptyWeakRefLibUtils_fn libutilisFifoNotEmptyCallback = NULL;

//Wifi
wifiUpdateVBLANKARM7LibUtils_fn wifiUpdateVBLANKARM7LibUtilsCallback = NULL;
wifiInterruptARM7LibUtils_fn wifiInterruptARM7LibUtilsCallback = NULL;
timerWifiInterruptARM9LibUtils_fn timerWifiInterruptARM9LibUtilsCallback = NULL;

// SoundStream / Sound Sample ctx
SoundSampleContextInitARM7LibUtils_fn SoundSampleContextInitARM7LibUtilsCallback = NULL;
SoundSampleContextEnableARM7LibUtils_fn SoundSampleContextEnableARM7LibUtilsCallback = NULL;
SoundSampleContextDisableARM7LibUtils_fn SoundSampleContextDisableARM7LibUtilsCallback = NULL;

SoundStreamTimerHandlerARM7LibUtils_fn SoundStreamTimerHandlerARM7LibUtilsCallback = NULL;
SoundStreamStopSoundARM7LibUtils_fn SoundStreamStopSoundARM7LibUtilsCallback = NULL;
SoundStreamSetupSoundARM7LibUtils_fn SoundStreamSetupSoundARM7LibUtilsCallback = NULL;

#ifdef ARM7
initMallocARM7LibUtils_fn initMallocARM7LibUtilsCallback = NULL;
#endif

#ifdef ARM9
SoundStreamStopSoundStreamARM9LibUtils_fn SoundStreamStopSoundStreamARM9LibUtilsCallback = NULL;
SoundStreamUpdateSoundStreamARM9LibUtils_fn SoundStreamUpdateSoundStreamARM9LibUtilsCallback = NULL;
#endif

//Setup components to bse used from ARM9 TGDS project because it decides how much functionality used
#ifdef ARM9
void initializeLibUtils9(
		HandleFifoNotEmptyWeakRefLibUtils_fn HandleFifoNotEmptyWeakRefLibUtilsCall, //ARM7 & ARM9
		timerWifiInterruptARM9LibUtils_fn timerWifiInterruptARM9LibUtilsCall, //ARM9 
		SoundSampleContextEnableARM7LibUtils_fn SoundSampleContextEnableARM7LibUtilsCall, // ARM7 & ARM9: void EnableSoundSampleContext(int SndSamplemode)
		SoundSampleContextDisableARM7LibUtils_fn SoundSampleContextDisableARM7LibUtilsCall,	//ARM7 & ARM9: void DisableSoundSampleContext()
		SoundStreamStopSoundStreamARM9LibUtils_fn SoundStreamStopSoundStreamARM9LibUtilsCall,	//ARM9: bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType)
		SoundStreamUpdateSoundStreamARM9LibUtils_fn SoundStreamUpdateSoundStreamARM9LibUtilsCall //ARM9: void updateStream() 
	){
	libutilisFifoNotEmptyCallback = HandleFifoNotEmptyWeakRefLibUtilsCall;
	timerWifiInterruptARM9LibUtilsCallback = timerWifiInterruptARM9LibUtilsCall;
	SoundSampleContextEnableARM7LibUtilsCallback = SoundSampleContextEnableARM7LibUtilsCall;
	SoundSampleContextDisableARM7LibUtilsCallback = SoundSampleContextDisableARM7LibUtilsCall;
	SoundStreamStopSoundStreamARM9LibUtilsCallback = SoundStreamStopSoundStreamARM9LibUtilsCall;
	SoundStreamUpdateSoundStreamARM9LibUtilsCallback = SoundStreamUpdateSoundStreamARM9LibUtilsCall;
	
	//ARM9 libUtils component initialization
	fifoInit();
}
#endif

#ifdef ARM7
void initializeLibUtils7(
	HandleFifoNotEmptyWeakRefLibUtils_fn HandleFifoNotEmptyWeakRefLibUtilsCall, //ARM7 & ARM9
	wifiUpdateVBLANKARM7LibUtils_fn wifiUpdateVBLANKARM7LibUtilsCall, //ARM7
	wifiInterruptARM7LibUtils_fn wifiInterruptARM7LibUtilsCall,  //ARM7
	SoundStreamTimerHandlerARM7LibUtils_fn SoundStreamTimerHandlerARM7LibUtilsCall, //ARM7: void TIMER1Handler()
	SoundStreamStopSoundARM7LibUtils_fn SoundStreamStopSoundARM7LibUtilsCall, 	//ARM7: void stopSound()
	SoundStreamSetupSoundARM7LibUtils_fn SoundStreamSetupSoundARM7LibUtilsCall,	//ARM7: void setupSound()
	SoundSampleContextInitARM7LibUtils_fn SoundSampleContextInitARM7LibUtilsCall, //ARM7: initSoundSampleContext()
	SoundSampleContextEnableARM7LibUtils_fn SoundSampleContextEnableARM7LibUtilsCall, // ARM7 & ARM9: void EnableSoundSampleContext(int SndSamplemode)
	SoundSampleContextDisableARM7LibUtils_fn SoundSampleContextDisableARM7LibUtilsCall,	//ARM7 & ARM9: void DisableSoundSampleContext()
	initMallocARM7LibUtils_fn initMallocARM7LibUtilsCall	//ARM7: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);
){
	libutilisFifoNotEmptyCallback = HandleFifoNotEmptyWeakRefLibUtilsCall;
	wifiUpdateVBLANKARM7LibUtilsCallback = wifiUpdateVBLANKARM7LibUtilsCall;
	wifiInterruptARM7LibUtilsCallback = wifiInterruptARM7LibUtilsCall;
	SoundStreamTimerHandlerARM7LibUtilsCallback = SoundStreamTimerHandlerARM7LibUtilsCall;
	SoundStreamStopSoundARM7LibUtilsCallback = SoundStreamStopSoundARM7LibUtilsCall;
	SoundStreamSetupSoundARM7LibUtilsCallback = SoundStreamSetupSoundARM7LibUtilsCall;
	SoundSampleContextInitARM7LibUtilsCallback = SoundSampleContextInitARM7LibUtilsCall;
	SoundSampleContextEnableARM7LibUtilsCallback = SoundSampleContextEnableARM7LibUtilsCall;
	SoundSampleContextDisableARM7LibUtilsCallback = SoundSampleContextDisableARM7LibUtilsCall;
	initMallocARM7LibUtilsCallback = initMallocARM7LibUtilsCall;
}
#endif

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int isNTROrTWLBinary(char * filename){
	int mode = notTWLOrNTRBinary;
	FILE * fh = fopen(filename, "r+");
	int headerSize = sizeof(struct sDSCARTHEADER);
	u8 * NDSHeader = (u8 *)TGDSARM9Malloc(headerSize*sizeof(u8));
	u8 passmeRead[16];
	memset(passmeRead, 0, sizeof(passmeRead));
	if (fread(NDSHeader, 1, headerSize, fh) != headerSize){
		TGDSARM9Free(NDSHeader);
		fclose(fh);
		return notTWLOrNTRBinary;
	}
	else{
		fseek(fh, 0xA0, SEEK_SET);
		fread(&passmeRead[0], 1, sizeof(passmeRead), fh);
	}
	struct sDSCARTHEADER * NDSHdr = (struct sDSCARTHEADER *)NDSHeader;
	u32 arm9EntryAddress = NDSHdr->arm9entryaddress;
	u32 arm7EntryAddress = NDSHdr->arm7entryaddress;
	int checkCounter = 0;
	int i = 0;
	for(i = 0; i < sizeof(NDSHdr->reserved1); i++){
		checkCounter += NDSHdr->reserved1[i];
	}
	checkCounter += NDSHdr->reserved2;
	//NTR: (02000000-023FFFFF) 4M
	//TWL: (02000000- 02FFFFFF) 16M
	if(
		(checkCounter == 0) &&
		(arm9EntryAddress >= 0x02000000) && (arm9EntryAddress < 0x02400000) 
		&&
		(
			//passme v1 (pre 2008 NTR homebrew)
			(0x00 == passmeRead[0x0])
			&&	(0x00 == passmeRead[0x1])
			&&	(0x00 == passmeRead[0x2])
			&&	(0x00 == passmeRead[0x3])
			&&	(0x00 == passmeRead[0x4])
			&&	(0x00 == passmeRead[0x5])
			&&	(0x00 == passmeRead[0x6])
			&&	(0x00 == passmeRead[0x7])
			&&	(0x00 == passmeRead[0x8])
			&&	(0x00 == passmeRead[0x9])
			&&	(0x00 == passmeRead[0xA])
			&&	(0x00 == passmeRead[0xB])
			&&	(0x50 == passmeRead[0xC])
			&&	(0x41 == passmeRead[0xD])
			&&	(0x53 == passmeRead[0xE])
			&&	(0x53 == passmeRead[0xF])
		)
	){
		mode = isNDSBinaryV1;
	}
	else if(
		(checkCounter == 0) &&
		(arm9EntryAddress >= 0x02000000) && (arm9EntryAddress < 0x02400000) 
		&&
		(
			//passme v2 (2009+ NTR homebrew)
			(0x53 == passmeRead[0x0])
			&&	(0x52 == passmeRead[0x1])
			&&	(0x41 == passmeRead[0x2])
			&&	(0x4D == passmeRead[0x3])
			&&	(0x5F == passmeRead[0x4])
			&&	(0x56 == passmeRead[0x5])
			&&	(0x31 == passmeRead[0x6])
			&&	(0x31 == passmeRead[0x7])
			&&	(0x30 == passmeRead[0x8])
			&&	(0x00 == passmeRead[0x9])
			&&	(0x00 == passmeRead[0xA])
			&&	(0x00 == passmeRead[0xB])
		)
	){
		mode = isNDSBinaryV2;
	}
	
	//TWL validates both ARM7 and ARM9 entry address
	else if( 
		(checkCounter >= 0) && (arm9EntryAddress >= 0x02000000) && (arm9EntryAddress <= 0x02FFFFFF) &&
		(
			((arm7EntryAddress >= 0x02000000) && (arm7EntryAddress <= 0x02FFFFFF))
			||
			((arm7EntryAddress >= 0x037F8000) && (arm7EntryAddress <= 0x03810000))
		)
	){
		mode = isTWLBinary;
	}
	TGDSARM9Free(NDSHeader);
	fclose(fh);
	return mode;
}
#endif