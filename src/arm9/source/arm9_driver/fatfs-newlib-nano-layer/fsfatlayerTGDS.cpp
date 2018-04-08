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
//newlib libc nano ARM Toolchain. dirent.h is not supported in this newlib version so we restore it

//C++ part
using namespace std;
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include "limitsTGDS.h"
#include <fcntl.h>
#include <dirent.h>
#include "fileHandleTGDS.h"
#include "fsfatlayerTGDS.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "devoptab_devices.h"
#include "consoleTGDS.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "posixHandleTGDS.h"
#include "dldi.h"
#include "clockTGDS.h"

//coffee
//fatfs
FATFS dldiFs;

//LongFilename scratchpad: has the latest fullfilepath accessed by updateCurDir("dir");
char lfnName[MAX_TGDSFILENAME_LENGTH+1];

//current dir emulation (for start);
char curDirListed[MAX_TGDSFILENAME_LENGTH+1];

//scratchpad struct fd
struct fd fdCur;

/* functions */

//For initializing Filesystem
int		FS_init()
{
	return fatfs_init();
}

//For de-initializing Filesystem
int		FS_deinit()
{
	return fatfs_deinit();
}

//converts a "folder/folder.../file.fil" into a proper filesystem fullpath
volatile sint8 charbuf[MAX_TGDSFILENAME_LENGTH+1];
sint8 * getfatfsPath(sint8 * filename){
	
	sprintf((sint8*)charbuf,"%s%s",devoptab_fatfs.name,filename);
	return (sint8*)&charbuf[0];
}

int fresult2errno(FRESULT result)
{
    int err;

    switch (result)
    {
        case FR_OK:
            err = 0;
            break;
        case FR_EXIST:
            err = EEXIST;
            break;
        case FR_NO_FILE:
            err = ENOENT;
            break;
        case FR_NO_PATH:
        case FR_INVALID_NAME:
            err = ENOTDIR;
            break;
        case FR_WRITE_PROTECTED:
            err = EROFS;
            break;
        case FR_DENIED:
            err = EACCES;
            break;
        case FR_TOO_MANY_OPEN_FILES:
            err = ENFILE;
            break;
        case FR_NOT_ENOUGH_CORE:
            err = ENOMEM;
            break;
        case FR_DISK_ERR:
        case FR_INT_ERR:
        case FR_NOT_READY:
        case FR_INVALID_OBJECT:
        case FR_INVALID_DRIVE:
        case FR_NOT_ENABLED:
        case FR_NO_FILESYSTEM:
        case FR_MKFS_ABORTED:
        case FR_TIMEOUT:
        case FR_LOCKED:
        case FR_INVALID_PARAMETER:
        /* TODO */
        default:
            err = EINVAL;
            break;
    }

    return err;
}

BYTE flags2mode(int flags)
{
    BYTE mode;
    int accmode;

    mode = 0;
    accmode = flags & O_ACCMODE;

    if ((accmode == O_RDONLY) || (accmode == O_RDWR))
    {
        mode |= FA_READ;
    }
    if ((accmode == O_WRONLY) || (accmode == O_RDWR))
    {
        mode |= FA_WRITE;

        if (!(flags & O_CREAT))
        {
            mode |= FA_OPEN_EXISTING;
        }
        else if (flags & O_EXCL)
        {
            mode |= FA_CREATE_NEW;
        }
        else if (flags & O_TRUNC)
        {
            mode |= FA_CREATE_ALWAYS;
        }
        else
        {
            mode |= FA_OPEN_ALWAYS;
        }
    }

    return mode;
}



//returns / allocates a new struct fd index with either DIR or FIL structure allocated
int fatfs_fildir_alloc(int isfilordir)
{
    int i_fil = 0;
	
    i_fil = FileHandleAlloc((struct devoptab_t *)&devoptab_fatfs);	//returns / allocates a new struct fd index 
    if (i_fil != -1)
    {
		if(isfilordir == structfd_isfile){
			files[i_fil].filPtr	=	(FIL *)&files[i_fil].fil;
			files[i_fil].dirPtr	= NULL;
		}
		if(isfilordir == structfd_isdir){
			files[i_fil].dirPtr	=	(DIR *)&files[i_fil].dir;
			files[i_fil].filPtr	= NULL;
		}
		
	}
    return i_fil;
}


void fatfs_free(struct fd *pfd)
{
	int i_fil;

    i_fil = FileHandleFree(pfd->cur_entry.d_ino);	//conversion
	
    if (i_fil != -1)	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
    {
		if(pfd->filPtr){	//must we clean a FIL?
			pfd->filPtr = NULL;
		}
		
		if(pfd->dirPtr){	//must we clean a DIR?
			pfd->dirPtr = NULL;
		}
    }
	else{
		//file_free failed
	}
	
}

int fatfs_write (int fd, sint8 *ptr, int len)	//(FileDescriptor :struct fd index)
{
    int ret;
    struct fd *pfd = fd_struct_get(fd);

    if (pfd == NULL)	//not file? not alloced struct fd?
    {
        errno = EBADF;
        ret = -1;
    }

    else if (
	(pfd->filPtr == NULL)	//no FIL descriptor?
	&&
	(pfd->dirPtr == NULL)	//and no DIR descriptor? definitely can't write to this file descriptor
	)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (S_ISREG(pfd->stat.st_mode))
	{
        FIL *filp;
        FRESULT result;
        UINT written;

        filp = pfd->filPtr;

        if (pfd->status_flags & O_APPEND)
        {
            DWORD size;

            size = f_size(filp);
            result = f_lseek(filp, size);
        }
		else if(filp == NULL){
			result = FR_INVALID_OBJECT;
		}
        else
        {
            result = FR_OK;
        }

        if (result == FR_OK)
        {
            result = f_write(filp, ptr, len, &written);	//writes success: if ret directory file invalid (7) check permission flags passed through fopen
        }
        if (result == FR_OK)
        {
            ret = written;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }
    else if (S_ISDIR(pfd->stat.st_mode))
	{
		errno = EISDIR;
        ret = -1;
    }
    else
    {
        errno = EBADF;
        ret = -1;
    }

    return ret;
}

//read (get struct FD index from FILE * handle)
int fatfs_read (int fd, sint8 *ptr, int len)	
{
    int ret;
    struct fd *pfd = fd_struct_get(fd);

    if ( (pfd == NULL) || (pfd->filPtr == NULL) )	//not file/dir? not alloced struct fd?
    {
        errno = EBADF;
        ret = -1;
    }
    else if (S_ISREG(pfd->stat.st_mode))
    {
        FIL *filp;
        FRESULT result;
        UINT nbytes_read;

        filp = pfd->filPtr;

        result = f_read(filp, ptr, len, &nbytes_read);
        if (result == FR_OK)
        {
            ret = nbytes_read;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        errno = EISDIR;
        ret = -1;
    }
    else
    {
        errno = EBADF;
        ret = -1;
    }

    return ret;
}

//receives a new struct fd index with either DIR or FIL structure allocated so it can be closed.
//returns 0 if success, 1 if error
int fatfs_close (int structFDIndex)
{
    int ret;
    struct fd * pfd = fd_struct_get(structFDIndex);

    if ( (pfd == NULL) || ((pfd->filPtr == NULL) && (pfd->dirPtr == NULL)) )	//not file/dir? not alloced struct fd?
    {
		errno = EBADF;
        ret = -1;
    }
	
	//File?
    else if (S_ISREG(pfd->stat.st_mode))
    {
        FIL *filp;
        FRESULT result;

        filp = pfd->filPtr;

        result = f_close(filp);
		
        if (
		(result == FR_OK)			//file sync with hardware SD ok
		||
		(result == FR_DISK_ERR)		//create file (fwrite + w): file didn't exist before open(); thus file descriptor didn't have any data of sectors to compare with. Exception expected
		)
        {
			//struct stat buf;	//flush stat
			memset (&pfd->stat, 0, sizeof(pfd->stat));
			
			fatfs_free(pfd);
			FileHandleFree(pfd->cur_entry.d_ino);	//deallocates a file descriptor index that is struct fd
            ret = 0;
			
			//update d_ino here (POSIX compliant)
			pfd->cur_entry.d_ino = (sint32)dirent_default_d_ino;
			
        }
        else
        {
			errno = fresult2errno(result);
            ret = -1;
        }
    }
	
	//Directory?
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        DIR *dp;
		
        FRESULT result;
		dp = pfd->dirPtr;
		
        result = f_closedir(dp);
        if (result == FR_OK)
        {
			FileHandleFree(pfd->cur_entry.d_ino);
            ret = 0;
        }
        else
        {
			errno = fresult2errno(result);
            ret = -1;
        }
    }
    else
    {
		errno = EBADF;
        ret = -1;
    }
	return ret;
}

void fill_stat(const FILINFO *fno, struct stat *out)
{
    mode_t mode;

    memset(out, 0, sizeof(struct stat));

    out->st_size = fno->fsize;
    if ((fno->fattrib & AM_MASK) & AM_DIR)
    {
        mode = S_IFDIR;
    }
    else
    {
        mode = S_IFREG;
    }
    mode |= (S_IRUSR|S_IRGRP|S_IROTH);
    mode |= (S_IXUSR|S_IXGRP|S_IXOTH);
    if (!((fno->fattrib & AM_MASK) & AM_RDO))
    {
        /* rwxrwxrwx */
        mode |= (S_IWUSR|S_IWGRP|S_IWOTH);
    }
    else
    {
        /* r-xr-xr-x */
    }
    out->st_mode = mode;
	
#if 0
    /* not present in newlib struct stat */
    struct timespec ts;
    fattime_to_timespec(fno->fdate, fno->ftime, &ts);
    pfd->stat.st_atim = ts;
    pfd->stat.st_mtim = ts;
    pfd->stat.st_ctim = ts;
#endif
}


//update struct fd with new FIL
void fill_fd_fil(int fd, FIL *fp, int flags, const FILINFO *fno, char * fullFilePath)	//(FileDescriptor :struct fd index)
{
    struct fd * fdinst = fd_struct_get(fd);
    fill_fd(fdinst, flags, fno);
    fdinst->filPtr = fp;
	//copy full file path (posix <- fsfat)
	int topsize = strlen(fullFilePath)+1;
	if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
		topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
	}
	strncpy(fdinst->fd_name, fullFilePath, topsize);
}

//update struct fd with new DIR
void fill_fd_dir(int fd, DIR *fp, int flags, const FILINFO *fno, char * fullFilePath)	//(FileDescriptor :struct fd index)
{
    struct fd *fdinst = fd_struct_get(fd);
	fill_fd(fdinst, flags, fno);
    fdinst->dirPtr = fp;
	//copy full directory path (posix <- fsfat)
	int topsize = strlen(fullFilePath)+1;
	if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
		topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
	}
	strncpy(fdinst->fd_name, fullFilePath, topsize);
}

//called from :
	//fill_fd_dir && fill_fd_fil
	//stat (newlib implementation)
	
void fill_fd(struct fd *pfd, int flags, const FILINFO *fno)
{
    pfd->isatty = 0;
    pfd->status_flags = flags;
    pfd->descriptor_flags = 0;
	
	//loc must NOT be modified here, only through fatfs_seek like functions
	
	fill_stat(fno, &pfd->stat);
}

//returns an internal index struct fd allocated
int fatfs_open_file(const sint8 *pathname, int flags, const FILINFO *fno)
{
    BYTE mode;
	FRESULT result;
	
	//not allocating properly, instead return the fildes allocated and access from there
	int structfdIndex = fatfs_fildir_alloc(structfd_isfile);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	struct fd * fdinst = fd_struct_get(structfdIndex);	//fd_struct_get requires struct fd index
	if ((structfdIndex == -1) || (fdinst == NULL))
    {
		// file handle invalid
    }
	else
    {
        FILINFO fno_after;
        mode = flags2mode(flags);
        result = f_open(fdinst->filPtr, pathname, mode);
		if (result == FR_OK)
        {
            result = f_stat(pathname, &fno_after);
            fno = &fno_after;			
			if (result == FR_OK)
			{
				fill_fd_fil(fdinst->cur_entry.d_ino, fdinst->filPtr, flags, fno, (char*)pathname);
			}
			else
			{
				fatfs_free(fdinst);
				structfdIndex = -1;
			}
		}
        else if(result == FR_NO_FILE){
			fatfs_free(fdinst);
			structfdIndex = -1;
		}
		else{
			
		}
    }
	
	return structfdIndex;
}

//returns an internal index struct fd allocated
int fatfs_open_dir(const sint8 *pathname, int flags, const FILINFO *fno)
{
    FRESULT result;
    int fdret = fatfs_fildir_alloc(structfd_isdir);	//allocates an internal struct fd (DIR) descriptor (user) which is then exposed, struct fd index included
	struct fd * fdinst = fd_struct_get(fdret);
    
	if (fdinst == NULL)
    {
        result = FR_TOO_MANY_OPEN_FILES;
    }
    else
    {
		result = f_opendir(fdinst->dirPtr, pathname);
		
        if (result == FR_OK)
        {
			fill_fd_dir(fdret, fdinst->dirPtr, flags, fno, (char*)pathname);
        }
        else
        {
            fatfs_free(fdinst);
        }
    }

    return fdret;
}

//returns / allocates a new struct fd index with either DIR or FIL structure allocated
int fatfs_open_file_or_dir(const sint8 *pathname, int flags)
{
	FRESULT result;
    FILINFO fno;
	int structFD = -1;
	fno.fname[0] = '\0'; /* initialize as invalid */
	
	result = f_stat(pathname, &fno);
    
	if ((result != FR_OK) && (result != FR_NO_FILE))
    {
        // just return if invalid path
	}
	
    else if ((result == FR_OK) && ((fno.fattrib & AM_MASK) & AM_DIR))
    {
        structFD = fatfs_open_dir(pathname, flags, &fno);
	}
	//IS FILE or NEW FILE
    else
    {
		structFD = fatfs_open_file(pathname, flags, &fno);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	}
	
    return structFD;
}



/* exported functions */

/* Use NDS RTC to update timestamps into files when certain operations require it*/
DWORD get_fattime (void)
{
	struct tm * tmStruct = getTime();
    return (
            (((sint32)tmStruct->tm_year-60)<<25)
            |
            (((sint32)tmStruct->tm_mon)<<21)
            |
            (((sint32)tmStruct->tm_mday)<<16)
			|
			(((sint32)tmStruct->tm_hour)<<11)
			|
			(((sint32)tmStruct->tm_min)<<5)
			|
			(((sint32)tmStruct->tm_sec)<<0)
           );
}

//returns / allocates a new struct fd index with either DIR or FIL structure allocated
int fatfs_open(const sint8 *pathname, int flags)
{
	int structFDIndex = fatfs_open_file_or_dir(pathname, flags);	
	if (structFDIndex == -1)
	{
		//there is error, no struct fd handle (index) was allocated
	}
	else{
		//update d_ino here (POSIX compliant) - through correct (internal struct fd) file handle
		struct fd *pfd = fd_struct_get(structFDIndex);
		pfd->cur_entry.d_ino = structFDIndex;
	}
	return structFDIndex;
}

off_t fatfs_lseek(int fd, off_t offset, int whence )	//(FileDescriptor :struct fd index)
{
    off_t ret;
    struct fd *pfd = fd_struct_get(fd);
    if (pfd == NULL)
    {
		errno = EBADF;
        ret = -1;
    }
    else if ((pfd->isused == structfd_isunused)||(!pfd->filPtr))
    {
		errno = EBADF;
        ret = -1;
    }
	else if (S_ISSOCK(pfd->stat.st_mode))	//socket? cant use
	{
		ret = EPIPE;
	}
    else if (S_ISREG(pfd->stat.st_mode))	//only file allows this
    {
		
        FIL *filp;
        FRESULT result;
        DWORD pos;
		
        filp = pfd->filPtr;
        if (whence == SEEK_CUR)
        {
            pos = f_tell(filp);
        }
        else if (whence == SEEK_END)
        {
            pos = f_size(filp);
        }
        else if (whence == SEEK_CUR)
        {
            pos = 0;
        }
        else
        {
            /* TODO: error */
            pos = 0;
        }
        pos += offset;

        result = f_lseek(filp, pos);
		
        if (result == FR_OK)
        {
			ret = pos;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int fatfs_unlink(const sint8 *path)
{
    int ret;
    FRESULT result;

    result = f_unlink(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_link(const sint8 *path1, const sint8 *path2)
{
    int ret;

    /* FAT does not support links */
    errno = EMLINK; /* maximum number of "links" is 1: the file itself */
    ret = -1;
    (void)path1; /* ignore */
    (void)path2; /* ignore */

    return ret;
}

int fatfs_rename(const sint8 *oldfname, const sint8 * newfname)
{
    int ret;
    FRESULT result;

    result = f_rename(oldfname, newfname);
    if (result != FR_OK)
    {
        errno = fresult2errno(result);
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

int fatfs_fsync(int fd)	//uses struct fd indexing
{
    int ret;
    struct fd *pfd;

    pfd = fd_struct_get(fd);

    if (pfd == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (pfd->filPtr == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (S_ISREG(pfd->stat.st_mode))
    {
        FIL *filp;
        FRESULT result;

        filp = pfd->filPtr;

        result = f_sync(filp);
        if (result == FR_OK)
        {
            ret = 0;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int fatfs_stat(const sint8 *path, struct stat *buf)
{
    int ret;
    FRESULT result;
    FILINFO fno;

    result = f_stat(path, &fno);
    if (result == FR_OK)
    {
        fill_stat(&fno, buf);
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_mkdir(const sint8 *path, mode_t mode)
{
    int ret;
    FRESULT result;

    (void)mode; /* ignored */
    result = f_mkdir(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_rmdir(const sint8 *path)
{
    int ret;
    FRESULT result;

    result = f_unlink(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else if (result == FR_DENIED)
    {
        FILINFO fno;

        /* the dir was readonly or not empty: check */
        result = f_stat(path, &fno);
        if (result == FR_OK)
        {
            if ((fno.fattrib & AM_MASK) & AM_RDO)
            {
                errno = EACCES;
            }
            else
            {
                errno = ENOTEMPTY;
            }
        }
        else
        {
            errno = fresult2errno(result);
        }
        ret = -1;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_chdir(const sint8 *path)
{
    int ret;
    FRESULT result;

    result = f_chdir(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

sint8 *fatfs_getcwd(sint8 *buf, size_t size)
{
    sint8 *ret;
    FRESULT result;

    result = f_getcwd(buf, size);
    if (result == FR_OK)
    {
        ret = buf;
    }
    else
    {
        errno = fresult2errno(result);
        ret = NULL;
    }

    return ret;
}

DIR *fatfs_opendir(const sint8 *path)
{
    DIR *ret;
    int retindex = fatfs_open(path, O_RDONLY);	//returns an internal index struct fd allocated
	if (retindex == -1)
    {
		//Invalid Path
        ret = NULL;
    }
    else
    {
        struct fd *pfd = fd_struct_get(retindex);
        if (pfd == NULL)
        {
            errno = EBADF;
            ret = NULL;
        }
        else
        {
            ret = pfd->dirPtr;
        }
    }

    return ret;
}

int fatfs_closedir(DIR *dirp)
{
	//we need a conversion from DIR * to struct fd *
	int fd = getInternalFileDescriptorFromDIR(dirp);
	return fatfs_close(fd);	//requires a struct fd(file descriptor), returns 0 if success, -1 if error
}

int fatfs_dirfd(DIR *dirp)
{
    int ret;
	
	//we need a conversion from DIR * to struct fd *
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * pfd = fd_struct_get(fd);
	
    if (S_ISDIR(pfd->stat.st_mode))
	{
		ret = 0;	//could cause bugs
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

DIR *fatfs_fdopendir(int fd)	//(FileDescriptor :struct fd index)
{
    DIR *ret;
    struct fd *pfd;

    pfd = fd_struct_get(fd);
    if (pfd == NULL)
    {
        errno = EBADF;
        ret = NULL;
    }
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        ret = pfd->dirPtr;
    }
    else
    {
        errno = ENOTDIR;
        ret = NULL;
    }

    return ret;
}

struct dirent *fatfs_readdir(DIR *dirp)
{
    struct dirent *ret;
	
	//we need a conversion from DIR * to struct fd *
    int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
	
    if ((S_ISDIR(fdinst->stat.st_mode)) && (fdinst != NULL) )
	{
        fatfs_readdir_r(dirp, (struct dirent *)&fdinst->cur_entry, &ret);
        /* ignore return value */
    }
	
	//should we read a file here as well isnt
	
    else
    {
        errno = EBADF;
        ret = NULL;
    }
	//ret dirent * with read file/dir properties, namely: d_name
    return ret;
}

//will read directory object into DIR * either file / dir 
int fatfs_readdir_r(
        DIR *dirp,
        struct dirent *entry,	//current dirent of file handle struct fd
        struct dirent **result)	//pointer to rewrite above file handle struct fd
{
    int ret;
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
    
	FRESULT fresult;
	FILINFO fno;
	fresult = f_readdir(dirp, &fno);
	if (fresult != FR_OK)
	{
		errno = fresult2errno(fresult);
		ret = -1;
		*result = NULL;
	}
	else if (fno.fname[0] == '\0')
	{
		/* end of entries */
		ret = 0;
		*result = NULL;
	}
	else
	{
		ret = 0;
		
		//fill dir/file stats
		fdinst->loc++;
		
		//d_name : dir or file name. NOT full path (posix <- fsfat)
		int topsize = strlen(fno.fname)+1;
		if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
			topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
		}
		strncpy((sint8*)entry->d_name, (sint8*)fno.fname, topsize);
		*result = entry;
	}
    return ret;
}

void fatfs_rewinddir(DIR *dirp)
{
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
	
    if (!S_ISDIR(fdinst->stat.st_mode))
	{
		//Not Directory!	/* POSIX says no errors are defined */
    }
    else
    {
        int result;

        result = f_readdir(dirp, NULL);
        if (result == FR_OK)
        {
            fdinst->loc	= 0;
        }
        else
        {
            //Error handling the request.	/* POSIX says no errors are defined */
        }
    }
	
}

long fatfs_tell(struct fd *f)	//NULL check already outside
{
    long ret;
	//dir
	if ((f->dirPtr) && (S_ISDIR(f->stat.st_mode)))
	{
		ret = f->loc;
    }
	//file
	else if ((f->filPtr) && (S_ISREG(f->stat.st_mode)))
	{
		ret = f->loc;
	}
    else
    {
        ret = -1;
    }
    return ret;
}

void fatfs_seekdir(DIR *dirp, long loc)
{
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
	
    if (S_ISDIR(fdinst->stat.st_mode))
	{
        long cur_loc;

        cur_loc = fatfs_tell(fdinst);
        if (loc < cur_loc)
        {
            fatfs_rewinddir(dirp);
        }
        while(loc > cur_loc)
        {
            int ret;
            struct dirent entry;
            struct dirent *result;

            ret = fatfs_readdir_r(dirp, &entry, &result);
            if ((result == NULL) || (ret != 0))
            {
                /* POSIX says no errors are defined */
                break;
            }
            cur_loc = fatfs_tell(fdinst);
        }
    }
    else
    {
        /* POSIX says no errors are defined */
    }
}


int fatfs_init()
{
    return (f_mount(&dldiFs, "0:", 1));
}

int fatfs_deinit(){
	return (f_unmount("0:"));
}


//this copies stat from internal struct fd to external code
int _fstat_r ( struct _reent *_r, int fd, struct stat *buf )	//(FileDescriptor :struct fd index)
{
    int ret;
    struct fd * f = fd_struct_get(fd);
    if (f == NULL)
    {
        _r->_errno = EBADF;
        ret = -1;
    }
    else if (f->isused == structfd_isunused)
    {
        _r->_errno = EBADF;
        ret = -1;
    }
    else
    {
        *buf = f->stat;
        ret = 0;
    }
    return ret;
}


int rename(const sint8 *oldfname, const sint8 *newfname)
{
    return fatfs_rename(oldfname, newfname);
}

int fsync(int fd)	//(FileDescriptor :struct fd index)
{
    return fatfs_fsync(fd);
}

int mkdir(const sint8 *path, mode_t mode)
{
    return fatfs_mkdir(path, mode);
}

int rmdir(const sint8 *path)
{
    return fatfs_rmdir(path);
}

int chdir(const sint8 *path)
{
    return fatfs_chdir(path);
}

sint8 *getcwd(sint8 *buf, size_t size)
{
    return fatfs_getcwd(buf, size);
}

DIR *opendir(const sint8 *path)
{
	return fatfs_opendir(path);
}

int closedir(DIR *dirp)
{
    return fatfs_closedir(dirp);
}

struct dirent *readdir(DIR *dirp)
{
    return fatfs_readdir(dirp);
}

int  readdir_r(DIR * dirp,struct dirent * entry,struct dirent ** result)
{
    return fatfs_readdir_r(dirp, entry, result);
}

void rewinddir(DIR *dirp)
{
    fatfs_rewinddir(dirp);
}

int dirfd(DIR *dirp)
{
    return fatfs_dirfd(dirp);
}


DIR *fdopendir(int fd)	//(FileDescriptor :struct fd index)
{
    return fatfs_fdopendir(fd);
}

void seekdir(DIR *dirp, long loc)
{
    fatfs_seekdir(dirp, loc);
}



//misc directory functions

//filename must be at least MAX_TGDSFILENAME_LENGTH+1
bool setLFN(char* filename){
	if (filename == NULL){
		return false;
	}
	strncpy (lfnName, filename, (MAX_TGDSFILENAME_LENGTH+1) - 1);
	lfnName[(MAX_TGDSFILENAME_LENGTH+1) - 1] = '\0';
	return true;	
}

//filename must be at least MAX_TGDSFILENAME_LENGTH+1
bool getLFN(char* filename){	//FAT_GetLongFilename(char* filename)
	if (filename == NULL){
		return false;
	}
	strncpy(filename, lfnName, (MAX_TGDSFILENAME_LENGTH+1) - 1);
	filename[(MAX_TGDSFILENAME_LENGTH+1) - 1] = '\0';
	return true;
}

//int FAT_FindFirstFile(char* filename) //filename must be at least MAX_TGDSFILENAME_LENGTH+1 in size
int FAT_FindFirstFile(char* filename){
	if(strlen(filename) == 0){
		sprintf(filename,"%s","/");	//will start at "/" if filename is empty
	}
	int type = updateCurDir(filename); 	//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
										//			or FT_NONE if invalid file
	if(type == FT_FILE){
		getLFN(filename);
	}
	else if(type == FT_DIR){
		sprintf(filename,"%s",curDirListed);
		filename[(MAX_TGDSFILENAME_LENGTH+1) - 1] = '\0';
	}
	return type;
}

int FAT_FindNextFile(char* filename){
	int type = getNextFile(curDirListed);	//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
								//			or FT_NONE if invalid file
	if(type == FT_FILE){
		getLFN(filename);
	}
	else if(type == FT_DIR){
		sprintf(filename,"%s",curDirListed);
		filename[(MAX_TGDSFILENAME_LENGTH+1) - 1] = '\0';
	}
	return type;
}


FILINFO getFirstFileFILINFO(char * path){
	/*//ori works
	FILINFO finfo;
	DIR dirp;
	if (f_opendir(&dirp, path) == FR_OK) {
        if((f_readdir(&dirp, &finfo) == FR_OK) && finfo.fname[0]) {
            //ok
        }
		f_closedir(&dirp);
    }
	return finfo;
	*/
	
	FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s ", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }
	
	return fno;
}

bool getNextFileFILINFO(char * path,FILINFO * finfo){
	bool retval = false;
	FRESULT fr;     /* Return value */
    DIR dj;         /* Directory search object */
    FILINFO fno;    /* File information */
	
    fr = f_findfirst(&dj, &fno, "/", "dsc*.jpg");  /* Start to search for photo files */

    while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
        printf("%s ", fno.fname);                /* Display the object name */
        fr = f_findnext(&dj, &fno);               /* Search for next item */
		retval = true;
	}
	
	f_closedir(&dj);
	return retval;
}

//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int getFirstFile(char * path){
	int type = 0;
	FILINFO FILINFOObj = getFirstFileFILINFO(path);
	if (FILINFOObj.fattrib & AM_DIR) {	//dir
		type = FT_DIR;
	}
	else if (	//file
	(FILINFOObj.fattrib & AM_RDO)
	||
	(FILINFOObj.fattrib & AM_HID)
	||
	(FILINFOObj.fattrib & AM_SYS)
	||
	(FILINFOObj.fattrib & AM_DIR)
	||
	(FILINFOObj.fattrib & AM_ARC)
	){
		char fout[MAX_TGDSFILENAME_LENGTH+1] = {0};
		sprintf(fout,"%s%s%s", "0:",path, &FILINFOObj.fname[0]);
		setLFN(fout);	//update last full path given a path
		type = FT_FILE;
	}
	else{	//invalid
		type = FT_NONE;
	}
	return type;
}


//requires fullpath of the CURRENT file, it will return the next one
//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int getNextFile(char * fullpath){
	int type = 0;
	FILINFO fno;
	char fout[512] = {0};
	sprintf(fout,"%s%s","0:",fullpath);
	printf("getNextFile:getNextFileFILINFO");
	printf("trying:%s",fout);
	if(getNextFileFILINFO(fout,&fno) == true) {
		if (fno.fattrib & AM_DIR) {	//dir
			type = FT_DIR;
		}
		else if (	//file
		(fno.fattrib & AM_RDO)
		||
		(fno.fattrib & AM_HID)
		||
		(fno.fattrib & AM_SYS)
		||
		(fno.fattrib & AM_DIR)
		||
		(fno.fattrib & AM_ARC)
		){
			char fout[MAX_TGDSFILENAME_LENGTH+1] = {0};
			sprintf(fout,"%s%s%s", "0:",curDirListed, &fno.fname[0]);
			setLFN(fout);	//update last full path given a path
			type = FT_FILE;
		}
	}
	else{	//invalid
		type = FT_NONE;
		printf("getNextFileFILINFO:fail");
	}
	return type;
}

//updates the global latest directory opened
//format: "/directory0/directory1/etc"
//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int updateCurDir(char * curDir){
	if(curDir){
		sprintf(curDirListed,"%s",curDir);
		curDirListed[(MAX_TGDSFILENAME_LENGTH+1) - 1] = '\0';
		//if getfirstFile == FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file found after updateCurDir
		return getFirstFile(curDirListed);
	}
	return FT_NONE;
}


/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/
/* Hidden API for hacks and disk tools */
 
DWORD clust2sect (  /* !=0:Sector number, 0:Failed (invalid cluster#) */
    FATFS* fs,      /* File system object */
    DWORD clst      /* Cluster# to be converted */
)
{
    clst -= 2;
    if (clst >= fs->n_fatent - 2) return 0;       /* Invalid cluster# */
    return clst * fs->csize + fs->database;
}

//FATFS: The file handle start cluster is in fp->obj.sclust as long as you haven't read from that file (otherwise its in fp->clust)
sint32	getStructFDFirstCluster(struct fd *fdinst){
	if(fdinst){
		if( (int)fdinst->filPtr->fptr == (int)0 ){
			return (int)(fdinst->filPtr->obj.sclust);
		}
		else{
			if(fdinst->filPtr){
				return (int)(fdinst->filPtr->clust);
			}
			else{
				return -1;
			}
		}
	}
	return -1;
}

//args: int ClusterOffset (1) : int SectorOffset (N). = 1 physical sector in disk. Each sector is getDiskSectorSize() bytes. Cluster 0 + Sector 0 == Begin of FileHandle
//returns -1 if : file not open, directory or fsfat error
sint32 getStructFDSectorOffset(struct fd *fdinst,int ClusterOffset,int SectorOffset){	//	struct File Descriptor (FILE * open through fopen() -> then converted to int32 from fileno())
	if(fdinst->filPtr){
		return clust2sect(fdinst->filPtr->obj.fs, fdinst->filPtr->obj.sclust + ClusterOffset) + SectorOffset; 
	}
	else{
		return -1;
	}
}

sint32 getDiskClusterSize(){		/* Cluster size [sectors] */
	return(sint32)(dldiFs.csize);
}

sint32 getDiskClusterSizeBytes(){	/* Cluster size [sectors] in bytes */
	return (sint32)(getDiskClusterSize() * getDiskSectorSize());
}

sint32 getDiskSectorSize(){
	int diskSectorSize = 0;
	if(FF_MAX_SS == FF_MIN_SS){
		diskSectorSize = (int)FF_MIN_SS;
	}
	else{
		#if (FF_MAX_SS != FF_MIN_SS)
		diskSectorSize = dldiFs.ssize;	//when fsfat sectorsize is variable, by default its 512
		#endif
	}
	return diskSectorSize;
}

char * dldi_tryingInterface(){
	//DS DLDI
	struct DLDI_INTERFACE * DLDI_INTERFACEInst = dldiGet();
	return DLDI_INTERFACEInst->friendlyName;
}
