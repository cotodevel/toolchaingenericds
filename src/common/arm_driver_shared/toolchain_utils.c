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

#include "toolchain_utils.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "common_shared.h"



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

#include "devoptab_devices.h"
#include "fsfat_layer.h"
#include "posix_hook_shared.h"
#include "toolchain_utils.h"

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
		return strdup (string);
	}

	newstr = strdup (string);

	while ( ( tok = strstr( newstr, substr ) ) )
	{
		oldstr = newstr;
		newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );

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
	
	f = fopen_fs(filename, "r");	//old: f = fopen(filename, "rb");
	if (f == NULL)
	{
		return -1;
	}
	fseek_fs(f, 0, SEEK_END);	//old: fseek(f, 0, SEEK_END);
	file_size = ftell_fs(f);		//old:	file_size = ftell(f);
	if (file_size < size)
	{
		fclose_fs(f);	//old: fclose(f);
		return -1;
	}
	
    fseek_fs(f, 0, SEEK_SET);	//old: fseek(f, 0, SEEK_SET);
    fread_fs(buf, 1, size, f);		//old: fread(buf, 1, size, f);
	
    fclose_fs(f);	//old:	fclose(f);	
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
	
	if((f = fopen_fs(filename, (sint8*)var)) == NULL)
	{
		return -1;
  	}
	
	fwrite_fs(buf, 1, size, f);	//old: fwrite(buf, 1, size, f);
	fclose_fs(f);	//old: fclose(f);	
	return 0;
}

int	FS_getFileSize(sint8 *filename)
{	
	FILE * f = fopen_fs(filename, "r");
	if (f == NULL)
	{
		FS_unlock();
		return -1;
	}
	fseek_fs(f, 0, SEEK_END);
	int size = ftell_fs(f);
	fseek_fs(f, 0, SEEK_SET);
	fclose_fs(f);
	
	return size;
}

int	setBacklight(int flags)
{
	#ifdef ARM7
	//todo ARM7
	#endif
	
	#ifdef ARM9
	//SendArm7Command(0x00000008,(flags << 16),0x00000000,0x00000000);
	SendMultipleWordACK(0x00000008, (flags << 16), 0, NULL);
	#endif
	
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


sint8	**FS_getDirectoryList(sint8 *path, sint8 *mask, int *cnt)
{	
	int			size;
		
	FS_lock();	
	DIR *dir = opendir (path); 
	*cnt = size = 0;
	if( NULL != dir )
	{
		while (1)
		{
			struct dirent* pent = readdir(dir);
			if(pent == NULL){
				break;
			}
			
			struct fd * fdinst = fd_struct_get(pent->d_ino);
			if(fdinst){
				//OK
			}
			else{
				//NULL!. This should never happen.
				continue;
			}
			
			if (!S_ISDIR(fdinst->stat.st_mode)) { 
				continue;
			}
			
			if (!strcmp(pent->d_name, ".")){
				continue;
			}
			
			if (mask)
			{
				sint8 *ext = _FS_getFileExtension(pent->d_name);
				if (ext && strstr(mask, ext))
				{
					//filecount Increase
					(*cnt)++;
					size += strlen(pent->d_name)+1;
				}
			} 
			else
			{
				//filecount Increase
				(*cnt)++;
				size += strlen(pent->d_name)+1;
			}
		}
	}
	rewinddir(dir);
	
	sint8	**list = (sint8	**)malloc((*cnt)*sizeof(sint8 *)+size);
	sint8	*ptr = ((sint8 *)list) + (*cnt)*sizeof(sint8 *);
	
	int i = 0; 
	if( NULL != dir )
	{
		while (1)
		{
			struct dirent* pent = readdir(dir);	//if NULL already not a dir
			if(pent == NULL){
				break;
			}
			
			struct fd * fdinst = fd_struct_get(pent->d_ino);
			if (!S_ISDIR(fdinst->stat.st_mode)) {
				continue;
			}
			
			if (!strcmp(pent->d_name, ".")){
				continue;		
			}
			
			if (mask)
			{
				sint8 *ext = _FS_getFileExtension(pent->d_name);
				if (ext && strstr(mask, ext))
				{
					strcpy(ptr, pent->d_name);
					list[i++] = ptr;
					ptr += strlen(pent->d_name)+1;  
				}
			} else
			{
				strcpy(ptr, pent->d_name);
				list[i++] = ptr;
				ptr += strlen(pent->d_name)+1;
			}
		}
	}
	
	closedir(dir);
	FS_unlock();
	return list;
}

#endif
//