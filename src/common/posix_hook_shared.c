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

#ifdef ARM7

#endif

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
//	-	Newlib nano dictates to override reentrant weak functions, overriding non reentrant is undefined behaviour.

//required:

int fork()
{
	return -1;
}

//C++ requires this
void _exit (int status)
{
	//clrscr();
	//printf("C++ abort()!");
	//printf("End.");
	//for (;;) { }
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
	
	if( (fdinst != NULL) && ((sint32)fdinst->fd_posix != (sint32)structfd_posixFileDescrdefault)  ) {
		return (_ssize_t)devoptab_list[fdinst->fd_posix]->read_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	
	return -1;
}
//ok _write_r reentrant called
//write (get struct FD index from FILE * handle)
_ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt ){
	
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);
	
	if( (fdinst != NULL) && ((sint32)fdinst->fd_posix != (sint32)structfd_posixFileDescrdefault)  ) {
		return (_ssize_t)devoptab_list[fdinst->fd_posix]->write_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	
	return -1;
}

int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode ){	
	sint8 **tokens;
	int count = 0, i = 0;
	volatile sint8 str[256];	//file safe buf
	memcpy ( (uint8*)str, (uint8*)file, 256);

	count = split ((const sint8*)str, '/', &tokens);	
	volatile sint8 token_str[64];
	
	sint32 countPosixFDescOpen = open_posix_filedescriptor_devices() + 1;
	/* search for "file:/" in "file:/folder1/folder.../file.test" in dotab_list[].name */
	for (i = 0; i < countPosixFDescOpen ; i++){
		if(count > 0){
			sprintf((sint8*)token_str,"%s/",tokens[0]);	//format properly
			if (strcmp((sint8*)token_str,devoptab_list[i]->name) == 0)
			{
				return devoptab_list[i]->open_r( NULL, file, flags, mode ); //returns / allocates a new struct fd index with either DIR or FIL structure allocated
			}
		}
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
int _close_r ( struct _reent *ptr, int fd )
{
	//Conversion here 
	struct fd * fdinst = fd_struct_get(fd);
	
	if( (fdinst != NULL) && ((sint32)fdinst->fd_posix != (sint32)structfd_posixFileDescrdefault)  ) {
		return (_ssize_t)devoptab_list[fdinst->fd_posix]->close_r( NULL, fdinst->cur_entry.d_ino );
	}	
	return -1;
}

/*
 isatty
 Query whether output stream is a terminal.
 */
int _isatty(int file)
{
	return  1;
}	// _isatty()

int _end(int file)
{
	return  1;
}	// _isatty()



//	-	All below high level posix calls for FSFAT access must use the function getfatfsPath("file_or_dir_path") for file (dldi sd) handling

_off_t _lseek_r(struct _reent *ptr,int fd, _off_t offset, int whence ){	//(FileDescriptor :struct fd index)
	return fatfs_lseek(fd, offset, whence);	
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
int	_stat_r ( struct _reent *_r, const char *file, struct stat *pstat )
{
	return fatfs_stat(file, pstat);
}

int _gettimeofday(struct timeval *ptimeval,void *ptimezone){
	return 0;
}



//todo: retest all of them in default newlib posix fs.
/*
//toolchain newlib nano lib stripped buffered fwrite support.
//so we restore POSIX file implementation. User code. 
FILE *	fopen(const char * filepath, sint8 * args){

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
	sint32 fd = open(filepath, posix_flags,0);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	
	if (fd < 0)
	{
		return NULL;	//return NULL filehandle
	}
	
	FILE * FRET = fdopen(fd,args);	//Use "fdopen()" for struct fd index ->FILE handle conversion
	return FRET;
}

size_t	fread(_PTR buf, size_t blocksize, size_t readsize, FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return read(fd, buf, readsize);	//returns read size from file to buffer
}

size_t fwrite (_PTR buf, size_t blocksize, size_t readsize, FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return write(fd, buf, (sint32)readsize);	//returns written size from buf to file
}

int	fclose(FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return _close_r(NULL,fd);	//no reentrancy so we don't care. Close handle
}

int	fseek(FILE *f, long offset, int whence){
	return fseek(f,offset,whence);	//call fseek so _lseek_r (overriden) is called
}

//parse: FILE * -> Struct fd FieldDescriptor -> ... 
long ftell (FILE * fileInst){
	sint32 fd = fileno(fileInst);
	return fatfs_lseek(fd, 0, SEEK_CUR);
}

int fputs(const char * s , FILE * fileInst ){
	int length = strlen(s);
	int wlen = 0;
	int res;
	
	sint32 fd = fileno(fileInst);
	wlen = write(fd, s, (sint32)length);	//returns written size from buf to file
	wlen += write(fd, "\n", 1);	//returns written size from buf to file 

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

int fputc(int c, FILE * fileInst){
	char ch = (char) c;
	sint32 fd = fileno(fileInst);
	if (write(fd, &ch, 1) != 1){
		c = 0;	//EOF
	}
	return c;
}

int putc(int c, FILE * fileInst){
	return fputc(c,fileInst);
}

int fprintf (FILE *stream, const char *format, ...)
{
	va_list arg;
	volatile sint8 g_printfbuf[512];
	va_start (arg, format);
	vsnprintf((sint8*)g_printfbuf, 100, format, arg);
	va_end (arg);
	
	return fwrite((char*)g_printfbuf, 1, strlen((char*)g_printfbuf), stream);
}


int fgetc(FILE *fp)
{
	unsigned char ch;
    return (fread(&ch, 1, 1, fp) == 1) ? (int)ch : 0; // EOF
}


char *fgets(char *s, int n, FILE * f)
{
    int ch;
    char *p = s;

    while (n > 1) {
		ch = fgetc(f);
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

int feof(FILE * stream)
{
	int offset = -1;
	sint32 fd = fileno(stream);
	struct fd * fdinst = fd_struct_get(fd);
	offset = ftell(stream);
	if(fdinst->stat.st_size <= offset){
		stream->_flags &= ~_EOF;
	}
	else{
		stream->_flags |= _EOF;
		offset = 0;
	}
	
	return offset;
}

//stub
int ferror(FILE * stream){
	if(!stream){
		return 1;
	}
	
	return 0;
}
*/
#endif