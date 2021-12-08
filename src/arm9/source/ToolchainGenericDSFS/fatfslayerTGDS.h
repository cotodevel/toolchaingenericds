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

#ifndef __fatfslayerTGDS_h__
#define __fatfslayerTGDS_h__

////////////////////////////////////////////////////////////////////////////INTERNAL CODE START/////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/stat.h>
#include "devoptab_devices.h"

#ifdef ARM9
#include <dirent.h>
#include "ff.h"
#include "posixHandleTGDS.h"
#endif

#if defined (MSDOS) || defined(WIN32)

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "..\misc\vs2012TGDS-FS\TGDSFSVS2012\TGDSFSVS2012\TGDSTypes.h"
#include "fatfs\source\ff.h"	//DIR struct definition here. dirent.h´s DIR was removed, and rewritten

#define fatfs_O_ACCMODE (FA_READ|FA_WRITE)
#define	O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)

#define	_IFMT		0170000	/* type of file */
#define		_IFDIR	0040000	/* directory */
#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */

#define 	S_BLKSIZE  1024 /* size of a block */

#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */
#define	S_ISVTX		0001000	/* save swapped text even after use */
#ifndef	_POSIX_SOURCE
#define	S_IREAD		0000400	/* read permission, owner */
#define	S_IWRITE 	0000200	/* write permission, owner */
#define	S_IEXEC		0000100	/* execute/search permission, owner */
#define	S_ENFMT 	0002000	/* enforcement-mode locking */
#endif	/* !_POSIX_SOURCE */

#define	S_IFMT		_IFMT
#define	S_IFDIR		_IFDIR
#define	S_IFCHR		_IFCHR
#define	S_IFBLK		_IFBLK
#define	S_IFREG		_IFREG
#define	S_IFLNK		_IFLNK
#define	S_IFSOCK	_IFSOCK
#define	S_IFIFO		_IFIFO


#define	S_IRWXU 	(S_IRUSR | S_IWUSR | S_IXUSR)
#define		S_IRUSR	0000400	/* read permission, owner */
#define		S_IWUSR	0000200	/* write permission, owner */
#define		S_IXUSR 0000100/* execute/search permission, owner */
#define	S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define	S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */

#define	S_ISBLK(m)	(((m)&_IFMT) == _IFBLK)
#define	S_ISCHR(m)	(((m)&_IFMT) == _IFCHR)
#define	S_ISDIR(m)	(((m)&_IFMT) == _IFDIR)
#define	S_ISFIFO(m)	(((m)&_IFMT) == _IFIFO)
#define	S_ISREG(m)	(((m)&_IFMT) == _IFREG)
#define	S_ISLNK(m)	(((m)&_IFMT) == _IFLNK)
#define	S_ISSOCK(m)	(((m)&_IFMT) == _IFSOCK)


//this dirent is guaranteed to work with fatfs, add items if you want but don't edit those definitions.
struct dirent {
	ino_t d_ino;				/* file serial number -- Index == Posix d_ino "Serial Number" file handle = getStructFD(index) */
    char d_name[MAX_TGDSFILENAME_LENGTH+1];	/* name must be no longer than this */
};
#endif

//TGDS FS defs: These must be typecasted otherwise compiler will generate wrong strb opcodes
#define FT_NONE (int)(0)
#define FT_FILE (int)(1)
#define FT_DIR (int)(2)
#define FT_BUFFER (int)(3)

#define structfd_isused 	(sint32)(1)
#define structfd_isunused	(sint32)(0)
#define structfd_isattydefault	(sint32)(0)
#define structfd_descriptorflagsdefault	(sint32)(0)
#define structfd_status_flagsdefault	(sint32)(0)
#define structfd_fildir_offsetdefault	(sint32)(0)

//These must be invalid values so false positives do not arise in lookup functions
//FileList specific (internal retCode)
#define structfd_posixInvalidFileDirOrBufferHandle	(sint32)(-1)	//used by fatfs_xxx layer, FAT_xxx layer (libfat wrapper), cluster functions. Marks a TGDS FileHandle as invalid (TGDS FileHandle: structFD index file descriptor)
#define structfd_posixExcludedFileDirOrBufferHandle	(sint32)(-2)	//used by fatfs_xxx layer, FAT_xxx layer (libfat wrapper), cluster functions. Marks a TGDS FileHandle as excluded and unused by TGDS FS API (TGDS FileHandle: structFD index file descriptor)

#define dirent_default_d_name	(sint8 *)("")
#define structfd_posixInvalidFileSize	(sint32)(-1)
#define structfd_posixInvalidFileHandleOffset	(sint32)(-1)	//internal offset currently held by the struct fd which exposes to POSIX API
#define structfd_FileClassListInvalidEntry		(sint32)(-1)	//TGDS FS API -> Directory Iterator (FileClass): Invalid Entry within a FileClassList

//Libfat attributes to add compatibility with it
#ifndef ATTRIB_ARCH
#define ATTRIB_ARCH	(int)(0x20)			// Archive
#define ATTRIB_DIR	(int)(0x10)			// Directory
#define ATTRIB_LFN	(int)(0x0F)			// Long file name
#define ATTRIB_VOL	(int)(0x08)			// Volume
#define ATTRIB_SYS	(int)(0x04)			// System
#define ATTRIB_HID	(int)(0x02)			// Hidden
#define ATTRIB_RO	(int)(0x01)			// Read only
#endif

//Up to N items to be listed by TGDS High level API functions
#define FileClassItems (int)(300)

//Directory Separator used by TGDS FS
#define TGDSDirectorySeparator ((char*)("/"))

//FileClass Start directory if there is none
#define FileClassStartDirectory TGDSDirectorySeparator

//FileClass parts (not used by POSIX at all, but ToolchainGenericDS high level API (for parsing fullpath directories and high level descriptors, and completely unrelated to TGDS FileOrBuffer handles)
struct FileClass{
	int type;	//FT_DIR / FT_FILE / FT_NONE	//  setup on Constructor / updated by getFileFILINFOfromPath(); / must be init from the outside 
	sint8 fd_namefullPath[MAX_TGDSFILENAME_LENGTH+1];
	bool isIterable;	//true = usable for buildfrompath / false = ignore lookup in buildfrompath
	int d_ino;	//if any, assign it here
	int curIndexInsideFileClassList;	//Current Index inside FileClassList
	struct FileClassList * parentFileClassList;	//Pointer to its parent FileClass List object
};

//FileClass List context. To handle TGDS FileSystem + Directory functions you need to define it.
struct FileClassList{
	//Current Working Directory held by the TGDS FS Directory Iterator (of which, is an instance)
	char TGDSCurrentWorkingDirectory[MAX_TGDSFILENAME_LENGTH+1];

	//The actual pointer inside the directory listing
	int CurrentFileDirEntry;	
	//These update on getFirstFile/Dir getNextFile/Dir
	int LastFileEntry;
	int LastDirEntry;
	int FileDirCount;	//actual Directory/File count
	struct FileClass fileList[FileClassItems];
};

struct fd {
	//If file handle: FILE/DIR Context in here
	FIL  * filPtr;	//reuse C ability to parse NULL structs later
	DIR  * dirPtr;
	FIL  fil;	//if aboveptr is NULL then it is not FIL
	DIR  dir;	//if aboveptr is NULL then it is not DIR
	bool fileIsUnicode;	//true: file is Unicode, false file is not (just standard ASCII encoding)
	struct devoptab_t * devoptabFileDescriptor;
	int isatty;
	int isused;					//1 means in use (structfd_isused) / 0 free (structfd_isunused)
	int descriptor_flags;
	int status_flags;
	struct dirent cur_entry;	//dirent POSIX extension used in fopen / fatfs_readdir_r
	sint8 fd_name[MAX_TGDSFILENAME_LENGTH+1];	/* d_name dirent but holds full file path. Only when this is a file */
	
	//If buffer mapped as file handle:
	u8* bufferFD;
	int bufferFDSize;
	
	//Shared
	struct stat stat;			//File attributes : POSIX Compliant
	long loc;		//current file pointer if this struct fd is DIR * (generated by opendir) Note: Only valid if struct fd == FT_FILE. Otherwise it has no effect. 
	int StructFDType; //FT_DIR / FT_FILE / FT_NONE
	int StructFD;	//TGDS File Descriptor (struct fd). Identical as d_ino (POSIX File Descriptor), except, it is valid if the file context is correct from FATFS API.
	//filehandle -> fileSize;	//use f_size(filPtr)
};


#ifdef ARM9
typedef bool(*SoundStreamStopSoundStreamARM9LibUtils_fn)(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType);
typedef void(*SoundStreamUpdateSoundStreamARM9LibUtils_fn)();
#endif

//This should be implemented in newlib, but it's not exposed 
#ifdef ARM9
inline 
#endif
static int charPosixToFlagPosix(char * flags){
	int flagsPosix = 0;
	if(strcmp(flags, "r") == 0){
		flagsPosix |= O_RDONLY;
	}
	
	if(strcmp(flags, "rb") == 0){
		flagsPosix |= O_RDONLY;
	}
	
	if(strcmp(flags, "r+") == 0){
		flagsPosix |= O_RDWR;
	}
	
	if(strcmp(flags, "w") == 0){
		flagsPosix |= (O_CREAT|O_WRONLY);
	}
	
	if(strcmp(flags, "wb") == 0){
		flagsPosix |= (O_CREAT|O_WRONLY);
	}
	
	if(strcmp(flags, "w+") == 0){
		flagsPosix |= (O_CREAT|O_RDWR);
	}
	return flagsPosix;
}

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9
extern SoundStreamStopSoundStreamARM9LibUtils_fn SoundStreamStopSoundStreamARM9LibUtilsCallback;
extern SoundStreamUpdateSoundStreamARM9LibUtils_fn SoundStreamUpdateSoundStreamARM9LibUtilsCallback;

extern void initializeLibUtils9(
	HandleFifoNotEmptyWeakRefLibUtils_fn HandleFifoNotEmptyWeakRefLibUtilsCall, //ARM7 & ARM9
	timerWifiInterruptARM9LibUtils_fn timerWifiInterruptARM9LibUtilsCall, //ARM9 
	SoundSampleContextEnableARM7LibUtils_fn SoundSampleContextEnableARM7LibUtilsCall, // ARM7 & ARM9: void EnableSoundSampleContext(int SndSamplemode)
	SoundSampleContextDisableARM7LibUtils_fn SoundSampleContextDisableARM7LibUtilsCall,	//ARM7 & ARM9: void DisableSoundSampleContext()
	SoundStreamStopSoundStreamARM9LibUtils_fn SoundStreamStopSoundStreamARM9LibUtilsCall,	//ARM9: bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType)
	SoundStreamUpdateSoundStreamARM9LibUtils_fn SoundStreamUpdateSoundStreamARM9LibUtilsCall //ARM9: void updateStream() 
);
#endif

// Function prototypes 
extern FATFS dldiFs;
extern bool FS_InitStatus;

////////////////////////////////////////////////////////////////////////////USER CODE START/////////////////////////////////////////////////////////////////////////////////////
extern int		FS_init();
extern int		FS_deinit();
extern sint8 charbuf[MAX_TGDSFILENAME_LENGTH+1];
extern sint8 * getfatfsPath(sint8 * filename);
extern void getDirFromFilePath(char * filePath, char* outDirectory);
extern int FileExists(char * filename);
extern int rename(const sint8 *oldpathfile, const sint8 *newpathfile);
extern int fsync(int fd);
extern int mkdir(const sint8 *path, mode_t mode);
extern int rmdir(const sint8 *path);
extern int chdir(const sint8 *path);
extern sint8 *getcwd(sint8 *buf, size_t size);
extern DIR *opendir(const sint8 *path);
extern int closedir(DIR *dirp);
extern struct dirent *readdir(DIR *dirp);
extern void rewinddir(DIR *dirp);
extern int dirfd(DIR *dirp);
extern int remove(const char *filename);
#ifdef ARM9
extern int chmod(const char *pathname, mode_t mode);
#endif
extern DIR *fdopendir(int fd);
extern void seekdir(DIR *dirp, long loc);
extern int fatfs2libfatAttrib(int fatfsFlags);
extern int libfat2fatfsAttrib(int libfatFlags);
extern void SetfatfsAttributesToFile(char * filename, int Newgccnewlibnano_to_fatfsAttributes, int mask);

extern DWORD clust2sect (  /* !=0:Sector number, 0:Failed (invalid cluster#) */
    FATFS* fs,      /* File system object */
    DWORD clst      /* Cluster# to be converted */
);

extern uint32 getStructFDFirstCluster(struct fd *fdinst);
extern uint32 getStructFDNextCluster(struct fd *fdinst, int currCluster);
extern uint32 getStructFDStartSectorByCluster(struct fd *fdinst, int ClusterIndex);
extern sint32 getDiskClusterSize();			/* Cluster size [sectors] */
extern sint32 getDiskClusterSizeBytes();	/* Cluster size [sectors] in bytes */
extern sint32 getDiskSectorSize();
extern char * dldi_tryingInterface();

/////////////////////////////////////////Libfat wrapper layer. Call these as if you were calling libfat code./////////////////////////////////////////
extern struct FileClass * FAT_FindFirstFile(char* filename, struct FileClassList * lst, int startFromGivenIndex);
extern struct FileClass * 	FAT_FindNextFile(char* filename, struct FileClassList * lst);
extern bool FAT_feof(FILE * fh);
extern u32 flength(FILE* fh);
extern u8 	FAT_GetFileAttributesFromFileClass(struct FileClass * fileInst);
extern u8 	FAT_GetFileAttributes(struct FileClassList * lst);
extern u8 	FAT_SetFileAttributes(const char* filename, u8 attributes, u8 mask);
extern bool readFileNameFromFileClassIndex(char* filename_out, struct FileClass * FileClassInst);
extern bool FAT_GetAlias(char* alias, struct FileClassList * lst);
extern void FAT_preserveVars();
extern void FAT_restoreVars();
extern u32	disc_HostType(void);
extern bool FAT_GetLongFilename(char* Longfilename, struct FileClassList * lst);
extern u32 	FAT_GetFileSize(struct FileClassList * lst);
extern u32 	FAT_GetFileCluster(struct FileClassList * lst);
extern bool disableWriting;
extern void FAT_DisableWriting (void);
extern int 	FAT_FileExists(char* filename);
extern bool FAT_FreeFiles (void);
extern bool FAT_InitFiles (void);
/////////////////////////////////////////////Libfat wrapper layer End/////////////////////////////////////////////

extern struct FileClass * getFirstFile(char * path, struct FileClassList * lst, int startFromGivenIndex);
extern struct FileClass * getNextFile(char * path, struct FileClassList * lst);
extern int	getCurrentDirectoryCount(struct FileClassList * lst);
extern void	setCurrentDirectoryCount(struct FileClassList * lst, int value);
extern bool	getFileFILINFOfromFileClass(struct FileClass * fileInst, FILINFO * finfo);
extern struct FileClass getFileClassFromPath(char * path);
extern struct FileClass * getFirstDirEntryFromList(struct FileClassList * lst);
extern struct FileClass * getFirstFileEntryFromList(struct FileClassList * lst);
extern struct FileClass * getFileClassFromList(int FileClassListIndex, struct FileClassList * lst);
extern bool setFileClass(bool iterable, char * fullPath, int FileClassListIndex, int Typ, int StructFD, struct FileClassList * lst);
extern bool TGDSFS_detectUnicode(struct fd *pfd);
extern bool readDirectoryIntoFileClass(char * dir, struct FileClassList * thisClassList);
extern int buildFileClassByExtensionFromList(struct FileClassList * inputClassList, struct FileClassList * targetClassList, char ** scratchPadMemory, char * filterString);
extern void sortFileClassListAsc(struct FileClassList * thisClassList, char ** scratchPadMemory, bool ignoreFirstFileClass);
////////////////////////////////////////////////////////////////////////////USER CODE END/////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////INTERNAL CODE START/////////////////////////////////////////////////////////////////////////////////////
extern int fatfs_init();
extern int fatfs_deinit();
extern int fatfs_write (int fd, u8 *ptr, int len);
extern int fatfs_read (int fd, u8 *ptr, int len);
extern int fatfs_close (int fd);
extern long fatfs_ftell(struct fd * fdinst);
extern int fatfs_fsize (int structFDIndex);
extern void fillPosixStatStruct(const FILINFO *fno, struct stat *out);
extern BYTE posixToFatfsAttrib(int flags);
extern int fatfsToPosixAttrib(BYTE flags);
extern int fresultToErrno(FRESULT result);	/* POSIX ERROR Handling */
extern void initStructFDHandle(struct fd *pfd, int flags, const FILINFO *fno, int structFD, int StructFDType);

extern int fatfs_open_file(const sint8 *pathname, int flags, const FILINFO *fno);	//(FRESULT is the file properties that must be copied to stat st)/ returns an internal index struct fd allocated
extern int fatfs_open_dir(const sint8 *pathname, int flags, const FILINFO *fno);	//(FRESULT is the file properties that must be copied to stat st)/ returns an internal index struct fd allocated
extern int fatfs_open_file_or_dir(const sint8 *pathname, int flags);	//returns an internal index struct fd allocated
extern DWORD get_fattime (void);
extern int fatfs_open(const sint8 *pathname, int flags);
extern off_t fatfs_lseek(int fd, off_t offset, int whence);
extern int fatfs_unlink(const sint8 *path);
extern int fatfs_link(const sint8 *path1, const sint8 *path2);
extern int fatfs_rename(const sint8 *oldpathfile, const sint8 *newpathfile);
extern int fatfs_fsync(int fd);
extern int fatfs_stat(const sint8 *path, struct stat *buf);
extern int fatfs_mkdir(const sint8 *path, mode_t mode);
extern int fatfs_rmdir(const sint8 *path);
extern int fatfs_chdir(const sint8 *path);
extern sint8 *fatfs_getcwd(sint8 *buf, size_t size);
extern DIR *fatfs_opendir(const sint8 *path);
extern int fatfs_closedir(DIR *dirp);
extern int fatfs_dirfd(DIR *dirp);
extern DIR *fatfs_fdopendir(int fd);
extern struct dirent *fatfs_readdir(DIR *dirp);
extern int fatfs_readdir_r(DIR *dirp,struct dirent *entry,struct dirent **result);
extern void fatfs_rewinddir(DIR *dirp);
extern long fatfs_tell(struct fd *f);
extern void fatfs_seekdir(DIR *dirp, long loc);
extern int _fstat_r ( struct _reent *_r, int fd, struct stat *buf );
extern int  readdir_r(DIR * dirp,struct dirent * entry,struct dirent ** result);
extern int OpenFileFromPathGetStructFD(char * fullPath);
extern bool closeFileFromStructFD(int StructFD);
extern struct fd * files;	//File/Dir MAX handles: OPEN_MAXTGDS

extern int getStructFDIndexByDIR(DIR *dirp);
extern sint8 * getDeviceNameByStructFDIndex(int StructFDIndex);
extern int FileHandleFree(int fd);
extern int FileHandleAlloc(struct devoptab_t * devoptabInst);
extern int getStructFDIndexByFileName(char * filename);
extern struct fd * getStructFDByFileHandle(FILE * fh);
extern FILE * getPosixFileHandleByStructFD(struct fd * fdinst, const char * mode);
extern void initTGDS(char * devoptabFSName);
extern struct fd *getStructFD(int fd);
extern int	FS_getFileSizeFromOpenStructFD(struct fd * tgdsfd);
extern int	FS_getFileSizeFromOpenHandle(FILE * f);

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS /////////////////////////////////////////////////////////////////////
extern bool buildFileClassListFromPath(char * path, struct FileClassList * lst, int startFromGivenIndex);
extern bool enterDir(char* newDir, char* destinationPath);
extern bool leaveDir(char* oldNewDir);
extern bool updateTGDSFSDirectoryIteratorCWD(char * newCWD, struct FileClassList * lst);
/////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS END //////////////////////////////////////////////////////////////////

///////////////////////////////////////////////TGDS FileDescriptor Callbacks Implementation Start ///////////////////////////////////////////////
//Link C to C++ related dependencies
extern FILE *fdopen(int fd, const char *mode);
extern int fileno(FILE *);
extern bool setFileClassObj(int FileClassListIndex, struct FileClass * FileClassObj, struct FileClassList * lst);

extern int TGDSFSUserfatfs_write(u8 *ptr, int len, int offst, struct fd *tgdsfd);
extern int TGDSFSUserfatfs_read(u8 *ptr, int len, int offst, struct fd *tgdsfd);
extern int TGDSFSUserfatfs_close(struct fd * tgdsfd);
extern int TGDSFSUserfatfs_open_file(const sint8 *pathname, char * posixFlags, int * tgdsfd);
extern off_t TGDSFSUserfatfs_lseek(struct fd *pfd, off_t offset, int whence);
extern long TGDSFSUserfatfs_tell(struct fd *f);

extern bool openDualTGDSFileHandleFromFile(char * filenameToOpen, int * tgdsStructFD1, int * tgdsStructFD2);
extern bool closeDualTGDSFileHandleFromFile(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2);
#if defined(WIN32)
extern int getLastDirFromPath(char * stream, char * haystack, char * outBuf);
#endif

extern int ARM7FS_close_ARM9ImplementationTGDSFD(struct fd * fdinstOut);
extern int ARM7FS_ReadBuffer_ARM9ImplementationTGDSFD(u8 * outBuffer, int fileOffset, struct fd * fdinstIn, int bufferSize);
extern int ARM7FS_WriteBuffer_ARM9ImplementationTGDSFD(u8 * inBuffer, int fileOffset, struct fd * fdinstOut, int bufferSize);
///////////////////////////////////////////////TGDS FileDescriptor Callbacks Implementation End ///////////////////////////////////////////////

//direct structFD
extern int fatfs_open_fileIntoTargetStructFD(const sint8 *pathname, char * posixFlags, int * tgdsfd, struct fd * directStructFD);	//Copies newly alloced struct fd / Creates duplicate filehandles when opening a new file
extern int fatfs_readDirectStructFD(struct fd * pfd, u8 *ptr, int len);
extern int fatfs_closeDirectStructFD(struct fd * pfd);
extern int fatfs_seekDirectStructFD(struct fd * pfd, int offst);
extern int str_split(char * stream, char * haystack, char * outBuf, int itemSize, int blockSize);

extern void separateExtension(char *str, char *ext);
extern struct FileClassList * initFileList();

#ifdef __cplusplus
}
#endif


//return: true if clean success
//false if lst == NULL
#ifdef ARM9
inline 
#endif
static bool cleanFileList(struct FileClassList * lst){
	if(lst != NULL){
		memset((u8*)lst, 0, sizeof(struct FileClassList));
		lst->CurrentFileDirEntry = 0;
		lst->LastDirEntry=structfd_posixInvalidFileDirOrBufferHandle;
		lst->LastFileEntry=structfd_posixInvalidFileDirOrBufferHandle;
		setCurrentDirectoryCount(lst, 0);
		return true;
	}
	return false;
}

#ifdef ARM9
inline 
#endif
static void freeFileList(struct FileClassList * lst){
	if(lst != NULL){
		TGDSARM9Free(lst);
	}
}

//returns: the struct FileClassList * context if success
//if error: returns NULL, which means, operation failed.

#ifdef ARM9
inline 
#endif
static struct FileClassList * pushEntryToFileClassList(bool iterable, char * fullPath, int Typ, int StructFD, struct FileClassList * lst){
	if(lst != NULL){		
		int FileClassListIndex = lst->FileDirCount;
		setFileClass(iterable, fullPath, FileClassListIndex, Typ, StructFD, lst);
		return lst;
	}
	return NULL;
}

#ifdef ARM9
inline
#endif
static struct FileClassList * popEntryfromFileClassList(struct FileClassList * lst){
	if(lst != NULL){		
		int FileClassListIndex = lst->FileDirCount;
		if(FileClassListIndex > 0){
			if(FileClassListIndex < FileClassItems){	//prevent overlapping current FileClassList 
				struct FileClass * FileClassInst = getFileClassFromList(FileClassListIndex, lst);
				lst->FileDirCount = FileClassListIndex - 1;
				memset(FileClassInst, 0, sizeof(struct FileClass));
				return lst;
			}
		}
	}
	return NULL;
}

#ifdef ARM9
inline 
#endif
static bool contains(int * arr, int arrSize, int value){
    int i=0;
	for(i=0; i < arrSize; i++){
        if(arr[i] == value){
            return true;
        }
    }
    return false;
} 

#ifdef ARM9
inline 
#endif
static struct FileClassList * randomizeFileClassList(struct FileClassList * lst){
	if(lst != NULL){
		//Build rand table
		int listCount = lst->FileDirCount;
		int uniqueArr[FileClassItems];
		int i=0;
		int indx=0;
		for(i=0; i < FileClassItems; i++){
            uniqueArr[i] = -1;
        }
		for(;;){
		    int randVal = rand() % (listCount);
		    if (contains(uniqueArr, listCount, randVal) == false){
		        uniqueArr[indx] = randVal;
		        indx++;
		    }
		    if(indx == (listCount)){
		        break;
		    }
		}
		
		//Shuffle it
		for(indx=0; indx < listCount; indx++){
			struct FileClass fileListSrc;
			struct FileClass fileListTarget;
			int targetIndx = uniqueArr[indx];
			int srcIndx = indx;
			memset(&fileListSrc, 0, sizeof(struct FileClass));
			memset(&fileListTarget, 0, sizeof(struct FileClass));
			if(targetIndx != srcIndx){
				struct FileClass * FileClassInstSource = getFileClassFromList(srcIndx, lst);
				struct FileClass * FileClassInstTarget = getFileClassFromList(targetIndx, lst);
				
				memcpy((u8*)&fileListSrc, (u8*)FileClassInstSource, sizeof(struct FileClass));
				memcpy((u8*)&fileListTarget, (u8*)FileClassInstTarget, sizeof(struct FileClass));
				
				memcpy((u8*)FileClassInstSource, (u8*)&fileListTarget, sizeof(struct FileClass));
				memcpy((u8*)FileClassInstTarget, (u8*)&fileListSrc, sizeof(struct FileClass));
			}
		}
		
		//done 
		return lst;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END/////////////////////////////////////////////////////////////////////////////////////

#endif
