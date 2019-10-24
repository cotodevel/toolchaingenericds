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


#ifndef __utilstgds_h__
#define __utilstgds_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>


#ifdef ARM9
#include "limitsTGDS.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif


#define toolchain_generic_version ((char*)"1.5");

#endif

//splitCustom logic
typedef void(*splitCustom_fn)(const char *, size_t, char * ,int indexToLeftOut, char * delim);
#define TOP_ITEMS_SPLIT (int)(10)

#endif



#ifdef __cplusplus
extern "C"{
#endif

extern size_t ucs2tombs(uint8* dst, const unsigned short* src, size_t len);

#ifdef ARM9

//misc
extern int		FS_loadFile(sint8 *filename, sint8 *buf, int size);
extern int		FS_saveFile(sint8 *filename, sint8 *buf, int size,bool force_file_creation);
extern int		FS_getFileSize(sint8 *filename);
extern int		FS_extram_init();
extern void		FS_lock();
extern void		FS_unlock();
extern char *	print_ip(uint32 ip, char * bufOut);

//FileSystem utils
extern sint8 	*_FS_getFileExtension(sint8 *filename);
extern sint8 	*FS_getFileName(sint8 *filename);
extern int		FS_chdir(const sint8 *path);
extern sint8	**FS_getDirectoryList(sint8 *path, sint8 *mask, int *cnt);

//splitCustom string by delimiter implementation in C, that does not use malloc/calloc and re-uses callbacks
extern char * outSplitBuf[TOP_ITEMS_SPLIT][MAX_TGDSFILENAME_LENGTH+1];
extern int count_substr(const char *str, const char* substr, bool overlap);
extern void splitCustom(const char *str, char sep, splitCustom_fn fun, char * outBuf, int indexToLeftOut);
extern void buildPath(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim);
extern int getLastDirFromPath(char * stream, char * haystack, char * outBuf);
extern int str_split(char * stream, char * haystack, char * outBuf);
extern int inet_pton(int af, const char *src, void *dst);

#endif

extern int		setBacklight(int flags);

#ifdef __cplusplus
}
#endif
