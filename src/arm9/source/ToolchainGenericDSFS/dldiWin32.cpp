#if defined(WIN32) || !defined(ARM9)

#include <stdio.h>
#include <string.h>
#include "dldiWin32.h"
#include "TGDSTypes.h"
#include "ff.h"
#include "fs.h"
#include "winDir.h"
#include "fatfslayerTGDS.h"
#include "..\snes9x\snemulds_memmap.h"
#if defined(WIN32)
FILE * virtualDLDIDISKImg = NULL;
u8 _io_dldi_stub[16384];
#endif

const uint32  DLDI_MAGIC_NUMBER = 
	0xBF8DA5ED;	
	
// Stored backwards to prevent it being picked up by DLDI patchers
const sint8 DLDI_MAGIC_STRING_BACKWARDS [DLDI_MAGIC_STRING_LEN] =
	{'\0', 'm', 'h', 's', 'i', 'h', 'C', ' '} ;


// 2/2 : once DLDI has been setup
struct DLDI_INTERFACE* dldiGet(void) {
	#if defined(WIN32)
	return (struct DLDI_INTERFACE*)&_io_dldi_stub[0];
	#endif
	#if !defined(WIN32)
	return NULL;
	#endif
}

#if defined(WIN32)
bool __dsimode = false; //always NTR mode for debugging purposes
#endif

#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_init() {
	#if defined(WIN32)
	fseek(virtualDLDIDISKImg, 0, SEEK_SET);
	int res = ftell(virtualDLDIDISKImg);
	if(res != 0){
		return false;
	}
	#endif	
	return true;
}

#ifdef ARM9
__attribute__ ((optnone))
#endif
void dldi_handler_deinit() {
	
}

//////////////////////////////////////////////// RAM Disk DLDI Implementation End ///////////////////////////////////////////
//future optimization, make it EWRAM-only so we can DMA directly!
#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer) {
	#if defined(WIN32)
	fseek(virtualDLDIDISKImg, 512*sector, SEEK_SET);
	int fetch = fread(buffer, 1, 512*numSectors, virtualDLDIDISKImg);
	if(fetch != (512*numSectors)){
		return false;
	}
	#endif
	return true;
}

#ifdef ARM9
__attribute__ ((optnone))
#endif
bool dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer) {
	#if defined(WIN32)
	fseek(virtualDLDIDISKImg, 512*sector, SEEK_SET);
	int fetch = fwrite((void*)buffer, 1, 512*numSectors, virtualDLDIDISKImg);
	if(fetch != (512*numSectors)){
		return false;
	}
	#endif
	return true;
}

void initDLDISnemulDS(char * snesFileName) {
	//Init DLDI
	char cwdPath[256];
	getCWDWin(cwdPath, "\\snes9x\\fatfs\\virtualDLDI\\");
	printf("dir: %s\n", cwdPath);

	std::vector<std::string> virtualDiskImgFiles = findFiles(std::string(cwdPath), std::string("img"));
	std::string virtualDiskImgFile = virtualDiskImgFiles.at(0);

	std::vector<std::string> virtualDldiFiles = findFiles(std::string(cwdPath), std::string("dldi"));
	std::string virtualDldiFile = virtualDldiFiles.at(0);

	virtualDLDIDISKImg = fopen(virtualDiskImgFile.c_str(), "rb+");
	FILE * virtualDLDIFH = fopen(virtualDldiFile.c_str(), "rb");

	if ((virtualDLDIDISKImg != NULL) && (virtualDLDIFH != NULL)) {
		printf("file: %s open OK\n\n\n\n", virtualDiskImgFile.c_str());
		printf("file: %s open OK\n\n\n\n", virtualDldiFile.c_str());

		if (fread((char*)&_io_dldi_stub[0], 1, sizeof(_io_dldi_stub), virtualDLDIFH) == sizeof(_io_dldi_stub)) {
			printf("read DLDI OK.");

			//Initialize TGDS FS
			int ret = FS_init();
			if (ret == 0)
			{
				printf("FS Init ok.");

				//DLDI tasks

				//sort list alphabetically
				struct FileClassList * playlistfileClassListCtx = NULL;
				playlistfileClassListCtx = initFileList();
				cleanFileList(playlistfileClassListCtx);
				bool ret = readDirectoryIntoFileClass("/", playlistfileClassListCtx);
				if (ret == true) {
					bool ignoreFirstFileClass = true;
					sortFileClassListAsc(playlistfileClassListCtx, ignoreFirstFileClass);
				}
				else {
					printf("fail");
				}

				//remove("0:/fileOut.tvs");

				//copy fileOut.tvs to disk image
				int tgdsfd = -1;
				//int res = TGDSFSUserfatfs_open_file("0:/fileOut.tvs", "w+", &tgdsfd);
				//if(res >= 0){
				//	struct fd * fdOpened = getStructFD(tgdsfd);
				//	//char bufferTestToRead[512];
				//	//int read = ARM7FS_ReadBuffer_ARM9ImplementationTGDSFD((u8*)&bufferTestToRead[0], 0, fdOpened, 100);
				//	//printf("%s ", bufferTestToRead);
				//	int fileOffset = 0;
				//	//int writtenTest = fatfs_write(fdOpened->cur_entry.d_ino, (u8 *)TVSFileStream, TVSFileSize);
				//	fsync(tgdsfd);
				//	int res = TGDSFSUserfatfs_close(fdOpened);
				//	printf("");
				//}

				//read it
				/*
				tgdsfd = -1;
				char * tvsFile = "0:/Mega Man X3 (USA).sfc";

				fatfs_open_fileIntoTargetStructFD(tvsFile, "r", &tgdsfd, NULL);
				videoHandleFD = getStructFD(tgdsfd);
				
				if (videoHandleFD != NULL) {
					printf("SNES File : %s found!", tvsFile);
				}
				else {
					printf("SNES File : %s not found ", tvsFile);
					while (1 == 1) {
					}
				}
				*/

				//find SNES file name in DLDI
				string NDSFileInDLDI = getFileName(snesFileName, true);
				char NDSFileInDLDIChar[MAX_PATH];
				strcpy(NDSFileInDLDIChar, "0:/");
				strcat(NDSFileInDLDIChar, NDSFileInDLDI.c_str());
				if (FT_FILE == FileExists((char*)NDSFileInDLDIChar)) {
					unsigned char *ROM = (unsigned char*)malloc(4*1024*1024);
					FS_loadROMForPaging(ROM, (char*)NDSFileInDLDIChar, PAGE_SIZE_HIROM);
					free(ROM);
				}
				else {
					printf("file NOT exists in DLDI Disk Image: %s", NDSFileInDLDIChar);
				}
			}
			else
			{
				printf("FS Init error.");
			}
		}
		else {
			printf("read DLDI FAIL");
		}

		//ROM streams from DLDI now, keep open
		//fclose(virtualDLDIDISKImg);
		//fclose(virtualDLDIFH);
	}
	else {
		printf("failure opening: %s", virtualDiskImgFile.c_str());
		printf("failure opening: %s", virtualDldiFile.c_str());
	}
}
#endif
