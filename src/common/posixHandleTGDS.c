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
#include "posixHandleTGDS.h"
#include "memoryHandleTGDS.h"
#include "dsregs_asm.h"
#include "devoptab_devices.h"
#include "errno.h"
#include "sys/stat.h"
#include "dirent.h"
#include "consoleTGDS.h"
#include "fileHandleTGDS.h"
#include "fsfatlayerTGDS.h"
#include "utilsTGDS.h"
#include "limitsTGDS.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <_ansi.h>
#include <reent.h>

//Notes:
//	- 	Before you get confused, the layer order is: POSIX file operations-> newlib POSIX fd assign-> devoptab filesystem -> fsfat_layer (n file descriptors for each file op) -> fsfat driver -> dldi.
//		So we can have a portable/compatible filesystem with multiple file operations.
//
//	-	Newlib dictates to override reentrant weak functions, overriding non reentrant is undefined behaviour.

//required:
void abort(){
	
}

int fork(){
	return -1;
}

//C++ requires this
void _exit (int status){
	
}

int _kill (pid_t pid, int sig){
}

pid_t _getpid (void){

}
//C++ requires this end




//read (get struct FD index from FILE * handle)

//ok _read_r reentrant called
_ssize_t _read_r ( struct _reent *ptr, int fd, void *buf, size_t cnt ){
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->read_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	return -1;
}

//ok _write_r reentrant called
//write (get struct FD index from FILE * handle)
_ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt ){
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->write_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	return -1;
}

int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode ){
	if(file != NULL){	
		int i = 0;
		char * token_rootpath = (char*)&outSplitBuf[0][0];
		str_split(file, "/", NULL);
		sint8 token_str[MAX_TGDSFILENAME_LENGTH+1] = {0};
		sint32 countPosixFDescOpen = open_posix_filedescriptor_devices() + 1;
		/* search for "file:/" in "file:/folder1/folder.../file.test" in dotab_list[].name */
		for (i = 0; i < countPosixFDescOpen ; i++){
			sprintf((sint8*)token_str,"%s%s",(char*)token_rootpath,"/");	//format properly
			if (strcmp((sint8*)token_str,devoptab_struct[i]->name) == 0)
			{
				return devoptab_struct[i]->open_r( NULL, file, flags, mode ); //returns / allocates a new struct fd index with either DIR or FIL structure allocated
			}
		}
			
		return -1;
	}
	return -1;
}


//POSIX Logic: hook devoptab descriptor into devoptab functions
int _close (int fd){
	return _close_r(NULL, fd);
}

int close (int fd){
	return _close(fd);
}
//allocates a new struct fd index with either DIR or FIL structure allocated
//not overriden, we force the call from fd_close
int _close_r ( struct _reent *ptr, int fd ){
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);	
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->close_r( NULL, fdinst->cur_entry.d_ino );
	}	
	return -1;
}


//isatty: Query whether output stream is a terminal.
int _isatty(int file){
	return  1;
}

int _end(int file)
{
	return  1;
}



//	-	All below high level posix calls for FSFAT access must use the function getfatfsPath("file_or_dir_path") for file (dldi sd) handling
_off_t _lseek_r(struct _reent *ptr,int fd, _off_t offset, int whence ){	//(FileDescriptor :struct fd index)
	return fatfs_lseek(fd, offset, whence);	
}


int _link(const sint8 *path1, const sint8 *path2){
    return fatfs_link(path1, path2);
}

int _unlink(const sint8 *path){
    return f_unlink(path);
}

int	_stat_r ( struct _reent *_r, const char *file, struct stat *pstat ){
	return fatfs_stat(file, pstat);
}

int _gettimeofday(struct timeval *ptimeval,void *ptimezone){
	return 0;
}


//TGDS Filesystem <- unix <- posix <- linux implementation


mode_t umask(mode_t mask)
{
  __set_errno (ENOSYS);
  return -1;
}

int readlink(const char *path, char *buf, size_t bufsize){
	
	/*
	//save TGDS FS context
	char TGDSCurrentWorkingDirectorySaved[MAX_TGDSFILENAME_LENGTH+1];
	strcpy (TGDSCurrentWorkingDirectorySaved, getTGDSCurrentWorkingDirectory());
	int ctxCurrentFileDirEntry = CurrentFileDirEntry;	
	int ctxLastFileEntry = LastFileEntry;
	int ctxLastDirEntry = LastDirEntry;

	char fname[MAX_TGDSFILENAME_LENGTH+1] = {0};
	strcpy(fname, path);
	int retf = FAT_FindFirstFile(fname);
	while(retf != FT_NONE){
		char * FilenameOut = NULL;
		//directory?
		if(retf == FT_DIR){
			struct FileClass * fileClassInst = getFileClassFromList(LastDirEntry);
			FilenameOut = (char*)&fileClassInst->fd_namefullPath[0];
		}
		//file?
		else if(retf == FT_FILE){
			FilenameOut = (char*)&fname[0];
		}
		
		//do logic here
		
		
		//more file/dir objects?
		retf = FAT_FindNextFile(fname);
	}
	
	//restore TGDS FS context
	strcpy (getTGDSCurrentWorkingDirectory(),TGDSCurrentWorkingDirectorySaved);
	buildFileClassListFromPath(getTGDSCurrentWorkingDirectory());
	CurrentFileDirEntry = ctxCurrentFileDirEntry;	
	LastFileEntry = ctxLastFileEntry;
	LastDirEntry = ctxLastDirEntry;
	
	*/
	
	__set_errno(ENOSYS);
	return -1;
}


int getrlimit(int resource, struct rlimit *rlim){
	__set_errno(ENOSYS);
	return -1;
}

int setrlimit(int resource, const struct rlimit *rlim){
	__set_errno(ENOSYS);
	return -1;
}

int getrusage(int who, struct rusage *usage){
	__set_errno(ENOSYS);
	return -1;
}

pid_t setsid(void){
	__set_errno(ENOSYS);
	return -1;
}

void sigaction(int sig){
	__set_errno(ENOSYS);
	return -1;
}

int getgrgid(gid_t gid, struct group *grp, char *buffer,size_t bufsize, struct group **result){
	__set_errno(ENOSYS);
	return -1;
}

struct group *getgrnam(const char *name){
	__set_errno(ENOSYS);
	return NULL;
}


int lstat(const char * path, struct stat *buf){
	return fatfs_stat((const sint8 *)path,buf);
}

// High level POSIX functions:
// Alternate TGDS high level API if current posix implementation is missing or does not work. 
// Note: uses int filehandles (or StructFD index, being a TGDS internal file handle index)
int	open_tgds(const char * filepath, sint8 * args){
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
	return _open_r(NULL , filepath, posix_flags,0);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
}

size_t	read_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd){
	return (size_t)_read_r(NULL, fd, buf, readsize);	//returns read size from file to buffer
}


size_t write_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd){
	return (size_t)_write_r(NULL, fd, buf, (sint32)readsize);	//returns written size from buf to file
}

int	close_tgds(int fd){
	return _close(fd);	//no reentrancy so we don't care. Close handle
}

int	fseek_tgds(int fd, long offset, int whence){
	return _lseek_r(NULL,fd,(_off_t)offset,whence);	//call fseek so _lseek_r (overriden) is called
}

long ftell_tgds(int fd){
	return fatfs_lseek(fd, 0, SEEK_CUR);
}

int fputs_tgds(const char * s , int fd){
	int length = strlen(s);
	int wlen = 0;
	int res = 0;
	wlen = write_tgds(s, 0, (sint32)length, fd);	//returns written size from buf to file
	wlen += write_tgds("\n", 0, (sint32)1, fd);	//returns written size from buf to file 
	if (wlen == (length+1))
	{
		res = 0;
	}
	else
	{
		res = 0;	// EOF
	}
	return res;
}

int fputc_tgds(int c, int fd){
	char ch = (char) c;
	if (write_tgds(&ch, 0, (sint32)1, fd) != 1){
		c = 0;	//EOF
	}
	return c;
}

int putc_tgds(int c, int fd){
	return fputc_tgds(c,fd);
}

int fprintf_tgds(int fd, const char *format, ...){
	va_list arg;
	volatile sint8 printfbuf[MAX_TGDSFILENAME_LENGTH+1];
	va_start (arg, format);
	vsnprintf((sint8*)printfbuf, 100, format, arg);
	va_end (arg);
	return write_tgds((char*)printfbuf, 1, strlen((char*)printfbuf), fd);
}


int fgetc_tgds(int fd){
	unsigned char ch;
    return (read_tgds(&ch, 1, 1, fd) == 1) ? (int)ch : 0; // EOF
}

char *fgets_tgds(char *s, int n, int fd){
    int ch;
    char *p = s;
    while (n > 1){
		ch = fgetc_tgds(fd);
		if (ch == 0) {	// EOF
			*p = '\0';
			return (p == s) ? NULL : s;
		}
		*p++ = ch;
		if (ch == '\n'){
			break;
		}
		n--;
    }
    if (n){
		*p = '\0';
	}
    return s;
}

int feof_tgds(int fd){
	int offset = -1;
	struct fd * fdinst = fd_struct_get(fd);
	offset = ftell_tgds(fd);
	if(fdinst->stat.st_size <= offset){
		//stream->_flags &= ~_EOF;
	}
	else{
		//stream->_flags |= _EOF;
		offset = 0;
	}
	return offset;
}

//stub
int ferror_tgds(int fd){
	if(fd == structfd_posixInvalidFileDirHandle){
		return 1;
	}
	return 0;
}

#endif