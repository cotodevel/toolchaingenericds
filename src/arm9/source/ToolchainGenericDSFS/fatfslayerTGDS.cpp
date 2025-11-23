#if (1==1) //only WIN32 and NDS has TGDS FS DLDI support for now

#if defined(_MSC_VER)
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#endif
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
//newlib libc ARM Toolchain. <dirent.h> implementation is platform-specific, thus, implemented for ToolchainGenericDS.

#include "fatfslayerTGDS.h"

#ifdef ARM9
#include "ff.h"
#include "dldi.h"
#include "clockTGDS.h"
#include "typedefsTGDS.h"
#include "exceptionTGDS.h"
#endif

#if defined(WIN32)
#include "TGDSTypes.h"
#endif
#include "dldiWin32.h"
#include "ff.h"
#include "devoptab_devices.h"

//fatfs
FATFS dldiFs;

/* functions */
bool FS_InitStatus = false;	

// Physical struct fd file handles
struct fd files[OPEN_MAXTGDS];	//File/Dir MAX handles: 

////////////////////////////////////////////////////////////////////////////USER CODE START/////////////////////////////////////////////////////////////////////////////////////

//Usercall: For initializing Filesystem:
//FS_init() returns true: SD card initialization success 
//FS_init() returns true: SD card initialization failure
int		FS_init(){
	char * devoptabFSName = (char*)"0:/";
	initTGDS(devoptabFSName);
	FRESULT ret = (f_mount(&dldiFs, (const TCHAR*)"0:", 1));
	if(ret != FR_OK){	//FRESULT: FR_OK == 0
		//Throw exception always
		u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
		int TGDSDebuggerStage = 10;
		sprintf((char*)ConsolePrintfBuf, "ARM9: FS_init(): f_mount(): failed. (%d)", (u32)ret);
		handleDSInitOutputMessage((char*)ConsolePrintfBuf);
		handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
	}
    return (int)ret; 
}

//Usercall: For de-initializing Filesystem
//if FS_deinit() or sd driver uninitialized (after calling FS_init()) equals true: Deinit sucess
//else  FS_deinit() equals false: Could not deinit/free the card SD access 
int		FS_deinit(){
	int fd = 0;
	/* search in all struct fd instances, close file handle if open*/
	for (fd = 0; fd < OPEN_MAXTGDS; fd++){	
		fatfs_close(fd);
	}
	int ret = f_unmount((const TCHAR*)"0:");
	//dldi_handler_deinit(); //can't because disables SD card permanently 
	if (ret == 0){
		FS_InitStatus = false;
	}
	return ret;
}

//converts a "folder/folder.../file.fil" into a proper filesystem fullpath
sint8 charbuf[MAX_TGDSFILENAME_LENGTH+1];
sint8 * getfatfsPath(sint8 * filename){
	if( isNotNullString((const char *)filename) == true ){
		sprintf((sint8*)charbuf,"%s%s",devoptab_sdFilesystem.name,filename);
		return (sint8*)&charbuf[0];
	}
	else{
		return NULL;
	}
}

//Extracts the Current Working Directory out of a full FatFS filepath: 
//char * filepath = "0://dir1/dir2/dir3/somefile.ext";
//char outDir[256+1] = {0};
//getDirFromFilePath(filepath, (char*)&outDir[0]);
//printf("getDirFromFilePath():%s", outDir);

void getDirFromFilePath(char * filePath, char* outDirectory){
	if( isNotNullString((const char *)filePath) == true ){
		char tempDir[256+1] = {0};
		strcpy(tempDir, filePath);
		int offset = 0;
		int len = strlen(tempDir)+1;
		//Remove leading "0:/"
		char chr = tempDir[offset];
		while(
			(chr == '0')
			||
			(chr == ':')
			||
			(chr == '/')
		)
		{
			offset++;
			if(offset < (len-1)){
				chr = tempDir[offset];
			}
		}
		strncpy(outDirectory, (char*)&tempDir[offset], len - offset);
		
		//Remove last filename and extension
		chr = outDirectory[len];
		
		//int ofst = 0;
		while(
			(chr != '/')
		)
		{
			outDirectory[len] = '\0';
			len--;
			chr = outDirectory[len];
		}
	}
}

//these two work together (usually)
int OpenFileFromPathGetStructFD(char * fullPath){
	if( isNotNullString((const char *)fullPath) == true ){
		FILE* fil = fopen(fullPath,"r");
		if(fil != NULL){
			return fileno(fil);
		}
		return structfd_posixInvalidFileDirOrBufferHandle;
	}
	else{
		return structfd_posixInvalidFileDirOrBufferHandle;
	}
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
	if( isNotNullString((const char *)filename) == true ){
		int Type = FT_NONE;
		FILINFO fno;
		f_stat((const TCHAR*)filename, &fno);
		if ((fno.fattrib & AM_MASK) & AM_DIR) {
			Type = FT_DIR;
		}
		else if ((fno.fattrib & AM_MASK) & AM_ARC) {
			Type = FT_FILE;
		}
		return Type;
	}
	else{
		return FT_NONE;
	}
}

int rename(const sint8 *oldfname, const sint8 *newfname){
    return fatfs_rename(oldfname, newfname);
}

int fsync(int structFDIndex){	//(FileDescriptor :struct fd index)
    return fatfs_fsync(structFDIndex);
}

#if defined(WIN32) || defined(ARM9)
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

#endif
void rewinddir(DIR *dirp){
    fatfs_rewinddir(dirp);
}

int dirfd(DIR *dirp){
    return fatfs_dirfd(dirp);
}

#ifdef ARM9 //only implement remove in NDS otherwise we can't use remove() from WIN32 DLLs
int remove(const char *filename){
	return fatfs_unlink((const sint8 *)filename);
}
#endif

int chmod(const char *pathname, mode_t mode){
	BYTE fatfsFlags = posixToFatfsAttrib(mode);
	return f_chmod((const TCHAR*)pathname, fatfsFlags, AM_SYS );	//only care about the system bit (if the file we are changing is SYSTEM)
}

DIR *fdopendir(int structFDIndex){	//(FileDescriptor :struct fd index)
    return fatfs_fdopendir(structFDIndex);
}

void seekdir(DIR *dirp, long loc){
    fatfs_seekdir(dirp, loc);
}

//Input: libfat directory flags
//Output: FILINFO.fattrib 
int libfat2fatfsAttrib(int libfatFlags){
	int fatfsFlags = 0;
	if(libfatFlags & ATTRIB_RO){	/* Read only */
		fatfsFlags|=AM_RDO;
	}
	if(libfatFlags & ATTRIB_HID){	/* Hidden */
		fatfsFlags|=AM_HID;
	}	
	if(libfatFlags & ATTRIB_SYS){	/* System */
		fatfsFlags|=AM_SYS;
	}
	if(libfatFlags & ATTRIB_DIR){	/* Directory */
		fatfsFlags|=AM_DIR;
	}
	if(libfatFlags & ATTRIB_ARCH){	/* Archive */
		fatfsFlags|=AM_ARC;
	}
	return fatfsFlags;
}

//Input: FILINFO.fattrib 
//Output: libfat directory flags
int fatfs2libfatAttrib(int fatfsFlags){
	int libfatFlags = 0;
	if(fatfsFlags & AM_RDO){	/* Read only */
		libfatFlags|=ATTRIB_RO;
	}
	if(fatfsFlags & AM_HID){	/* Hidden */
		libfatFlags|=ATTRIB_HID;
	}
	if(fatfsFlags & AM_SYS){	/* System */
		libfatFlags|=ATTRIB_SYS;
	}
	if(fatfsFlags & AM_DIR){	/* Directory */
		libfatFlags|=ATTRIB_DIR;
	}
	if(fatfsFlags & AM_ARC){	/* Archive */
		libfatFlags|=ATTRIB_ARCH;
	}
	return libfatFlags;
}

void SetfatfsAttributesToFile(char * filename, int Newgccnewlibnano_to_fatfsAttributes, int mask){
	f_chmod((const TCHAR*)filename, Newgccnewlibnano_to_fatfsAttributes, mask);
}

//posix -> fatfs
BYTE posixToFatfsAttrib(int flags){
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

//fatfs -> posix
int fatfsToPosixAttrib(BYTE flags){
    #define fatfs_O_ACCMODE (FA_READ|FA_WRITE)
	int mode = 0;
    BYTE accmode = flags & fatfs_O_ACCMODE;
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

/* POSIX ERROR Handling */
int fresultToErrno(FRESULT result){
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
uint32	getStructFDFirstCluster(struct fd *fdinst){
	if(fdinst->filPtr){
		if( (int)fdinst->filPtr->fptr == (int)0 ){
			return (int)(fdinst->filPtr->obj.sclust);
		}
		else{
			return (int)(fdinst->filPtr->clust);
		}
	}
	return (uint32)structfd_posixInvalidFileDirOrBufferHandle;
}

uint32	getStructFDNextCluster(struct fd *fdinst, int currCluster){
	if((fdinst != NULL) && (fdinst->filPtr)){
		DWORD clst = getStructFDFirstCluster(fdinst);
		FIL * fil = fdinst->filPtr;
		FATFS *fs = fil->obj.fs;
		clst = get_fat(&fil->obj, (DWORD)((int)clst + (int)currCluster) );		/* Get next cluster */
		if ( (clst == 0xFFFFFFFF) || (clst < 2) || (clst >= fs->n_fatent) ){ 	/* Disk error , or  Reached to end of table or internal error */
			return (uint32)structfd_posixInvalidFileDirOrBufferHandle;	
		}
		else{
			return (sint32)clst;
		}
	}
	return (uint32)structfd_posixInvalidFileDirOrBufferHandle;
}

//Returns the First sector pointed at ClusterIndex, or, structfd_posixInvalidFileDirOrBufferHandle if fails.
uint32 getStructFDStartSectorByCluster(struct fd *fdinst, int ClusterIndex){	//	struct File Descriptor (FILE * open through fopen() -> then converted to int32 from fileno())
	if(fdinst->filPtr != NULL){
		return (uint32)(clust2sect(fdinst->filPtr->obj.fs, getStructFDFirstCluster(fdinst) + ClusterIndex));
	}
	return structfd_posixInvalidFileDirOrBufferHandle;
}

sint32 getDiskClusterSize(){		/* Sectors per Cluster */
	return(sint32)(dldiFs.csize);
}

sint32 getDiskClusterSizeBytes(){	/* Sectors per Cluster in bytes */
	return (sint32)(getDiskClusterSize() * getDiskSectorSize());
}

sint32 getDiskSectorSize(){
	int diskSectorSize = 0;
	if(FF_MAX_SS == FF_MIN_SS){
		diskSectorSize = (int)FF_MIN_SS;
	}
	else{
		#if (FF_MAX_SS != FF_MIN_SS)
		diskSectorSize = dldiFs.ssize;	//when fatfs sectorsize is variable, by default its 512
		#endif
	}
	return diskSectorSize;
}

char * dldi_tryingInterface(){
	//DS DLDI
	struct  DLDI_INTERFACE* dldiInterface = (struct  DLDI_INTERFACE*)dldiGet();	//ARM7DLDI / ARM9DLDI must read from DLDI section
	return (char *)&dldiInterface->friendlyName[0];
}


///////////////////////////////////This is the TGDS FS API extension. It's a wrapper for libfat FAT_xxx functions.//////////////////////////////////

/////////////////////////////////////////Libfat wrapper layer. Call these as if you were calling libfat code./////////////////////////////////////////

//Filename must be at least MAX_TGDSFILENAME_LENGTH+1 in size
struct FileClass * FAT_FindFirstFile(char* filename, struct FileClassList * lst, int startFromGivenIndex){	
	return getFirstFile(filename, lst, startFromGivenIndex);
}

//Filename must be at least MAX_TGDSFILENAME_LENGTH+1 in size
struct FileClass * FAT_FindNextFile(char* filename, struct FileClassList * lst){
	return getNextFile(filename, lst);
}

u8 FAT_GetFileAttributesFromFileClass(struct FileClass * fileInst){
	u8	libfatAttributes = 0;
	FILINFO finfo; 
	if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){
		libfatAttributes = (uint8)fatfs2libfatAttrib((int)finfo.fattrib);
	}
	return libfatAttributes;
}

u8 FAT_GetFileAttributes(struct FileClassList * lst){
	u8	libfatAttributes = 0;
	int curFileDirEntry = lst->CurrentFileDirEntry;
	if(curFileDirEntry > 0){
		struct FileClass * fileInst = getFileClassFromList((curFileDirEntry - 1), lst);
		FILINFO finfo; 
		if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){
			libfatAttributes = (uint8)fatfs2libfatAttrib((int)finfo.fattrib);
		}
	}
	return libfatAttributes;
}

u8 FAT_SetFileAttributes (const char* filename, u8 attributes, u8 mask){
	u8	libfatAttributesIn = 0;
	u8	libfatAttributesOut= 0;
	FILINFO finfo;
	struct FileClass fileInst;
	sprintf(fileInst.fd_namefullPath, "%s", filename);
	if(getFileFILINFOfromFileClass(&fileInst, &finfo) == true){	
		libfatAttributesIn = (uint8)fatfs2libfatAttrib((int)finfo.fattrib);
		libfatAttributesOut = (libfatAttributesIn & ~(mask & 0x27)) | (attributes & 0x27);
		int	NEWgccnewlibnano_to_fatfsAttributes = libfat2fatfsAttrib((int)libfatAttributesOut);
		int NEWmask = libfat2fatfsAttrib((int)mask);
		SetfatfsAttributesToFile((char*)filename, NEWgccnewlibnano_to_fatfsAttributes, NEWmask);
	}
	return libfatAttributesOut;
}

//Internal

//FAT_GetAlias
//Get the alias (short name) of the last file or directory entry read
//char* alias OUT: will be filled with the alias (short filename),
//	should be at least 13 bytes long
//bool return OUT: return true if successful
bool FAT_GetAlias(char* alias, struct FileClassList * lst){
	if (alias == NULL){
		return false;
	}
	int CurEntry = structfd_posixInvalidFileDirOrBufferHandle;
	
	int lastFileEntry = lst->LastFileEntry;
	int lastDirEntry = lst->LastDirEntry;
	
	if(lastFileEntry > lastDirEntry){
		CurEntry = lastFileEntry;
	}
	else{
		CurEntry = lastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == structfd_posixInvalidFileDirOrBufferHandle){
		return false;
	}
	struct FileClass * fileInst = getFileClassFromList(lst->CurrentFileDirEntry, lst);	//assign a FileClass to the StructFD generated before
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
void FAT_preserveVars(){
}

void FAT_restoreVars(){
}

u32	disc_HostType(void){
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
bool FAT_GetLongFilename(char* Longfilename, struct FileClassList * lst){
	FILINFO finfo;
	if (Longfilename == NULL){
		return false;
	}
	int CurEntry = structfd_posixInvalidFileDirOrBufferHandle;
	if(lst->LastFileEntry > lst->LastDirEntry){
		CurEntry = lst->LastFileEntry;
	}
	else{
		CurEntry = lst->LastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == structfd_posixInvalidFileDirOrBufferHandle){
		return false;
	}
	struct FileClass * fileInst = getFileClassFromList(CurEntry, lst);	//assign a FileClass to the StructFD generated before
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
u32 FAT_GetFileSize(struct FileClassList * lst){
	u32 fileSize = 0;
	struct FileClass * fileInst = getFileClassFromList(lst->LastFileEntry, lst);	//assign a FileClass to the StructFD generated before
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
u32 FAT_GetFileCluster(struct FileClassList * lst){
	u32 FirstClusterFromLastFileOpen = structfd_posixInvalidFileDirOrBufferHandle;
	struct FileClass * fileInst = getFileClassFromList(lst->LastFileEntry, lst);	//assign a FileClass to the StructFD generated before
	char * FullPathStr = fileInst->fd_namefullPath;	//must store proper filepath	must return fullPath here (0:/folder0/filename.ext)
	FILE * f = fopen(FullPathStr,"r");
	sint32 structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;
	struct fd * fdinst = NULL;
	if(f){
		structFDIndex = fileno(f);
		fdinst = getStructFD(structFDIndex);
		FirstClusterFromLastFileOpen = getStructFDFirstCluster(fdinst);
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
FILE_TYPE return: OUT returns FT_NONE if file doesn't exists, FT_FILE if file exists, or FT_DIR if it's a directory and exists
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
bool FAT_FreeFiles (void){
	FS_deinit();
	// Return status of card
	DLDI_INTERFACE* dldiInterface = dldiGet();
	return (bool)dldiInterface->ioInterface.isInserted();
}


/*-----------------------------------------------------------------
FAT_InitFiles
Reads the FAT information from the CF card.
You need to call this before reading any files.
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_InitFiles (void){
	return true;	//TGDS assumes the card was already inited if you followed the TGDS Standard ARM9 Init code start
}

/////////////////////////////////////////////Libfat wrapper layer End/////////////////////////////////////////////
//

bool setFileClass(bool iterable, char * fullPath, int FileClassListIndex, int Typ, int StructFD, struct FileClassList * lst){
	if( (lst != NULL) && (FileClassListIndex < FileClassItems) ){	//prevent overlapping current FileClassList 
		struct FileClass * FileClassInst = getFileClassFromList(FileClassListIndex, lst);
		FileClassInst->isIterable = iterable;
		strcpy(FileClassInst->fd_namefullPath, (const char *)fullPath);
		FileClassInst->type = Typ;
		FileClassInst->d_ino = StructFD;
		FileClassInst->curIndexInsideFileClassList = FileClassListIndex;
		FileClassInst->parentFileClassList = lst;	//Parent FileClass List node
		lst->FileDirCount++;
		return true;
	}
	return false;
}

bool setFileClassObj(int FileClassListIndex, struct FileClass * FileClassObj, struct FileClassList * lst){
	if( (lst != NULL) && (FileClassListIndex < FileClassItems) ){	//prevent overlapping current FileClassList		
		struct FileClass * thisFileClass = getFileClassFromList(FileClassListIndex, lst);
		memcpy((u8*)thisFileClass, (u8*)FileClassObj, sizeof(struct FileClass));
		coherent_user_range_by_size((uint32)thisFileClass, sizeof(struct FileClass)); //update changes on physical ram inmediately
		lst->FileDirCount++;
		return true;
	}
	return false;
}

struct FileClass * getFileClassFromList(int FileClassListIndex, struct FileClassList * lst){
	if( (lst == NULL) || ( (FileClassListIndex > FileClassItems) || (FileClassListIndex < 0) )  ){ //order is important in order to prevent &catch null referencing of out of bounds FileClass by index
		return NULL;
	}
	return (struct FileClass *)&lst->fileList[FileClassListIndex];
}

//path can be either Directory or File, a proper FileClass will be returned.
struct FileClass getFileClassFromPath(char * path){
	struct FileClass FileClassOut;
	FILE * fh = fopen(path,"r");
	FileClassOut.type = FT_NONE;
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
	FileClassOut.d_ino = structfd_posixInvalidFileDirOrBufferHandle;	//destroyable FileClass are always invalid filehandles
	return FileClassOut;
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
			errno = fresultToErrno(result);
		}
	}
	return false;
}

//Note: Requires a fresh call to buildFileClassListFromPath prior to calling this
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct FileClass * getFirstDirEntryFromList(struct FileClassList * lst){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClassFromList(i, lst);
		if(fileClassInst->type == FT_DIR){
			FileClassRet = fileClassInst;
			break;
		}
	}
	lst->CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromList() now
	return FileClassRet;
}

//Note: Requires a fresh call to buildFileClassListFromPath prior to calling this
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct FileClass * getFirstFileEntryFromList(struct FileClassList * lst){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClassFromList(i, lst);
		if(fileClassInst->type == FT_FILE){
			FileClassRet = fileClassInst;
			break;
		}
	}
	lst->CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromList() now
	return FileClassRet;
}

//returns: true if read success, otherwise error if false 
//filename_out must be at least MAX_TGDSFILENAME_LENGTH+1
bool readFileNameFromFileClassIndex(char* filename_out, struct FileClass * FileClassInst){
	if ((filename_out == NULL) || (FileClassInst == NULL)){
		return false;
	}
	char *  FullPathStr = FileClassInst->fd_namefullPath;
	strcpy(filename_out, FullPathStr);
	return true;
}

//returns:  Valid FileClass entry or NULL
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct FileClass * getFirstFile(char * path, struct FileClassList * lst, int startFromGivenIndex){
	struct FileClass * fileInst = NULL;
	//path will have the current working directory the FileClass was built around. getFirstFile builds everything, and getNextFile iterates over each file until there are no more.
	if(buildFileClassListFromPath(path, lst, startFromGivenIndex) == true){
		lst->CurrentFileDirEntry = startFromGivenIndex;
		fileInst = getFileClassFromList(lst->CurrentFileDirEntry, lst);
		int fType = fileInst->type;	//FT_NONE / FT_FILE / FT_DIR
		switch(fType){
			//dir
			case(FT_DIR):{
				lst->LastDirEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}
			break;
			//file
			case(FT_FILE):{
				lst->LastFileEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}			
			break;
			//empty / no dir or file
			default:{
				return NULL;	//End the list regardless, no more room available!. 
			}
			break;
		}
		//increase the file/dir counter after operation only if valid entry, otherwise it doesn't anymore
		if(lst->CurrentFileDirEntry < (int)(FileClassItems)){ 
			lst->CurrentFileDirEntry++;	
		}
		else{
			lst->CurrentFileDirEntry = 0;
			lst->LastDirEntry=structfd_posixInvalidFileDirOrBufferHandle;
			lst->LastFileEntry=structfd_posixInvalidFileDirOrBufferHandle;
			return NULL;	//End the list regardless, no more room available!
		}
	}
	return fileInst;
}

//requires fullpath of the CURRENT file, it will return the next one
//return:  Valid FileClass entry or NULL
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct FileClass * getNextFile(char * path, struct FileClassList * lst){
	struct FileClass * fileInst = getFileClassFromList(lst->CurrentFileDirEntry, lst);	//NULL == invalid. Should not happen 
	if(fileInst != NULL){
		int fType = fileInst->type;
		switch(fType){
			//dir
			case(FT_DIR):{
				lst->LastDirEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}
			break;
			//file
			case(FT_FILE):{
				lst->LastFileEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}
			break;
		}	
		//increase the file counter after operation
		if(lst->CurrentFileDirEntry < (int)(FileClassItems) && (lst->CurrentFileDirEntry < lst->FileDirCount)){ 
			lst->CurrentFileDirEntry++;	
		}
		else{
			lst->CurrentFileDirEntry = 0;
			lst->LastDirEntry=structfd_posixInvalidFileDirOrBufferHandle;
			lst->LastFileEntry=structfd_posixInvalidFileDirOrBufferHandle;
			return NULL;	//End the list regardless, no more room available!
		}
	}
	return fileInst;
}

//TGDS FS Directory Iterator object:
//returns current count of such context.
//Otherwise error (structfd_FileClassListInvalidEntry) if: 
//	- FileClassList exceeded the FileClass items to store
//	- FileClassList exceeded the FileClass items to store

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int getCurrentDirectoryCount(struct FileClassList * lst){
	if(lst != NULL){
		return lst->FileDirCount;
	}
	return structfd_FileClassListInvalidEntry;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setCurrentDirectoryCount(struct FileClassList * lst, int value){
	lst->FileDirCount = value;
} 

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool readDirectoryIntoFileClass(char * dir, struct FileClassList * thisClassList){
	//Create TGDS Dir API context
	cleanFileList(thisClassList);
	thisClassList->FileDirCount = 0;
	
	//Use TGDS Dir API context
	struct FileClass filStub;
	{
		filStub.type = FT_NONE;
		strcpy(filStub.fd_namefullPath, "[notVisibleDirByPrintfIsIgnored]");
		filStub.isIterable = true;
		filStub.d_ino = -1;
		filStub.parentFileClassList = thisClassList;
	}
	char curPath[MAX_TGDSFILENAME_LENGTH+1];
	strcpy(curPath, dir);
	pushEntryToFileClassList(true, filStub.fd_namefullPath, filStub.type, -1, thisClassList);
	
	int startFromIndex = 1;
	//Init TGDS FS Directory Iterator Context(s). Mandatory to init them like this!! Otherwise several functions won't work correctly.
	struct FileClassList * tempfileClassList = initFileList();
	cleanFileList(tempfileClassList);
		
	struct FileClass * fileClassInst = NULL;
	fileClassInst = FAT_FindFirstFile(curPath, tempfileClassList, startFromIndex);

	while(fileClassInst != NULL){
		pushEntryToFileClassList(true, fileClassInst->fd_namefullPath, fileClassInst->type, startFromIndex, thisClassList);
		startFromIndex++;

		//more file/dir objects?
		fileClassInst = FAT_FindNextFile(curPath, tempfileClassList);
	}

	//Free TGDS Dir API context
	freeFileList(tempfileClassList);
	thisClassList->FileDirCount = startFromIndex - 1;
	return true;
}

//builds a FileClass list sorted from args priority
//returns: item count pushed to targetClassList

//Usage:
//struct FileClassList * playlistfileClassListCtx = NULL;
//playlistfileClassListCtx = initFileList();
//cleanFileList(playlistfileClassListCtx);
//bool ret = readDirectoryIntoFileClass("/music", playlistfileClassListCtx);
//#define pattern "/ima/wav/it/mod/s3m/xm/mp3/mp2/mpa/ogg/aac/m4a/m4b/flac/sid/nsf/spc/sndh/snd/sc68/gbs"
//struct FileClassList * foundPlayList = NULL;
//foundPlayList = initFileList();
//cleanFileList(foundPlayList);
//int itemsFound = buildFileClassByExtensionFromList(playlistfileClassListCtx, foundPlayList, pattern);

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int buildFileClassByExtensionFromList(struct FileClassList * inputClassList, struct FileClassList * targetClassList, char * filterString){
	char * outBuf = (char *)TGDSARM9Malloc(256 * FileClassItems);
	int i = 0, j = 0;
	int matchCount = str_split((char*)filterString, (char*)"/", outBuf, FileClassItems, 256);
	
	if ((matchCount + 3) > FileClassItems){
		matchCount = FileClassItems;
	}
	else{
		matchCount = (matchCount + 3);
	}
	
	int fileClassListSize = getCurrentDirectoryCount(inputClassList) + 1; 
	for(i = 0; i < matchCount; i++){
		char * token_rootpath = (char*)(outBuf + (256*i));
		char extToFind[256];
		strcpy(extToFind, ".");
		strcat(extToFind, token_rootpath);
		for(j = 0; j < fileClassListSize; j++){
			if(j < fileClassListSize){
				FileClass * curFile = (FileClass *)&inputClassList->fileList[j];
				char tmpName[MAX_TGDSFILENAME_LENGTH+1];
				char extInFile[MAX_TGDSFILENAME_LENGTH+1];
				strcpy(tmpName, curFile->fd_namefullPath);	
				separateExtension(tmpName, extInFile);
				strlwr(extInFile);		
				if(strcmpi(extToFind, extInFile) == 0){
					pushEntryToFileClassList(true, curFile->fd_namefullPath, curFile->type, -1, targetClassList);
				}
			}
		}
	}
	
	//Add directories
	for(j = 0; j < fileClassListSize; j++){
		FileClass * curFile = (FileClass *)&inputClassList->fileList[j];
		if(curFile->type == FT_DIR){
			pushEntryToFileClassList(true, curFile->fd_namefullPath, curFile->type, -1, targetClassList);
		}
	}
	
	TGDSARM9Free(outBuf);
	return targetClassList->FileDirCount;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int pcmpstr(const void* a, const void* b){
    const FileClass * aa = (const FileClass *)a;
    const FileClass * bb = (const FileClass *)b;
	
    return stricmp((char*)&aa->fd_namefullPath, (char*)&bb->fd_namefullPath);
}

//Returns:	true if success
//			false if error
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void sortFileClassListAsc(struct FileClassList * thisClassList, bool ignoreFirstFileClass){
	qsort((struct FileClass*)&thisClassList->fileList, thisClassList->FileDirCount, sizeof(FileClass), pcmpstr);
}

///////////////////////////////////////////////////////TGDS FS API extension end. /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////USER CODE END/////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////INTERNAL CODE START///////////////////////////////////////////////////////////////////////////////
int  readdir_r(DIR * dirp,struct dirent * entry,struct dirent ** result){
    return fatfs_readdir_r(dirp, entry, result);
}

int fatfs_write(int structFDIndex, u8 *ptr, int len){	//(FileDescriptor :struct fd index)
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd *pfd = getStructFD(structFDIndex);
    if (pfd == NULL){	//not file? not alloced struct fd?
		errno = EBADF;
    }
    else if (
	(pfd->filPtr == NULL)	//no FIL descriptor?
	&&
	(pfd->dirPtr == NULL)	//and no DIR descriptor? definitely can't write to this file descriptor
	){
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode)){
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
        if (result == FR_OK){
            result = f_write(filp, ptr, len, &written);	//writes success: if ret directory file invalid (7) check permission flags passed through fopen
        }
        if (result == FR_OK){
            ret = written;
        }
        else{
            errno = fresultToErrno(result);
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

//read (get struct FD index from FILE * handle)
int fatfs_read(int structFDIndex, u8 *ptr, int len){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd *pfd = getStructFD(structFDIndex);
    if ( (pfd == NULL) || (pfd->filPtr == NULL) ){	//not file/dir? not alloced struct fd?
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode)){
        FIL *filp;
        FRESULT result;
        UINT nbytes_read;
        filp = pfd->filPtr;
        result = f_read(filp, ptr, len, &nbytes_read);
        if (result == FR_OK){
            ret = nbytes_read;
        }
        else{
            errno = fresultToErrno(result);
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
int fatfs_close (int structFDIndex){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd * pfd = getStructFD(structFDIndex);
    if ( (pfd == NULL) || ((pfd->filPtr == NULL) && (pfd->dirPtr == NULL)) || (pfd->isused != (sint32)structfd_isused) || (structFDIndex == structfd_posixInvalidFileDirOrBufferHandle) ){	//not file/dir? not alloced struct fd? or not valid file handle(two cases)?
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
		){
			//struct stat buf;	//flush stat
			memset (&pfd->stat, 0, sizeof(pfd->stat));
			
			//free struct fd
			int i_fil = FileHandleFree(pfd->cur_entry.d_ino);	//returns structfd index that was deallocated
			if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
				if(pfd->filPtr){	//must we clean a FIL?
					pfd->filPtr = NULL;
				}
				if(pfd->dirPtr){	//must we clean a DIR?
					pfd->dirPtr = NULL;
				}
				//clean filename
				sprintf((char*)&pfd->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
			}
			else{
				//file_free failed
			}
			
			ret = 0;
			//update d_ino here (POSIX compliant)
			pfd->cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirOrBufferHandle;
			pfd->loc = 0;
        }
        else{
			errno = fresultToErrno(result);
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
        else{
			errno = fresultToErrno(result);
        }
    }
    else{
		errno = EBADF;
    }
	return ret;
}

long fatfs_ftell(struct fd * fdinst){
	return fatfs_lseek(fdinst->cur_entry.d_ino, 0, SEEK_CUR);
}

int fatfs_fsize (int structFDIndex){
	struct fd * structfd = getStructFD(structFDIndex);
	if(structfd != NULL){
		return f_size(structfd->filPtr);
	}
	return structfd_posixInvalidFileSize;
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

//called from : stat (newlib implementation)
void initStructFDHandle(struct fd *pfd, int flags, const FILINFO *fno, int structFD, int StructFDType){
    pfd->isatty = 0;
    pfd->status_flags = flags;
    pfd->descriptor_flags = 0;
	pfd->loc = 0;	//internal file/dir offset zero
	pfd->StructFD = structFD;
	pfd->StructFDType = StructFDType;
	fillPosixStatStruct(fno, &pfd->stat);
}

//returns an internal index struct fd allocated
int fatfs_open_file(const sint8 *pathname, int flags, const FILINFO *fno){
	//Lookup if file is already open.
	BYTE mode;
	FRESULT result;
	int structFDIndex = getStructFDIndexByFileName((char*)pathname);
	if(structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		return structFDIndex;
	}
	//If not, then allocate a new file handle (struct FD)
	
	//allocates a new struct fd index, allocating a FIL structure, for the devoptab_sdFilesystem object.
	structFDIndex = FileHandleAlloc((struct devoptab_t *)&devoptab_sdFilesystem);	
    if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		files[structFDIndex].filPtr	=	(FIL *)&files[structFDIndex].fil;
		files[structFDIndex].dirPtr	= NULL;
		files[structFDIndex].StructFDType = FT_FILE;
	}
	
	struct fd * fdinst = getStructFD(structFDIndex);	//getStructFD requires struct fd index
	if (fdinst == NULL){
		result = FR_TOO_MANY_OPEN_FILES;
    }
	else{
		FILINFO fno_after;
        if(flags & O_CREAT){
			result = f_unlink((const TCHAR*)pathname);
			if (result == FR_OK){
				//file was deleted
			}
			else{
				//file does not exist
			}
		}
		mode = posixToFatfsAttrib(flags);
		result = f_open(fdinst->filPtr, (const TCHAR*)pathname, mode);	/* Opens an existing file. If not exist, creates a new file. */
		if (result == FR_OK){		
			result = f_stat((const TCHAR*)pathname, &fno_after);
			fno = &fno_after;			
			if ((result == FR_OK) && (TGDSFS_detectUnicode(fdinst) == true)){
				//Update struct fd with new FIL
				initStructFDHandle(fdinst, flags, fno, structFDIndex, FT_FILE);
				//copy full file path (posix <- fatfs)
				int topsize = strlen(pathname)+1;
				if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
					topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
				}
				strncpy(fdinst->fd_name, pathname, topsize);
			}
			else{
				errno = fresultToErrno(result);
				
				//free struct fd
				int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
				if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
					if(fdinst->filPtr){	//must we clean a FIL?
						fdinst->filPtr = NULL;
					}
					if(fdinst->dirPtr){	//must we clean a DIR?
						fdinst->dirPtr = NULL;
					}
					//clean filename
					sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
				}
				else{
					//file_free failed
				}
			
				structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;	//file stat was valid but something happened while IO operation, so, invalid.
			}
		}
        else {	//invalid file or O_CREAT wasn't issued
			
			//free struct fd
			int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
			if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
				if(fdinst->filPtr){	//must we clean a FIL?
					fdinst->filPtr = NULL;
				}
				if(fdinst->dirPtr){	//must we clean a DIR?
					fdinst->dirPtr = NULL;
				}
				//clean filename
				sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
			}
			else{
				//file_free failed
			}
			structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;	//file handle generated, but file open failed, so, invalid.
		}
    }// failed to allocate a file handle / allocated file handle OK end.
	
	return structFDIndex;
}

//returns an internal index struct fd allocated
int fatfs_open_dir(const sint8 *pathname, int flags, const FILINFO *fno){
    FRESULT result;
    
	//allocates a new struct fd index, allocating a DIR structure, for the devoptab_sdFilesystem object.
	int structFDIndex = FileHandleAlloc((struct devoptab_t *)&devoptab_sdFilesystem);	
    if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		files[structFDIndex].dirPtr	=	(DIR *)&files[structFDIndex].dir;
		files[structFDIndex].filPtr	= NULL;
		files[structFDIndex].StructFDType = FT_DIR;
	}
	
	struct fd * fdinst = getStructFD(structFDIndex);
	if (fdinst == NULL){
		result = FR_TOO_MANY_OPEN_FILES;
    }
    else{
		result = f_opendir(fdinst->dirPtr, (const TCHAR*)pathname);
        if (result == FR_OK){
			//Update struct fd with new DIR
			initStructFDHandle(fdinst, flags, fno, structFDIndex, FT_DIR);
			//copy full directory path (posix <- fatfs)
			int topsize = strlen(pathname)+1;
			if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
				topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
			}
			strncpy(fdinst->fd_name, pathname, topsize);
        }
        else{
			errno = fresultToErrno(result);            
			//free struct fd
			int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
			if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
				if(fdinst->filPtr){	//must we clean a FIL?
					fdinst->filPtr = NULL;
				}
				if(fdinst->dirPtr){	//must we clean a DIR?
					fdinst->dirPtr = NULL;
				}
				//clean filename
				sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
			}
			else{
				//file_free failed
			}
        }
    }

    return structFDIndex;
}

//returns true : Valid string!
//returns false: NULL string!
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool isNotNullString(const char * str){
	if( (str != NULL) && (strlen(str) > 0) ){
		return true;
	}
	return false;
}

//returns / allocates a new struct fd index with either DIR or FIL structure allocated
int fatfs_open_file_or_dir(const sint8 *pathname, int flags){
	if( isNotNullString((const char *)pathname) == true ){
		FILINFO fno;
		int structFD = structfd_posixInvalidFileDirOrBufferHandle;
		fno.fname[0] = '\0'; /* initialize as invalid */
		FRESULT result = f_stat((const TCHAR*)pathname, &fno);
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
			errno = fresultToErrno(result);
		}
		return structFD;
	}
	else{
		return structfd_posixInvalidFileDirOrBufferHandle;
	}
}

// Use NDS RTC to update timestamps into files when certain operations require it
DWORD get_fattime (void){
	#ifdef ARM9
	struct tm * tmStruct = getTime();
    return ((((sint32)tmStruct->tm_year-60)<<25)
				|
				(((sint32)tmStruct->tm_mon)<<21)
				|
				(((sint32)tmStruct->tm_mday)<<16)
				|
				(((sint32)tmStruct->tm_hour)<<11)
				|
				(((sint32)tmStruct->tm_min)<<5)
				|
				(((sint32)tmStruct->tm_sec)<<0)	);
	#endif
	return 0;
}

/////////////////////////////// direct struct fd methods start

int fatfs_readDirectStructFD(struct fd * pfd, u8 *ptr, int len){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    if ( (pfd == NULL) || (pfd->filPtr == NULL) ){	//not file/dir? not alloced struct fd?
        errno = EBADF;
    }
    FIL *filp;
	FRESULT result;
	UINT nbytes_read;
	filp = pfd->filPtr;
	result = f_read(filp, ptr, len, &nbytes_read);
	if (result == FR_OK){
		ret = nbytes_read;
	}
	else{
		errno = fresultToErrno(result);
	}
    return ret;
}

int fatfs_seekDirectStructFD(struct fd * pfd, int offst){
	return f_lseek (
			pfd->filPtr,   /* Pointer to the file object structure */
			(DWORD)offst       /* File offset in unit of byte */
		);
}

//receives a new struct fd index with either DIR or FIL structure allocated so it can be closed.
//returns 0 if success, 1 if error
int fatfs_closeDirectStructFD(struct fd * pfd){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    if ( (pfd == NULL) || (pfd->filPtr == NULL) ){
		errno = EBADF;
    }
	else{
		FIL *filp;
		FRESULT result;
		filp = pfd->filPtr;
		result = f_close(filp);
		if (
		(result == FR_OK)			//file sync with hardware SD ok
		||
		(result == FR_DISK_ERR)		//create file (fwrite + w): file didn't exist before open(); thus file descriptor didn't have any data of sectors to compare with. Exception expected
		){
			//struct stat buf;	//flush stat
			memset (&pfd->stat, 0, sizeof(pfd->stat));
			
			//free struct fd
			if(pfd->filPtr){	//must we clean a FIL?
				pfd->filPtr = NULL;
			}
			if(pfd->dirPtr){	//must we clean a DIR?
				pfd->dirPtr = NULL;
			}
			//clean filename
			sprintf((char*)&pfd->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
			
			ret = 0;
			//update d_ino here (POSIX compliant)
			pfd->cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirOrBufferHandle;
			pfd->loc = 0;
		}
		else{
			errno = fresultToErrno(result);
		}
	}
	return ret;
}


/////////////////////////////// direct struct fd methods end


//Maps a new struct fd / Creates filehandles when opening a new file
int fatfs_open_fileIntoTargetStructFD(const sint8 *pathname, char * posixFlags, struct fd * directStructFD){
	BYTE mode;
	FRESULT result;
	struct fd * fdinst = NULL;
	int structFDIndex = -2;
	if(directStructFD == NULL){
		//allocates a new struct fd index, allocating a FIL structure, for the devoptab_sdFilesystem object.
		structFDIndex = FileHandleAlloc((struct devoptab_t *)&devoptab_sdFilesystem);	
		if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
			files[structFDIndex].filPtr	=	(FIL *)&files[structFDIndex].fil;
			files[structFDIndex].dirPtr	= NULL;
			files[structFDIndex].StructFDType = FT_FILE;
		}
		else{
			//ran out of filehandles
		}
		fdinst = getStructFD(structFDIndex);	//getStructFD requires struct fd index
	}
	else{
		fdinst = directStructFD;
		memset(fdinst, 0, sizeof(struct fd));
		fdinst->filPtr	=	(FIL *)&fdinst->fil;
		fdinst->dirPtr	= NULL;
		fdinst->StructFDType = FT_FILE;
	}
	if ((fdinst == NULL) || (isNotNullString((const char *)pathname) == false) || (isNotNullString((const char *)posixFlags) == false) ){
		result = FR_TOO_MANY_OPEN_FILES;
    }
	else{
		FILINFO fno_after;
		int flags = charPosixToFlagPosix(posixFlags);
        if(flags & O_CREAT){
			result = f_unlink((const TCHAR*)pathname);
			if (result == FR_OK){
				//file was deleted
			}
			else{
				//file does not exist
			}
		}
		mode = posixToFatfsAttrib(flags);
		result = f_open(fdinst->filPtr, (const TCHAR*)pathname, mode);	/* Opens an existing file. If not exist, creates a new file. */
		if (result == FR_OK){		
			result = f_stat((const TCHAR*)pathname, &fno_after);
			if ((result == FR_OK) && (TGDSFS_detectUnicode(fdinst) == true)){
				//Update struct fd with new FIL
				initStructFDHandle(fdinst, flags, &fno_after, structFDIndex, FT_FILE);
		
				//copy full file path (posix <- fatfs)
				int topsize = strlen(pathname)+1;
				if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
					topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
				}
				strncpy(fdinst->fd_name, pathname, topsize);
			}
			else{
				errno = fresultToErrno(result);
				
				//free struct fd
				int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
				if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
					if(fdinst->filPtr){	//must we clean a FIL?
						fdinst->filPtr = NULL;
					}
					if(fdinst->dirPtr){	//must we clean a DIR?
						fdinst->dirPtr = NULL;
					}
					//clean filename
					sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
				}
				else{
					//file_free failed
				}
			
				structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;	//file stat was valid but something happened while IO operation, so, invalid.
			}
		}
        else {	//invalid file or O_CREAT wasn't issued
			if(directStructFD == NULL){
				//free struct fd
				int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
				if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
					if(fdinst->filPtr){	//must we clean a FIL?
						fdinst->filPtr = NULL;
					}
					if(fdinst->dirPtr){	//must we clean a DIR?
						fdinst->dirPtr = NULL;
					}
					//clean filename
					sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
				}
				else{
					//file_free failed
				}
			}
			structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;	//file handle generated, but file open failed, so, invalid.
		}
    }// failed to allocate a file handle / allocated file handle OK end.
	
	return structFDIndex;
}

//returns / allocates a new struct fd index with either DIR or FIL structure allocated
//if error, returns structfd_posixInvalidFileDirOrBufferHandle (invalid structFD file handle index)
int fatfs_open(const sint8 *pathname, int flags){
	int structFDIndex = fatfs_open_file_or_dir(pathname, flags);	
	if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		//update d_ino here (POSIX compliant) - through correct (internal struct fd) file handle
		struct fd *pfd = getStructFD(structFDIndex);
		pfd->cur_entry.d_ino = structFDIndex;
	}
	return structFDIndex;
}
// ret: structfd_posixInvalidFileDirOrBufferHandle if invalid posixFDStruct
//		else if POSIX retcodes (if an error happened)
//		else return new offset position (offset + current file position (internal)) in file handle
off_t fatfs_lseek(int structFDIndex, off_t offset, int whence){	//(FileDescriptor :struct fd index)
    off_t ret = (off_t)structfd_posixInvalidFileDirOrBufferHandle;
    struct fd *pfd = getStructFD(structFDIndex);
	if(pfd != NULL){
		if(pfd->filPtr != NULL){
			if (pfd->isused == structfd_isunused){
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
						if(offset > (topFile - 1)){
							offset = (topFile - 1);	//Mapped SEEK_SET is treated as inmediate offset, which starts from 0 so -1 here
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
						if((int)pos > topFile){
							pos = topFile;	//Mapped SEEK_CUR is treated as inmediate offset, which starts from 0, BUT (current) ahead offset-by-1 must not exceed physical filesize
						}
						validArg = true;
					}
					break;
					//<<SEEK_END>>---<[offset]> is relative to the current end of file.
					//<[offset]> can meaningfully be either positive (to increase the size
					//of the file) or negative.
					case(SEEK_END):{
						pos = topFile;				//file end is fileSize
						pos += offset;
						if((int)pos < 0){
							pos = 0;
						}
						if((int)pos > topFile){
							pos = topFile;
						}
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
						errno = fresultToErrno(result);
					}
				}
			}
			else{
				errno = EINVAL;
			}
		}
	}

	return ret;
}

//delete / remove
int fatfs_unlink(const sint8 *path){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    FRESULT result;
    result = f_unlink((const TCHAR*)path);
    if (result == FR_OK){
        ret = 0;
    }
    else{
        errno = fresultToErrno(result);
    }
    return ret;
}

int fatfs_link(const sint8 *path1, const sint8 *path2){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    // FAT does not support links
    errno = EMLINK; /* maximum number of "links" is 1: the file itself */
    return ret;
}

int fatfs_rename(const sint8 *oldfname, const sint8 * newfname){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	if( (isNotNullString((const char *)oldfname) == true) && (isNotNullString((const char *)newfname) == true) ){
		FRESULT result;
		result = f_rename((const TCHAR*)oldfname, (const TCHAR*)newfname);
		if (result != FR_OK){
			errno = fresultToErrno(result);
		}
		else{
			ret = 0;
		}
	}
    return ret;
}

int fatfs_fsync(int structFDIndex){	//uses struct fd indexing
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd * pfd = getStructFD(structFDIndex);
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
            errno = fresultToErrno(result);
        }
    }
    else{
        errno = EINVAL;
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle / struct stat * is empty or invalid
int fatfs_stat(const sint8 *path, struct stat *buf){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	if( (isNotNullString((const char *)path) == true) && (buf != NULL) ){
		FILINFO fno;
		FRESULT result = f_stat((const TCHAR*)path, &fno);
		if ((result == FR_OK) && (buf != NULL)){
			fillPosixStatStruct(&fno, buf);
			ret = 0;
		}
		else{
			errno = fresultToErrno(result);
		}
	}
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle if path exists or failed due to other IO reason
int fatfs_mkdir(const sint8 *path, mode_t mode){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	if(isNotNullString((const char *)path) == true){
		FRESULT result = f_mkdir((const TCHAR*)path);
		if (result == FR_OK){
			ret = 0;
		}
		else{
			errno = fresultToErrno(result);
		}
	}
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle if readonly or not empty, or failed due to other IO reason
int fatfs_rmdir(const sint8 *path){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	if(isNotNullString((const char *)path) == true){
		FRESULT result = f_unlink((const TCHAR*)path);
		if (result == FR_OK){
			ret = 0;
		}
		else if (result == FR_DENIED){
			FILINFO fno;
			// the dir was readonly or not empty: check 
			result = f_stat((const TCHAR*)path, &fno);
			if (result == FR_OK){
				if ((fno.fattrib & AM_MASK) & AM_RDO){
					errno = EACCES;
				}
				else{
					errno = ENOTEMPTY;
				}
			}
			else{
				errno = fresultToErrno(result);
			}
		}
		else{
			errno = fresultToErrno(result);
		}
	}
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle if incorrect path, or failed due to other IO reason
int fatfs_chdir(const sint8 *path){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    if(isNotNullString((const char *)path) == true){
		FRESULT result = f_chdir((const TCHAR*)path);
		if (result == FR_OK){
			ret = 0;
		}
		else{
			errno = fresultToErrno(result);   
		}
	}
    return ret;
}

//if ok: return char * cwf path
//else if error return NULL if path is incorrect, or failed due to other IO reason
sint8 *fatfs_getcwd(sint8 *buf, size_t size){
	sint8 *ret = NULL;
    FRESULT result = f_getcwd((TCHAR*)buf, size);
    if (result == FR_OK){
        ret = buf;
    }
    else{
        errno = fresultToErrno(result);
    }
    return ret;
}

//if correct path: returns: iterable DIR * entry
//else returns NULL
DIR *fatfs_opendir(const sint8 *path){
    DIR *ret = NULL;
    int structFDIndex = fatfs_open(path, O_RDONLY);	//returns an internal index struct fd allocated
	if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		struct fd *pfd = getStructFD(structFDIndex);
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
//else returns structfd_posixInvalidFileDirOrBufferHandle if incorrect iterable DIR *
int fatfs_closedir(DIR *dirp){
	//we need a conversion from DIR * to struct fd *
	int structFDIndex = getStructFDIndexByDIR(dirp);
	return fatfs_close(structFDIndex);	//requires a struct fd(file descriptor), returns 0 if success, structfd_posixInvalidFileDirOrBufferHandle if error
}

//if iterable DIR * is directory, return 0
//else return structfd_posixInvalidFileDirOrBufferHandle (not valid iterable DIR * entry or not directory )
int fatfs_dirfd(DIR *dirp){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	//we need a conversion from DIR * to struct fd *
	int structFDIndex = getStructFDIndexByDIR(dirp);
	struct fd * pfd = getStructFD(structFDIndex);
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
DIR *fatfs_fdopendir(int structFDIndex){	//(FileDescriptor :struct fd index)
    DIR *ret = NULL;
    struct fd *pfd = getStructFD(structFDIndex);
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

//if iterable DIR * has a struct dirent *(StructFD index, being an object), return such struct dirent * (internal fatfs file handle descriptor)
//else return NULL (not valid iterable DIR * entry or not directory )
struct dirent *fatfs_readdir(DIR *dirp){
    struct dirent *ret = NULL;
    int structFDIndex = getStructFDIndexByDIR(dirp);	//we need a conversion from DIR * to struct fd *
	struct fd * fdinst = getStructFD(structFDIndex);
	if( (fdinst != NULL) && (fatfs_readdir_r(dirp, (struct dirent *)&fdinst->cur_entry, &ret) != structfd_posixInvalidFileDirOrBufferHandle) ){
		return ret;
	}
	else{
        errno = EBADF;
	}
	//ret dirent * with read file/dir properties, namely: d_name
    return ret;
}

//if return equals 0 : means the struct dirent * (internal read file / dir) coming from DIR * was correctly parsed. 
//else if return equals structfd_posixInvalidFileDirOrBufferHandle: the DIR * could not parse the output struct dirent * (internal read file / dir), being NULL.
int fatfs_readdir_r(
        DIR *dirp,
        struct dirent *entry,	//current dirent of file handle struct fd
        struct dirent **result){	//pointer to rewrite above file handle struct fd

    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	int structFDIndex = getStructFDIndexByDIR(dirp);
	struct fd * fdinst = getStructFD(structFDIndex);
	if( (fdinst != NULL) && (entry != NULL) && (result != NULL) ){
		FRESULT fresult;
		FILINFO fno;
		fresult = f_readdir(dirp, &fno);
		if (fresult != FR_OK){
			errno = fresultToErrno(fresult);
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
			
			//d_name : dir or file name. NOT full path (posix <- fatfs)
			int topsize = strlen((char*)fno.fname)+1;
			if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
				topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
			}
			strncpy((sint8*)entry->d_name, (sint8*)fno.fname, topsize);
			*result = entry;
			
			//update FT_FILE / FT_DIR
			if (fno.fattrib & AM_DIR) {
				fdinst->StructFDType = FT_DIR;
			}
			
			if (fno.fattrib & AM_ARC) {
				fdinst->StructFDType = FT_FILE;
			}
		}
    }
	return ret;
}

void fatfs_rewinddir(DIR *dirp){
	int structFDIndex = getStructFDIndexByDIR(dirp);
	if(dirp != NULL){
		struct fd * fdinst = getStructFD(structFDIndex);
		if(fdinst != NULL){
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
	}
}

long fatfs_tell(struct fd *f){
    long ret = structfd_posixInvalidFileDirOrBufferHandle;
	if(f != NULL){
		//dir
		if ((f->dirPtr) && (S_ISDIR(f->stat.st_mode))){
			ret = f->loc;
		}
		//file
		else if ((f->filPtr) && (S_ISREG(f->stat.st_mode))){
			ret = f->loc;
		}
	}
    return ret;
}

void fatfs_seekdir(DIR *dirp, long loc){
	if(dirp != NULL){
		int structFDIndex = getStructFDIndexByDIR(dirp);
		struct fd * fdinst = getStructFD(structFDIndex);
		if ( (fdinst != NULL) && S_ISDIR(fdinst->stat.st_mode)){
			long cur_loc = fatfs_tell(fdinst);
			if (loc < cur_loc){
				fatfs_rewinddir(dirp);
			}
			while(loc > cur_loc){
				int ret = structfd_posixInvalidFileDirOrBufferHandle;
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
}

//this copies stat from internal struct fd to output stat
//if StructFD (structFDIndex) index is created by fatfs_open (either file / dir), return 0 and the such struct stat * is updated.
//else return structfd_posixInvalidFileDirOrBufferHandle (not valid struct stat * entry, being NULL)
int _fstat_r( struct _reent *_r, int structFDIndex, struct stat *buf ){	//(FileDescriptor :struct fd index)
    struct fd * f = getStructFD(structFDIndex);
	if ((f == NULL) || (buf == NULL)){
		#ifdef ARM9
		_r->_errno = EBADF;
		#endif
		return structfd_posixInvalidFileDirOrBufferHandle;
	}
	else if (f->isused == structfd_isunused){
        #ifdef ARM9
		_r->_errno = EBADF;
		#endif
		return structfd_posixInvalidFileDirOrBufferHandle;
    }
    *buf = f->stat;
    return 0;
}


//true: able to detect if file was unicode or not
//false: couldn't tell if file was unicode or not
bool TGDSFS_detectUnicode(struct fd *pfd){
	#ifdef ARM9
	if(pfd != NULL){
		FILE * fil = fdopen(pfd->cur_entry.d_ino, "r");
		if(fil != NULL){
			char tStr[4];
			u32 cPos = ftell(fil);
			fseek(fil, 0, SEEK_SET);
			fread((uint8*)&tStr[0], 1, 4, fil);
			if(tStr[0] == 0 && tStr[2] == 0 && tStr[1] != 0 && tStr[3] != 0){	// fairly good chance it's unicode
				pfd->fileIsUnicode = true;
			}
			else{
				pfd->fileIsUnicode = false;
			}
			fseek(fil, cPos, SEEK_SET);
			return true;
		}
	}
	return false;
	#endif
	
	#if defined(WIN32)
		return true;
	#endif
}


/////////////////////////////////////////



//Takes an open file handle, gets filesize without updating its internal file pointer
int	FS_getFileSizeFromOpenHandle(FILE * f){
	int size = -1;
	if (f != NULL){
		int fLoc = ftell(f);
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, fLoc, SEEK_SET);
	}
	return size;
}

int	FS_getFileSizeFromOpenStructFD(struct fd * tgdsfd){
	int size = -1;
	if ((tgdsfd != NULL) && (tgdsfd->filPtr != NULL)){
		size = f_size(tgdsfd->filPtr);
	}
	return size;
}

struct fd *getStructFD(int fd){
    struct fd *fhandle = NULL;
    if((fd < OPEN_MAXTGDS) && (fd >= 0)){
		fhandle = &files[fd];
        if(files[fd].dirPtr){
			fhandle->dirPtr = (DIR *)&files[fd].dir;
		}
		if(files[fd].filPtr){
			fhandle->filPtr = (FIL *)&files[fd].fil;
		}
	}
    return fhandle;
}

//char * devoptabFSName must be a buffer already allocated if bool defaultDriver == false
void initTGDS(char * devoptabFSName){
	memset((u8*)&dldiFs, 0, sizeof(dldiFs));
	if(devoptabFSName == NULL){
		return;
	}
	
	initTGDSDevoptab();
	memset((uint8*)&files, 0, sizeof(files));
	int fd = 0;
	/* search in all struct fd instances*/
	for (fd = 0; fd < OPEN_MAXTGDS; fd++){
		files[fd].isused = (sint32)structfd_isunused;
		//Internal default invalid value (overriden later)
		files[fd].cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirOrBufferHandle;	//Posix File Descriptor
		files[fd].StructFD = (sint32)structfd_posixInvalidFileDirOrBufferHandle;		//TGDS File Descriptor (struct fd)
		files[fd].StructFDType = FT_NONE;	//struct fd type invalid.
		files[fd].isatty = (sint32)structfd_isattydefault;
		files[fd].descriptor_flags = (sint32)structfd_descriptorflagsdefault;
		files[fd].status_flags = (sint32)structfd_status_flagsdefault;
		files[fd].loc = (sint32)structfd_fildir_offsetdefault;
		
		files[fd].filPtr = NULL;
		files[fd].dirPtr = NULL;
	}
	//Set up proper devoptab device mount name.
	memcpy((uint32*)&devoptab_sdFilesystem.name[0], (uint32*)devoptabFSName, strlen(devoptabFSName));
}

FILE * getPosixFileHandleByStructFD(struct fd * fdinst, const char * mode){
	if((fdinst != NULL) && (mode != NULL)){
		return fdopen(fdinst->cur_entry.d_ino, mode);
	}
	return NULL;
}

struct fd * getStructFDByFileHandle(FILE * fh){
	if(fh != NULL){
		int StructFD = fileno(fh);
		return getStructFD(StructFD);
	}
	return NULL;
}

int getStructFDIndexByFileName(char * filename){
	int ret = structfd_posixInvalidFileDirOrBufferHandle;
	if(isNotNullString((const char *)filename) == true){
		int fd = 0;
		/* search in all struct fd instances*/
		for (fd = 0; fd < OPEN_MAXTGDS; fd++){
			if(files[fd].isused == (sint32)structfd_isused){
				if(strcmp((char*)&files[fd].fd_name, filename) == 0){
					//printfDebugger("getStructFDIndexByFileName(): idx:%d - f:%s",fd, files[fd]->fd_name);
					return fd;
				}
			}
		}
	}
	return ret;
}

//returns / allocates a new struct fd index ,  for a certain devoptab_t so we can allocate different handles for each devoptab
int FileHandleAlloc(struct devoptab_t * devoptabInst){
    int fd = 0;
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	if(devoptabInst != NULL){
		for (fd = 0; fd < OPEN_MAXTGDS; fd++){
			struct fd * internalFHandle = &files[fd];
			if ((sint32)internalFHandle->isused == (sint32)structfd_isunused){
				internalFHandle->isused = (sint32)structfd_isused;
				
				//PosixFD default valid value (overriden now)
				internalFHandle->devoptabFileDescriptor = devoptabInst;
				
				//Internal default value (overriden now)
				internalFHandle->cur_entry.d_ino = (sint32)fd;	//Posix File Descriptor
				internalFHandle->StructFD = (sint32)structfd_posixInvalidFileDirOrBufferHandle;		//TGDS File Descriptor (struct fd) not valid yet. Set up by initStructFDHandle()
				internalFHandle->StructFDType = FT_NONE;	//struct fd type not valid yet. Will be assigned by fatfs_open_file (FT_FILE), or fatfs_open_dir (FT_DIR)
				
				internalFHandle->isatty = (sint32)structfd_isattydefault;
				internalFHandle->descriptor_flags = (sint32)structfd_descriptorflagsdefault;
				internalFHandle->status_flags = (sint32)structfd_status_flagsdefault;
				internalFHandle->loc = (sint32)structfd_fildir_offsetdefault;
				
				internalFHandle->filPtr = NULL;
				internalFHandle->dirPtr = NULL;
				
				ret = fd;
				break;
			}
		}
		//if for some reason all the file handles are exhausted, discard the last one and assign that one. (fixes homebrew that opens a lot of file handles and doesn't close them up accordingly)
		if(ret == structfd_posixInvalidFileDirOrBufferHandle){
			int retClose = fatfs_close(OPEN_MAXTGDS - 1);	//requires a struct fd(file descriptor), returns 0 if success, structfd_posixInvalidFileDirOrBufferHandle if error
			if(retClose == structfd_posixInvalidFileDirOrBufferHandle){
				//couldnt really close file handle
				ret = retClose;
			}
			else{
				//file handle close success!
				return FileHandleAlloc(devoptabInst);	//ret == return value here
			}
		}
	}
    return ret;
}

//deallocates a posix index, returns such index deallocated
int FileHandleFree(int fd){
	int ret = structfd_posixInvalidFileDirOrBufferHandle;
    if( (fd < OPEN_MAXTGDS) && (fd >= 0) ){
		struct fd * internalFHandle = &files[fd];
		if(internalFHandle->isused == structfd_isused){
			internalFHandle->isused = (sint32)structfd_isunused;
			internalFHandle->cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirOrBufferHandle;	//Posix File Descriptor
			internalFHandle->StructFD = (sint32)structfd_posixInvalidFileDirOrBufferHandle;		//TGDS File Descriptor (struct fd)
			internalFHandle->StructFDType = FT_NONE;	//struct fd type invalid.
			internalFHandle->loc = structfd_posixInvalidFileHandleOffset;
			ret = fd;
		}
	}
	return ret;
}

sint8 * getDeviceNameByStructFDIndex(int StructFDIndex){
	sint8 * out = NULL;
	if( (StructFDIndex < OPEN_MAXTGDS) && (StructFDIndex >= 0) ){
		sint32 i = 0;
		/* search in all struct fd instances*/
		for (i = 0; i < OPEN_MAXTGDS; i++)
		{
			if (files[i].cur_entry.d_ino == StructFDIndex)
			{
				out = (sint8*)(&devoptab_struct[i]->name);
			}
		}
	}
	return out;
}

//useful for handling native DIR * to Internal File Descriptors (struct fd index)
int getStructFDIndexByDIR(DIR *dirp){
	int fd = 0;
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	if(dirp != NULL){
		/* search in all struct fd instances*/
		for (fd = 0; fd < OPEN_MAXTGDS; fd++)
		{
			struct fd * fhandleInst = &files[fd];
			if (fhandleInst->dirPtr)
			{
				if(fhandleInst->dirPtr->obj.id == dirp->obj.id){
					ret = fhandleInst->cur_entry.d_ino;
					break;
				}
			}
		}
	}
	return ret;
}

u32 flength(FILE* fh){
	if(fh != NULL){
		u32 cPos = ftell(fh);
		fseek(fh, 0, SEEK_END);
		u32 aPos = ftell(fh);
		fseek(fh, cPos, SEEK_SET);
		return aPos;		
	}
	return -1;
}

bool FAT_feof(FILE * fh){
	if( (fh != NULL) && ( (int)ftell(fh) < (int)(((int)flength(fh))-1) ) ){
		return false;	
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS /////////////////////////////////////////////////////////////////////

//returns the first free StructFD
bool buildFileClassListFromPath(char * path, struct FileClassList * lst, int startFromGivenIndex){
	//Decide wether we have a Working directory or not, if valid dir, enter that dir. If not, use the default dir and point to it. + Clear TGDS FS Directory Iterator context
	if( (isNotNullString((const char *)path) == true) && (lst != NULL) && (cleanFileList(lst) == true) && (updateTGDSFSDirectoryIteratorCWD(path, lst) == true) && (chdir(path) == 0) ){
		//rebuild filelist
		FRESULT result;
		DIR dir;
		int i = 0;
		FILINFO fno;
		result = f_opendir(&dir, (const TCHAR*)path);                       /* Open the directory */
		if (result == FR_OK) {
			for(;;){
				result = f_readdir(&dir, &fno);                   /* Read a directory item */
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
				
				if (result != FR_OK || fno.fname[0] == 0){	//Error or end of dir. No need to handle the error here.
					break;
				}
				else if(i >= FileClassItems){
					break;
				}
				else if((type == FT_FILE) || (type == FT_DIR)){
					char builtFilePath[MAX_TGDSFILENAME_LENGTH+1];
					if( path[strlen(path)] != '/' && path[strlen(path)-1] != '/' ){
						strcat(path, "/");
					}
					if(type == FT_DIR){	
						sprintf(builtFilePath,"%s%s",path,fno.fname);
					}
					else if(type == FT_FILE){
						sprintf(builtFilePath,"0:%s%s",path,fno.fname);
					}		
					//Generate FileClass from either a Directory or a File
					bool iterable = true;
					setFileClass(iterable, (char*)&builtFilePath[0], startFromGivenIndex + i, type, structfd_posixInvalidFileDirOrBufferHandle, lst);
					i++;
				}			
			}
			f_closedir(&dir);
			lst->FileDirCount = (i + startFromGivenIndex);
		}
		else{
			errno = fresultToErrno(result);
			return false;
		}
		return true;
	}
	return false;
}

//Directory Functions
bool enterDir(char* newDir, char* destinationPath){
	if((newDir != NULL) && (destinationPath != NULL)){
		strcpy(destinationPath, newDir); //update target DIR
		return true;
	}
	return false;
}

//passes the full working directory held by the CWD from within this context is called from, then, 
//removes the last directory of such context 
bool leaveDir(char* oldNewDir){
	if(oldNewDir != NULL){
		char tempnewDir[MAX_TGDSFILENAME_LENGTH+1] = {0};
		char tempnewDiroutPath[MAX_TGDSFILENAME_LENGTH+1] = {0};    //used by splitCustom function as output path buffer
		strcpy(tempnewDir, (const char *)oldNewDir);
		getLastDirFromPath(tempnewDir, TGDSDirectorySeparator, tempnewDiroutPath);
		strcpy(oldNewDir, tempnewDiroutPath);
		return true;
	}
	return false;
}

//Global Current Working Directory updated by the FileClass iterator. char * path : If a valid directory is passed, it will be used to populate the FileClassList. Otherwise the default Start Directory is used.
//Note: 
//Step 1): The Global CWD will be overriden by the char * path passed
//Step 2): If any File or Directory is found then char * path will be destroyed by the current iterated File / Directory item.
bool updateTGDSFSDirectoryIteratorCWD(char * newCWD, struct FileClassList * lst){
	if( (newCWD != NULL) && (lst != NULL) ){
		char * CurrentWorkingDirectory = (char*)&lst->TGDSCurrentWorkingDirectory[0];
		if(strlen(newCWD) > 0){
			strcpy(CurrentWorkingDirectory, (const char*)newCWD);
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS END //////////////////////////////////////////////////////////////////



///////////////////////////////////////////////TGDS FileDescriptor Callbacks Implementation Start ///////////////////////////////////////////////
//These callbacks are required when setting up initARM7FSTGDSFileHandle()	or performARM7MP2FSTestCaseTGDSFileDescriptor()
int ARM7FS_ReadBuffer_ARM9ImplementationTGDSFD(u8 * outBuffer, int fileOffset, struct fd * fdinstIn, int bufferSize){
	return TGDSFSUserfatfs_read(outBuffer, bufferSize, fileOffset, fdinstIn);
}

int ARM7FS_WriteBuffer_ARM9ImplementationTGDSFD(u8 * inBuffer, int fileOffset, struct fd * fdinstOut, int bufferSize){
	return TGDSFSUserfatfs_write(inBuffer, bufferSize, fileOffset, fdinstOut);
}

int ARM7FS_close_ARM9ImplementationTGDSFD(struct fd * fdinstOut){
	return TGDSFSUserfatfs_close(fdinstOut);
}
///////////////////////////////////////////////TGDS FileDescriptor Callbacks Implementation End ///////////////////////////////////////////////
//FATFS layer which allows to open N file handles for a file (because TGDS FS forces 1 File Handle to a file)
int TGDSFSUserfatfs_write(u8 *ptr, int len, int offst, struct fd *tgdsfd){	//(FileDescriptor :struct fd index)
	if( (tgdsfd != NULL) && (ptr != NULL) ){
		f_lseek (
				tgdsfd->filPtr,   /* Pointer to the file object structure */
				(DWORD)offst       /* File offset in unit of byte */
			);
		return fatfs_write(tgdsfd->cur_entry.d_ino, ptr, len);
	}
	return structfd_posixInvalidFileDirOrBufferHandle;
}

//read (get struct FD index from FILE * handle)
int TGDSFSUserfatfs_read(u8 *ptr, int len, int offst, struct fd *tgdsfd){
	if( (tgdsfd != NULL) && (ptr != NULL) ){
		f_lseek (
				tgdsfd->filPtr,   /* Pointer to the file object structure */
				(DWORD)offst       /* File offset in unit of byte */
			);
		return fatfs_read(tgdsfd->cur_entry.d_ino, ptr, len);
	}
	return structfd_posixInvalidFileDirOrBufferHandle;
}

//receives a new struct fd index with either DIR or FIL structure allocated so it can be closed.
//returns 0 if success, 1 if error
int TGDSFSUserfatfs_close(struct fd * tgdsfd){
	if(tgdsfd != NULL){
		return fatfs_close(tgdsfd->cur_entry.d_ino);
	}
	return structfd_posixInvalidFileDirOrBufferHandle;
}

//returns an internal index struct fd allocated. Requires DLDI enabled
int TGDSFSUserfatfs_open_file(const sint8 *pathname, char * posixFlags){
	//Copies newly alloced struct fd / Creates duplicate filehandles when opening a new file
	return fatfs_open_fileIntoTargetStructFD(pathname, posixFlags, NULL);
}

long TGDSFSUserfatfs_tell(struct fd *f){	//NULL check already outside
    return fatfs_tell(f);
}

// ret: structfd_posixInvalidFileDirOrBufferHandle if invalid posixFDStruct
//		else if POSIX retcodes (if an error happened)
//		else return new offset position (offset + current file position (internal)) in file handle
off_t TGDSFSUserfatfs_lseek(struct fd *pfd, off_t offset, int whence){	//(FileDescriptor :struct fd index)
    return fatfs_lseek(pfd->cur_entry.d_ino, offset, whence);
}

bool openDualTGDSFileHandleFromFile(char * filenameToOpen, int * tgdsStructFD1, int * tgdsStructFD2){

	if( (isNotNullString((const char *)filenameToOpen) == true) && (tgdsStructFD1 != NULL) && (tgdsStructFD2 != NULL) ){
		//Test case for 2 file handles out a single file
		int retStatus1 = TGDSFSUserfatfs_open_file((const sint8 *)filenameToOpen, (char *)"r");	//returns structFD Video File Handle as posix file descriptor (int fd) and struct fd *
		int retStatus2 = TGDSFSUserfatfs_open_file((const sint8 *)filenameToOpen, (char *)"r");	//returns structFD Audio File Handle as posix file descriptor (int fd) and struct fd *
		
		*tgdsStructFD1 = retStatus1;
		*tgdsStructFD2 = retStatus2;
		
		if( (retStatus1 != -1) && (retStatus2 != -1)){
			return true;
		}
		
		struct fd * fd1 = getStructFD(*tgdsStructFD1);
		struct fd * fd2 = getStructFD(*tgdsStructFD2); 	
		closeDualTGDSFileHandleFromFile(fd1, fd2);
	}	
	
	return false;
}

bool closeDualTGDSFileHandleFromFile(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2){
	if(tgdsStructFD1 != NULL){
		TGDSFSUserfatfs_close(tgdsStructFD1);
	}
	if(tgdsStructFD2 != NULL){
		TGDSFSUserfatfs_close(tgdsStructFD2);
	}
	return true;
}
///////////////////////////////////////////////TGDS FileDescriptor Callbacks Implementation End ///////////////////////////////////////////////

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void separateExtension(char *str, char *ext)
{
	if( (str == NULL) || (ext == NULL) ){
		return;
	}

	int x = 0;
	int y = 0;
	for(y = strlen(str) - 1; y > 0; y--)
	{
		if(str[y] == '.')
		{
			// found last dot
			x = y;
			break;
		}
		if(str[y] == '/')
		{
			// found a slash before a dot, no ext
			ext[0] = 0;
			return;
		}
	}
	
	if(x > 0)
	{
		int y = 0;
		while(str[x] != 0)
		{
			ext[y] = str[x];
			str[x] = 0;
			x++;
			y++;
		}
		ext[y] = 0;
	}
	else
		ext[0] = 0;	
}

#if defined(WIN32)
static bool cv_snprintf(char* buf, int len, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int res = vsnprintf((char *)buf, len, fmt, va);
    va_end(va);
#if defined _MSC_VER
    // maybe truncation maybe error 
    if(res < 0)
        //check for last errno 
    res = len -1;
    // ensure null terminating on VS2013	
#if def_MSC_VER<=1800
    buf[res] = 0; 
#endif
#endif
    return res >= 0 && res < len;
}

//taken from https://stackoverflow.com/questions/9052490/find-the-count-of-substring-in-string
//modified by Coto
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
static int indexParse = 0;
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int count_substr(const char *str, const char* substr, bool overlap) {
  if (strlen(substr) == 0) return -1; // forbid empty substr

  int count = 0;
  int increment = overlap ? 1 : strlen(substr);
  char* s = NULL;
  for ( s =(char*)str; (s = strstr(s, substr)); s += increment)
    ++count;
  return count;
}

typedef void(*splitCustom_fn)(const char *, size_t, char * ,int indexToLeftOut, char * delim);
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void splitCustom(const char *str, char sep, splitCustom_fn fun, char * outBuf, int indexToLeftOut)
{
    unsigned int start = 0, stop = 0;
    for (stop = 0; str[stop]; stop++) {
        if (str[stop] == sep) {
            fun(str + start, stop - start, outBuf, indexToLeftOut, &sep);
            start = stop + 1;
        }
    }
    fun(str + start, stop - start, outBuf, indexToLeftOut, &sep);
}

//this callback debugs every character separated from splitCustom()
/*
void print(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    if(indexParse != indexToLeftOut){
        char localBuf[MAX_TGDSFILENAME_LENGTH+1];
        snprintf(localBuf,len+1,"%s",str);
        nocashMessage(" %d:%s%s:%d\n", (int)len, localBuf, delim, indexParse);
        indexParse++;
    }
}
*/

//this callback builds an output path (outBuf) and filters out the desired index. (used as a trim last directory callback)
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void buildPath(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    if(indexParse != indexToLeftOut){
        if(strlen(outBuf) == 0){
            cv_snprintf(outBuf,len+2,"%s%s",str, delim);
        }
        else{
            char localBuf[MAX_TGDSFILENAME_LENGTH+1];
            sprintf(localBuf,"%s",outBuf);
            cv_snprintf(outBuf,strlen(outBuf)+len+2,"%s%s%s",localBuf,str,delim);
        }
        indexParse++;
    }
}

//this callback splits the haystack found in a stream, in the outBuf
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void splitCallback(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    cv_snprintf( ((char*)outBuf + (indexParse*256)), len+1, "%s", str);
    indexParse++;
} 

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int getLastDirFromPath(char * stream, char * haystack, char * outBuf){
    indexParse = 0;
    //leading / always src stream
    int topval = strlen(stream); 
    if(stream[topval-1] != '/'){
        stream[topval-1] = '/';
    }
    int indexToLeftOut = count_substr(stream, haystack, false);
    int indexToLeftOutCopy = indexToLeftOut;
    if(indexToLeftOutCopy > 1){ //allow index 0 to exist, so it's always left the minimum directory
        indexToLeftOutCopy--;
    }
    splitCustom(stream, (char)*haystack, buildPath, outBuf, indexToLeftOutCopy);
    //remove 0: out stream
    topval = strlen(outBuf) + 1;
    if((outBuf[0] == '0') && (outBuf[1] == ':')){
        char temp[MAX_TGDSFILENAME_LENGTH+1];
        cv_snprintf(temp,topval-2,"%s",(char*)&outBuf[2]);
        sprintf(outBuf,"%s",temp);
    }
	//remove leading / in out stream 
    topval = strlen(outBuf); 
    if(outBuf[topval-1] == '/'){
        outBuf[topval-1] = '\0';
		
		//count how many slashes there are, if zero, force dir to be "/"
		int count = 0;
		int iter = topval-1;
		while(iter >= 0){
			if(outBuf[iter] == '/'){
				count++;
			}
			iter--;
		}
		if(count == 0){
			topval = 1;
			outBuf[topval] = '\0';
		}
	}
	//edge case: the only directory was the leading / and was just removed, if so restore item
    if(topval == 1){
        outBuf[topval-1] = '/';
    }
	//edge case: remove double leading / 
	if((outBuf[0] == '/') && (outBuf[1] == '/')){
		char temp[MAX_TGDSFILENAME_LENGTH+1];
		cv_snprintf(temp,strlen(outBuf)+1-1,"%s",(char*)&outBuf[1]);	//strlen(charBuf) +1 ending char - current offset we start to copy from
		sprintf(outBuf,"%s",temp);
	}
    return indexToLeftOut;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int str_split(char * stream, char * haystack, char * outBuf, int itemSize, int blockSize){
	int i = 0;
	for(i = 0; i < itemSize; i++){
		*( outBuf + (i*blockSize) ) = '\0';
	}
	
	indexParse = 0;
    int indexToLeftOut = count_substr(stream, haystack, false);
    splitCustom(stream, (char)*haystack, splitCallback, outBuf, indexToLeftOut);
    return indexToLeftOut;
}
#endif

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
#endif
struct FileClassList * initFileList(){
	return (struct FileClassList *)TGDSARM9Malloc(sizeof(struct FileClassList));
}
#endif