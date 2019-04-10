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
#include "utilsTGDS.h" 
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
sint8 charbuf[MAX_TGDSFILENAME_LENGTH+1];
sint8 * getfatfsPath(sint8 * filename){
	sprintf((sint8*)charbuf,"%s%s",devoptab_sdFilesystem.name,filename);
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
        else if ((flags & O_TRUNC) || (flags & O_CREAT))
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


sint32	getStructFDNextCluster(struct fd *fdinst){
	if((fdinst) && (fdinst->filPtr)){
		DWORD clst;
		FIL * fil = fdinst->filPtr; 
		if( (int)fil->fptr == (int)0 ){
			clst = fil->obj.sclust;
		}
		else{
			clst = fil->clust;
		}
		clst = get_fat(&fil->obj, clst);		/* Get next cluster */
		return (sint32)clst;
	}
	return structfd_posixInvalidFileDirHandle;
}

//Cluster count is generated from a file range
bool isStructFDOutOfBoundsCluster(struct fd *fdinst, int curCluster){
	FILE * fh = fdopen(fdinst->cur_entry.d_ino, "r");
	int filePos = ftell(fh);	//save cur FilePos
	fseek(fh, 0, SEEK_END);
	int FileSize = ftell(fh);
	fseek(fh, filePos, SEEK_SET);
	int clustCnt = (int)FileSize/getDiskClusterSizeBytes();
	if(getStructFDNextCluster(fdinst) > clustCnt){
		return true;
	}
	return false;
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
		struct FileClass * fileInst = getFileClassFromList((CurrentFileDirEntry - 1));
		FILINFO finfo; 
		if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){
			libfatAttributes = (uint8)fsfat2libfatAttrib((int)finfo.fattrib);
		}
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
	FILINFO finfo; 
	if(getFileFILINFOfromFileClass(&fileInst, &finfo) == true){	
		libfatAttributesIn = (uint8)fsfat2libfatAttrib((int)finfo.fattrib);
		libfatAttributesOut = (libfatAttributesIn & ~(mask & 0x27)) | (attributes & 0x27);
		int	NEWgccnewlibnano_to_fsfatAttributes = libfat2fsfatAttrib((int)libfatAttributesOut);
		int NEWmask = libfat2fsfatAttrib((int)mask);
		SetfsfatAttributesToFile((char*)filename, NEWgccnewlibnano_to_fsfatAttributes, NEWmask);
	}
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

uint8* FileClassListPtr = NULL;

void setFileClass(bool iterable, char * fullPath, int FileClassListIndex, int Typ, int structFD){
	struct FileClass * FileClassInst = (struct FileClass *)&FileClassListPtr[FileClassListIndex*sizeof(struct FileClass)];
	FileClassInst->isIterable = iterable;
	strcpy(FileClassInst->fd_namefullPath, (const char *)fullPath);
	FileClassInst->type = Typ;
	FileClassInst->d_ino = structFD;
}

struct FileClass * getFileClassFromList(int FileClassListIndex){
	return (struct FileClass *)&FileClassListPtr[FileClassListIndex*sizeof(struct FileClass)];
}

//path can be either Directory or File, a proper FileClass will be returned.
struct FileClass getFileClassFromPath(char * path){
	struct FileClass FileClassOut;
	FileClassOut.type = FT_NONE;
	FILE * fh = fopen(path,"r");
	if(fh != NULL){
		FileClassOut.type = FT_FILE;
		fclose(fh);
	}
	else{
		//try dir
		DIR * dirHandle = opendir((const sint8 *)path);
		if(dirHandle != NULL){
			FileClassOut.type = FT_DIR;
			closedir(dirHandle);
		}
	}
	if((FileClassOut.type == FT_FILE)||(FileClassOut.type == FT_DIR)){
		strcpy(FileClassOut.fd_namefullPath, path);
	}
	else{
		strcpy(FileClassOut.fd_namefullPath, "");
	}
	
	FileClassOut.isIterable = false;	//destroyable FileClass item, does not belong to a list
	FileClassOut.d_ino = structfd_posixInvalidFileDirHandle;	//destroyable FileClass are always invalid filehandles
	return FileClassOut;
}


void InitGlobalFileClass(){
	CurrentFileDirEntry = 0;
	LastDirEntry=structfd_posixInvalidFileDirHandle;
	LastFileEntry=structfd_posixInvalidFileDirHandle;
	int i = 0;
	for(i = 0; i < FileClassItems; i++){
		bool iterable = false;
		setFileClass(iterable, dirent_default_d_name, i, FT_NONE, structfd_posixInvalidFileDirHandle);
	}
}

//requires a previously generated struct fd *
bool getFileFILINFOfromFileClass(struct FileClass * fileInst, FILINFO * finfo){
	if(fileInst){
		FRESULT result = f_stat((const TCHAR*)fileInst->fd_namefullPath, finfo);					/* Get file status */
		if (result == FR_OK)
		{
			//stat ok
			return true;
		}
		else{
			//stat error
		}
	}
	return false;
}

//Note: Requires a fresh call to buildFileClassListFromPath prior to calling this
struct FileClass * getFirstDirEntryFromList(){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClassFromList(i);
		if(fileClassInst->type == FT_DIR){
			FileClassRet = fileClassInst;
			break;
		}
	}
	CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromList() now
	return FileClassRet;
}

//Note: Requires a fresh call to buildFileClassListFromPath prior to calling this
struct FileClass * getFirstFileEntryFromList(){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClassFromList(i);
		if(fileClassInst->type == FT_FILE){
			FileClassRet = fileClassInst;
			break;
		}
	}
	CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromList() now
	return FileClassRet;
}

//The actual pointer inside the directory listing
int CurrentFileDirEntry = 0;	
//These update on getFirstFile/Dir getNextFile/Dir
int LastFileEntry = 0;
int LastDirEntry = 0;
static bool fsfatFileClassInited = false;

//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int getFirstFile(char * path){
	int fType = FT_NONE;	//invalid. Should not happen 
	//Just run once... and run only when using specific fsfatlayerTGDS fileClass functionality
	if(fsfatFileClassInited == false){
		FileClassListPtr = malloc(sizeof(struct FileClass)*FileClassItems);
		fsfatFileClassInited = true;
	}
	//path will have the current working directory the FileClass was built around. getFirstFile builds everything, and getNextFile iterates over each file until there are no more.
	if(buildFileClassListFromPath(path) == true){
		CurrentFileDirEntry = 0;
		
		//struct FileClass * fileInst = getFirstDirEntryFromList();					//get First directory entry	:	so it generates a valid DIR CurrentFileDirEntry
		//struct FileClass * fileInst = getFirstFileEntryFromList();				//get First file entry 		:	so it generates a valid FILE CurrentFileDirEntry
		struct FileClass * fileInst = getFileClassFromList(CurrentFileDirEntry);
		fType = fileInst->type;
		switch(fType){
			//dir
			case(FT_DIR):{
				LastDirEntry=CurrentFileDirEntry;
				char *  FullPathStr = fileInst->fd_namefullPath;
				setLFN((char*)FullPathStr);		//update last full path access
				getLFN((char*)path);			//update source path				
			}
			break;
			//file
			case(FT_FILE):{
				LastFileEntry=CurrentFileDirEntry;
				char *  FullPathStr = fileInst->fd_namefullPath;
				setLFN((char*)FullPathStr);		//update last full path access
				getLFN((char*)path);			//update source path
			}
			break;
		}
		//increase the file/dir counter after operation only if valid entry, otherwise it doesn't anymore
		if(CurrentFileDirEntry < (int)(FileClassItems)){ 
			CurrentFileDirEntry++;	
		}
		else{
			CurrentFileDirEntry = 0;
			LastDirEntry=structfd_posixInvalidFileDirHandle;
			LastFileEntry=structfd_posixInvalidFileDirHandle;
			return FT_NONE;	//End the list regardless, no more room available!
		}
	}
	return fType;
}

//requires fullpath of the CURRENT file, it will return the next one
//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int getNextFile(char * path){
	int fType = FT_NONE;	//invalid. Should not happen 
	struct FileClass * fileInst = getFileClassFromList(CurrentFileDirEntry);
	if(fileInst != NULL){
		fType = fileInst->type;
		switch(fType){
			//dir
			case(FT_DIR):{
				LastDirEntry=CurrentFileDirEntry;
				char *  FullPathStr = fileInst->fd_namefullPath;
				setLFN((char*)FullPathStr);		//update last full path access
				getLFN((char*)path);			//update source path				
			}
			break;
			//file
			case(FT_FILE):{
				LastFileEntry=CurrentFileDirEntry;
				char *  FullPathStr = fileInst->fd_namefullPath;
				setLFN((char*)FullPathStr);		//update last full path access
				getLFN((char*)path);			//update source path
			}
			break;
		}	
		//increase the file counter after operation
		if(CurrentFileDirEntry < (int)(FileClassItems)){ 
			CurrentFileDirEntry++;	
		}
		else{
			CurrentFileDirEntry = 0;
			LastDirEntry=structfd_posixInvalidFileDirHandle;
			LastFileEntry=structfd_posixInvalidFileDirHandle;
			return FT_NONE;	//End the list regardless, no more room available!
		}
	}
	return fType;
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
	
	struct FileClass * fileInst = getFileClassFromList(CurrentFileDirEntry);	//assign a FileClass to the StructFD generated before
	FILINFO finfo;
	if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){			//actually open the file and check attributes (rather than read dir contents)
		if (	 
		(	//file
		(finfo.fattrib & AM_RDO)
		||
		(finfo.fattrib & AM_HID)
		||
		(finfo.fattrib & AM_SYS)
		||
		(finfo.fattrib & AM_DIR)
		||
		(finfo.fattrib & AM_ARC)
		)
		||	//dir
		(finfo.fattrib & AM_DIR)
		)
		{
			strcpy((char*)alias,(const char *)fileInst->fd_namefullPath);	//update source path using short file/directory name				
		}
		//not file or dir
		else{
			return false;
		}
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
	struct FileClass * fileInst = getFileClassFromList(CurEntry);	//assign a FileClass to the StructFD generated before
	FILINFO finfo;
	if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){			//actually open the file and check attributes (rather than read dir contents)
		if (	 
		(	//file
		(finfo.fattrib & AM_RDO)
		||
		(finfo.fattrib & AM_HID)
		||
		(finfo.fattrib & AM_SYS)
		||
		(finfo.fattrib & AM_DIR)
		||
		(finfo.fattrib & AM_ARC)
		)
		||	//dir
		(finfo.fattrib & AM_DIR)
		)
		{
			strcpy ((char*)Longfilename, (const char *)fileInst->fd_namefullPath);	//update source path using Long file/directory name
		}
		//not file or dir
		else{
			return false;
		}
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
	struct FileClass * fileInst = getFileClassFromList(LastFileEntry);	//assign a FileClass to the StructFD generated before
	FILINFO finfo; 
	if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){			//actually open the file and check attributes (rather than read dir contents)	
		if (//file
		(finfo.fattrib & AM_RDO)
		||
		(finfo.fattrib & AM_HID)
		||
		(finfo.fattrib & AM_SYS)
		||
		(finfo.fattrib & AM_DIR)
		||
		(finfo.fattrib & AM_ARC)
		)
		{
			fileSize = (u32)finfo.fsize;
		}
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
	struct FileClass * fileInst = getFileClassFromList(LastFileEntry);	//assign a FileClass to the StructFD generated before
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



/*-----------------------------------------------------------------
FAT_FreeFiles
Closes all open files then resets the CF card.
Call this before exiting back to the GBAMP
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_FreeFiles (void)
{
	char * devoptabFSName = (char*)"0:/";
	initTGDS(devoptabFSName);
	if(FileClassListPtr != NULL){
		free(FileClassListPtr);
	}
	// Return status of card
	return (bool)dldiGetInternal()->isInserted;
}


/*-----------------------------------------------------------------
FAT_InitFiles
Reads the FAT information from the CF card.
You need to call this before reading any files.
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_InitFiles (void)
{
	return true;	//TGDS assumes the card was already inited if you followed the TGDS Standard ARM9 Init code start
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
    int i_fil = FileHandleAlloc((struct devoptab_t *)&devoptab_sdFilesystem);	//returns / allocates a new struct fd index for the devoptab_sdFilesystem object.
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

//returns the file handle that was discarded: and if the file handle was DIR / FILE
struct packedFDRet fatfs_free(struct fd *pfd){
	struct packedFDRet retStatus;
	int i_fil = FileHandleFree(pfd->cur_entry.d_ino);	//returns structfd index that was deallocated
    retStatus.StructFD = i_fil;
	retStatus.type = FT_NONE;
	if (i_fil != structfd_posixInvalidFileDirHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
		if(pfd->filPtr){	//must we clean a FIL?
			pfd->filPtr = NULL;
			retStatus.type = FT_FILE;
		}
		if(pfd->dirPtr){	//must we clean a DIR?
			pfd->dirPtr = NULL;
			retStatus.type = FT_DIR;
		}
		
		//clean filename
		sprintf((char*)&pfd->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
    }
	else{
		//file_free failed
	}
	return retStatus;
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
    if ( (pfd == NULL) || ((pfd->filPtr == NULL) && (pfd->dirPtr == NULL)) || (pfd->isused != (sint32)structfd_isused) || (structFDIndex == structfd_posixInvalidFileDirHandle) ){	//not file/dir? not alloced struct fd? or not valid file handle(two cases)?
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
            ret = 0;
			
			//update d_ino here (POSIX compliant)
			pfd->cur_entry.d_ino = (sint32)dirent_default_d_ino;
			pfd->loc = 0;
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
        if (result == FR_OK){
			FileHandleFree(pfd->cur_entry.d_ino);
            ret = 0;
			pfd->loc = 0;
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

void fillPosixStatStruct(const FILINFO *fno, struct stat *out){
    mode_t mode;
    memset(out, 0, sizeof(struct stat));
    out->st_size = fno->fsize;
    if ((fno->fattrib & AM_MASK) & AM_DIR){
        mode = S_IFDIR;
    }
    else{
        mode = S_IFREG;
    }
    mode |= (S_IRUSR|S_IRGRP|S_IROTH);
    mode |= (S_IXUSR|S_IXGRP|S_IXOTH);
    if (!((fno->fattrib & AM_MASK) & AM_RDO)){
        /* rwxrwxrwx */
        mode |= (S_IWUSR|S_IWGRP|S_IWOTH);
    }
    else{
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
void fill_fd_fil(int fd, FIL *fp, int flags, const FILINFO *fno, char * fullFilePath){	//(FileDescriptor :struct fd index)
    struct fd * fdinst = fd_struct_get(fd);
    initStructFD(fdinst, flags, fno);
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
	initStructFD(fdinst, flags, fno);
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
void initStructFD(struct fd *pfd, int flags, const FILINFO *fno){
    pfd->isatty = 0;
    pfd->status_flags = flags;
    pfd->descriptor_flags = 0;
	pfd->loc = 0;	//internal file/dir offset zero
	fillPosixStatStruct(fno, &pfd->stat);
}

//returns an internal index struct fd allocated
int fatfs_open_file(const sint8 *pathname, int flags, const FILINFO *fno){
	//Lookup if file is already open.
	int structfdIndex = getStructFDByFileName((char*)pathname);
	if(structfdIndex != structfd_posixInvalidFileDirHandle){
		return structfdIndex;
	}
	
	//If not, then allocate a new file handle (struct FD)
	BYTE mode;
	FRESULT result;
	structfdIndex = fatfs_fildir_alloc(structfd_isfile);	//returns / allocates a new struct fd index with FIL structure allocated
	struct fd * fdinst = fd_struct_get(structfdIndex);	//fd_struct_get requires struct fd index
	if (structfdIndex != structfd_posixInvalidFileDirHandle){
		FILINFO fno_after;
        if(flags & O_CREAT){
			result = f_unlink(pathname);
			if (result == FR_OK){
				//file was deleted
			}
			else{
				//file doesn´t exist
			}
		}
		mode = posix2fsfatAttrib(flags);
		result = f_open(fdinst->filPtr, pathname, mode);	/* Opens an existing file. If not exist, creates a new file. */
		if (result == FR_OK){		
			result = f_stat(pathname, &fno_after);
			fno = &fno_after;			
			if (result == FR_OK){
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
    }// failed to allocate a file handle / allocated file handle OK end.
	
	return structfdIndex;
}

//returns an internal index struct fd allocated
int fatfs_open_dir(const sint8 *pathname, int flags, const FILINFO *fno){
    FRESULT result;
    int fdret = fatfs_fildir_alloc(structfd_isdir);	//returns / allocates a new struct fd index with DIR structure allocated
	struct fd * fdinst = fd_struct_get(fdret);
	if (fdinst == NULL){
        result = FR_TOO_MANY_OPEN_FILES;
    }
    else{
		result = f_opendir(fdinst->dirPtr, pathname);
        if (result == FR_OK){
			fill_fd_dir(fdret, fdinst->dirPtr, flags, fno, (char*)pathname);
        }
        else{
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
	
	//dir case // todo: same logic as below file if dir does not exists and must be created
    if ((result == FR_OK) && ((fno.fattrib & AM_MASK) & AM_DIR)){
        structFD = fatfs_open_dir(pathname, flags, &fno);
	}
	else if (
		(result == FR_OK)	//file exists case
		||
		(flags & O_CREAT)	//new file?
	){
        structFD = fatfs_open_file(pathname, flags, &fno);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	}
    else {
		//file/dir does not exist, couldn't create
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
	if (structFDIndex != structfd_posixInvalidFileDirHandle){
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
					offset = (topFile - 1);	//offset starts from 0 so -1 here
				}
				pos = offset;
				validArg = true;
			}
			break;
			//<<SEEK_CUR>>---<[offset]> is relative to the current file position.
			//<[offset]> can meaningfully be either positive or negative.
			case(SEEK_CUR):{
				pos = f_tell(filp);
				pos += offset;
				if((int)pos < 0){
					pos = 0;
				}
				if(pos >= topFile){
					pos = (topFile - 1);	//offset starts from 0 so -1 here
				}
				validArg = true;
			}
			break;
			//<<SEEK_END>>---<[offset]> is relative to the current end of file.
			//<[offset]> can meaningfully be either positive (to increase the size
			//of the file) or negative.
			case(SEEK_END):{
				pos = topFile;				//file end is fileSize
				validArg = true;
			}
			break;	
		}
		if(validArg == true){
			result = f_lseek(filp, pos);
			if (result == FR_OK){
				ret = pos;
				//update stat st here
				
				//update current offset
				pfd->loc = pos;
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
        fillPosixStatStruct(&fno, buf);
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
	if ((f->dirPtr) && (S_ISDIR(f->stat.st_mode))){
		ret = f->loc;
    }
	//file
	else if ((f->filPtr) && (S_ISREG(f->stat.st_mode))){
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

//internal: SD init code: call fatfs_init() to have TGDS Filesystem support (posix file functions fopen/fread/fwrite/fclose/etc working). Always call first.
int fatfs_init(){
	char * devoptabFSName = (char*)"0:/";
	initTGDS(devoptabFSName);
    return (f_mount(&dldiFs, "0:", 1));
}

//internal: SD de-init code: requires to call fatfs_init() at least once before.
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


//true: able to detect if file was unicode or not
//false: couldn't tell if file was unicode or not
bool TGDSFS_detectUnicode(struct fd *pfd){
	if(pfd != NULL){
		FILE * fil = fdopen(pfd->cur_entry.d_ino, "r");
		if(fil != NULL){
			char tStr[4];
			u32 cPos = ftell(fil);
			fseek(fil, 0, SEEK_SET);
			fread((uint8*)&tStr[0], 1, 4, fil);
			if(tStr[0] == 0 && tStr[2] == 0 && tStr[1] != 0 && tStr[3] != 0){	// fairly good chance it's unicode
				pfd->UnicodeFileDetected = true;
			}
			else{
				pfd->UnicodeFileDetected = false;
			}
			fseek(fil, cPos, SEEK_SET);
			return true;
		}
	}
	return false;
}
//these two require a prior call to TGDSFS_detectUnicode()
bool TGDSFS_GetStructFDUnicode(struct fd *pfd){
	if(pfd != NULL){
		return pfd->UnicodeFileDetected;
	}
	return false;
}

void TGDSFS_SetStructFDUnicode(struct fd *pfd, bool unicode){
	if(pfd != NULL){
		pfd->UnicodeFileDetected = unicode;
	}
}

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS /////////////////////////////////////////////////////////////////////

//returns the first free StructFD
bool buildFileClassListFromPath(char * path){

	//decide wether we have a Working directory or not, if valid dir, enter that dir. If not, use the default dir and point to it.
	if(updateFileClassList(path) == true){
		//rebuild filelist
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
				else if((type == FT_FILE) || (type == FT_DIR)){
					char builtFilePath[MAX_TGDSFILENAME_LENGTH+1];
					if(type == FT_DIR){
						sprintf(builtFilePath,"%s%s%s",path,"/",fno.fname);
					}
					else if(type == FT_FILE){
						sprintf(builtFilePath,"%s%s%s",getfatfsPath((sint8*)path),"/",fno.fname);
					}		
					//populate
					bool iterable = true;
					setFileClass(iterable, (char*)&builtFilePath[0], i, type, structfd_posixInvalidFileDirHandle);
					i++;
				}			
			}
			f_closedir(&dir);
		}
		return true;
	}
	
	return false;
}

//internal: used by the current working directory iterator in TGDS Filesystem. Also used by directory functions
char TGDSCurrentWorkingDirectory[MAX_TGDSFILENAME_LENGTH+1];

void setTGDSCurrentWorkingDirectory(char * path){
	strcpy(TGDSCurrentWorkingDirectory, (const char *)path);
}
char * getTGDSCurrentWorkingDirectory(){
	return (char*)&TGDSCurrentWorkingDirectory[0];
}

//Directory Functions
bool enterDir(char* newDir){
	char * CurrentWorkingDirectory = (char*)&TGDSCurrentWorkingDirectory[0];
	//Update dir only when source dir is different
	if(strcmp(CurrentWorkingDirectory, newDir) != 0){
		strcpy(CurrentWorkingDirectory, (const char *)newDir);
		//clrscr();
	}	
	if(chdir((char *)CurrentWorkingDirectory) == 0){
		return true;
	}
	return false;
}

//passes the full working directory, removes the last directory and reloads the current directory context
bool leaveDir(char* newDir){
	char tempnewDir[MAX_TGDSFILENAME_LENGTH+1] = {0};
	char tempnewDiroutPath[MAX_TGDSFILENAME_LENGTH+1] = {0};    //used by splitCustom function as output path buffer
	strcpy(tempnewDir, (const char *)newDir);
    getLastDirFromPath(tempnewDir, TGDSDirectorySeparator, tempnewDiroutPath);
	char * CurrentWorkingDirectory = (char*)&TGDSCurrentWorkingDirectory[0];
	strcpy(CurrentWorkingDirectory, (const char *)tempnewDiroutPath);
	if(chdir((char *)CurrentWorkingDirectory) == 0){
		return true;
	}
	return false;
}

//Current iterator (FileClass from a directory). char * path : If a valid directory is passed, it will be used to populate the FileClassList. Otherwise the default Start Directory is used.
//Note: If any File or Directory is found then char * path will be destroyed by the current iterated File / Directory item.
bool updateFileClassList(char * path){
	char * CurrentWorkingDirectory = (char*)&TGDSCurrentWorkingDirectory[0];
	//Set the base directory if the TGDS Current Working Directory is missing.
	if(strlen(CurrentWorkingDirectory) == 0){
		strcpy(CurrentWorkingDirectory, (const char*)FileClassStartDirectory);
	}
	//Set the Current Working Directory as base directory to the destroyable filename source always.
	strcpy(path, (const char*)CurrentWorkingDirectory);
	return enterDir(path);
}

/////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS END //////////////////////////////////////////////////////////////////
