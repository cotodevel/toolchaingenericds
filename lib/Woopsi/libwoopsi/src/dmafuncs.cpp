#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "dmafuncs.h"
#include "nds_cp15_misc.h"
#include "dmaTGDS.h"

#ifdef USING_SDL

void woopsiDmaCopy(const u16* source, u16* dest, u32 count) {
	for (u32 i = 0; i < count; i++) {
		*(dest + i) = *(source + i);
	}
}

void woopsiDmaFill(u16 fill, u16* dest, u32 count) {
	for (u32 i = 0; i < count; i++) {
		*(dest + i) = fill;
	}
}

#else

void woopsiDmaCopy(const u16* source, u16* dest, u32 count) {

	// Get memory addresses of source and destination
	u32 srca = (u32)source;
	u32 dsta = (u32)dest;

	// Precalculate the size of a single framebuffer for speed
	u32 bmpSize = SCREEN_WIDTH * SCREEN_HEIGHT * 2;

	// Precalculate boundaries of framebuffer VRAM
	u32 bmp[4];
	bmp[0] = 0x06000000;
	bmp[1] = 0x06000000 + bmpSize;
	bmp[2] = 0x06200000;
	bmp[3] = 0x06200000 + bmpSize;

	// Use DMA hardware if both source and destination are within VRAM
	if (((dsta >= bmp[0]) && (dsta < bmp[1])) ||
		((dsta >= bmp[2]) && (dsta < bmp[3]))) {

		// libnds DMA functions work in bytes
		count *= 2;

		coherent_user_range_by_size((uint32)source, count);
		
		// Choose fastest DMA copy mode
		if((srca|dsta|count) & 3){
			dmaTransferHalfWord(3, (u32)source, (u32)dest, (uint32)count);
		}
		else{
			dmaTransferWord(3, (u32)source, (u32)dest, (uint32)count);
		}
		return;
	}

	// Cannot use DMA as not working exclusively with VRAM
	// Use for-loop instead
	for (u32 i = 0; i < count; i++) {
		*(dest + i) = *(source + i);
	}
}

void woopsiDmaFill(u16 fill, u16* dest, u32 count) {

	// Draw initial pixel
	*dest = fill;

	// Stop copying if there are no more pixels to draw
	if (count > 1) {

		u32 dsta = (u32)dest + 1;

		// Precalculate the size of a single framebuffer for speed
		u32 bmpSize = SCREEN_WIDTH * SCREEN_HEIGHT * 2;

		// Precalculate boundaries of framebuffer VRAM
		u32 bmp[4];
		bmp[0] = 0x06000000;
		bmp[1] = 0x06000000 + bmpSize;
		bmp[2] = 0x06200000;
		bmp[3] = 0x06200000 + bmpSize;

		// Use DMA hardware if destination is within VRAM
		if (((dsta >= bmp[0]) && (dsta < bmp[1])) ||
			((dsta >= bmp[2]) && (dsta < bmp[3]))) {

			// libnds DMA functions work in bytes
			count *= 2;

			if((dsta|count) & 3){
				dmaFillHalfWord(3, (uint32)fill, (uint32)dest,(uint32)count);
			}
			else{
				dmaFillWord(3, (uint32)fill, (uint32)dest,(uint32)count);
			}
			return;
		}
	}

	// Cannot use DMA as not working exclusively with VRAM
	// Use for-loop instead
	for (u32 i = 0; i < count; i++) {
		*(dest + i) = fill;
	}
}

void parseDirNameTGDS(char * dirName){
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

void parsefileNameTGDS(char * fileName){ //todo: pass the buffer size here!!
	int filelen = strlen(fileName) + 1;
	if(filelen > 4){
		if ((fileName[2] == '/') && (fileName[3] == '/')) {
		    int offset = 2;
			//copy
			while(fileName[offset] == '/'){
				offset++;
			}

			char fixName[MAX_TGDSFILENAME_LENGTH+1];       //trim the starting // if it has one (since getfspath appends 0:/)
		    memset(fixName, 0, sizeof(fixName));
		    strcpy(fixName, getfatfsPath((char*)&fileName[offset]));
		    memset(fileName, 0, 256);
			strcpy(fileName, fixName);
		}
	}
}

#endif
