#ifdef ARM9

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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <_ansi.h>
#include <reent.h>
#include "typedefsTGDS.h"
#include "ipcfifoTGDS.h"
#include "posixHandleTGDS.h"
#include "linkerTGDS.h"


#include "dsregs_asm.h"
#include "devoptab_devices.h"
#include "errno.h"
#include "sys/stat.h"
#include "dirent.h"
#include "consoleTGDS.h"
#include "clockTGDS.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "limitsTGDS.h"
#include "dldi.h"

uint32 get_lma_libend(){
	return (uint32)(&__vma_stub_end__);	//linear memory top (start)
}

//(ewram end - linear memory top ) = malloc free memory (bottom, end)
uint32 get_lma_wramend(){
	#ifdef ARM7
	extern uint32 sp_USR;	//the farthest stack from the end memory is our free memory (in ARM7, shared stacks)
	return (uint32)(&sp_USR);
	#endif
	#ifdef ARM9
	return (uint32)(&_ewram_end);	//EWRAM has no stacks shared so we use the end memory 
	#endif
}

uint32 get_ewram_start(){
	return (uint32)(&_ewram_start);
}

sint32 get_ewram_size(){
	return (sint32)((uint8*)(uint32*)&_ewram_end - (sint32)(&_ewram_start));
}

uint32 get_itcm_start(){
	return (uint32)(&_itcm_start);
}

sint32 get_itcm_size(){
	return (sint32)((uint8*)(uint32*)&_itcm_end - (sint32)(&_itcm_start));
}

uint32 get_dtcm_start(){
	return (uint32)(&_dtcm_start);
}

sint32 get_dtcm_size(){
	return (sint32)((uint8*)(uint32*)&_dtcm_end - (sint32)(&_dtcm_start));
}

/* ------------------------------------------------------------------------- */
/*!Increase program data space
   This is a minimal implementation.  Assuming the heap is growing upwards
   from __heap_start towards __heap_end.
   See linker file for definition.
   @param[in] incr  The number of bytes to increment the stack by.
   @return  A pointer to the start of the new block of memory                */
/* ------------------------------------------------------------------------- */
void *
_sbrk (int  incr)
{
	extern char __heap_start;//set by linker
	extern char __heap_end;//set by linker

	static char *heap_end;		/* Previous end of heap or 0 if none */
	char        *prev_heap_end;

	if (0 == heap_end) {
		heap_end = (sint8*)get_lma_libend();			/* Initialize first time round */
	}

	prev_heap_end  = heap_end;
	heap_end      += incr;
	//check
	if( heap_end < (char*)get_lma_wramend()) {

	} else {
		errno = ENOMEM;
		return (char*)-1;
	}
	return (void *) prev_heap_end;

}	/* _sbrk () */

void * _sbrk_r (struct _reent * reent, int size){
	return _sbrk (size);
}

//NDS Memory Map (valid):
//todo: detect valid maps according to MPU settings
bool isValidMap(uint32 addr){
	if( 
		#ifdef ARM9
		((addr >= (uint32)0x06000000) && (addr < (uint32)0x06020000))	//NDS VRAM BG Engine Region protected	0x06000000 - 0x0601ffff
		||
		((addr >= (uint32)0x06020000) && (addr < (uint32)0x06040000))	//NDS VRAM BG Engine Region protected	0x06020000 - 0x0603ffff
		||
		((addr >= (uint32)0x06040000) && (addr < (uint32)0x06060000))	//NDS VRAM BG Engine Region protected	0x06040000 - 0x0605ffff
		||
		((addr >= (uint32)0x06060000) && (addr < (uint32)0x06080000))	//NDS VRAM BG Engine Region protected	0x06060000 - 0x0607ffff
		||
		((addr >= (uint32)0x06200000) && (addr < (uint32)(0x06200000 + 0x20000)))	//NDS VRAM Engine Region protected	//theoretical map
		||
		((addr >= (uint32)0x06400000) && (addr < (uint32)(0x06400000 + 0x14000)))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x06600000) && (addr < (uint32)(0x06600000 + 0x20000)))	//NDS VRAM Engine Region protected	// theoretical map
		||
		((addr >= (uint32)0x06800000) && (addr < (uint32)(0x06800000 + (656 * 1024) )))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x07000000) && (addr < (uint32)(0x07000000 + (2 * 1024) )))	//NDS VRAM OAM Region protected	// theoretical map?
		||
		((addr >= (uint32)get_ewram_start()) && (addr <= (uint32)(get_ewram_start() + get_ewram_size())))	//NDS EWRAM Region protected
		||
		((addr >= (uint32)(get_itcm_start())) && (addr <= (uint32)(get_itcm_start()+get_itcm_size())))	//NDS ITCM Region protected
		||
		((addr >= (uint32)(get_dtcm_start())) && (addr <= (uint32)(get_dtcm_start()+get_dtcm_size())))	//NDS DTCM Region protected
		||
		((addr >= (uint32)(0x05000000)) && (addr <= (uint32)(0x05000000 + 2*1024)))	//NDS Palette Region protected
		||
		( (WRAM_CR & WRAM_32KARM9_0KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 32K Region protected
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )
		#endif
		#ifdef ARM7
		((addr >= (uint32)get_iwram_start()) && (addr <= (uint32)(get_iwram_start() + get_iwram_size())))	//NDS EWRAM Region protected
		||
		( (WRAM_CR & WRAM_0KARM9_32KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 32K Region protected
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )
		||
		( (WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )
		#endif
		||
		((addr >= (uint32)(0x04000000)) && (addr <= (uint32)(0x04000000 + 4*1024)))	//NDS IO Region protected
		||
		((addr >= (uint32)(0x08000000)) && (addr <= (uint32)(0x08000000 + (32*1024*1024))))	//GBA ROM MAP (allows to read GBA carts over GDB)
	){
		return true;
	}
	return false;
}


//ARM9 Malloc implementation: Blocking, because several processes running on ARM7 may require ARM9 having a proper malloc impl.
u32 ARM9MallocBaseAddress = 0;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setTGDSARM9MallocBaseAddress(u32 address) {
	ARM9MallocBaseAddress = address;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u32 getTGDSARM9MallocBaseAddress() {
	return ARM9MallocBaseAddress;
}
//Global
bool customMallocARM9 = false;
TGDSARM9MallocHandler 			TGDSMalloc9;
TGDSARM9CallocHandler 			TGDSCalloc9;
TGDSARM9FreeHandler				TGDSFree9;
TGDSARM9MallocFreeMemoryHandler	TGDSMallocFreeMemory9;

//Prototype
struct AllocatorInstance CustomAllocatorInstance;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void initARMCoresMalloc(u32 ARM7MallocStartAddress, int ARM7MallocSize,											//ARM7
						u32 ARM9MallocStartaddress, u32 ARM9MallocSize, u32 * mallocHandler, u32 * callocHandler, //ARM9
						u32 * freeHandler, u32 * MallocFreeMemoryHandler, bool customAllocator, u32 dldiMemAddress,
						u32 TargetARM7DLDIAddress
) {
	
	if (_io_dldi_stub.ioInterface.features & FEATURE_SLOT_GBA) {
		SetBusSLOT1ARM9SLOT2ARM7();
	}
	if (_io_dldi_stub.ioInterface.features & FEATURE_SLOT_NDS) {
		SetBusSLOT1ARM7SLOT2ARM9();
	}
	
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (uint32)ARM7MallocStartAddress;
	fifomsg[1] = (uint32)ARM7MallocSize;
	fifomsg[2] = (uint32)customAllocator;
	fifomsg[3] = (uint32)dldiMemAddress;
	fifomsg[4] = (uint32)TargetARM7DLDIAddress;
	
	setTGDSARM9MallocBaseAddress(ARM9MallocStartaddress);
	if(customAllocator == true){
		if(mallocHandler != NULL){
			TGDSMalloc9 = (TGDSARM9MallocHandler)mallocHandler;
		}
		if(callocHandler != NULL){
			TGDSCalloc9 = (TGDSARM9CallocHandler)callocHandler;
		}
		if(freeHandler != NULL){
			TGDSFree9 = (TGDSARM9FreeHandler)freeHandler;
		}
		if(MallocFreeMemoryHandler != NULL){
			TGDSMallocFreeMemory9 = (TGDSARM9MallocFreeMemoryHandler)MallocFreeMemoryHandler;
		}
	}
	TGDSInitLoopCount = 0;
	setupLibUtils(); //ARM9 libUtils Setup
	SendFIFOWords(TGDS_ARM7_SETUPMALLOCDLDI, 0xFF);	//ARM7 Setup: DLDI, and extensions if enabled through libutils
	while(fifomsg[4] == TargetARM7DLDIAddress){
		if(TGDSInitLoopCount > (1048576 << 3) ){
			u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
			int stage = 1;
			handleDSInitError(stage, (u32)fwNo);
		}
		TGDSInitLoopCount++;
	}
	
	__dsimode = (bool)fifomsg[2];
	TWLModeInternalSDAccess = fifomsg[3]; //ARM7 DLDI decides the current TGDS FS mode
	bool dldiInitStatus = (bool)fifomsg[4]; //DLDI / SDIO init: true: OK, false: error
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setTGDSMemoryAllocator(struct AllocatorInstance * TGDSMemoryAllocator) {
	//Enable Default/Custom TGDS Memory Allocator
	u32 ARM9MallocStartaddress = (u32)sbrk(0);
	if(TGDSMemoryAllocator->customMalloc == false){
		initARMCoresMalloc(
			TGDSMemoryAllocator->ARM7MallocStartAddress, TGDSMemoryAllocator->ARM7MallocSize,	//ARM7 Malloc
			ARM9MallocStartaddress, getMaxRam(), NULL, NULL, NULL, NULL, customMallocARM9,		//ARM9 Malloc
			TGDSMemoryAllocator->DLDI9StartAddress,
			TGDSMemoryAllocator->TargetARM7DLDIAddress
		);
	}
	else{
		customMallocARM9 = true;
		initARMCoresMalloc(
			TGDSMemoryAllocator->ARM7MallocStartAddress, TGDSMemoryAllocator->ARM7MallocSize,				//ARM7 Malloc
			(u32)TGDSMemoryAllocator->ARM9MallocStartaddress, (u32)TGDSMemoryAllocator->memoryToAllocate,		//ARM9 Malloc
			(u32 *)TGDSMemoryAllocator->CustomTGDSMalloc9, (u32 *)TGDSMemoryAllocator->CustomTGDSCalloc9, 
			(u32 *)TGDSMemoryAllocator->CustomTGDSFree9, (u32 *)TGDSMemoryAllocator->CustomTGDSMallocFreeMemory9, customMallocARM9,
			TGDSMemoryAllocator->DLDI9StartAddress,
			TGDSMemoryAllocator->TargetARM7DLDIAddress
		);
	}
	customMallocARM9 = TGDSMemoryAllocator->customMalloc;
}

int printf(const char *fmt, ...){
	//Indentical Implementation as GUI_printf
	va_list args;
	va_start (args, fmt);
	vsnprintf ((sint8*)ConsolePrintfBuf, (int)sizeof(ConsolePrintfBuf), fmt, args);
	va_end (args);
	
    // FIXME
    bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &smallfont_7_font;
	
	int color = (int)TGDSPrintfColor_LightGrey;	//default color
	//int stringSize = (int)strlen((const char*)ConsolePrintfBuf);
	
	//Separate the TGDS Console font color if exists
	char cpyBuf[256+1] = {0};
	strcpy(cpyBuf, (const char*)ConsolePrintfBuf);
	char * outBuf = (char *)TGDSARM9Malloc(256*10);
	char * colorChar = (char*)((char*)outBuf + (1*256));
	int matchCount = str_split((char*)cpyBuf, ">", outBuf, 10, 256);
	if(matchCount > 0){
		color = atoi(colorChar);
		ConsolePrintfBuf[strlen((const char*)ConsolePrintfBuf) - (strlen(colorChar)+1) ] = '\0';
	}
	
    GUI_drawText(&zone, 0, GUI.printfy, color, (sint8*)ConsolePrintfBuf, readAndBlendFromVRAM);
    GUI.printfy += GUI_getFontHeight(&zone);
	TGDSARM9Free(outBuf);
	return strlen((const char*)ConsolePrintfBuf)+1;
}

//same as printf but having X, Y coords (relative to char width and height)
void printfCoords(int x, int y, const char *fmt, ...){
	va_list args;
	va_start (args, fmt);
	vsnprintf ((sint8*)ConsolePrintfBuf, 64, fmt, args);
	va_end (args);
	
	// FIXME
    bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &smallfont_7_font;
	GUI.printfy = y * GUI_getFontHeight(&zone);
	x = x * zone.font->height;
	int color = (int)TGDSPrintfColor_LightGrey;	//default color
	//int stringSize = (int)strlen((const char*)ConsolePrintfBuf);
	
	//Separate the TGDS Console font color if exists
	char cpyBuf[256+1] = {0};
	strcpy(cpyBuf, (const char*)ConsolePrintfBuf);
	char * outBuf = (char *)TGDSARM9Malloc(256*10);
	char * colorChar = (char*)((char*)outBuf + (1*256));
	int matchCount = str_split((char*)cpyBuf, ">", outBuf, 10, 256);
	if(matchCount > 0){
		color = atoi(colorChar);
		ConsolePrintfBuf[strlen((const char*)ConsolePrintfBuf) - (strlen(colorChar)+1) ] = '\0';
	}
	
    GUI_drawText(&zone, x, GUI.printfy, color, (sint8*)ConsolePrintfBuf, readAndBlendFromVRAM);
    GUI.printfy += GUI_getFontHeight(&zone);
	TGDSARM9Free(outBuf);
}

int _vfprintf_r(struct _reent * reent, FILE *fp, const sint8 *fmt, va_list args){
	coherent_user_range_by_size((uint32)fmt, strlen(fmt)+1);
	char * stringBuf = (char*)&ConsolePrintfBuf[0];
	vsnprintf ((sint8*)stringBuf, 64, fmt, args);
	
	//if the fprintf points to stdout (unix shell C/C++ source code), redirect to DS printf.
	if(fp == stdout){
		int stringSize = (int)strlen(stringBuf);
		if(stringSize > 64){
			stringSize = 64;
			stringBuf[stringSize-1] = '\0';
		}
		//Indentical Implementation as GUI_printf
		
		// FIXME
		bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
		t_GUIZone zone;
		zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
		zone.font = &smallfont_7_font;
		int color = 0xff;	//white
		GUI_drawText(&zone, 0, GUI.printfy, color, (sint8*)stringBuf, readAndBlendFromVRAM);
		GUI.printfy += GUI_getFontHeight(&zone);
		return stringSize;
	}
	return fputs (stringBuf, fp);
}


//Notes:
//	- 	Before you get confused, the layer order is: POSIX file operations-> newlib POSIX fd assign-> devoptab filesystem -> fatfs_layer (n file descriptors for each file op) -> fatfs driver -> dldi.
//		So we can have a portable/compatible filesystem with multiple file operations.
//
//	-	Newlib dictates to override reentrant weak functions, overriding non reentrant is undefined behaviour.

//TGDS Filesystem <- unix <- posix and/or linux implementation

int fork(){
	return -1;
}

//C++ requires this
void _exit (int status){
	
	//todo: add some exception handlers to notify ARM cores program has ran	
	
	clrscr();
	loggerARM9LibUtilsCallback("----");
	loggerARM9LibUtilsCallback("----");
	loggerARM9LibUtilsCallback("----");
	loggerARM9LibUtilsCallback("----");
	loggerARM9LibUtilsCallback("TGDS APP Halt: Error Status: %d", status);
	while(1==1){
		IRQVBlankWait();
	}
}

int _kill (pid_t pid, int sig){
	return -1;
}

pid_t _getpid (void){
	return 0;	//shall always return valid (0)
}
//C++ requires this end


//read (get struct FD index from FILE * handle)

//ok _read_r reentrant called
_ssize_t _read_r ( struct _reent *ptr, int fd, void *buf, size_t cnt ){
	//Conversion here 
	struct fd * fdinst = getStructFD(fd);
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->read_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	return -1;
}

//ok _write_r reentrant called
//write (get struct FD index from FILE * handle)
_ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt ){
	//Conversion here 
	struct fd * fdinst = getStructFD(fd);
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->write_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	return -1;
}

int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode ){
	if(file != NULL){
		char * outBuf = (char *)malloc(256*10);
		int i = 0;
		char * token_rootpath = (char*)((char*)outBuf + (0*256));
		str_split((char*)file, "/", outBuf, 10, 256);
		sint8 token_str[MAX_TGDSFILENAME_LENGTH+1] = {0};
		sint32 countPosixFDescOpen = open_posix_filedescriptor_devices() + 1;
		/* search for "file:/" in "file:/folder1/folder.../file.test" in dotab_list[].name */
		for (i = 0; i < countPosixFDescOpen ; i++){
			if(strlen(token_rootpath) > 0){
				sprintf((sint8*)token_str,"%s%s",(char*)token_rootpath,"/");	//format properly
				if (strcmp((sint8*)token_str,devoptab_struct[i]->name) == 0){
					free(outBuf);
					return devoptab_struct[i]->open_r( NULL, file, flags, mode ); //returns / allocates a new struct fd index with either DIR or FIL structure allocated
				}
			}
		}
		free(outBuf);
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
	struct fd * fdinst = getStructFD(fd);	
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

//	-	All below high level posix calls for fatfs access must use the function getfatfsPath("file_or_dir_path") for file (dldi sd) handling
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

int _gettimeofday(struct timeval *tv, struct timezone *tz){
	if (tv == NULL)
    {
		__set_errno (EINVAL);
		return -1;
    }
    const struct tm *tmp = getTime();
    if(tmp == NULL){
		return -1;
    }
	tv->tv_sec = (long int)getNDSRTCInSeconds();
	tv->tv_usec = 0L;
	if (tz != NULL)
    {
		const time_t timer = tv->tv_sec;
		struct tm tm;
		const long int save_timezone = 0; //__timezone;
		const long int save_daylight = 0; //__daylight;
		char *save_tzname[2];
		save_tzname[0] = ""; //__tzname[0];
		save_tzname[1] = "";//__tzname[1];
		//tmp = localtime_r (&timer, &tm);
		//tz->tz_minuteswest = 0; //__timezone / 60;
		//tz->tz_dsttime = 0; //__daylight;
		//__timezone = save_timezone;
		//__daylight = save_daylight;
		//__tzname[0] = save_tzname[0];
		//__tzname[1] = save_tzname[1];
    }
	return 0;
}

mode_t umask(mode_t mask)
{
  __set_errno (ENOSYS);
  return -1;
}

int readlink(const char *path, char *buf, size_t bufsize){
	__set_errno(ENOSYS);
	return -1;
}

int lstat(const char * path, struct stat *buf){
	return fatfs_stat((const sint8 *)path,buf);
}

int getMaxRam(){
	int maxRam = ((int)(&_ewram_end)  - (int)sbrk(0) );
	return (int)maxRam;
}

//Memory is too fragmented up to this point, causing to have VERY little memory left. 
//Luckily for us this memory hack allows dmalloc to re-arrange and free more memory for us! Also fixing malloc memory fragmentation!! WTF Dude.
void TryToDefragmentMemory(){
	int freeRam = getMaxRam();
	//I'm not kidding, this allows to de-fragment memory. Relative to how much memory we have and re-allocate it
	char * defragMalloc[1024];	//4M / 4096. DS Mem can't be higher than this
	int memSteps = (freeRam / 4096);
	int i = 0;
	for(i = 0; i < memSteps; i++){
		defragMalloc[i] = malloc(4);	//dmalloc blocks are 4K each 
	}
	for(i = 0; i < memSteps; i++){
		if(defragMalloc[i] != NULL){
			free(defragMalloc[i]);
		}
	}
}




/*
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
	wlen = write_tgds((void*)s, 0, (sint32)length, fd);	//returns written size from buf to file
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
	struct fd * fdinst = getStructFD(fd);
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
	if(fd == structfd_posixInvalidFileDirOrBufferHandle){
		return 1;
	}
	return 0;
}
*/

#endif


#ifdef ARM9

u8* TGDSARM9Malloc(int size){
	if(customMallocARM9 == false){
		return (u8*)malloc((const int)size);
	}
	else{
		return (u8*)TGDSMalloc9((const int)size);
	}
}

u8 * TGDSARM9Calloc(int blockCount, int blockSize){
	if(customMallocARM9 == false){
		return (u8*)calloc(blockSize, blockCount);	//reverse order
	}
	else{
		return (u8*)TGDSCalloc9(blockCount, blockSize);	//same order
	}
}

u8 * TGDSARM9Realloc(void *ptr, int size){
	if(customMallocARM9 == false){
		if(ptr != NULL){
			free(ptr);
		}
		return (u8*)malloc(size);
	}
	else{
		if(ptr != NULL){
			TGDSFree9(ptr);
		}
		return (u8*)TGDSMalloc9(size);
	}
}

void TGDSARM9Free(void *ptr){
	if(customMallocARM9 == false){
		free(ptr);
	}
	else{
		TGDSFree9(ptr);
	}
}

u32 TGDSARM9MallocFreeMemory(){
	if(customMallocARM9 == false){
		return (u32)getMaxRam();
	}
	else{
		return (u32)TGDSMallocFreeMemory9();
	}
}

#endif