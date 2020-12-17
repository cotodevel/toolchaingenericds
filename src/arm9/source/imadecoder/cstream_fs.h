#ifndef __cstream_fs_h__
#define __cstream_fs_h__

#include <stdlib.h>
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "fatfslayerTGDS.h"
#include "ff.h"
#include "dldi.h"
#include "cstream_fs.h"

#ifdef __cplusplus

class CStreamFS
{
  CStreamFS(const CStreamFS&);
  CStreamFS& operator=(const CStreamFS&);
public:
  CStreamFS(struct fd *_file);
  ~CStreamFS(void);
  struct fd *file;
  int size;
  int GetOffset(void) const;
  void SetOffset(int _ofs);
  int GetSize(void) const;
  void OverrideSize(int _size);
  bool eof(void) const;
  u8 Readu8(void);
  u16 Readu16(void);
  u32 Readu32(void);
  int ReadBuffer(u8 *ptr, int len, int offst, struct fd *tgdsfd);
  // fast request 16bit aligned file position and write buffer
  int ReadBuffer16bit(void *_dstbuf,const int _readsize);
  int ReadBuffer32bit(void *_dstbuf,const int _readsize);
  void ReadSkip(const int _readsize);
};
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int TGDSFSUserfatfs_write(u8 *ptr, int len, int offst, struct fd *tgdsfd);
extern int TGDSFSUserfatfs_read(u8 *ptr, int len, int offst, struct fd *tgdsfd);
extern int TGDSFSUserfatfs_close(struct fd * tgdsfd);
extern int TGDSFSUserfatfs_open_file(const sint8 *pathname, char * posixFlags, int * tgdsfd);
extern off_t TGDSFSUserfatfs_lseek(struct fd *pfd, off_t offset, int whence);
extern long TGDSFSUserfatfs_tell(struct fd *f);

extern bool openDualTGDSFileHandleFromFile(char * filenameToOpen, int * tgdsStructFD1, int * tgdsStructFD2);
extern bool closeDualTGDSFileHandleFromFile(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2);

#ifdef __cplusplus
}
#endif
