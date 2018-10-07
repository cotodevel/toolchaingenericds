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

#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "spifwTGDS.h"

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
#include "fsfatlayerTGDS.h"
#include "posixHandleTGDS.h"
#include "fileHandleTGDS.h"


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
volatile char app_version_static[MAX_TGDSFILENAME_LENGTH+1];

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
	#ifndef readvertoolchain
	return -1;
	#endif
	
	//Toolchain version
	if(strlen(readvertoolchain) > 0){
		memcpy((uint8*)versionInst->framework_version,readvertoolchain,strlen(readvertoolchain));
	}
	else{
		return -1;
	}
	//
	//Application Compiled Version
	if(strlen((char*)app_version_static) > 0){
		memcpy((uint8*)versionInst->app_version,(char*)app_version_static,strlen((char*)app_version_static));
	}
	else{
		return -1;		
	}
	
	return 0;
}

//Writes to versionInst the current version read from emuCore, plus timestamp from file
//returns -1 if readvertoolchain could not be read (compile time) or timestamp is invalid
//returns -2 if config is not set
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
	
	char methodfilename[MAX_TGDSFILENAME_LENGTH+1];
	sprintf(methodfilename,"%s.bin",method_inst->methodname);
	//replace by open source file parser: GUI_setConfigStrUpdateFile((char*)"AssemblyCore", (char*)method_inst->methodname, (char*)methodfilename);
	callback_export_file(methodfilename,method_inst);
	
	return 0;
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

//usage:
//setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);	//both lit screens
//setBacklight(POWMAN_BACKLIGHT_TOP_BIT);								//top lit screen
//setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);							//bottom lit screen
//setBacklight(0);														//non-lit both LCD screens (poweroff)
	
int	setBacklight(int flags)
{
	SendMultipleWordACK(FIFO_POWERMGMT_WRITE, UPDATE_TOP_SCREEN_PWR|UPDATE_BOTTOM_SCREEN_PWR, (uint32)(flags), NULL);
	return 0;
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


sint8 ip_decimal[0x10] = {0};

sint8 * print_ip(uint32 ip)
{
    uint8 bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;	
    sprintf(ip_decimal,"%d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);        

return ip_decimal;
}




//FileSystem utils


sint8 *_FS_getFileExtension(sint8 *filename){
	static sint8 ext[4];
	sint8	*ptr = filename;
	int		i = 0;
	
	do{
		ptr = strchr(ptr, '.');
		if (!ptr)
			return NULL;
		ptr++;
	}
	while (strlen(ptr) > 3);
	
	for (i = 0; i < (int)strlen(ptr); i++){
		ext[i] = toupper((int)(ptr[i])); 
	}
	ext[i] = 0;
	return ext;
}

sint8 *FS_getFileName(sint8 *filename){
	static sint8 name[100];
	sint8	*ptr = filename;
	int		i = 0;
	ptr = strrchr(ptr, '.');
	
	for (i = 0; i < ptr-filename; i++){
		name[i] = filename[i]; 
	}
	name[i] = 0;
	return name;
}

int		FS_chdir(const sint8 *path){
	int ret = fatfs_chdir(path);
	return ret;
}


//This is an example that uses opendir(); and readdir(); to iterate through dir/file contents.
sint8	**FS_getDirectoryList(sint8 *path, sint8 *mask, int *cnt){
	int	size = 0;
	*cnt = size;
	FS_lock();	
	DIR *dir = opendir (path);
	if( NULL != dir ){
		while (1){
			struct dirent* pent = readdir(dir);
			if(pent != NULL){
				struct fd * fdinst = fd_struct_get(pent->d_ino);	//struct stat st is generated at the moment readdir(); is called, so get access to it through fdinst->stat
				if(fdinst){
					if(!S_ISDIR(fdinst->stat.st_mode)){
						continue;
					}
					if(!strcmp(pent->d_name, ".")){
						continue;
					}
					if(mask){
						sint8 *ext = _FS_getFileExtension(pent->d_name);
						if (ext && strstr(mask, ext)){
							//filecount Increase
							(*cnt)++;
							size += strlen(pent->d_name)+1;
						}
					}
					else{
						//filecount Increase
						(*cnt)++;
						size += strlen(pent->d_name)+1;
					}
				}
			}
			else{
				break;
			}
		}
	}
	rewinddir(dir);
	
	sint8	**list = (sint8	**)malloc((*cnt)*sizeof(sint8 *)+size);
	sint8	*ptr = ((sint8 *)list) + (*cnt)*sizeof(sint8 *);
	
	int i = 0; 
	if(NULL != dir){
		while (1){
			struct dirent* pent = readdir(dir);	//if NULL already not a dir
			if(pent != NULL){
				struct fd * fdinst = fd_struct_get(pent->d_ino);
				if(fdinst){
					if(!S_ISDIR(fdinst->stat.st_mode)){
						continue;
					}
					if(!strcmp(pent->d_name, ".")){
						continue;
					}
					if(mask){
						sint8 *ext = _FS_getFileExtension(pent->d_name);
						if (ext && strstr(mask, ext)){
							strcpy(ptr, pent->d_name);
							list[i++] = ptr;
							ptr += strlen(pent->d_name)+1;  
						}
					}
					else{
						strcpy(ptr, pent->d_name);
						list[i++] = ptr;
						ptr += strlen(pent->d_name)+1;
					}
				}
			}
			else{
				break;
			}
		}
	}
	closedir(dir);
	FS_unlock();
	return list;
}


//taken from https://stackoverflow.com/questions/9052490/find-the-count-of-substring-in-string
//modified by Coto
int count_substr(const char *str, const char* substr, bool overlap) {
  if (strlen(substr) == 0) return -1; // forbid empty substr

  int count = 0;
  int increment = overlap ? 1 : strlen(substr);
  for (char* s = (char*)str; (s = strstr(s, substr)); s += increment)
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

int indexParse = 0;
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
char * outSplitBuf[TOP_ITEMS_SPLIT][MAX_TGDSFILENAME_LENGTH+1];
void splitCallback(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    snprintf((char*)&outSplitBuf[indexParse][0],len+1,"%s",str);
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

int str_split(char * stream, char * haystack, char * outBuf){
	indexParse = 0;
    int indexToLeftOut = count_substr(stream, haystack, false);
    splitCustom(stream, (char)*haystack, splitCallback, outBuf, indexToLeftOut);
    return indexToLeftOut;
}

#endif
