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

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include "file.h"
#include "fsfat_layer.h"
#include "typedefs.h"
#include "dsregs.h"
#include "devoptab_devices.h"
#include "console.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "posix_hook_shared.h"

//fatfs
FATFS dldiFs;

/* functions */

//For initializing Filesystem
int		FS_init()
{
	return fatfs_init();
}

//converts a "folder/folder.../file.fil" into a proper filesystem fullpath
volatile sint8 charbuf[NAME_MAX+1];
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

//todo
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
			result = -1;
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
void fill_fd_fil(int fd, FIL *fp, int flags, const FILINFO *fno)	//(FileDescriptor :struct fd index)
{
    struct fd *pfd = fd_struct_get(fd);
    fill_fd(pfd, flags, fno);
    pfd->filPtr = fp;
}

//update struct fd with new DIR
void fill_fd_dir(int fd, DIR *fp, int flags, const FILINFO *fno)	//(FileDescriptor :struct fd index)
{
    struct fd *pfd = fd_struct_get(fd);
	fill_fd(pfd, flags, fno);
    pfd->dirPtr = fp;
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
		
        if ((result == FR_OK) && (fno->fname[0] == '\0'))
        {
            result = f_stat(pathname, &fno_after);
            fno = &fno_after;
        }
        if (result == FR_OK)
        {
			fill_fd_fil(fdinst->cur_entry.d_ino, fdinst->filPtr, flags, fno);
		}
        else
        {
			fatfs_free(fdinst);
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
			fill_fd_dir(fdret, fdinst->dirPtr, flags, fno);
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

/* TODO: maybe put in diskio? use real time? */
DWORD get_fattime (void)
{
    return (
            ((2015-1980)<<25)
            |
            (1<<21)
            |
            (1<<16)
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

int fatfs_rename(const sint8 *old, const sint8 *new)
{
    int ret;
    FRESULT result;

    result = f_rename(old, new);
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
    else
    {
        errno = EBADF;
        ret = NULL;
    }
	//ret dirent * with read file/dir properties, namely: d_name
    return ret;
}

int fatfs_readdir_r(
        DIR *dirp,
        struct dirent *entry,	//current dirent of file handle struct fd
        struct dirent **result)	//pointer to rewrite above file handle struct fd
{
    int ret;
	
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
	
    if (!S_ISDIR(fdinst->stat.st_mode))
	{
        errno = EBADF;
        ret = -1;
    }
    else
    {
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
			
			//d_name for dir or file (posix <- fsfat)
			int topsize = strlen(fno.fname)+1;
			if((sint32)topsize > (sint32)(NAME_MAX+1)){
				topsize = (sint32)(NAME_MAX+1);
			}
			strncpy((sint8*)entry->d_name, (sint8*)fno.fname, topsize);
			
            *result = entry;
        }
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

//returns the First Sector for a given file opened:
//returns -1 if the file was not open or not a file (directory or fsfat error)
//	struct File Descriptor (FILE * opened through fopen_fs() -> then converted to int32 from fileno())
sint32 getStructFDFirstSector(struct fd *f){
	if(f->filPtr){
		return clust2sect(f->filPtr->obj.fs, f->filPtr->obj.sclust);  /* Return file start sector */
	}
	else{
		return -1;
	}
}

sint32 getDiskClusterSize(){
	return(sint32)(dldiFs.csize);
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
