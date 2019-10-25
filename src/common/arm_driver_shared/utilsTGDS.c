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
#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include "reent.h"	//sbrk

#ifdef ARM9
#include "dswnifi_lib.h"
#endif


//

//linker to C proper memory layouts.

//Shared:

//these toggle the WRAM_CR register depending on linker settings

uint32 get_arm7_start_address(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	return (uint32)TGDSIPC->arm7startaddress;
}

uint32 get_arm7_end_address(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	return (uint32)TGDSIPC->arm7endaddress;
}

sint32 get_arm7_ext_size(){
	return (sint32)((uint8*)(uint32*)get_arm7_end_address() - (sint32)(get_arm7_start_address()));
}


uint32 get_arm9_start_address(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	return (uint32)TGDSIPC->arm9startaddress;
}

uint32 get_arm9_end_address(){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	return (uint32)TGDSIPC->arm9endaddress;
}

sint32 get_arm9_ext_size(){
	return (sint32)((uint8*)(uint32*)get_arm9_end_address() - (sint32)(get_arm9_start_address()));
}


#ifdef ARM7
uint32 get_iwram_start(){
	return (uint32)(&_iwram_start);
}

sint32 get_iwram_size(){
	return (sint32)((uint8*)(uint32*)&_iwram_end - (sint32)(&_iwram_start));
}

uint32 get_iwram_end(){
	return (uint32)(&_iwram_end);
}
#endif

//Physical Memory Start: [ARM7/9 bin start ~ ARM7/9 bin end, 4 bytes alignment, get_lma_libend() <---- ] ------------------------------------------------------------ get_lma_wramend()
uint32 get_lma_libend(){
	u32 wram_start = (u32)&__lib__end__;
	return (uint32)((wram_start + (4 - 1)) & -4);  // Round up to 4-byte boundary // linear memory top (start)
}

//Physical Memory Start: [ARM7/9 bin start ~ ARM7/9 bin end, 4 bytes alignment, get_lma_libend()] ------------------------------------------------------------ get_lma_wramend() <----
uint32 get_lma_wramend(){
	#ifdef ARM7
	extern uint32 sp_USR;	//the farthest stack from the end memory is our free memory (in ARM7, shared stacks)
	u32 wram_end = ((uint32)&sp_USR - 0x400);
	return (uint32)((wram_end + (4 - 1)) & -4);
	#endif
	#ifdef ARM9
	u32 wram_end = (u32)&_ewram_end;
	return (uint32)((wram_end + (4 - 1)) & -4);  // Round up to 4-byte boundary // EWRAM has no stacks shared so we use the end memory
	#endif
}

#ifdef ARM9
uint32 get_ewram_start(){
	return (uint32)(&_ewram_start);
}

sint32 get_ewram_size(){
	return (sint32)((uint8*)(uint32*)get_lma_wramend() - (sint32)(&_ewram_start));
}

uint32 get_itcm_start(){
	return (uint32)(&_itcm_start);
}

sint32 get_itcm_size(){
	return (sint32)((uint8*)(uint32*)get_lma_wramend() - (sint32)(&_itcm_start));
}

uint32 get_dtcm_start(){
	return (uint32)(&_dtcm_start);
}

sint32 get_dtcm_size(){
	return (sint32)((uint8*)(uint32*)&_dtcm_end - (sint32)(&_dtcm_start));
}

#endif


/* ------------------------------------------------------------------------- */
/*!Increase program data space
   This is a minimal implementation.  Assuming the heap is growing upwards
   from __heap_start towards __heap_end.
   See linker file for definition.
   @param[in] incr  The number of bytes to increment the stack by.
   @return  A pointer to the start of the new block of memory                */
/* ------------------------------------------------------------------------- */

volatile char 		*heap_end = NULL;		/* Previous end of heap or 0 if none */
volatile char        *prev_heap_end = NULL;

void * _sbrk (int  incr)
{
	if (heap_end == NULL) {
		heap_end = (sint8*)get_lma_libend();			/* Initialize first time round */
	}

	prev_heap_end  = heap_end;
	heap_end      += incr;
	//check
	if( heap_end < (char*)get_lma_wramend()) {

	} else {
		#ifdef ARM9
		errno = ENOMEM;
		#endif
		return (char*)-1;
	}
	return (void *) prev_heap_end;

}	/* _sbrk () */

void * _sbrk_r (struct _reent * reent, int size){
	return _sbrk (size);
}

void Write8bitAddrExtArm(uint32 address, uint8 value){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->ipcmsg[0];
	fifomsg[0] = address;
	fifomsg[1] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_8, (uint32)fifomsg);
}

void Write16bitAddrExtArm(uint32 address, uint16 value){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->ipcmsg[0];
	fifomsg[0] = address;
	fifomsg[1] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_16, (uint32)fifomsg);
}

void Write32bitAddrExtArm(uint32 address, uint32 value){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->ipcmsg[0];
	fifomsg[0] = address;
	fifomsg[1] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_32, (uint32)fifomsg);
}

//NDS Memory Map (valid):
//todo: detect valid maps according to MPU settings
#ifdef ARM9
__attribute__ ((hot))
__attribute__((section(".itcm")))
bool isValidMap(uint32 addr){
	if(
		#ifdef ARM9
		((addr >= (uint32)0x06000000) && (addr < (uint32)0x06020000))	//NDS VRAM BG Engine Region protected	0x06000000 - 0x0601ffff
		||
		((addr >= (uint32)0x06020000) && (addr < (uint32)0x06040000))	//NDS VRAM BG Engine Region protected	0x06020000 - 0x0603ffff
		||
		((addr >= (uint32)0x06040000) && (addr < (uint32)0x06060000))	//NDS VRAM BG Engine Region protected	0x06040000 - 0x0605ffff
		||
		((addr >= (uint32)0x06060000) && (addr < (uint32)0x06080000))	//NDS VRAM BG Engine Region protected	0x06060000 - 0x0607ffff
		||
		((addr >= (uint32)0x06200000) && (addr < (uint32)(0x06200000 + 0x20000)))	//NDS VRAM Engine Region protected	//theoretical map
		||
		((addr >= (uint32)0x06400000) && (addr < (uint32)(0x06400000 + 0x14000)))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x06600000) && (addr < (uint32)(0x06600000 + 0x20000)))	//NDS VRAM Engine Region protected	// theoretical map
		||
		((addr >= (uint32)0x06800000) && (addr < (uint32)(0x06800000 + (656 * 1024) )))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x07000000) && (addr < (uint32)(0x07000000 + (2 * 1024) )))	//NDS VRAM OAM Region protected	// theoretical map?
		||
		((addr >= (uint32)get_ewram_start()) && (addr <= (uint32)(get_ewram_start() + get_ewram_size())))	//NDS EWRAM Region protected
		||
		((addr >= (uint32)(get_itcm_start())) && (addr <= (uint32)(get_itcm_start()+get_itcm_size())))	//NDS ITCM Region protected
		||
		((addr >= (uint32)(get_dtcm_start())) && (addr <= (uint32)(get_dtcm_start()+get_dtcm_size())))	//NDS DTCM Region protected
		||
		((addr >= (uint32)(0x05000000)) && (addr <= (uint32)(0x05000000 + 2*1024)))	//NDS Palette Region protected
		||
		( ((WRAM_CR & WRAM_32KARM9_0KARM7) == WRAM_32KARM9_0KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 32K ARM9 / 0K ARM7 Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) == WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )	//NDS Shared WRAM 16K ARM9 (first half) / 16K ARM7 Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) == WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )	//NDS Shared WRAM 16K ARM9 (second half) / 16K ARM7 Region protected
		#endif
		#ifdef ARM7
		((addr >= (uint32)get_iwram_start()) && (addr <= (uint32)(get_iwram_start() + get_iwram_size())))	//NDS IWRAM Region protected
		||
		( ((WRAM_CR & WRAM_0KARM9_32KARM7) == WRAM_0KARM9_32KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 0K ARM9 / 32K ARM7 Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) == WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )	//NDS Shared WRAM 16K ARM9 / 16K ARM7 (first half) Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) == WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )	//NDS Shared WRAM 16K ARM9 / 16K ARM7 (second half) Region 
		#endif
		||
		((addr >= (uint32)(0x04000000)) && (addr <= (uint32)(0x04000000 + 4*1024)))	//NDS IO Region protected
		||
		((addr >= (uint32)(0x08000000)) && (addr <= (uint32)(0x08000000 + (32*1024*1024))))	//GBA ROM MAP (allows to read GBA carts over GDB)
		#ifdef ARM9
		||
		((addr >= (uint32)(GDBMapFileAddress)) &&  (addr >= (uint32)(minGDBMapFileAddress)) && (addr < (uint32)(maxGDBMapFileAddress)))	//GDB File stream 
		#endif
	){
		return true;
	}
	return false;
}
#endif


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
		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
		uint32 * fifomsg = (uint32 *)&TGDSIPC->ipcmsg[0];
		fifomsg[0] = (uint32)FIFO_SCREENPOWER_WRITE;
		fifomsg[1] = (uint32)(flags);
		SendFIFOWords(FIFO_POWERMGMT_WRITE, (uint32)fifomsg);
	#endif
	return 0;
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
//returns: written data size
int	FS_saveFile(sint8 *filename, sint8 *buf, int size, bool force_file_creation){
	FILE * f;
	char var[16];
	int retWritten = -1;
	if(force_file_creation == true){
		sprintf(var,"%s","w+");
	}
	else{
		sprintf(var,"%s","w");
	}
	if( (f = fopen(filename, (sint8*)var)) != NULL){
		retWritten = fwrite(buf, 1, size, f);
		fclose(f);
  	}
	return retWritten;
}

int	FS_getFileSize(sint8 *filename){
	FILE * f = fopen(filename, "r");
	int size = -1;
	if (f != NULL){
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		fclose(f);
	}
	FS_unlock();
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

//char * bufOut is returned so you can use the return function 
char * print_ip(uint32 ip, char * bufOut){
    uint8 bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;	
    sprintf(bufOut,"%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
	return bufOut;
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
	return fatfs_chdir(path);
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
				struct fd * fdinst = getStructFD(pent->d_ino);	//struct stat st is generated at the moment readdir(); is called, so get access to it through fdinst->stat
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
				struct fd * fdinst = getStructFD(pent->d_ino);
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


/*
* Copyright (C) 1996-2001  Internet Software Consortium.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
* DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
* INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
* FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
* WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "socket.h"


#define NS_INT16SZ       2
#define NS_INADDRSZ      4
#define NS_IN6ADDRSZ    16

/*
* WARNING: Don't even consider trying to compile this on a system where
* sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
*/

static int inet_pton4(const char *src, unsigned char *dst);
static int inet_pton6(const char *src, unsigned char *dst);

/* int
* isc_net_pton(af, src, dst)
*      convert from presentation format (which usually means ASCII printable)
*      to network format (which is usually some kind of binary format).
* return:
*      1 if the address was valid for the specified address family
*      0 if the address wasn't valid (`dst' is untouched in this case)
*      -1 if some other error occurred (`dst' is untouched in this case, too)
* author:
*      Paul Vixie, 1996.
*/
int inet_pton(int af, const char *src, void *dst)
{
	switch (af) {
	case AF_INET:
		return (inet_pton4(src, dst));
	#ifdef INET6
	case AF_INET6:
		return (inet_pton6(src, dst));
	#endif
	default:
		errno = EAFNOSUPPORT;
	return (-1);
}
/* NOTREACHED */
}

/* int
* inet_pton4(src, dst)
*      like inet_aton() but without all the hexadecimal and shorthand.
* return:
*      1 if `src' is a valid dotted quad, else 0.
* notice:
*      does not touch `dst' unless it's returning 1.
* author:
*      Paul Vixie, 1996.
*/
static int inet_pton4(src, dst)
const char *src;
unsigned char *dst;
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[NS_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;
		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int new = *tp * 10 + (pch - digits);
			if (new > 255)
				return (0);
			*tp = new;
			if (! saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} 
		else
			return (0);
	}
	if (octets < 4)
		return (0);
	memcpy(dst, tmp, NS_INADDRSZ);
	return (1);
}

/* int
* inet_pton6(src, dst)
*      convert presentation level address to network order binary form.
* return:
*      1 if `src' is a valid [RFC1884 2.2] address, else 0.
* notice:
*      (1) does not touch `dst' unless it's returning 1.
*      (2) :: in a full address is silently ignored.
* credit:
*      inspired by Mark Andrews.
* author:
*      Paul Vixie, 1996.
*/
#ifdef INET6
static int inet_pton6(const char *src, unsigned char *dst)
{
	static const char xdigits_l[] = "0123456789abcdef",
	xdigits_u[] = "0123456789ABCDEF";
	unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, saw_xdigit;
	unsigned int val;

	memset((tp = tmp), '\0', NS_IN6ADDRSZ);
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			return (0);
	
	curtok = src;
	saw_xdigit = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;
		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (val > 0xffff)
				return (0);
			saw_xdigit = 1;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			}
			if (tp + NS_INT16SZ > endp)
				return (0);
			*tp++ = (unsigned char) (val >> 8) & 0xff;
			*tp++ = (unsigned char) val & 0xff;
			saw_xdigit = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) && inet_pton4(curtok, tp) > 0) {
			tp += NS_INADDRSZ;
			saw_xdigit = 0;
			break;  /* '\0' was seen by inet_pton4(). */
		}
		return (0);
	}
	if (saw_xdigit) {
		if (tp + NS_INT16SZ > endp)
			return (0);
		*tp++ = (unsigned char) (val >> 8) & 0xff;
		*tp++ = (unsigned char) val & 0xff;
	}
	if (colonp != NULL) {
		/*
		* Since some memmove()'s erroneously fail to handle
		* overlapping regions, we'll do the shift by hand.
		*/
		const int n = tp - colonp;
		int i = 0;
		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memcpy(dst, tmp, NS_IN6ADDRSZ);
	return (1);
}
#endif



/*
 * Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif
/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */
static const char *inet_ntop4 (const u_char *src, char *dst, socklen_t size);
static const char *inet_ntop6 (const u_char *src, char *dst, socklen_t size);
/* char *
 * inet_ntop(af, src, dst, size)
 *        convert a network format address to presentation format.
 * return:
 *        pointer to presentation format address (`dst'), or NULL (see errno).
 * author:
 *        Paul Vixie, 1996.
 */
const char *
inet_ntop (int af, const void *src, char *dst, socklen_t size)
{
        switch (af) {
        case AF_INET:
                return (inet_ntop4(src, dst, size));
        case AF_INET6:
                return (inet_ntop6(src, dst, size));
        default:
                __set_errno (EAFNOSUPPORT);
                return (NULL);
        }
        /* NOTREACHED */
}

/* const char *
 * inet_ntop4(src, dst, size)
 *        format an IPv4 address
 * return:
 *        `dst' (as a const)
 * notes:
 *        (1) uses no statics
 *        (2) takes a u_char* not an in_addr as input
 * author:
 *        Paul Vixie, 1996.
 */

static const char *
inet_ntop4 (const u_char *src, char *dst, socklen_t size)
{
        static const char fmt[] = "%u.%u.%u.%u";
        char tmp[sizeof "255.255.255.255"];
        if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) >= size) {
                __set_errno (ENOSPC);
                return (NULL);
        }
        return strcpy(dst, tmp);
}
/* const char *
 * inet_ntop6(src, dst, size)
 *        convert IPv6 binary address into presentation (printable) format
 * author:
 *        Paul Vixie, 1996.
 */
static const char *
inet_ntop6 (const u_char *src, char *dst, socklen_t size)
{
        /*
         * Note that int32_t and int16_t need only be "at least" large enough
         * to contain a value of the specified size.  On some systems, like
         * Crays, there is no such thing as an integer variable with 16 bits.
         * Keep this in mind if you think this function should have been coded
         * to use pointer overlays.  All the world's not a VAX.
         */
        char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
        struct { int base, len; } best, cur;
        u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
        int i;
        /*
         * Preprocess:
         *        Copy the input (bytewise) array into a wordwise array.
         *        Find the longest run of 0x00's in src[] for :: shorthanding.
         */
        memset(words, '\0', sizeof words);
        for (i = 0; i < NS_IN6ADDRSZ; i += 2)
                words[i / 2] = (src[i] << 8) | src[i + 1];
        best.base = -1;
        cur.base = -1;
        best.len = 0;
        cur.len = 0;
        for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
                if (words[i] == 0) {
                        if (cur.base == -1)
                                cur.base = i, cur.len = 1;
                        else
                                cur.len++;
                } else {
                        if (cur.base != -1) {
                                if (best.base == -1 || cur.len > best.len)
                                        best = cur;
                                cur.base = -1;
                        }
                }
        }
        if (cur.base != -1) {
                if (best.base == -1 || cur.len > best.len)
                        best = cur;
        }
        if (best.base != -1 && best.len < 2)
                best.base = -1;
        /*
         * Format the result.
         */
        tp = tmp;
        for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
                /* Are we inside the best run of 0x00's? */
                if (best.base != -1 && i >= best.base &&
                    i < (best.base + best.len)) {
                        if (i == best.base)
                                *tp++ = ':';
                        continue;
                }
                /* Are we following an initial run of 0x00s or any real hex? */
                if (i != 0)
                        *tp++ = ':';
                /* Is this address an encapsulated IPv4? */
                if (i == 6 && best.base == 0 &&
                    (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
                        if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)))
                                return (NULL);
                        tp += strlen(tp);
                        break;
                }
                tp += SPRINTF((tp, "%x", words[i]));
        }
        /* Was it a trailing run of 0x00's? */
        if (best.base != -1 && (best.base + best.len) ==
            (NS_IN6ADDRSZ / NS_INT16SZ))
                *tp++ = ':';
        *tp++ = '\0';
        /*
         * Check for overflow, copy, and we're done.
         */
        if ((socklen_t)(tp - tmp) > size) {
                __set_errno (ENOSPC);
                return (NULL);
        }
        return strcpy(dst, tmp);
}
#endif
