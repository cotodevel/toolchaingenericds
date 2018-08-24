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

//This has all the legacy C code (or: fsfatlayerTGDSLegacy.c + fsfatlayerTGDSNew.cpp / or: fsfatlayerTGDSLegacy.c). Projects that support C/C++ code link this by default.

#include "fsfatlayerTGDS.h"
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include "limitsTGDS.h"
#include <fcntl.h>
#include <dirent.h>
#include "fileHandleTGDS.h"
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
 
#include "fsfatlayerTGDS.h"
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include "limitsTGDS.h"
#include <fcntl.h>
#include <dirent.h>
#include "fileHandleTGDS.h"
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


//fatfs
FATFS dldiFs;

//LongFilename scratchpad: has the latest fullfilepath accessed
char lfnName[MAX_TGDSFILENAME_LENGTH+1];

//current dir emulation (for start);
char curDirListed[MAX_TGDSFILENAME_LENGTH+1];

//scratchpad struct fd
struct fd fdCur;

/* functions */
bool FS_InitStatus = false;	

////////////////////////////////////////////////////////////////////////////USER CODE START/////////////////////////////////////////////////////////////////////////////////////

//Usercall: For initializing Filesystem:
//if FS_init() init SD equals true: Init success
//else  FS_init() equals false: Could not init the card SD access 
int		FS_init(){
	int ret = fatfs_init();
	if (ret == 0){
		FS_InitStatus = true;
	}
	else{
		FS_InitStatus = false;
	}
	return ret;
}

//Usercall: For de-initializing Filesystem
//if FS_deinit() or sd driver uninitialized (after calling FS_init()) equals true: Deinit sucess
//else  FS_deinit() equals false: Could not deinit/free the card SD access 
int		FS_deinit(){
	int ret = fatfs_deinit();
	if (ret == 0){
		FS_InitStatus = false;
	}
	return ret;
}

//converts a "folder/folder.../file.fil" into a proper filesystem fullpath
volatile sint8 charbuf[MAX_TGDSFILENAME_LENGTH+1];
sint8 * getfatfsPath(sint8 * filename){
	sprintf((sint8*)charbuf,"%s%s",devoptab_fatfs.name,filename);
	return (sint8*)&charbuf[0];
}


//these two work together (usually)
int OpenFileFromPathGetStructFD(char * fullPath){
	FILE* fil = fopen(fullPath,"r");
	if(fil != NULL){
		return fileno(fil);
	}
	return structfd_posixInvalidFileDirHandle;
}
bool closeFileFromStructFD(int StructFD){
	FILE* fh = fdopen(StructFD, "r");
	if(fh != NULL){
		fclose(fh);
		return true;
	}
	return false;
}

//retcode: FT_NONE , FT_DIR or FT_FILE
int FileExists(char * filename){
	int Type = FT_NONE;
	int StructFD = OpenFileFromPathGetStructFD((char *)filename);
	struct fd *pfd = fd_struct_get(StructFD);
	if(pfd != NULL){
		//file?
		if(S_ISREG(pfd->stat.st_mode)){
			Type = FT_FILE;
		}
		//dir?
		else if(S_ISDIR(pfd->stat.st_mode)){
			Type = FT_DIR;
		}
		closeFileFromStructFD(StructFD);
	}
	return Type;
}

int rename(const sint8 *oldfname, const sint8 *newfname){
    return fatfs_rename(oldfname, newfname);
}

int fsync(int fd){	//(FileDescriptor :struct fd index)
    return fatfs_fsync(fd);
}

int mkdir(const sint8 *path, mode_t mode){
    return fatfs_mkdir(path, mode);
}

int rmdir(const sint8 *path){
    return fatfs_rmdir(path);
}

int chdir(const sint8 *path){
    return fatfs_chdir(path);
}

sint8 *getcwd(sint8 *buf, size_t size){
    return fatfs_getcwd(buf, size);
}

DIR *opendir(const sint8 *path){
	return fatfs_opendir(path);
}

int closedir(DIR *dirp){
    return fatfs_closedir(dirp);
}

struct dirent *readdir(DIR *dirp){
    return fatfs_readdir(dirp);
}

void rewinddir(DIR *dirp){
    fatfs_rewinddir(dirp);
}

int dirfd(DIR *dirp){
    return fatfs_dirfd(dirp);
}

int remove(const char *filename){
	return fatfs_unlink((const sint8 *)filename);
}


int chmod(const char *pathname, mode_t mode){
	BYTE fsfatFlags = posix2fsfatAttrib(mode);
	return f_chmod(pathname, fsfatFlags, AM_SYS );	//only care about the system bit (if the file we are changing is SYSTEM)
}

DIR *fdopendir(int fd)	//(FileDescriptor :struct fd index)
{
    return fatfs_fdopendir(fd);
}

void seekdir(DIR *dirp, long loc){
    fatfs_seekdir(dirp, loc);
}

//Input: libfat directory flags
//Output: FILINFO.fattrib 
int libfat2fsfatAttrib(int libfatFlags){
	int fsfatFlags = 0;
	if(libfatFlags & ATTRIB_RO){	/* Read only */
		fsfatFlags|=AM_RDO;
	}
	
	if(libfatFlags & ATTRIB_HID){	/* Hidden */
		fsfatFlags|=AM_HID;
	}
	
	if(libfatFlags & ATTRIB_SYS){	/* System */
		fsfatFlags|=AM_SYS;
	}
	
	if(libfatFlags & ATTRIB_DIR){	/* Directory */
		fsfatFlags|=AM_DIR;
	}
	
	if(libfatFlags & ATTRIB_ARCH){	/* Archive */
		fsfatFlags|=AM_ARC;
	}
	return fsfatFlags;
}

//Input: FILINFO.fattrib 
//Output: libfat directory flags
int fsfat2libfatAttrib(int fsfatFlags){
	int libfatFlags = 0;
	if(fsfatFlags & AM_RDO){	/* Read only */
		libfatFlags|=ATTRIB_RO;
	}
	
	if(fsfatFlags & AM_HID){	/* Hidden */
		libfatFlags|=ATTRIB_HID;
	}
	
	if(fsfatFlags & AM_SYS){	/* System */
		libfatFlags|=ATTRIB_SYS;
	}
	
	if(fsfatFlags & AM_DIR){	/* Directory */
		libfatFlags|=ATTRIB_DIR;
	}
	
	if(fsfatFlags & AM_ARC){	/* Archive */
		libfatFlags|=ATTRIB_ARCH;
	}
	return libfatFlags;
}

void SetfsfatAttributesToFile(char * filename, int Newgccnewlibnano_to_fsfatAttributes, int mask){
	f_chmod(filename, Newgccnewlibnano_to_fsfatAttributes, mask);
}

//posix -> fsfat
BYTE posix2fsfatAttrib(int flags){
    BYTE mode = 0;
    int accmode = flags & O_ACCMODE;
    if ((accmode == O_RDONLY) || (accmode == O_RDWR)){
        mode |= FA_READ;
    }
    if ((accmode == O_WRONLY) || (accmode == O_RDWR)){
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

//fsfat -> posix
int fsfat2posixAttrib(BYTE flags){
    #define FSFAT_O_ACCMODE (FA_READ|FA_WRITE)
	int mode = 0;
    BYTE accmode = flags & FSFAT_O_ACCMODE;
    if (accmode & FA_READ){
        mode |= O_RDONLY;
    }
	if(accmode & FA_OPEN_EXISTING){
		mode |= O_RDWR;
	} 
    if (accmode == FA_CREATE_NEW){
        mode |= O_WRONLY;
        if (!(flags & FA_OPEN_EXISTING)){
            mode |= O_CREAT;
        }
        else if (flags & FA_CREATE_NEW){
            mode |= O_EXCL;
        }
        else if (flags & FA_CREATE_ALWAYS){
            mode |= O_TRUNC;
        }
        else{
            mode |= O_RDONLY;
        }
    }
    return mode;
}

char lastCurrentPath[MAX_TGDSFILENAME_LENGTH];
void updateLastGlobalPath(char * path){
	if(strlen(path) == 0){
		sprintf(path,"%s",getfatfsPath(path));	//logic here should split the file handle, iterate it through devoptabs and give the devoptab name, but this is faster (and defaults to fsfat)
	}
	sprintf(lastCurrentPath,"%s",path);
}

//Get sector from cluster                                             
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
				return structfd_posixInvalidFileDirHandle;
			}
		}
	}
	return structfd_posixInvalidFileDirHandle;
}

//args: int ClusterOffset (1) : int SectorOffset (N). = 1 physical sector in disk. Each sector is getDiskSectorSize() bytes. Cluster 0 + Sector 0 == Begin of FileHandle
//returns structfd_posixInvalidFileDirHandle if : file not open, directory or fsfat error
sint32 getStructFDSectorOffset(struct fd *fdinst,int ClusterOffset,int SectorOffset){	//	struct File Descriptor (FILE * open through fopen() -> then converted to int32 from fileno())
	if(fdinst->filPtr){
		return clust2sect(fdinst->filPtr->obj.fs, fdinst->filPtr->obj.sclust + ClusterOffset) + SectorOffset; 
	}
	return structfd_posixInvalidFileDirHandle;
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















///////////////////////////////////This is the TGDS FS API extension. It emulates libfat FAT_xxx functions.//////////////////////////////////
/////////////// For an example, please refer to https://bitbucket.org/Coto88/toolchaingenericds-template/src , main.cpp file/////////////////



//Filename must be at least MAX_TGDSFILENAME_LENGTH+1 in size
int FAT_FindFirstFile(char* filename){	
	return getFirstFile(filename);
}

//Filename must be at least MAX_TGDSFILENAME_LENGTH+1 in size
int FAT_FindNextFile(char* filename){
	return getNextFile(filename);
}

u8 FAT_GetFileAttributes (void){
	u8	libfatAttributes = 0;
	if(CurrentFileDirEntry > 0){
		struct FileClass * fileInst = getFileClass((CurrentFileDirEntry - 1));
		FILINFO finfo = getFileFILINFOfromFileClass(fileInst);
		libfatAttributes = (uint8)fsfat2libfatAttrib((int)finfo.fattrib);
	}
	return libfatAttributes;
}

u8 FAT_SetFileAttributes (const char* filename, u8 attributes, u8 mask){
	u8	libfatAttributesIn = 0;
	u8	libfatAttributesOut= 0;
	struct FileClass fileInst;
	int sizeToCopy = 0;
	if(strlen(filename) > sizeof(fileInst.fd_namefullPath)){
		sizeToCopy = sizeof(fileInst.fd_namefullPath);
	}
	else{
		sizeToCopy = strlen(filename);
	}
	snprintf(fileInst.fd_namefullPath, sizeToCopy, "%s", filename);
	FILINFO finfo = getFileFILINFOfromFileClass(&fileInst);
	libfatAttributesIn = (uint8)fsfat2libfatAttrib((int)finfo.fattrib);
	libfatAttributesOut = (libfatAttributesIn & ~(mask & 0x27)) | (attributes & 0x27);
	int	NEWgccnewlibnano_to_fsfatAttributes = libfat2fsfatAttrib((int)libfatAttributesOut);
	int NEWmask = libfat2fsfatAttrib((int)mask);
	SetfsfatAttributesToFile((char*)filename, NEWgccnewlibnano_to_fsfatAttributes, NEWmask);
	return libfatAttributesOut;
}

//Internal
//Filename must be at least MAX_TGDSFILENAME_LENGTH+1
bool setLFN(char* filename){
	if (filename == NULL){
		return false;
	}
	strncpy (lfnName, filename, (MAX_TGDSFILENAME_LENGTH+1) - 1);
	lfnName[(MAX_TGDSFILENAME_LENGTH+1) - 1] = '\0';
	return true;	
}

//Filename must be at least MAX_TGDSFILENAME_LENGTH+1
bool getLFN(char* filename){
	if (filename == NULL){
		return false;
	}
	strncpy(filename, lfnName, (MAX_TGDSFILENAME_LENGTH+1) - 1);
	filename[(MAX_TGDSFILENAME_LENGTH+1) - 1] = '\0';
	return true;
}

void buildListFromPath(char * path){
	FRESULT res;
    DIR dir;
	int i = 0;
    FILINFO fno;
	InitGlobalFileClass();
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for(;;){
			res = f_readdir(&dir, &fno);                   /* Read a directory item */
			int type = 0;
			if (fno.fattrib & AM_DIR) {			           /* It is a directory */
				type = FT_DIR;	
            }
			else if (									   /* It is a file */
			(fno.fattrib & AM_RDO)
			||
			(fno.fattrib & AM_HID)
			||
			(fno.fattrib & AM_SYS)
			||
			(fno.fattrib & AM_ARC)
			){
				type = FT_FILE;			
			}
			else{	/* It is Invalid. */
				type = FT_NONE;
			}
			
			if (res != FR_OK || fno.fname[0] == 0){	//error or end of dir
				break;
			}
			else if(i >= FileClassItems){
				break;
			}
			else{
				//open that full path and open a file handle , if it is file(get internal StructFD)
				if((type == FT_FILE) || (type == FT_DIR)){
					char builtFilePath[MAX_TGDSFILENAME_LENGTH+1];
					sprintf(builtFilePath,"%s%s",path,fno.fname);
					//populate
					bool iterable = true;
					setFileClass(iterable, (char*)&builtFilePath[0], i, type, structfd_posixInvalidFileDirHandle);
					i++;
				}
			}
		}
        f_closedir(&dir);
    }
}

volatile struct FileClass FileClassList[FileClassItems];

void setFileClass(bool iterable, char * fullPath, int FileClassListIndex, int Typ, int structFD){
	struct FileClass * FileClassInst = (struct FileClass *)&FileClassList[FileClassListIndex];
	FileClassInst->isIterable = iterable;
	sprintf(FileClassInst->fd_namefullPath,"%s",fullPath);
	FileClassInst->type = Typ;
	FileClassInst->d_ino = structFD;
}

struct FileClass * getFileClass(int FileClassListIndex){
	return (struct FileClass *)&FileClassList[FileClassListIndex];
}

void InitGlobalFileClass(){
	int i = 0;
	for(i = 0; i < FileClassItems; i++){
		bool iterable = false;
		setFileClass(iterable, dirent_default_d_name, i, FT_NONE, structfd_posixInvalidFileDirHandle);
	}
}

//returns the first free StructFD
void updateGlobalListFromPath(char * path){
	//Update last path (destroys the last one)
	updateLastGlobalPath(path);
	buildListFromPath(path);
}

//requires a previously generated struct fd *
FILINFO getFileFILINFOfromFileClass(struct FileClass * fileInst){
	FILINFO finfo;
	FRESULT result;
	if(fileInst){
		result = f_stat((const TCHAR*)fileInst->fd_namefullPath, &finfo);					/* Get file status */
		if (result == FR_OK)
		{
			//printf("getFileFILINFOfromFileClass: stat ok");
			//while(1==1);
		}
		else{
			//printf("getFileFILINFOfromFileClass: stat error");
			//while(1==1);
		}
	}
	return finfo;
}

//Note: Requires a fresh call to updateGlobalListFromPath prior to calling this
struct FileClass * getFirstDirEntryFromGlobalList(){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClass(i);
		if(fileClassInst->type == FT_DIR){
			FileClassRet = fileClassInst;
			break;
		}
	}
	CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromGlobalList() now
	return FileClassRet;
}

//Note: Requires a fresh call to updateGlobalListFromPath prior to calling this
struct FileClass * getFirstFileEntryFromGlobalList(){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClass(i);
		if(fileClassInst->type == FT_FILE){
			FileClassRet = fileClassInst;
			break;
		}
	}
	CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromGlobalList() now
	return FileClassRet;
}

//The actual pointer inside the directory listing
int CurrentFileDirEntry = 0;	
//These update on getFirstFile/Dir getNextFile/Dir
int LastFileEntry = 0;
int LastDirEntry = 0;

//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int getFirstFile(char * path){
	
	//if path is different, rebuild filelist
	if (!(strcmp(lastCurrentPath, path) == 0)){
		updateLastGlobalPath(path);
	}
	
	//lastCurrentPath is globally accesible by all code. But updated only in getFirstFile (getNextFile just retrieves the next ptr file info)
	updateGlobalListFromPath(lastCurrentPath);
	CurrentFileDirEntry = 0;
	
	//struct FileClass * fileInst = getFirstDirEntryFromGlobalList();					//get First directory entry	:	so it generates a valid DIR CurrentFileDirEntry
	//struct FileClass * fileInst = getFirstFileEntryFromGlobalList();					//get First file entry 		:	so it generates a valid FILE CurrentFileDirEntry
	struct FileClass * fileInst = getFileClass(CurrentFileDirEntry);
	if (fileInst->type == FT_DIR) {	//dir
		LastDirEntry=CurrentFileDirEntry;
	}
	else if (fileInst->type == FT_FILE){
		LastFileEntry=CurrentFileDirEntry;
	}
	else{	
		//invalid. Should not happen 
		return FT_NONE;
	}
	//increase the file/dir counter after operation only if valid entry, otherwise it doesn't anymore
	if((fileInst->type == FT_FILE) || (fileInst->type == FT_DIR)){
		char *  FullPathStr = fileInst->fd_namefullPath;
		setLFN((char*)FullPathStr);		//update last full path access
		getLFN((char*)path);			//update source path
	}
	
	//is this index indexable? otherwise cleanup
	if(CurrentFileDirEntry < (int)(FileClassItems)){ 
		CurrentFileDirEntry++;	
	}
	else{
		CurrentFileDirEntry = 0;
		LastDirEntry=structfd_posixInvalidFileDirHandle;
		LastFileEntry=structfd_posixInvalidFileDirHandle;
		return FT_NONE;	//actually end of list
	}
	return fileInst->type;
}

//requires fullpath of the CURRENT file, it will return the next one
//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int getNextFile(char * path){
	struct FileClass * fileInst = getFileClass(CurrentFileDirEntry);
	if(fileInst->type == FT_DIR){
		LastDirEntry=CurrentFileDirEntry;
	}
	else if(fileInst->type == FT_FILE){
		char * FullPathStr = fileInst->fd_namefullPath;	//must return fullPath here (0:/folder0/filename.ext)
		setLFN((char*)FullPathStr);		//update last full path access
		getLFN((char*)path);					//update source path
		LastFileEntry=CurrentFileDirEntry;
	}
	else{	
		//invalid
		return FT_NONE;
	}
	
	//increase the file counter after operation
	if(CurrentFileDirEntry < (int)(FileClassItems)){ 
		CurrentFileDirEntry++;	
	}
	else{
		CurrentFileDirEntry = 0;
		LastDirEntry=structfd_posixInvalidFileDirHandle;
		LastFileEntry=structfd_posixInvalidFileDirHandle;
		return FT_NONE;	//actually end of list
	}
	return fileInst->type;
}

//FAT_GetAlias
//Get the alias (short name) of the last file or directory entry read
//char* alias OUT: will be filled with the alias (short filename),
//	should be at least 13 bytes long
//bool return OUT: return true if successful
bool FAT_GetAlias(char* alias){
	if (alias == NULL){
		return false;
	}
	int CurEntry = structfd_posixInvalidFileDirHandle;
	if(LastFileEntry > LastDirEntry){
		CurEntry = LastFileEntry;
	}
	else{
		CurEntry = LastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == structfd_posixInvalidFileDirHandle){
		return false;
	}
	struct FileClass * fileInst = getFileClass(CurrentFileDirEntry);	//assign a FileClass to the StructFD generated before
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(fileInst);			//actually open the file and check attributes (rather than read dir contents)
	if (	 
	(	//file
	(FILINFOObj.fattrib & AM_RDO)
	||
	(FILINFOObj.fattrib & AM_HID)
	||
	(FILINFOObj.fattrib & AM_SYS)
	||
	(FILINFOObj.fattrib & AM_DIR)
	||
	(FILINFOObj.fattrib & AM_ARC)
	)
	||	//dir
	(FILINFOObj.fattrib & AM_DIR)
	)
	{
		sprintf((char*)alias,"%s",fileInst->fd_namefullPath);					//update source path using short file/directory name
	}
	//not file or dir
	else{
		return false;
	}
	
	return true;
}

//stubbed because what these do is a workaround, described below:
//in TGDS: while listing a dir, create/read/update/delete a new file works
void FAT_preserveVars()
{
}

void FAT_restoreVars()
{
}

u32	disc_HostType(void)
{
	if(FS_InitStatus == true){
		return dldiGet()->ioInterface.ioType;
	}
	return 0;
}

/*-----------------------------------------------------------------
FAT_GetLongFilename
Get the long name of the last file or directory retrived with 
	GetDirEntry. Also works for FindFirstFile and FindNextFile.
	If a long name doesn't exist, it returns the short name
	instead.
char* filename: OUT will be filled with the filename, should be at
	least 256 bytes long
bool return OUT: return true if successful
-----------------------------------------------------------------*/
bool FAT_GetLongFilename(char* Longfilename){
	if (Longfilename == NULL){
		return false;
	}
	int CurEntry = structfd_posixInvalidFileDirHandle;
	if(LastFileEntry > LastDirEntry){
		CurEntry = LastFileEntry;
	}
	else{
		CurEntry = LastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == structfd_posixInvalidFileDirHandle){
		return false;
	}
	struct FileClass * fileInst = getFileClass(CurEntry);	//assign a FileClass to the StructFD generated before
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(fileInst);			//actually open the file and check attributes (rather than read dir contents)
	
	if (	 
	(	//file
	(FILINFOObj.fattrib & AM_RDO)
	||
	(FILINFOObj.fattrib & AM_HID)
	||
	(FILINFOObj.fattrib & AM_SYS)
	||
	(FILINFOObj.fattrib & AM_DIR)
	||
	(FILINFOObj.fattrib & AM_ARC)
	)
	||	//dir
	(FILINFOObj.fattrib & AM_DIR)
	)
	{
		char * FullPathStr = fileInst->fd_namefullPath;	//must store proper filepath	must return fullPath here (0:/folder0/filename.ext)
		sprintf((char*)Longfilename,"%s",FullPathStr);					//update source path using Long file/directory name
	}
	//not file or dir
	else{
		return false;
	}
	
	return true;
}

/*-----------------------------------------------------------------
FAT_GetFileSize
Get the file size of the last file found or openned.
This idea is based on a modification by MoonLight
u32 return OUT: the file size
-----------------------------------------------------------------*/
u32 FAT_GetFileSize(void){
	u32 fileSize = 0;
	struct FileClass * fileInst = getFileClass(LastFileEntry);	//assign a FileClass to the StructFD generated before
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(fileInst);			//actually open the file and check attributes (rather than read dir contents)
	
	if (//file
	(FILINFOObj.fattrib & AM_RDO)
	||
	(FILINFOObj.fattrib & AM_HID)
	||
	(FILINFOObj.fattrib & AM_SYS)
	||
	(FILINFOObj.fattrib & AM_DIR)
	||
	(FILINFOObj.fattrib & AM_ARC)
	)
	{
		fileSize = (u32)FILINFOObj.fsize;
	}
	return 	fileSize;
}

/*-----------------------------------------------------------------
FAT_GetFileCluster
Get the first cluster of the last file found or openned.
u32 return OUT: the file start cluster
-----------------------------------------------------------------*/
u32 FAT_GetFileCluster(void){
	u32 FirstClusterFromLastFileOpen = structfd_posixInvalidFileDirHandle;
	struct FileClass * fileInst = getFileClass(LastFileEntry);	//assign a FileClass to the StructFD generated before
	char * FullPathStr = fileInst->fd_namefullPath;	//must store proper filepath	must return fullPath here (0:/folder0/filename.ext)
	FILE * f = fopen(FullPathStr,"r");
	sint32 fd = structfd_posixInvalidFileDirHandle;
	struct fd * fdinst = NULL;
	if(f){
		fd = fileno(f);
		fdinst = fd_struct_get(fd);
		FirstClusterFromLastFileOpen = (u32)getStructFDFirstCluster(fdinst);
		fclose(f);
	}
	return 	FirstClusterFromLastFileOpen;
}

/*-----------------------------------------------------------------
FAT_DisableWriting
Disables writing to the card at the driver level.
Cannot be re-enabled.
-----------------------------------------------------------------*/
bool disableWriting = false;
void FAT_DisableWriting (void){
	disableWriting = true;
}

/*-----------------------------------------------------------------
FAT_FileExists
Returns the type of file 
char* filename: IN filename of the file to look for
FILE_TYPE return: OUT returns FT_NONE if there is now file with 
	that name, FT_FILE if it is a file and FT_DIR if it is a directory
-----------------------------------------------------------------*/
int FAT_FileExists(char* filename){
	return FileExists(filename);	//assign a StructFD inside
}
///////////////////////////////////////////////////////TGDS FS API extension end. /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////USER CODE END/////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////INTERNAL CODE START///////////////////////////////////////////////////////////////////////////////
int  readdir_r(DIR * dirp,struct dirent * entry,struct dirent ** result)
{
    return fatfs_readdir_r(dirp, entry, result);
}

int fresult2errno(FRESULT result)
{
    int err ;
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

//returns / allocates a new struct fd index with either DIR or FIL structure allocated
int fatfs_fildir_alloc(int isfilordir)
{
    int i_fil = FileHandleAlloc((struct devoptab_t *)&devoptab_fatfs);	//returns / allocates a new struct fd index 
    if (i_fil != structfd_posixInvalidFileDirHandle){
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
	int i_fil = FileHandleFree(pfd->cur_entry.d_ino);	//conversion
    if (i_fil != structfd_posixInvalidFileDirHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
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

int fatfs_write (int fd, sint8 *ptr, int len){	//(FileDescriptor :struct fd index)
    int ret = structfd_posixInvalidFileDirHandle;
    struct fd *pfd = fd_struct_get(fd);
    if (pfd == NULL){	//not file? not alloced struct fd?
		errno = EBADF;
    }
    else if (
	(pfd->filPtr == NULL)	//no FIL descriptor?
	&&
	(pfd->dirPtr == NULL)	//and no DIR descriptor? definitely can't write to this file descriptor
	)
    {
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode))
	{
        FIL *filp;
        FRESULT result;
        UINT written;
        filp = pfd->filPtr;
        if (pfd->status_flags & O_APPEND){
            DWORD size;
            size = f_size(filp);
            result = f_lseek(filp, size);
        }
		else if(filp == NULL){
			result = FR_INVALID_OBJECT;
		}
        else{
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
        }
    }
    else if (S_ISDIR(pfd->stat.st_mode))
	{
		errno = EISDIR;
    }
    else
    {
        errno = EBADF;
    }
    return ret;
}

//read (get struct FD index from FILE * handle)
int fatfs_read(int fd, sint8 *ptr, int len){
    int ret = structfd_posixInvalidFileDirHandle;
    struct fd *pfd = fd_struct_get(fd);
    if ( (pfd == NULL) || (pfd->filPtr == NULL) ){	//not file/dir? not alloced struct fd?
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode)){
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
        }
    }
    else if (S_ISDIR(pfd->stat.st_mode)){
        errno = EISDIR;
	}
    else{
        errno = EBADF;
    }
    return ret;
}

//receives a new struct fd index with either DIR or FIL structure allocated so it can be closed.
//returns 0 if success, 1 if error
int fatfs_close (int structFDIndex)
{
    int ret = structfd_posixInvalidFileDirHandle;
    struct fd * pfd = fd_struct_get(structFDIndex);
    if ( (pfd == NULL) || ((pfd->filPtr == NULL) && (pfd->dirPtr == NULL)) ){	//not file/dir? not alloced struct fd?
		errno = EBADF;
    }
	//File?
    else if (S_ISREG(pfd->stat.st_mode)){
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
        }
    }
	
	//Directory?
    else if (S_ISDIR(pfd->stat.st_mode)){
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
        }
    }
    else
    {
		errno = EBADF;
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
int fatfs_open_file(const sint8 *pathname, int flags, const FILINFO *fno){
    BYTE mode;
	FRESULT result;
	
	//not allocating properly, instead return the fildes allocated and access from there
	int structfdIndex = fatfs_fildir_alloc(structfd_isfile);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	struct fd * fdinst = fd_struct_get(structfdIndex);	//fd_struct_get requires struct fd index
	if ((structfdIndex == structfd_posixInvalidFileDirHandle) || (fdinst == NULL)){
		// file handle invalid
    }
	else{
        FILINFO fno_after;
        mode = posix2fsfatAttrib(flags);
        result = f_open(fdinst->filPtr, pathname, mode);	/* Opens an existing file. If not exist, creates a new file. */
		if (result == FR_OK)
        {
			//create file successfuly before trying to stat it
			if(flags & O_CREAT){
				f_close(fdinst->filPtr);
				result = f_open(fdinst->filPtr, pathname, mode);
			}
			
			result = f_stat(pathname, &fno_after);
			fno = &fno_after;			
			if (result == FR_OK)
			{
				fill_fd_fil(fdinst->cur_entry.d_ino, fdinst->filPtr, flags, fno, (char*)pathname);
			}
			else
			{
				fatfs_free(fdinst);
				structfdIndex = structfd_posixInvalidFileDirHandle;	//file stat was valid but something happened while IO operation, so, invalid.
			}
		}
        else {	//invalid file or O_CREAT wasn't issued
			fatfs_free(fdinst);
			structfdIndex = structfd_posixInvalidFileDirHandle;	//file handle generated, but file open failed, so, invalid.
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
int fatfs_open_file_or_dir(const sint8 *pathname, int flags){
	FILINFO fno;
	int structFD = structfd_posixInvalidFileDirHandle;
	fno.fname[0] = '\0'; /* initialize as invalid */
	FRESULT result = f_stat(pathname, &fno);
	//dir case
    if ((result == FR_OK) && ((fno.fattrib & AM_MASK) & AM_DIR)){
        structFD = fatfs_open_dir(pathname, flags, &fno);
	}
	else if (
		(result == FR_OK)	//file exists case
		||
		(flags & O_CREAT)	//new file?
	)
    {
        structFD = fatfs_open_file(pathname, flags, &fno);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	}
    else {
		//file/dir does not exist, didn't want to create
	}
	
    return structFD;
}

// Use NDS RTC to update timestamps into files when certain operations require it
DWORD get_fattime (void){
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
//if error, returns structfd_posixInvalidFileDirHandle (invalid structFD file handle index)
int fatfs_open(const sint8 *pathname, int flags){
	int structFDIndex = fatfs_open_file_or_dir(pathname, flags);	
	if (structFDIndex == structfd_posixInvalidFileDirHandle){
		//there is error, no struct fd handle (index) was allocated
	}
	else{
		//update d_ino here (POSIX compliant) - through correct (internal struct fd) file handle
		struct fd *pfd = fd_struct_get(structFDIndex);
		pfd->cur_entry.d_ino = structFDIndex;
	}
	return structFDIndex;
}
// ret: structfd_posixInvalidFileDirHandle if invalid posixFDStruct
//		else if POSIX retcodes (if an error happened)
//		else return new offset position (offset + current file position (internal)) in file handle
off_t fatfs_lseek(int fd, off_t offset, int whence){	//(FileDescriptor :struct fd index)
    off_t ret = (off_t)structfd_posixInvalidFileDirHandle;
    struct fd *pfd = fd_struct_get(fd);
    if (pfd == NULL){
		errno = EBADF;
    }
    else if ((pfd->isused == structfd_isunused)||(!pfd->filPtr)){
		errno = EBADF;
    }
	else if (S_ISSOCK(pfd->stat.st_mode)){	//socket? cant use
		ret = EPIPE;
	}
    else if (S_ISREG(pfd->stat.st_mode)){	//only file allows this
        FIL * filp = pfd->filPtr;
        FRESULT result;
        DWORD pos = 0;
        int topFile = f_size(filp);
		bool validArg = false;
		switch(whence&0x3){
			//<<SEEK_SET>>---<[offset]> is the absolute file position (an offset
			//from the beginning of the file) desired.  <[offset]> must be positive.
			case(SEEK_SET):{
				if(offset < 0){
					offset = 0;
				}
				if(offset >= topFile){
					offset = (topFile - 1);
				}
				validArg = true;
			}
			break;
			//<<SEEK_CUR>>---<[offset]> is relative to the current file position.
			//<[offset]> can meaningfully be either positive or negative.
			case(SEEK_CUR):{
				pos = f_tell(filp);
				validArg = true;
			}
			break;
			//<<SEEK_END>>---<[offset]> is relative to the current end of file.
			//<[offset]> can meaningfully be either positive (to increase the size
			//of the file) or negative.
			case(SEEK_END):{
				pos = topFile;
				validArg = true;
			}
			break;	
		}
		if(validArg == true){
			pos += offset;
			result = f_lseek(filp, pos);
			if (result == FR_OK){
				ret = pos;
			}
			else{
				errno = fresult2errno(result);
			}
		}
	}
    else{
        errno = EINVAL;
    }
    return ret;
}

//delete / remove
int fatfs_unlink(const sint8 *path){
    int ret = structfd_posixInvalidFileDirHandle;
    FRESULT result;
    result = f_unlink(path);
    if (result == FR_OK){
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
    }
    return ret;
}

int fatfs_link(const sint8 *path1, const sint8 *path2){
    int ret = structfd_posixInvalidFileDirHandle;
    // FAT does not support links
    errno = EMLINK; /* maximum number of "links" is 1: the file itself */
    return ret;
}

int fatfs_rename(const sint8 *oldfname, const sint8 * newfname){
    int ret = structfd_posixInvalidFileDirHandle;
    FRESULT result;
    result = f_rename(oldfname, newfname);
    if (result != FR_OK){
        errno = fresult2errno(result);
    }
    else{
        ret = 0;
    }
    return ret;
}

int fatfs_fsync(int fd){	//uses struct fd indexing
    int ret = structfd_posixInvalidFileDirHandle;
    struct fd * pfd = fd_struct_get(fd);
    if (pfd == NULL){
        errno = EBADF;
    }
    else if (pfd->filPtr == NULL){
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode)){
        FIL *filp = pfd->filPtr;
        FRESULT result = f_sync(filp);
        if (result == FR_OK){
            ret = 0;
        }
        else{
            errno = fresult2errno(result);
        }
    }
    else{
        errno = EINVAL;
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirHandle / struct stat * is empty or invalid
int fatfs_stat(const sint8 *path, struct stat *buf){
    int ret = structfd_posixInvalidFileDirHandle;
    FILINFO fno;
	FRESULT result = f_stat(path, &fno);
    if ((result == FR_OK) && (buf != NULL)){
        fill_stat(&fno, buf);
        ret = 0;
    }
    else{
        errno = fresult2errno(result);
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirHandle if path exists or failed due to other IO reason
int fatfs_mkdir(const sint8 *path, mode_t mode){
    int ret = structfd_posixInvalidFileDirHandle;
    FRESULT result = f_mkdir(path);
    if (result == FR_OK){
        ret = 0;
    }
    else{
        errno = fresult2errno(result);
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirHandle if readonly or not empty, or failed due to other IO reason
int fatfs_rmdir(const sint8 *path){
    int ret = structfd_posixInvalidFileDirHandle;
    FRESULT result = f_unlink(path);
    if (result == FR_OK){
        ret = 0;
    }
    else if (result == FR_DENIED){
        FILINFO fno;
        // the dir was readonly or not empty: check 
        result = f_stat(path, &fno);
        if (result == FR_OK){
            if ((fno.fattrib & AM_MASK) & AM_RDO){
                errno = EACCES;
            }
            else{
                errno = ENOTEMPTY;
            }
        }
        else{
            errno = fresult2errno(result);
        }
    }
    else{
        errno = fresult2errno(result);
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirHandle if incorrect path, or failed due to other IO reason
int fatfs_chdir(const sint8 *path){
    int ret = structfd_posixInvalidFileDirHandle;
    FRESULT result = f_chdir(path);
    if (result == FR_OK){
        ret = 0;
    }
    else{
        errno = fresult2errno(result);   
    }
    return ret;
}

//if ok: return char * cwf path
//else if error return NULL if path is incorrect, or failed due to other IO reason
sint8 *fatfs_getcwd(sint8 *buf, size_t size){
	sint8 *ret = NULL;
    FRESULT result = f_getcwd(buf, size);
    if (result == FR_OK){
        ret = buf;
    }
    else{
        errno = fresult2errno(result);
    }
    return ret;
}

//if correct path: returns: iterable DIR * entry
//else returns NULL
DIR *fatfs_opendir(const sint8 *path){
    DIR *ret = NULL;
    int retindex = fatfs_open(path, O_RDONLY);	//returns an internal index struct fd allocated
	if (retindex != structfd_posixInvalidFileDirHandle){
		struct fd *pfd = fd_struct_get(retindex);
        if (pfd != NULL){
            ret = pfd->dirPtr;
        }
        else{
			errno = EBADF;
        }
    }
    //else if: Invalid Path
    return ret;
}

//if the iterable DIR * was closed, return 0
//else returns structfd_posixInvalidFileDirHandle if incorrect iterable DIR *
int fatfs_closedir(DIR *dirp){
	//we need a conversion from DIR * to struct fd *
	int fd = getInternalFileDescriptorFromDIR(dirp);
	return fatfs_close(fd);	//requires a struct fd(file descriptor), returns 0 if success, structfd_posixInvalidFileDirHandle if error
}

//if iterable DIR * is directory, return 0
//else return structfd_posixInvalidFileDirHandle (not valid iterable DIR * entry or not directory )
int fatfs_dirfd(DIR *dirp){
    int ret = structfd_posixInvalidFileDirHandle;
	//we need a conversion from DIR * to struct fd *
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * pfd = fd_struct_get(fd);
    if ((pfd != NULL) && (S_ISDIR(pfd->stat.st_mode))){
		ret = 0;
    }
    else{
        errno = EINVAL;
    }
    return ret;
}

//if iterable DIR * is in the current StructFD index, return iterable DIR *
//else return NULL (not valid iterable DIR * entry or not directory )
DIR *fatfs_fdopendir(int fd){	//(FileDescriptor :struct fd index)
    DIR *ret = NULL;
    struct fd *pfd = fd_struct_get(fd);
    if (pfd == NULL){
        errno = EBADF;
    }
    else if (S_ISDIR(pfd->stat.st_mode)){
        ret = pfd->dirPtr;
    }
    else{
        errno = ENOTDIR;
    }
    return ret;
}

//if iterable DIR * has a struct dirent *(StructFD index, being an object), return such struct dirent * (internal fsfat file handle descriptor)
//else return NULL (not valid iterable DIR * entry or not directory )
struct dirent *fatfs_readdir(DIR *dirp){
    struct dirent *ret = NULL;
    int fd = getInternalFileDescriptorFromDIR(dirp);	//we need a conversion from DIR * to struct fd *
	struct fd * fdinst = fd_struct_get(fd);
	if(fatfs_readdir_r(dirp, (struct dirent *)&fdinst->cur_entry, &ret) != structfd_posixInvalidFileDirHandle){
		return ret;
	}
	else{
        errno = EBADF;
	}
	//ret dirent * with read file/dir properties, namely: d_name
    return ret;
}

//if return equals 0 : means the struct dirent * (internal read file / dir) coming from DIR * was correctly parsed. 
//else if return equals structfd_posixInvalidFileDirHandle: the DIR * could not parse the output struct dirent * (internal read file / dir), being NULL.
int fatfs_readdir_r(
        DIR *dirp,
        struct dirent *entry,	//current dirent of file handle struct fd
        struct dirent **result){	//pointer to rewrite above file handle struct fd

    int ret = structfd_posixInvalidFileDirHandle;
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
	if(fdinst != NULL){
		FRESULT fresult;
		FILINFO fno;
		fresult = f_readdir(dirp, &fno);
		if (fresult != FR_OK){
			errno = fresult2errno(fresult);
			*result = NULL;
		}
		// end of entries
		else if (fno.fname[0] == '\0'){
			ret = 0;
			*result = NULL;
		}
		else{
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
    }
	return ret;
}

void fatfs_rewinddir(DIR *dirp){
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
	FRESULT result = f_readdir(dirp, NULL);
	if (result == FR_OK){
		fdinst->loc	= 0;
	}
	else if (S_ISDIR(fdinst->stat.st_mode)){
		//Not Directory!	/* POSIX says no errors are defined */
    }
	else{
		//Error handling the request.	/* POSIX says no errors are defined */
	}
}

long fatfs_tell(struct fd *f){	//NULL check already outside
    long ret = structfd_posixInvalidFileDirHandle;
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
    return ret;
}

void fatfs_seekdir(DIR *dirp, long loc){
	int fd = getInternalFileDescriptorFromDIR(dirp);
	struct fd * fdinst = fd_struct_get(fd);
    if (S_ISDIR(fdinst->stat.st_mode)){
        long cur_loc = fatfs_tell(fdinst);
        if (loc < cur_loc){
            fatfs_rewinddir(dirp);
        }
        while(loc > cur_loc){
            int ret = structfd_posixInvalidFileDirHandle;
            struct dirent entry;
            struct dirent *result = NULL;
            ret = fatfs_readdir_r(dirp, &entry, &result);
            if ((result == NULL) || (ret != 0))
            {
                /* POSIX says no errors are defined */
                break;
            }
            cur_loc = fatfs_tell(fdinst);
        }
    }
    else{
        /* POSIX says no errors are defined */
    }
}

int fatfs_init(){
    return (f_mount(&dldiFs, "0:", 1));
}

int fatfs_deinit(){
	return (f_unmount("0:"));
}

//this copies stat from internal struct fd to external code
//if StructFD (fd) index is created by fatfs_open (either file / dir), return 0 and the such struct stat * is updated.
//else return structfd_posixInvalidFileDirHandle (not valid struct stat * entry, being NULL)
int _fstat_r( struct _reent *_r, int fd, struct stat *buf ){	//(FileDescriptor :struct fd index)
    int ret = structfd_posixInvalidFileDirHandle;
    struct fd * f = fd_struct_get(fd);
    if (f == NULL){
        _r->_errno = EBADF;
    }
    else if (f->isused == structfd_isunused){
        _r->_errno = EBADF;
    }
    else{
        *buf = f->stat;
        ret = 0;
    }
    return ret;
}

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END///////////////////////////////////////////////////////////////////////////////
