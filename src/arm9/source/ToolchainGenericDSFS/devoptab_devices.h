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

#ifndef __devoptab_devices_h__
#define __devoptab_devices_h__

#ifdef ARM9
#include "posixHandleTGDS.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#endif

#if defined(WIN32) || !defined(ARM9)
#include "../TGDSVideoConverter/TGDSTypes.h"
#include "fatfs/source/ff.h"

#define OPEN_MAXTGDS (int)(20)					//Available POSIX File Descriptors (from POSIX -> TGDS)
#define MAX_TGDSFILENAME_LENGTH (int)(256)				//NAME_MAX: Max filename (POSIX) that inherits into TGDSFILENAME_LENGTH

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

#include "devoptab_devices.h"

#ifdef ARM9
#include "limitsTGDS.h"
#include "ff.h"
#include "dsregs_asm.h"
#endif


#ifdef __cplusplus
extern "C"{
#endif

//stdin / stdout name descriptors 
extern sint8 * stdin_name_desc;
extern sint8 * stdout_name_desc;
extern sint8 * stderr_name_desc;
extern sint8 * stdstub_name_desc;

extern struct devoptab_t devoptab_stdin;
extern struct devoptab_t devoptab_stdout;
extern struct devoptab_t devoptab_sterr;
extern struct devoptab_t devoptab_sdFilesystem;
extern struct devoptab_t devoptab_stub;	//initialize up to OPEN_MAXTGDS POSIX
extern struct devoptab_t *devoptab_struct[OPEN_MAXTGDS];

extern int open_r_stdin ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_stdin ( struct _reent *r, int fd );
extern _ssize_t write_r_stdin( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_stdin ( struct _reent *r, int fd, sint8 *ptr, int len );

extern int open_r_stdout ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_stdout ( struct _reent *r, int fd );
extern _ssize_t write_r_stdout( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_stdout ( struct _reent *r, int fd, sint8 *ptr, int len );

extern int open_r_stderr ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_stderr ( struct _reent *r, int fd );
extern _ssize_t write_r_stderr( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_stderr ( struct _reent *r, int fd, sint8 *ptr, int len );

extern int open_r_fatfs ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_fatfs ( struct _reent *r, int fd );
extern _ssize_t write_r_fatfs( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_fatfs ( struct _reent *r, int fd, sint8 *ptr, int len );

extern sint32 open_posix_filedescriptor_devices();
extern void initTGDSDevoptab();

#ifdef __cplusplus
}
#endif

#endif

