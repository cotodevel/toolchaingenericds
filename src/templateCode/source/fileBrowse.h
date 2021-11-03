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

#ifndef __fileBrowse_h__
#define __fileBrowse_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "keypadTGDS.h"
#include "InterruptsARMCores_h.h"
#include "limitsTGDS.h"
#include "fatfslayerTGDS.h"

#define ARM7_PAYLOAD (u32)((int)0x02400000 - 0x18000)

static inline void parseDirNameTGDS(char * dirName){
	int dirlen = strlen(dirName);
	if(dirlen > 2){
	    int i = 0;
		
		//trim the starting / if it has one
		if ( (dirName[0] == '/') && (dirName[1] == '/') ) {
			char fixDir[MAX_TGDSFILENAME_LENGTH+1];
		    strcpy(fixDir, (char*)&dirName[1]);
			strcpy(dirName, "");	//clean
            strcpy(dirName, fixDir);
		}
		char tempDir[MAX_TGDSFILENAME_LENGTH+1];
		strcpy(tempDir, dirName);
		
		if(tempDir[strlen(tempDir)-1]== '/'){
		   tempDir[strlen(tempDir)-1] = '\0'; 
		}
		strcpy(dirName, "");	//clean
		strcpy(dirName, tempDir);
	}
	
}

static inline void parsefileNameTGDS(char * fileName){
	int filelen = strlen(fileName) + 1;
	if(filelen > 4){
	    if (fileName[0] == '/') {
			char fixDir[MAX_TGDSFILENAME_LENGTH+1];       //trim the starting / if it has one
		    memset(fixDir, 0, sizeof(fixDir));
		    strcpy(fixDir, (char*)&fileName[1]);
		    memset(fileName, 0, strlen(fileName));
            strcat(fileName, fixDir);
		}
		if ((fileName[2] == '/') && (fileName[3] == '/')) {
		    char fixDir[MAX_TGDSFILENAME_LENGTH+1];       //trim the starting // if it has one (since getfspath appends 0:/)
		    memset(fixDir, 0, strlen(fixDir));
		    fixDir[0] = fileName[0];
		    fixDir[1] = fileName[1];
		    fixDir[2] = fileName[2];
		    strcat(fixDir, (char*)&fileName[4]);
		    strcpy(fileName, fixDir);
		    
		    if(fileName[2] != '/'){	//if we trimmed by accident the only leading / such as 0:filename instead of 0:/filename, restore it so it becomes the latter
				filelen = strlen(fileName)+1;
				char tmpBuf[2];
				tmpBuf[0] = fileName[0];
				tmpBuf[1] = fileName[1];
				
				char fixFile[MAX_TGDSFILENAME_LENGTH+1];       //trim the starting / if it has one
		        memset(fixFile, 0, sizeof(fixFile));
		        strcat(fixFile, tmpBuf);
		        strcat(fixFile, "/");
		        strcat(fixFile, (char*)&fileName[3]);
			}
		}
	}
}

//a variation
static inline void removeLastPath2(char * inPath, char * outPath){
    int len = strlen(inPath);
    if (len < 3){
        outPath[0] = '/';
        outPath[1] = '\0';
        return;
    }
    //remove trailing "/"
    if(inPath[len-1] == '/'){
        inPath[len-1] = '\0';
    }
    int i = 0;
    //remove leading "//"
    if((inPath[i] == '/') && (inPath[i + 1] == '/')){
        i++;
    }
    
    //copy
    while(i < len){
        outPath[i] = inPath[i];
        i++;
    }
    outPath[i] = '\0';
    
    //get and strip last delta
    int wordsToRemove = 0;
    while(outPath[len-wordsToRemove] != '/'){
        wordsToRemove++;
    }
    outPath[len-wordsToRemove] = '\0';
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern bool ShowBrowser(char * Path, char * outBuf);

#ifdef __cplusplus
}
#endif

#endif