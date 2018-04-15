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

//This has all new CPP code (fsfatlayerTGDSLegacy.c + fsfatlayerTGDSNew.cpp) . Projects that support C++ code link this as well.
//Projects that do NOT support C++ code (smaller footprint, simpler projects) will resort to simpler implementation @ fsfatlayerTGDSLegacy.c, so by logic, core logic should
//be dealt in C code, and extension should be in C++ code

#ifdef __cplusplus

//C++ part
using namespace std;
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iterator>
#include <string>

#include "fsfatlayerTGDSNew.h"
#include "fsfatlayerTGDSLegacy.h"
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

/////////////////////////////////////////misc directory functions////////////////////////////////////////////////////

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
		FileClass FileClassInst = getEntryFromGlobalListByIndex((CurrentFileDirEntry - 1)); 
		FILINFO finfo = getFileFILINFOfromFileClass(&FileClassInst);
		libfatAttributes = (uint8)gccnewlibnano_to_fsfat2libfatAttrib((int)finfo.fattrib);
	}
	return libfatAttributes;
}

u8 FAT_SetFileAttributes (const char* filename, u8 attributes, u8 mask){
	u8	libfatAttributesIn = 0;
	u8	libfatAttributesOut= 0;	
	FileClass FileClassInst = getFirstFileEntryFromPath((char *)filename); 	//get entry from path
	FILINFO finfo = getFileFILINFOfromFileClass(&FileClassInst);
	libfatAttributesIn = (uint8)gccnewlibnano_to_fsfat2libfatAttrib((int)finfo.fattrib);
	libfatAttributesOut = (libfatAttributesIn & ~(mask & 0x27)) | (attributes & 0x27);
	int	NEWgccnewlibnano_to_fsfatAttributes = libfat2gccnewlibnano_to_fsfatAttrib((int)libfatAttributesOut);
	int NEWmask = libfat2gccnewlibnano_to_fsfatAttrib((int)mask);
	Setgccnewlibnano_to_fsfatAttributesToPath((char*)filename, NEWgccnewlibnano_to_fsfatAttributes, NEWmask);
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

std::list<FileClass> buildListFromPath(char * path){
	FRESULT res;
    DIR dir;
    sint32 i = 0;
    FILINFO fno;
	std::list<FileClass> CurrentFileList;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            
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
			(fno.fattrib & AM_DIR)
			||
			(fno.fattrib & AM_ARC)
			){
				type = FT_FILE;			
			}
			else{	/* It is Invalid. */
				type = FT_NONE;
			}
			CurrentFileList.push_back(FileClass(i,std::string(fno.fname),std::string(path),type));
			i++;
		}
        f_closedir(&dir);
    }
	return CurrentFileList;
}

std::list<FileClass> * GlobalFileList = NULL;
std::list<FileClass> * InitGlobalFileList(){
	return (std::list<FileClass> *) new std::list<FileClass>;
}

void DeInitGlobalFileList(std::list<FileClass> * List){
	delete List;
}

void updateGlobalListFromPath(char * path){
	
	//Update last path (destroys the last one)
	updateLastGlobalPath(path);
	
	if(GlobalFileList){
		DeInitGlobalFileList(GlobalFileList);
	}
	GlobalFileList = InitGlobalFileList();
	std::list<FileClass> FileList = buildListFromPath(path);	//Do copy here (safer)
	std::list<FileClass>::iterator it;
	for (it=FileList.begin(); it!=FileList.end(); ++it){
		FileClass fileInst = *it;
		GlobalFileList->push_back(fileInst);
	}
}


std::string buildFullPathFromFileClass(FileClass * FileClassInst){
	char FullPath[MAX_TGDSFILENAME_LENGTH] = {0};
	std::string PathFix = std::string(getfatfsPath(""));
	PathFix.erase(PathFix.length()-1);
	
	std:string fname = string(FileClassInst->getfilename());
	//if getfilename/path has no leading / add one
	if ( (fname.find("/") == string::npos) && (FileClassInst->getpath().find("/") == string::npos)){
		char TempName[MAX_TGDSFILENAME_LENGTH] = {0};
		sprintf(TempName,"/%s",fname.c_str());
		fname = string(TempName);
	}
	else{
		//At least one of them has a leading / which is used below
		
		//but if both have /, then ... 
		if ( (fname.find("/") != string::npos) && (FileClassInst->getpath().find("/") != string::npos)){
			fname.erase(0,1);	//Delete the first 1 char(s) from filename
		}
	}
	sprintf(FullPath,"%s%s%s",PathFix.c_str(),FileClassInst->getpath().c_str(),fname.c_str());
	std::string FullPathStr = std::string(FullPath);
	return FullPathStr;
}

FILINFO getFileFILINFOfromFileClass(FileClass * FileClassInst){
	std::string FullPathStr = buildFullPathFromFileClass(FileClassInst);
	FILINFO finfo;
	FRESULT result;
	sint32 fd = -1;
	struct fd * fdinst = NULL;
	FILE * FileInst = fopen(FullPathStr.c_str(),"r");
	if(FileInst){
		fd = fileno(FileInst);
		fdinst = fd_struct_get(fd);
		if(fdinst->filPtr){
			result = f_stat(FullPathStr.c_str(), &finfo);
			if (result == FR_OK){
				//ok works correctly
			}
			else{
				//finfo fill error
			}
		}
		else{
			//filPtr open ERROR
		}
		fclose(FileInst);
	}
	else{
		//fopen failed
	}
	return finfo;
}

vector<string> splitCustom(string str, string token){
    vector<string>result;
    while(str.size()){
        int index = str.find(token);
        if(index!=string::npos){
            result.push_back(str.substr(0,index));
            str = str.substr(index+token.size());
            if(str.size()==0)result.push_back(str);
        }else{
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

//Type will tell us if this file exists or not(FT_NONE == no, FT_FILE == yes)
FileClass getFirstFileEntryFromPath(char * path){
	FILE * FileHandle = fopen(path,"r");
	FileClass FileInst(InvalidFileListIndex, std::string(""), std::string(""), FT_NONE);
	vector<string> vecOut = splitCustom(std::string(path),std::string("/"));	//path == "0:/folder/file.bin"
    int fileIndex = vecOut.size() - 1;
	std::string Filename = vecOut.at(fileIndex);
	std::string Path;
	if(fileIndex > 1){
	    int PathIndex = fileIndex;  //File is excluded
	    int i = 0;
	    for(i = 0; i < PathIndex; i++){
	        char temp[512] = {0};
	        sprintf(temp,"%s%s",vecOut.at(i).c_str(),"/");
	        Path+= temp;
	    }
	}
	if(FileHandle){
		FileInst.settype(FT_FILE);
		FileInst.setfilename(Filename);
		FileInst.setpath(Path);
		fclose(FileHandle);
	}
	return FileInst;
}

FileClass getEntryFromGlobalListByIndex(int EntryIndex){
	FileClass FileInst(0, std::string(""), std::string(""), 0);
	if(EntryIndex > (GlobalFileList->size() - 1)){
		return FileInst;
	}
	std::list<FileClass>::iterator it = GlobalFileList->begin();
    std::advance(it, EntryIndex);
	FileInst = *it;
	return FileInst;
}

//Note: Requires a fresh call to updateGlobalListFromPath prior to calling this
FileClass getFirstDirEntryFromGlobalList(){
	int CurFileDirIndex = 0;
	FileClass FileInst(0, std::string(""), std::string(""), 0);
	std::list<FileClass>::iterator it;
	for (it=GlobalFileList->begin(); it!=GlobalFileList->end(); ++it){
		if((*it).gettype() == FT_DIR){
			FileInst = *it;
			break;
		}
		CurFileDirIndex++;
	}
	CurrentFileDirEntry = CurFileDirIndex;	//CurrentFileDirEntry is relative to getFirstDirEntryFromGlobalList() now
	return FileInst;
}

//Note: Requires a fresh call to updateGlobalListFromPath prior to calling this
FileClass getFirstFileEntryFromGlobalList(){
	int CurFileDirIndex = 0;
	FileClass FileInst(0, std::string(""), std::string(""), 0);
	std::list<FileClass>::iterator it;
	for (it=GlobalFileList->begin(); it!=GlobalFileList->end(); ++it){
		if((*it).gettype() == FT_FILE){
			FileInst = *it;
			break;
		}
		CurFileDirIndex++;
	}
	CurrentFileDirEntry = CurFileDirIndex;	//CurrentFileDirEntry is relative to getFirstFileEntryFromGlobalList() now
	return FileInst;
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
	if (!(strcmp(lastCurrentPath, path) == 0))
	{
		updateLastGlobalPath(path);
	}
	
	//lastCurrentPath is globally accesible by all code. But updated only in getFirstFile (getNextFile just retrieves the next ptr file info)
	updateGlobalListFromPath(lastCurrentPath);
	CurrentFileDirEntry = 0;
	
	//FileClass fileInst = getFirstDirEntryFromGlobalList();					//get First directory entry	:	so it generates a valid DIR CurrentFileDirEntry
	//FileClass fileInst = getFirstFileEntryFromGlobalList();					//get First file entry 		:	so it generates a valid FILE CurrentFileDirEntry
	FileClass fileInst = getEntryFromGlobalListByIndex(CurrentFileDirEntry);	//get First file/directory	:	can be file/dir/none(invalid)
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(&fileInst);				//actually open the file and check attributes
	
	if (FILINFOObj.fattrib & AM_DIR) {	//dir
		LastDirEntry=CurrentFileDirEntry;
	}
	else if (	//file
	(FILINFOObj.fattrib & AM_RDO)
	||
	(FILINFOObj.fattrib & AM_HID)
	||
	(FILINFOObj.fattrib & AM_SYS)
	||
	(FILINFOObj.fattrib & AM_DIR)
	||
	(FILINFOObj.fattrib & AM_ARC)
	){
		std::string FullPathStr = buildFullPathFromFileClass(&fileInst);	//must store proper filepath
		setLFN((char*)FullPathStr.c_str());		//update last full path access
		getLFN((char*)path);					//update source path
		
		LastFileEntry=CurrentFileDirEntry;
	}
	else{	//invalid
	}
	
	//increase the file counter after operation
	if(CurrentFileDirEntry < (GlobalFileList->size()-1)){ 
		CurrentFileDirEntry++;	
	}
	else{
		CurrentFileDirEntry = 0;
		LastDirEntry=InvalidFileDirEntry;
		LastFileEntry=InvalidFileDirEntry;
		return FT_NONE;	//actually end of list
	}
	return fileInst.gettype();
}

//requires fullpath of the CURRENT file, it will return the next one
//return:  FT_DIR or FT_FILE: use getLFN(char buf[MAX_TGDSFILENAME_LENGTH+1]); to receive full first file
//			or FT_NONE if invalid file
int getNextFile(char * path){
	
	FileClass fileInst = getEntryFromGlobalListByIndex(CurrentFileDirEntry);	//get next FILE listed
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(&fileInst);			//actually open the file and check attributes (rather than read dir contents)
	
	if(fileInst.gettype() == FT_DIR){
		LastDirEntry=CurrentFileDirEntry;
	}
	else if(fileInst.gettype() == FT_FILE){
		std::string FullPathStr = buildFullPathFromFileClass(&fileInst);
		setLFN((char*)FullPathStr.c_str());		//update last full path access
		getLFN((char*)path);					//update source path
		LastFileEntry=CurrentFileDirEntry;
	}
	else{	//invalid
	}
	
	//increase the file counter after operation
	if(CurrentFileDirEntry < (GlobalFileList->size()-1)){ 
		CurrentFileDirEntry++;	
	}
	else{
		CurrentFileDirEntry = 0;
		LastDirEntry=InvalidFileDirEntry;
		LastFileEntry=InvalidFileDirEntry;
		return FT_NONE;	//actually end of list
	}
	return fileInst.gettype();
}

//FAT_GetAlias
//Get the alias (short name) of the last file or directory entry read
//char* alias OUT: will be filled with the alias (short filename),
//	should be at least 13 bytes long
//bool return OUT: return true if successful

bool FAT_GetAlias(char* alias)
{
	if (alias == NULL)
	{
		return false;
	}
	int CurEntry = InvalidFileDirEntry;
	if(LastFileEntry > LastDirEntry){
		CurEntry = LastFileEntry;
	}
	else{
		CurEntry = LastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == InvalidFileDirEntry){
		return false;
	}
	FileClass fileInst = getEntryFromGlobalListByIndex(CurEntry);	//By current file/directory index
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(&fileInst);			//actually open the file and check attributes (rather than read dir contents)
	
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
		sprintf((char*)alias,"%s",fileInst.getfilename().c_str());					//update source path using short file/directory name
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
bool FAT_GetLongFilename(char* Longfilename)
{
	if (Longfilename == NULL){
		return false;
	}
	
	int CurEntry = InvalidFileDirEntry;
	if(LastFileEntry > LastDirEntry){
		CurEntry = LastFileEntry;
	}
	else{
		CurEntry = LastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == InvalidFileDirEntry){
		return false;
	}
	FileClass fileInst = getEntryFromGlobalListByIndex(CurEntry);	//By current file/directory index
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(&fileInst);			//actually open the file and check attributes (rather than read dir contents)
	
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
		std::string FullPathStr = buildFullPathFromFileClass(&fileInst);	//must store proper filepath
		sprintf((char*)Longfilename,"%s",FullPathStr.c_str());					//update source path using Long file/directory name
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
u32 FAT_GetFileSize(void)
{
	u32 fileSize = 0;
	FileClass fileInst = getEntryFromGlobalListByIndex(LastFileEntry);	//get last FILE opened (by firstFile/ nextFile)
	FILINFO FILINFOObj = getFileFILINFOfromFileClass(&fileInst);			//actually open the file and check attributes (rather than read dir contents)
	
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
u32 FAT_GetFileCluster(void)
{
	u32 FirstClusterFromLastFileOpen = -1;
	FileClass fileInst = getEntryFromGlobalListByIndex(LastFileEntry);	//get last FILE opened (by firstFile/ nextFile)
	std::string FullPathStr = buildFullPathFromFileClass(&fileInst);	//must store proper filepath
	FILE * f = fopen(FullPathStr.c_str(),"r");
	sint32 fd = -1;
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
void FAT_DisableWriting (void)
{
	disableWriting = true;
}

#endif