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

#ifdef __cplusplus

#ifndef __fsfatlayerTGDSNew_h__
#define __fsfatlayerTGDSNew_h__

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

#include "fsfatlayerTGDSLegacy.h"

//C++: Shared File Handling for misc directory functions
class FileClass
{
  public:
    int Index;
	std::string filename;
    std::string path;
    int type = 0;	//FT_DIR / FT_FILE / FT_NONE	//  setup on Constructor / updated by getFileFILINFOfromPath(); / must be init from the outside 
    // Constructor
    FileClass(int indexInst, std::string filenameInst, std::string pathInst, int typeInst)
	{
		Index = indexInst;
		filename = filenameInst;
		path = pathInst;
		type = typeInst;
	}
	
	//helpers if/when Constructor is not available
	int getindex()
    {
		return Index;
    }
    std::string getfilename()
    {
		return filename;
    }
	std::string getpath()
    {
		return path;
    }
	int gettype()
    {
		return type;
    }
	
	void setindex(int IndexInst){
		Index = IndexInst;
	}
	void setfilename(std::string filenameInst){
		filename = filenameInst;
	}
	void setpath(std::string pathInst){
		path = pathInst;
	}
	void settype(int typeInst){
		type = typeInst;
	}
};


//misc directory functions

//User
extern int 	FAT_FindFirstFile(char* filename);
extern int 	FAT_FindNextFile(char* filename);
extern u8 	FAT_GetFileAttributes(void);
extern u8 FAT_SetFileAttributes (const char* filename, u8 attributes, u8 mask);

//Internal
extern char lfnName[MAX_TGDSFILENAME_LENGTH+1];
extern char curDirListed[MAX_TGDSFILENAME_LENGTH+1];
extern struct fd fdCur;
extern bool FAT_GetLongFilename(char* filename);
extern int getFirstFile(char * path);
extern void updateGlobalListFromPath(char * path);
extern int getNextFile(char * path);
extern bool getLFN(char* filename);
extern bool setLFN(char* filename);
extern int CurrentFileDirEntry;
extern FileClass getFirstFileEntryFromPath(char * path);
extern FILINFO getFileFILINFOfromFileClass(FileClass * FileClassInst);
extern std::list<FileClass> * GlobalFileList;
extern std::list<FileClass> * InitGlobalFileList();
extern void DeInitGlobalFileList(std::list<FileClass> * List);
extern std::list<FileClass> buildListFromPath(char * path);
extern FileClass getEntryFromGlobalListByIndex(int EntryIndex);
extern FileClass getFirstDirEntryFromGlobalList();
extern FileClass getFirstFileEntryFromGlobalList();
extern std::string buildFullPathFromFileClass(FileClass * FileClassInst);
extern vector<string> splitCustom(string str, string token);

#endif
#endif