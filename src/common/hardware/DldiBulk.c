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

#if defined(WIN32)
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)

//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma error(disable:2143)

#endif

#include "DldiBulk.h"

#ifdef ARM9
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "limitsTGDS.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "dmaTGDS.h"
#include "dldi.h"
#include "lzss9.h"
#include "ipcfifoTGDS.h"
#include "nds_cp15_misc.h"
#endif

#if defined(WIN32)
#include "winDir.h"
#include "..\..\..\..\ToolchainGenericDSFS\fatfslayerTGDS.h"
#include "TGDSTypes.h"
#include "..\..\..\..\ToolchainGenericDSFS\dldi.h"
#endif

#if defined(WIN32) || defined(ARM9)
//Returns: clusterCount (each pointing to a start sector) allocated.
//Filesize = (clusterCount * sectorSize * sectorsPerCluster)
int DldiBulkReadSetupFromTGDSVideoFile(struct fd * videoHandleFD, u32* sector_table){
	//Generate cluster filemap
	int clusterCount=0;
	uint32_t* sector_tablePtr = sector_table;
	uint32_t  cur_cluster = getStructFDFirstCluster(videoHandleFD);
	while (cur_cluster >= 2 && cur_cluster != 0xFFFFFFFF)
	{
		*sector_tablePtr = clst2sect(&dldiFs, cur_cluster);
		sector_tablePtr++;
		cur_cluster = f_getFat(videoHandleFD->filPtr, cur_cluster);
		clusterCount++; //each cluster item * (diskSectorSize * ClusterSize(or sectorsPerCluster))
	}
	*(sector_tablePtr) = 0xFFFFFFFF;	
	
	coherent_user_range_by_size((uint32)sector_table, (sint32)(clusterCount * sizeof(u32)));
	return clusterCount;
}

#if defined(WIN32)
void clrscr(){

}
#endif

//ARM9 Implementation
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
int DldiBulkReadFromFileIntoEwramArm9(int fileOffsetInFileHandle, u8 * EwramSourceBuffer, int readSize, struct fd * StructFDHandle, u32 * sector_table){
	#if defined(WIN32)
	return DldiBulkReadFromFileIntoEwramArm7(fileOffsetInFileHandle, (u8 *)EwramSourceBuffer, readSize, FS_getFileSizeFromOpenStructFD(StructFDHandle), getDiskSectorSize(), getDiskClusterSize(), sector_table);
	#endif

	#ifdef ARM9
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	setValueSafe(&fifomsg[32], (uint32)fileOffsetInFileHandle);
	setValueSafe(&fifomsg[33], (uint32)EwramSourceBuffer);
	setValueSafe(&fifomsg[34], (uint32)readSize);
	setValueSafe(&fifomsg[35], (uint32)FS_getFileSizeFromOpenStructFD(StructFDHandle));
	setValueSafe(&fifomsg[36], (uint32)getDiskSectorSize());
	setValueSafe(&fifomsg[37], (uint32)getDiskClusterSize());
	setValueSafe(&fifomsg[38], (uint32)sector_table);
	setValueSafe(&fifomsg[39], (uint32)0xFFFFFFAA);
	
	SendFIFOWords(TGDS_BULK_READ_DLDI);
	while((u32)getValueSafe(&fifomsg[39]) == (u32)0xFFFFFFAA){
		swiDelay(1);
	}
	int read = (int)getValueSafe(&fifomsg[39]);
	coherent_user_range_by_size((uint32)EwramSourceBuffer, (sint32)read);
	return read;
	#endif
}
#endif

#if defined(WIN32) || defined(ARM7)
//ARM7 Implementation
int DldiBulkReadFromFileIntoEwramArm7(int fileOffsetInFileHandle, u8 * EwramSourceBuffer, int readSize, u32 wholeFileSize, int sectorSize, int sectorsPerCluster, u32* sector_tablePtrUser){
	#if defined(WIN32)
	u8 * outBuf = (u8 *)TGDSARM9Malloc(sectorSize * sectorsPerCluster);
	#endif
	
	#if defined(ARM7)
	u8 * outBuf = (u8 *)BulkBufferARM7;
	#endif
	
	u8 * EwramSourceBufferSeek = (u8*)EwramSourceBuffer;
	int globalPtr = 0; //this one maps the entire file in 512 bytes (sectorSize)
	u32 cur_clustersector = sector_tablePtrUser[0];
	uint32_t data_read = 0;
	int i = 0;
	while((cur_clustersector != 0xFFFFFFFF) && ((data_read * (sectorSize * sectorsPerCluster)) < wholeFileSize) ){
		//for each sector per cluster...
		for(i = 0; i < sectorsPerCluster; i++){
			//copy it into target payload
			if ( (globalPtr >= fileOffsetInFileHandle) && (globalPtr < (fileOffsetInFileHandle+readSize)) ){
				//full sector copy
				memset(outBuf, 0, sectorSize * sectorsPerCluster);
				dldi_handler_read_sectors(cur_clustersector, sectorsPerCluster, (void*)(outBuf));
				{
					int lastSectorOffset = (globalPtr - fileOffsetInFileHandle) / sectorSize;
					int lastOffset = (globalPtr - fileOffsetInFileHandle);
					if((lastOffset < sectorSize) ){ //first sector
						if(i>0){ //can read backwards
							memcpy (EwramSourceBufferSeek, outBuf + (i*sectorSize) - lastOffset, sectorSize + lastOffset);	
						}
						else{ //cant read backwards because past cluster has the data
							data_read--;
							cur_clustersector = (u32)sector_tablePtrUser[data_read];
							memset(outBuf, 0, sectorSize * sectorsPerCluster);
							dldi_handler_read_sectors(cur_clustersector, sectorsPerCluster, (void*)(outBuf));
							memcpy (EwramSourceBufferSeek, outBuf + (sectorSize * sectorsPerCluster) - lastOffset, lastOffset);
							
							data_read++;
							cur_clustersector = (u32)sector_tablePtrUser[data_read];
							dldi_handler_read_sectors(cur_clustersector, sectorsPerCluster, (void*)(outBuf));
							memcpy (EwramSourceBufferSeek + lastOffset, outBuf, (sectorSize * sectorsPerCluster) - lastOffset);
						}
						EwramSourceBufferSeek +=(sectorSize + lastOffset);
					}
					else{
						memcpy (EwramSourceBufferSeek, outBuf + (i*sectorSize), sectorSize);	
						EwramSourceBufferSeek +=(sectorSize);
					}
				}
			}
			globalPtr +=sectorSize;
		}
		data_read++;
		cur_clustersector = (u32)sector_tablePtrUser[data_read];
	}
	#if defined(WIN32)
	TGDSARM9Free(outBuf);
	#endif
	return (int)((int)EwramSourceBufferSeek - (int)EwramSourceBuffer);
}
#endif