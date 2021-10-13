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

#ifdef ARM9
#include "fileBrowse.h"
#include "loader.h"

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool ShowBrowser(char * Path, char * outBuf){
	scanKeys();
	while((keysDown() & KEY_START) || (keysDown() & KEY_A) || (keysDown() & KEY_B)){
		scanKeys();
		IRQWait(0, IRQ_VBLANK);
	}
	
	//Create TGDS Dir API context
	struct FileClassList * fileClassListCtx = initFileList();
	cleanFileList(fileClassListCtx);
	
	//Use TGDS Dir API context
	int pressed = 0;
	struct FileClass filStub;
	{
		filStub.type = FT_FILE;
		strcpy(filStub.fd_namefullPath, "");
		filStub.isIterable = true;
		filStub.d_ino = -1;
		filStub.curIndexInsideFileClassList = 0;
		filStub.parentFileClassList = fileClassListCtx;
	}
	char curPath[MAX_TGDSFILENAME_LENGTH+1];
	strcpy(curPath, Path);
	setFileClassObj(0, (struct FileClass *)&filStub, fileClassListCtx);
	
	int j = 1;
	int startFromIndex = 1;
	struct FileClass * fileClassInst = NULL;
	fileClassInst = FAT_FindFirstFile(curPath, fileClassListCtx, startFromIndex);
	
	//Sort list alphabetically
	bool ignoreFirstFileClass = true;
	sortFileClassListAsc(fileClassListCtx, (char**)ARM7_PAYLOAD, ignoreFirstFileClass);
	
	//actual file lister
	clrscr();
	
	j = 1;
	pressed = 0 ;
	int lastVal = 0;
	bool reloadDirA = false;
	bool reloadDirB = false;
	char * newDir = NULL;
	
	#define itemsShown (int)(15)
	int curjoffset = 0;
	int itemRead=1;
	
	while(1){
		int fileClassListSize = getCurrentDirectoryCount(fileClassListCtx) + 1;	//+1 the stub
		int itemsToLoad = (fileClassListSize - curjoffset);
		
		//check if remaining items are enough
		if(itemsToLoad > itemsShown){
			itemsToLoad = itemsShown;
		}
		
		while(itemRead < itemsToLoad ){		
			if(getFileClassFromList(itemRead+curjoffset, fileClassListCtx)->type == FT_DIR){
				printfCoords(0, itemRead, "--- %s >%d",getFileClassFromList(itemRead+curjoffset, fileClassListCtx)->fd_namefullPath, TGDSPrintfColor_Yellow);
			}
			else{
				printfCoords(0, itemRead, "--- %s",getFileClassFromList(itemRead+curjoffset, fileClassListCtx)->fd_namefullPath);
			}
			itemRead++;
		}
		
		scanKeys();
		pressed = keysDown();
		if (pressed&KEY_DOWN && (j < (itemsToLoad - 1) ) ){
			j++;
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysDown();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		//downwards: means we need to reload new screen
		else if(pressed&KEY_DOWN && (j >= (itemsToLoad - 1) ) && ((fileClassListSize - curjoffset - itemRead) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysDown();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		//LEFT, reload new screen
		else if(pressed&KEY_LEFT && ((curjoffset - itemsToLoad) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset - itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_LEFT){
				scanKeys();
				pressed = keysDown();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		//RIGHT, reload new screen
		else if(pressed&KEY_RIGHT && ((fileClassListSize - curjoffset - itemsToLoad) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_RIGHT){
				scanKeys();
				pressed = keysDown();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		else if (pressed&KEY_UP && (j > 1)) {
			j--;
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysDown();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		//upwards: means we need to reload new screen
		else if (pressed&KEY_UP && (j <= 1) && (curjoffset > 0) ) {
			//list only the remaining items
			clrscr();
			
			curjoffset--;
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysDown();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		//reload DIR (forward)
		else if( (pressed&KEY_A) && (getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_DIR) ){
			struct FileClass * fileClassChosen = getFileClassFromList(j+curjoffset, fileClassListCtx);
			newDir = fileClassChosen->fd_namefullPath;
			reloadDirA = true;
			break;
		}
		
		//file chosen
		else if( (pressed&KEY_A) && (getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_FILE) ){
			break;
		}
		
		//reload DIR (backward)
		else if(pressed&KEY_B){
			reloadDirB = true;
			break;
		}
		
		// Show cursor
		printfCoords(0, j, "*");
		if(lastVal != j){
			printfCoords(0, lastVal, " ");	//clean old
		}
		lastVal = j;
	}
	
	//enter a dir
	if(reloadDirA == true){
		//Enter to dir in Directory Iterator CWD
		enterDir((char*)newDir, Path);
		
		//Free TGDS Dir API context
		freeFileList(fileClassListCtx);
		return true;
	}
	
	//leave a dir
	if(reloadDirB == true){
		//Rewind to preceding dir in Directory Iterator CWD
		leaveDir(Path);
		//Free TGDS Dir API context
		freeFileList(fileClassListCtx);
		return true;
	}
	
	strcpy((char*)outBuf, getFileClassFromList(j+curjoffset, fileClassListCtx)->fd_namefullPath);
	clrscr();
	printf("                                   ");
	if(getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_DIR){
		//printf("you chose Dir:%s",outBuf);
	}
	else if(getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_FILE){
		//printf("you chose Dir:%s",outBuf);
	}
	
	//Free TGDS Dir API context
	freeFileList(fileClassListCtx);
	return false;
}

#endif