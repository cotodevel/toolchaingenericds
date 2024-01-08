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

#ifndef posixHandleTGDS_h__
#define posixHandleTGDS_h__

#include "typedefsTGDS.h"
#include <ctype.h>
#include <stdlib.h>
#include <malloc.h>
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

#ifdef ARM9
#include "ff.h"
#include "utilsTGDS.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "nds_cp15_misc.h"
#include "limitsTGDS.h"
//#include "fatfslayerTGDS.h" //can't

#endif

#define MAXPRINT7ARGVCOUNT (int)(20)

#ifdef ARM9
//ARM9 Malloc
typedef u8* (*TGDSARM9MallocHandler)(int);
typedef u8* (*TGDSARM9CallocHandler)(int, int);
typedef void (*TGDSARM9FreeHandler)(void *);
typedef u32 (*TGDSARM9MallocFreeMemoryHandler)();

struct AllocatorInstance
{
	bool customMalloc;
	
	//ARM7 Malloc settings
	u32 ARM7MallocStartAddress;
	int ARM7MallocSize;
	
	//ARM9 Malloc settings
	TGDSARM9MallocHandler 			CustomTGDSMalloc9;
	TGDSARM9CallocHandler 			CustomTGDSCalloc9;
	TGDSARM9FreeHandler				CustomTGDSFree9;
	TGDSARM9MallocFreeMemoryHandler	CustomTGDSMallocFreeMemory9;
	int memoryToAllocate;
	u32 ARM9MallocStartaddress;
	
	//EWRAM DLDI ARM9 physical address
	u32 DLDI9StartAddress;
	
	//Target IO ARM7 DLDI to run
	u32 TargetARM7DLDIAddress;
	
	//Only enabled when running NTR mode homebrew through TWL mode + TWL DLDI NOT through a slot 1 DLDI device (TGDS NTR + TWL SD mode)
	bool useTWLSDThroughDLDI;
};

#ifdef __cplusplus
extern "C"{
#endif

//POSIX Malloc implementation
extern int getMaxRam();

//TGDS Malloc implementation, before using them requires a call from ARM9: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 memSizeBytes)
extern bool customMallocARM9;
//weak symbols : the implementation of this is project-defined
extern  struct AllocatorInstance * getProjectSpecificMemoryAllocatorSetup(u32 ARM7MallocStartAddress, int ARM7MallocSize, bool isCustomTGDSMalloc, u32 TargetARM7DLDIAddress);

extern struct AllocatorInstance CustomAllocatorInstance;
extern TGDSARM9MallocHandler 			TGDSMalloc9;
extern TGDSARM9CallocHandler 			TGDSCalloc9;
extern TGDSARM9FreeHandler				TGDSFree9;
extern TGDSARM9MallocFreeMemoryHandler	TGDSMallocFreeMemory9;

extern u32 ARM9MallocBaseAddress;
extern void setTGDSARM9MallocBaseAddress(u32 address);
extern u32 getTGDSARM9MallocBaseAddress();
extern void initARMCoresMalloc(
								u32 ARM7MallocStartAddress, int ARM7MallocSize, //ARM7
								u32 ARM9MallocStartaddress, u32 ARM9MallocSize, u32 * mallocHandler, //ARM9
								u32 * callocHandler, u32 * freeHandler, u32 * MallocFreeMemoryHandler, 
								bool customAllocator,
								u32 dldiMemAddress,	//ARM9 source physical DLDI Location
								u32 TargetARM7DLDIAddress, //ARM7 target physical DLDI Location (ARM7DLDI runs from here)
								bool isDLDITWLSD //is DLDI TWL SD?
							);


extern void TGDSARM9Free(void *ptr);
extern u32 TGDSARM9MallocFreeMemory();
extern u8 * TGDSARM9Realloc(void *ptr, int size);
extern u8 * TGDSARM9Calloc(int blockCount, int blockSize);
extern u8* TGDSARM9Malloc(int size);

#ifdef __cplusplus
}
#endif

#endif

#ifdef __cplusplus
extern "C"{
#endif

//Shared ARM7 Malloc
extern void initARM7Malloc(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);

#ifdef ARM9
extern void setTGDSMemoryAllocator(struct AllocatorInstance * TGDSMemoryAllocator);
extern int argvCount;
#endif

#ifdef ARM9
extern int _unlink(const sint8 *path);
extern void printfCoords(int x, int y, const char *fmt, ...);
extern bool isTGDSCustomPrintf2DConsole;
extern int TGDSDefaultPrintf2DConsole(char * stringToRenderOnscreen);
extern void * _sbrk (int size);
extern void * _sbrk_r (struct _reent * reent, int size);

extern uint32 get_ewram_start();
extern sint32 get_ewram_size();
extern uint32 	get_dtcm_start();
extern sint32 	get_dtcm_size();

extern uint32 	get_itcm_start();
extern sint32 	get_itcm_size();

//NDS Memory Map
extern bool isValidMap(uint32 addr);
#endif

extern int _vfprintf_r(struct _reent * reent, FILE *fp,const sint8 *fmt, va_list args);
extern int fork();
extern int isatty(int file);
extern int vfiprintf(FILE *fp,const sint8 *fmt, va_list list);
extern _off_t _lseek_r(struct _reent *ptr,int fd, _off_t offset, int whence );
extern int _gettimeofday(struct timeval *tv, struct timezone *tz);
extern int _end(int file);
extern _ssize_t _read_r ( struct _reent *ptr, int fd, void *buf, size_t cnt );
extern _ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt );
extern int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode );
extern int _vfiprintf_r(struct _reent *reent, FILE *fp,const sint8 *fmt, va_list list);
extern int _link(const sint8 *path1, const sint8 *path2);
extern int	_stat_r ( struct _reent *_r, const char *file, struct stat *pstat );
extern void _exit (int status);
extern int exitValue;
extern int _kill (pid_t pid, int sig);
extern pid_t _getpid (void);
extern int _close (int fd);
extern int close (int fd);
extern int _close_r ( struct _reent *ptr, int fd );

// High level POSIX functions:
// Alternate TGDS high level API if current posix implementation is missing or does not work. 
// Note: uses int filehandles (or StructFD index, being a TGDS internal file handle index)
extern int	open_tgds(const char * filepath, sint8 * args);
extern size_t	read_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd);
extern size_t write_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd);
extern int	close_tgds(int fd);
extern int	fseek_tgds(int fd, long offset, int whence);
extern long ftell_tgds(int fd);
extern int fputs_tgds(const char * s , int fd);
extern int fputc_tgds(int c, int fd);
extern int putc_tgds(int c, int fd);
extern int fprintf_tgds(int fd, const char *format, ...);
extern int fgetc_tgds(int fd);
extern char *fgets_tgds(char *s, int n, int fd);
extern int feof_tgds(int fd);
extern int ferror_tgds(int fd);
extern void TryToDefragmentMemory();

//newlib
extern uint32 get_lma_libend();		//linear memory top
extern uint32 get_lma_wramend();	//(ewram end - linear memory top ) = malloc free memory

#ifdef __cplusplus
}
#endif

#ifdef ARM9
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

#endif

