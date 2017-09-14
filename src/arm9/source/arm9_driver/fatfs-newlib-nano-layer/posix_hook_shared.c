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
//The overriden stock POSIX calls are specifically targeted to newlib libc nano ARM Toolchain

#ifdef ARM9

#include "posix_hook_shared.h"
#include "mem_handler_shared.h"
#include "dsregs_asm.h"

#include "devoptab_devices.h"
#include "errno.h"
#include "sys/stat.h"
#include "dirent.h"
#include "console.h"
#include "file.h"
#include "fsfat_layer.h"

//Notes:
//	- 	Before you get confused, the layer order is: POSIX file operations-> newlib POSIX fd assign-> devoptab filesystem -> fsfat_layer (n file descriptors for each file op) -> fsfat driver -> dldi.
//		So we can have a portable/compatible filesystem with multiple file operations.
//
//	-	Newlib nano dictates to override reentrant weak functions, overriding non reentrant is undefined behaviour.

int _fork_r ( struct _reent *ptr )
{
   /* return "not supported" */
    ptr->_errno = ENOTSUP;
  return -1;
}


void *_sbrk_r (struct _reent *ptr, int nbytes){
	//Coto: own implementation, libc's own malloc implementation does not like it.
	//but helps as standalone sbrk implementation except it will always alloc in linear way and not malloc/free (requires keeping track of pointers)

	#ifdef own_allocator
	int retcode = calc_heap(alloc);
	void * retptr = (void *)alloc_failed;
	switch(retcode){
		case(0):{
			//ok
			retptr = this_heap_ptr;
		}
		break;
		//error handling code
		case(-1):{
			
		}
		break;
	}
	
	return retptr;
	#endif
	
	//Standard newlib implementation
	#ifndef own_allocator

	static sint8 *heap_end;
	sint8 *prev_heap_end;

	if (heap_end == NULL)
		heap_end = (sint8*)get_lma_libend();

	prev_heap_end = heap_end;

	if (heap_end + nbytes > (sint8*)get_lma_ewramend())
	{
		//errno = ENOMEM;
		return (void *) -1;
	}

	heap_end += nbytes;

	return prev_heap_end;
	#endif
}

//read (get struct FD index from FILE * handle)
_ssize_t _read_r ( struct _reent *ptr, int fd, void *buf, size_t cnt ) 
{
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);
	
	if( (fdinst != NULL) && ((sint32)fdinst->fd_posix != (sint32)structfd_posixFileDescrdefault)  ) {
		return (_ssize_t)devoptab_list[fdinst->fd_posix]->read_r( ptr, fdinst->cur_entry.d_ino, buf, cnt );
	}
	
	return -1;
}

//write (get struct FD index from FILE * handle)
_ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt )	
{
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);
	
	if( (fdinst != NULL) && ((sint32)fdinst->fd_posix != (sint32)structfd_posixFileDescrdefault)  ) {
		return (_ssize_t)devoptab_list[fdinst->fd_posix]->write_r( ptr, fdinst->cur_entry.d_ino, buf, cnt );
	}
	
	return -1;
}

//POSIX Logic: hook devoptab descriptor into devoptab functions

//allocates a new struct fd index with either DIR or FIL structure allocated
int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode )
{
	sint8 **tokens;
	int count = 0, i = 0;
	volatile sint8 str[256];	//file safe buf
	memcpy ( (u8*)str, (u8*)file, 256);

	count = split ((const sint8*)str, '/', &tokens);	
	volatile sint8 token_str[64];
	
	sint32 countPosixFDescOpen = open_posix_filedescriptor_devices() + 1;
	/* search for "file:/" in "file:/folder1/folder.../file.test" in dotab_list[].name */
	for (i = 0; i < countPosixFDescOpen ; i++){
		if(count > 0){
			sprintf((sint8*)token_str,"%s/",tokens[0]);	//format properly
			if (strcmp((sint8*)token_str,devoptab_list[i]->name) == 0)
			{
				return devoptab_list[i]->open_r( ptr, file, flags, mode ); //returns / allocates a new struct fd index with either DIR or FIL structure allocated
			}
		}
	}

	ptr->_errno = ENODEV;
	return -1;
}

int _close_r ( struct _reent *ptr, int fd )
{
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);
	
	if( (fdinst != NULL) && ((sint32)fdinst->fd_posix != (sint32)structfd_posixFileDescrdefault)  ) {
		return (_ssize_t)devoptab_list[fdinst->fd_posix]->close_r( ptr, fdinst->cur_entry.d_ino );
	}
	
	return -1;
}


/*
 isatty
 Query whether output stream is a terminal.
 */
int _isatty (int   file)
{
  return  1;

}       /* _isatty () */

//don't care for toolchain printf/iprintf, just override this so it's compatible with SnemulDS framebuffer console render.
int _vfprintf_r(struct _reent *reent, FILE *fp,const sint8 *fmt, va_list list){
	
	#ifdef ARM7
	volatile uint8 g_printfbuf[100];
	#endif
	
	//merge any "..." special arguments where sint8 * ftm requires , store into g_printfbuf
	vsnprintf ((sint8*)g_printfbuf, 64, fmt, list);
	
	#ifdef ARM7
	//redirect: todo
	#endif
	
	#ifdef ARM9
	// FIXME
	t_GUIZone zone;
	zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
	zone.font = &trebuchet_9_font;
	GUI_drawText(&zone, 0, GUI.printfy, 255, (sint8*)g_printfbuf);
	GUI.printfy += GUI_getFontHeight(&zone);
	#endif
	
	return (strlen((sint8*)&g_printfbuf[0]));
	
}

int _vfiprintf_r(struct _reent *reent, FILE *fp,const sint8 *fmt, va_list list){
	return _vfprintf_r(reent, fp,fmt, list);
}


//	-	All below high level posix calls for FSFAT access must use the function getfatfsPath("file_or_dir_path") for file (dldi sd) handling

_off_t _lseek_r(struct _reent *ptr,int fd, _off_t offset, int whence )		//(FileDescriptor :struct fd index)
{
	return fatfs_lseek(fd, offset, whence);
}

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

int _link(const sint8 *path1, const sint8 *path2)
{
    return fatfs_link(path1, path2);
}

int _unlink(const sint8 *path)
{
    return f_unlink(path);
}

//override stat();
int _stat(const sint8 *path, struct stat *buf)
{
	return fatfs_stat(path, buf);
}

int rename(const sint8 *old, const sint8 *new)
{
    return fatfs_rename(old, new);
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

//to rewrite these
int _gettimeofday(struct timeval *ptimeval,void *ptimezone){
	return 0;
}








//toolchain newlib nano lib stripped buffered fwrite support.
//so we restore POSIX file implementation. User code. 
FILE *	fopen_fs(sint8 * filepath, sint8 * args){

	sint32 posix_flags = 0;
	
	//"r"	read: Open file for input operations. The file must exist.
	//"w"	write: Create an empty file for output operations. If a file with the same name already exists, its contents are discarded and the file is treated as a new empty file.
	//"a"	append: Open file for output at the end of a file. Output operations always write data at the end of the file, expanding it. Repositioning operations (fseek, fsetpos, rewind) are ignored. The file is created if it does not exist.
	//"r+"	read/update: Open a file for update (both for input and output). The file must exist.
	//"w+"	write/update: Create an empty file and open it for update (both for input and output). If a file with the same name already exists its contents are discarded and the file is treated as a new empty file.
	//"a+"	append/update: Open a file for update (both for input and output) with all output operations writing data at the end of the file. Repositioning operations (fseek, fsetpos, rewind) affects the next input operations, but output operations move the position back to the end of file. The file is created if it does not exist.
	// rw / rw+ are used by some linux distros so we add that as well.
	
	//args to POSIX flags conversion:
	//http://pubs.opengroup.org/onlinepubs/9699919799/functions/fopen.html
	if ( (strcmp(args,"r") == 0) || (strcmp(args,"rb") == 0)){
		posix_flags |= O_RDONLY;
	}
	
	else if ( (strcmp(args,"w") == 0) || (strcmp(args,"wb") == 0)){
		posix_flags |= O_WRONLY|O_CREAT|O_TRUNC;
	}
	
	else if ( (strcmp(args,"a") == 0) || (strcmp(args,"ab") == 0)){
		posix_flags |= O_WRONLY|O_CREAT|O_APPEND;
	}
	
	else if ( (strcmp(args,"r+") == 0) || (strcmp(args,"rb+") == 0) || (strcmp(args,"r+b") == 0)){
		posix_flags |= O_RDWR;
	}
	
	else if ( (strcmp(args,"w+") == 0) || (strcmp(args,"wb+") == 0) || (strcmp(args,"w+b") == 0)){
		posix_flags |= O_RDWR|O_CREAT|O_TRUNC;
	}
	
	else if ( (strcmp(args,"a+") == 0) || (strcmp(args,"ab+") == 0) || (strcmp(args,"a+b") == 0)){
		posix_flags |= O_RDWR|O_CREAT|O_APPEND;
	}
	
	//must be struct fd Index returned here 
	sint32 fd = open(filepath, posix_flags);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	
	if (fd < 0)
	{
		return NULL;	//return NULL filehandle
	}
	
	return fdopen(fd,args);	//Use "fdopen()" for struct fd index ->FILE handle conversion
}

size_t	fread_fs(_PTR buf, size_t blocksize, size_t readsize, FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return read(fd, buf, readsize);	//returns read size from file to buffer
}

size_t fwrite_fs (_PTR buf, size_t blocksize, size_t readsize, FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return write(fd, buf, (sint32)readsize);	//returns written size from buf to file
}

int	fclose_fs(FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return close(fd);
}

int	fseek_fs(FILE *f, long offset, int whence){
	return fseek(f,offset,whence);	//call fseek so _lseek_r (overriden) is called
}

//parse: FILE * -> Struct fd FieldDescriptor -> ... 
long ftell_fs (FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return fatfs_lseek(fd, 0, SEEK_CUR);
}


#endif
