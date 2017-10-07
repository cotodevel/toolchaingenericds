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

#ifdef ARM9

#ifndef __posix_hook_shared_h__
#define __posix_hook_shared_h__

#include <sys/reent.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <_ansi.h>
#include <reent.h>
#include <sys/lock.h>
#include <fcntl.h>


#include "ff.h"
#include "mem_handler_shared.h"
#include "typedefs.h"
#include "dsregs.h"

//fseek

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

enum _flags {
    _READ = 01,     /* file open for reading */
    _WRITE = 02,    /* file open for writing */
    _UNBUF = 03,    /* file is unbuffered */
    _EOF    = 010,  /* EOF has occurred on this file */
    _ERR    = 020,  /* error occurred on this file */
};


//File Descriptor Semantics for Newlib
//Stream 0 is defined by convention to be the "standard input" stream, which newlib uses for the getc() and similar functions that don't otherwise specify an input stream. 
//Stream 1 is "standard output," the destination for printf() and puts(). 
//Stream 2 refers to "standard error," the destination conventionally reserved for messages of grave importance.

#define devoptab_fname_size 32

//devoptab_t that match file descriptors
struct devoptab_t{
   const sint8 name[devoptab_fname_size];
   int (*open_r )( struct _reent *r, const sint8 *path, int flags, int mode );
   int (*close_r )( struct _reent *r, int fd ); 
   _ssize_t (*write_r ) ( struct _reent *r, int fd, const sint8 *ptr, int len );
   _ssize_t (*read_r )( struct _reent *r, int fd, sint8 *ptr, int len );
};


#endif


#ifdef __cplusplus
extern "C"{
#endif
//ok
extern void * _sbrk (int size);
extern off_t _lseek(int fd, off_t pos, int whence);
extern int _gettimeofday(struct timeval *ptimeval,void *ptimezone);
extern int _isatty(int fd);

//overriden re entrant posix hooks

extern int _fork_r ( struct _reent *ptr );

extern _ssize_t _read_r ( struct _reent *ptr, int fd, void *buf, size_t cnt );
extern _ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt );
extern int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode );
extern int _close_r ( struct _reent *ptr, int fd );


extern int _vfprintf_r(struct _reent *reent, FILE *fp,const sint8 *fmt, va_list list);
extern int _vfiprintf_r(struct _reent *reent, FILE *fp,const sint8 *fmt, va_list list);

extern int _link(const sint8 *path1, const sint8 *path2);
extern int	_stat_r ( struct _reent *_r, const char *file, struct stat *pstat );
extern int _unlink(const sint8 *path);
extern int rename(const sint8 *old, const sint8 *new);
extern int fsync(int fd);
extern int mkdir(const sint8 *path, mode_t mode);
extern int rmdir(const sint8 *path);
extern int chdir(const sint8 *path);
extern sint8 *getcwd(sint8 *buf, size_t size);
extern DIR *opendir(const sint8 *path);
extern int closedir(DIR *dirp);
extern struct dirent *readdir(DIR *dirp);
extern int  readdir_r(DIR * dirp,struct dirent * entry,struct dirent ** result);
extern void rewinddir(DIR *dirp);
extern int dirfd(DIR *dirp);
extern DIR *fdopendir(int fd);
extern void seekdir(DIR *dirp, long loc);


extern _off_t _lseek_r(struct _reent *ptr,int fd, _off_t offset, int whence );
extern int _fstat_r ( struct _reent *_r, int fd, struct stat *buf );

//posix file descriptor replacement
extern FILE *	fopen_fs(sint8 * filepath, sint8 * args);
extern size_t	fread_fs(_PTR buf, size_t blocksize, size_t readsize, FILE * fileInst);
extern size_t 	fwrite_fs (_PTR buf, size_t blocksize, size_t readsize, FILE * fileInst);
extern int	fclose_fs(FILE * fileInst);
extern int	fseek_fs(FILE *fileInst, long offset, int whence);
extern long ftell_fs (FILE * fileInst);
extern int fputs_fs(const char * s , FILE * fileInst );
extern int fputc_fs(int c, FILE * fileInst);
extern int putc_fs(int c, FILE * fileInst);
extern int fprintf_fs (FILE *stream, const char *format, ...);
extern int fgetc_fs(FILE *fp);
extern char *fgets_fs(char *s, int n, FILE * f);

extern int feof_fs(FILE * stream);
extern int ferror_fs(FILE * stream);

#ifdef __cplusplus
}
#endif

#endif