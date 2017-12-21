/*-
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)dirent.h	8.1 (Berkeley) 6/2/93
 */

/*
 * The dirent structure defines the format of directory entries returned by 
 * the getdirentries(2) system call.
 *
 * A directory entry has a struct dirent at the front of it, containing its
 * inode number, the length of the entry, and the length of the name
 * contained in the entry.  These are followed by the name padded to a 4
 * byte boundary with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is MAXNAMLEN.
 */

//Coto: this was rewritten by me so it could fit the following setup:
//newlib libc nano ARM Toolchain. dirent.h is not supported in this newlib version so we restore it

//overriden dirent.h due to newlib nano deprecated dirent struct (no/light Filesystem support), so we restore it


#ifndef _DIRENT_H_
#define _DIRENT_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"


/* DAG */
#define	MAXNAMLEN	512		/* maximum filename length */
#define	NAME_MAX	(MAXNAMLEN - 1)	/* DAG -- added for POSIX */

struct dirent {
	ino_t d_ino;				/* file serial number */ //used as struct fd index ( fd_struct_get(fd) ), assigned when open newlib posix call, and freed when close newlib posix call
    sint8 d_name[NAME_MAX+1];	/* name must be no longer than this */
};

/* //deprecated because fsfat provides a DIR structure
// DIR substitute structure containing directory name.  The name is
// essential for the operation of ``rewinndir'' function. 
  typedef struct DIR {
    sint8          *dirname;                    // directory being scanned 
    struct dirent        current;                     // current entry 
    int           dirent_filled;               // is current un-processed? 

	//Operating system specific part 
	#  if defined(DIRENT_WIN32_INTERFACE)
    HANDLE        search_handle;
	#  elif defined(DIRENT_MSDOS_INTERFACE)
	#  endif
  } DIR;
 */
 
/*
 * File types
 */
#define	DT_UNKNOWN	 0
#define	DT_FIFO		 1
#define	DT_CHR		 2
#define	DT_DIR		 4
#define	DT_BLK		 6
#define	DT_REG		 8
#define	DT_LNK		10
#define	DT_SOCK		12

/*
 * Convert between stat structure types and directory types.
 */
#define	IFTODT(mode)	(((mode) & 0170000) >> 12)
#define	DTTOIF(dirtype)	((dirtype) << 12)

#endif /*_DIRENT_H_*/
