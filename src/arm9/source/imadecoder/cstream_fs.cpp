#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "cstream_fs.h"
#include "fatfslayerTGDS.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "dldi.h"
#include "ff.h"
#include "dmaTGDS.h"
#include "nds_cp15_misc.h"
#include "ARM7FS.h"

CStreamFS::CStreamFS(struct fd *_file):file(_file)
{
	file=_file;
	size=FS_getFileSizeFromOpenStructFD(file);
	SetOffset(0);	//internal file/dir offset zero
}

CStreamFS::~CStreamFS(void)
{
}

int CStreamFS::GetOffset(void) const
{
	return(file->loc);
}

void CStreamFS::SetOffset(int _ofs)
{
	file->loc=_ofs;
	if(size<file->loc) {
		file->loc=size;
	}
}

int CStreamFS::GetSize(void) const
{
	return(size);
}

void CStreamFS::OverrideSize(int _size)
{
	size=_size;
}

bool CStreamFS::eof(void) const
{
	if(file->loc>=size){
		return(true);
    }
	else{
		return(false);
	}
}

u8 CStreamFS::Readu8(void)
{
  if(eof()==true) return(0);
  u8 data=0;
  TGDSFSUserfatfs_read(&data, 1, file->loc, file);
  file->loc++;
  return(data);
}

u16 CStreamFS::Readu16(void)
{
  if(eof()==true) return(0);
  u16 data;
  TGDSFSUserfatfs_read((u8*)&data, 2, file->loc, file);
  file->loc+=2;
  return(data);
  
  data=(u16)Readu8();
  data=data | ((u16)Readu8() << 8);
  return(data);
}

u32 CStreamFS::Readu32(void)
{
  if(eof()==true) return(0);
  
  u32 data;
  TGDSFSUserfatfs_read((u8*)&data, 4, file->loc, file);
  file->loc+=4;
  return(data);
  
  data=(u32)Readu8();
  data=data | ((u32)Readu8() << 8);
  data=data | ((u32)Readu8() << 16);
  data=data | ((u32)Readu8() << 24);
  
  return(data);
}

int CStreamFS::ReadBuffer(u8 *ptr, int len, int offst, struct fd *tgdsfd){
	int read = TGDSFSUserfatfs_read(ptr, len, offst, tgdsfd);	//fread( datacache, 1, block, fin );
	tgdsfd->loc+=(read);
	return read;
}

int CStreamFS::ReadBuffer16bit(void *_dstbuf,const int _readsize)
{
  if(eof()==true) return(0);
  
  int readsize;
  
  if((file->loc+_readsize)<=size){
    readsize=_readsize;
  }
  else{
    readsize=size-file->loc;
    if(readsize<0) readsize=0;
  }
  
  if(readsize!=0){
      readsize=TGDSFSUserfatfs_read((u8*)_dstbuf, readsize, file->loc, file);
  }
  
  file->loc+=readsize;
  return(readsize);
}

int CStreamFS::ReadBuffer32bit(void *_dstbuf,const int _readsize)
{
  return(ReadBuffer16bit(_dstbuf,_readsize));
}

void CStreamFS::ReadSkip(const int _readsize)
{
  if(eof()==true) return;
  
  int readsize;
  
  if((file->loc+_readsize)<=size){
    readsize=_readsize;
    }else{
    readsize=size-file->loc;
    if(readsize<0) readsize=0;
  }
  
  file->loc+=readsize;
}

///////////////////////////////////////////////TGDS FileDescriptor Callbacks Implementation Start ///////////////////////////////////////////////
//												See ARM7FS.h, TGDS FileDescriptor Implementation)
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
	f_lseek (
			tgdsfd->filPtr,   /* Pointer to the file object structure */
			(DWORD)offst       /* File offset in unit of byte */
		);
	return fatfs_write(tgdsfd->cur_entry.d_ino, ptr, len);
}

//read (get struct FD index from FILE * handle)
int TGDSFSUserfatfs_read(u8 *ptr, int len, int offst, struct fd *tgdsfd){
	f_lseek (
			tgdsfd->filPtr,   /* Pointer to the file object structure */
			(DWORD)offst       /* File offset in unit of byte */
		);
	return fatfs_read(tgdsfd->cur_entry.d_ino, ptr, len);
}

//receives a new struct fd index with either DIR or FIL structure allocated so it can be closed.
//returns 0 if success, 1 if error
int TGDSFSUserfatfs_close(struct fd * tgdsfd){
	return fatfs_close(tgdsfd->cur_entry.d_ino);
}

//returns an internal index struct fd allocated. Requires DLDI enabled
int TGDSFSUserfatfs_open_file(const sint8 *pathname, char * posixFlags, int * tgdsfd){
	//Copies newly alloced struct fd / Creates duplicate filehandles when opening a new file
	return fatfs_open_fileIntoTargetStructFD(pathname, posixFlags, tgdsfd);
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
	//Test case for 2 file handles out a single file
	int retStatus1 = TGDSFSUserfatfs_open_file((const sint8 *)filenameToOpen, "r", tgdsStructFD1);	//returns structFD Video File Handle as posix file descriptor (int fd) and struct fd *
	int retStatus2 = TGDSFSUserfatfs_open_file((const sint8 *)filenameToOpen, "r", tgdsStructFD2);	//returns structFD Audio File Handle as posix file descriptor (int fd) and struct fd *
	if( (retStatus1 != -1) && (retStatus2 != -1)){
		return true;
	}
	
	struct fd * fd1 = getStructFD(*tgdsStructFD1);
	struct fd * fd2 = getStructFD(*tgdsStructFD2); 	
	closeDualTGDSFileHandleFromFile(fd1, fd2);
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