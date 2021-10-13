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

#ifdef ARM9

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "keypadTGDS.h"
#include "InterruptsARMCores_h.h"
#include "limitsTGDS.h"
#include "fatfslayerTGDS.h"

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

#endif

#ifdef __cplusplus
}
#endif
