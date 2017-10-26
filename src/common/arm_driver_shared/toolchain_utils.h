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

#ifndef __toolchain_utils_h__
#define __toolchain_utils_h__

#include "typedefs.h"
#include "dsregs.h"

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif


#define toolchain_generic_version ((char*)"0.1");

//Method Export
#define PLAINTEXT_METHOD_SEPARATOR	(uint32)(0xfedcba98)	//not valid ARM function
typedef	struct {
	uint32 * cback_address;
	int cback_size;
	char methodname[256];
}METHOD_DESCRIPTOR;
		
//Version Struct
#define PLAINTEXT_VERSION_SEPARATOR	(sint8*)("-")	//sint8 * is char *
typedef	struct {
	char app_version[256];	//"0.6a-mm/dd/yyyy" //generated when parsing config file, section [Version]
	char framework_version[256];	//"0.6a-mm/dd/yyyy" //generated when parsing config file, section [Version]
}VERSION_DESCRIPTOR;

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern size_t ucs2tombs(uint8* dst, const unsigned short* src, size_t len);

#ifdef ARM9

//reserved for project appVersion
extern volatile char app_version_static[256];

//METHOD_HANDLERS
extern METHOD_DESCRIPTOR Methods[8];
extern METHOD_DESCRIPTOR * callback_append_signature(uint32 * func_addr, uint32 * func_addr_end, char * methodname,METHOD_DESCRIPTOR * method_inst);
extern sint32 cback_build();
extern void cback_build_end();
extern sint32 callback_export_buffer(METHOD_DESCRIPTOR * method_inst, uint8 * buf_out);
extern sint32 callback_export_file(char * filename,METHOD_DESCRIPTOR * method_inst);

//VERSION_HANDLERS
extern volatile VERSION_DESCRIPTOR Version[1];

//Apps should update this at bootup
extern sint32 addAppVersiontoCompiledCode(VERSION_DESCRIPTOR * versionInst,char * appVersion,int appVersionCharsize);

//Framework sets this by default
extern sint32 updateVersionfromCompiledCode(VERSION_DESCRIPTOR * versionInst);

extern sint32 updateAssemblyParamsConfig(VERSION_DESCRIPTOR * versionInst);
extern sint32 glueARMHandlerConfig(VERSION_DESCRIPTOR * versionInst,METHOD_DESCRIPTOR * method_inst);

//misc
extern int	split (const sint8 *txt, sint8 delim, sint8 ***tokens);

#ifdef ARM9
extern int	FS_loadFile(sint8 *filename, sint8 *buf, int size);
extern int	FS_saveFile(sint8 *filename, sint8 *buf, int size,bool force_file_creation);
extern int	FS_getFileSize(sint8 *filename);

extern int		setBacklight(int flags);
extern int		FS_extram_init();
extern void		FS_lock();
extern void		FS_unlock();
extern char 	* str_replace (char *string, const char *substr, const char *replacement);


extern sint8 ip_decimal[0x10];
extern sint8 * print_ip(uint32 ip);

#endif

#endif

#ifdef __cplusplus
}
#endif