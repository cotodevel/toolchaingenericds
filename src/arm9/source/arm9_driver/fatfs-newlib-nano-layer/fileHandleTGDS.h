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
 
//Coto: this was rewritten by me so it could fit the following setup:
//newlib libc nano ARM Toolchain. dirent.h is stripped in newlib nano version so we add back proper POSIX implementation.

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <sys/types.h>
#include <sys/stat.h>

#include "fsfatlayerTGDS.h"
#include "limitsTGDS.h"
#include "devoptab_devices.h"
#include "posixHandleTGDS.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern volatile struct fd files[OPEN_MAXTGDS];	//file/dir attrs, pointers for below struct

extern void file_default_init();
extern struct fd *fd_struct_get(int fd);	//the fd here is the internal FD index

extern int FileHandleAlloc(struct devoptab_t * devoptabInst );
extern int FileHandleFree(int fd);

//useful for internal functions on DIR
extern int getInternalFileDescriptorFromDIR(DIR *dirp);

extern sint8 * getDeviceNameByStructFDIndex(int StructFDIndex);

extern void updateLastGlobalPath(char * path);
#ifdef __cplusplus
}
#endif


