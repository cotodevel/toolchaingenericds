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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "fileHandleTGDS.h"
#include "limits.h"

#include "errno.h"
#include "sys/stat.h"
#include "dirent.h"
#include "consoleTGDS.h"
#include "fsfatlayerTGDS.h"
#include "dsregs_asm.h"

volatile struct fd files[OPEN_MAXTGDS] __attribute__ ((aligned (4)));	//has file/dir descriptors and pointers

struct fd *fd_struct_get(int fd){
    struct fd *f = NULL;
    if((fd < OPEN_MAXTGDS) && (fd >= 0)){
        f = (struct fd *)&files[fd];
		if(f->dirPtr){
			f->dirPtr = (DIR *)&files[fd].dir;
		}
		if(f->filPtr){
			f->filPtr = (FIL *)&files[fd].fil;
		}
    }
    return f;
}

//must be called before FS_init(); (fsfatlayerTGDS.h)
// 1) initTGDSFS();
// 2) FS_init();

void initTGDSFS(){
	
	//newlib devoptabs
	memcpy ( (uint32*)&devoptab_stdin.name[0], (uint32*)stdin_name_desc, strlen(stdin_name_desc));
	memcpy ( (uint32*)&devoptab_stdout.name[0], (uint32*)stdout_name_desc, strlen(stdout_name_desc));
	memcpy ( (uint32*)&devoptab_sterr.name[0], (uint32*)stderr_name_desc, strlen(stderr_name_desc));
	memcpy ( (uint32*)&devoptab_stub.name[0], (uint32*)stdstub_name_desc, strlen(stdstub_name_desc));
	
	//for fopen/fread/fwrite/fclose we use the fsfat_internal_name_desc, then internally gets parsed to fsfat_internal_name_desc (fsfat layer)
	memcpy ( (uint32*)&devoptab_fatfs.name[0], (uint32*)fsfat_internal_name_desc, strlen(fsfat_internal_name_desc));
	
	int fd = 0;
	/* search in all struct fd instances*/
    for (fd = 0; fd < OPEN_MAXTGDS; fd++){
		memset((uint8*)&files[fd], 0, sizeof(struct fd));
        files[fd].isused = (sint32)structfd_isunused;			
		//PosixFD default invalid value (overriden later)
		files[fd].fd_posix = (sint32)structfd_posixInvalidFileDirHandle;
		
		//Internal default invalid value (overriden later)
		files[fd].cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirHandle;
		
		files[fd].isatty = (sint32)structfd_isattydefault;
		files[fd].descriptor_flags = (sint32)structfd_descriptorflagsdefault;
		files[fd].status_flags = (sint32)structfd_status_flagsdefault;
		files[fd].loc = (sint32)structfd_fildir_offsetdefault;
		
		files[fd].filPtr = NULL;
		files[fd].dirPtr = NULL;
    }
}

//returns / allocates a new struct fd index ,  for a certain devoptab_t so we can allocate different handles for each devoptab
int FileHandleAlloc(struct devoptab_t * devoptabInst ){
    int fd = 0;
    int ret = structfd_posixInvalidFileDirHandle;
    for (fd = 0; fd < OPEN_MAXTGDS; fd++){
        if ((sint32)files[fd].isused == (sint32)structfd_isunused){
			files[fd].isused = (sint32)structfd_isused;
			
			//PosixFD default valid value (overriden now)
			files[fd].fd_posix = (get_posix_fd_from_devicename((sint8*)devoptabInst->name));
			
			//Internal default value (overriden now)
			files[fd].cur_entry.d_ino = (sint32)fd;
			
			files[fd].isatty = (sint32)structfd_isattydefault;
			files[fd].descriptor_flags = (sint32)structfd_descriptorflagsdefault;
			files[fd].status_flags = (sint32)structfd_status_flagsdefault;
			files[fd].loc = (sint32)structfd_fildir_offsetdefault;
			
			files[fd].filPtr = NULL;
			files[fd].dirPtr = NULL;
			
            ret = fd;
            break;
        }
    }
    return ret;
}

//deallocates a posix index, returns such index deallocated
int FileHandleFree(int fd){
	int ret = structfd_posixInvalidFileDirHandle;
    if ((fd < OPEN_MAXTGDS) && (fd >= 0) && (files[fd].isused == structfd_isused)){
        files[fd].isused = (sint32)structfd_isunused;
		files[fd].fd_posix = (sint32)structfd_posixInvalidFileDirHandle;
		files[fd].cur_entry.d_ino = files[fd].fd_posix;
		ret = fd;
    }
	return ret;
}


sint8 * getDeviceNameByStructFDIndex(int StructFDIndex){
	sint8 * out;
	if((StructFDIndex < 0) || (StructFDIndex > OPEN_MAXTGDS)){
		out = NULL;
	}
	
	sint32 i = 0;
	/* search in all struct fd instances*/
    for (i = 0; i < OPEN_MAXTGDS; i++)
    {
        if (files[i].cur_entry.d_ino == StructFDIndex)
        {
			out = (sint8*)(&devoptab_struct[i]->name);
		}
    }
	return out;
}


//useful for handling native DIR * to Internal File Descriptors (struct fd index)
int getInternalFileDescriptorFromDIR(DIR *dirp){
	int fd = 0;
    int ret = structfd_posixInvalidFileDirHandle;
    /* search in all struct fd instances*/
    for (fd = 0; fd < OPEN_MAXTGDS; fd++)
    {
        if (files[fd].dirPtr)
        {
			if(files[fd].dirPtr->obj.id == dirp->obj.id){
				ret = files[fd].cur_entry.d_ino;
				break;
			}
		}
    }
	return ret;
}
