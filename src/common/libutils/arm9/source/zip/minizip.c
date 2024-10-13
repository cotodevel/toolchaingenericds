/* minizip.c
   Version 1.2.0, September 16th, 2017
   sample part of the MiniZip project

   Copyright (C) 2012-2017 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 2009-2010 Mathias Svensson
     Modifications for Zip64 support
     http://result42.com
   Copyright (C) 2007-2008 Even Rouault
     Modifications of Unzip for Zip64
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
#  include <direct.h>
#  include <io.h>
#else
#  include <unistd.h>
#  include <utime.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include "zip.h"

#ifdef _WIN32
#  define USEWIN32IOAPI
#  include "iowin32.h"
#endif

#include "minishared.h"
#include "posixHandleTGDS.h"

void minizip_banner()
{
    nocashMessage("MiniZip 1.2.0, demo of zLib + MiniZip64 package\n");
    nocashMessage("more info on MiniZip at https://github.com/nmoinvaz/minizip\n\n");
}

void minizip_help()
{
    nocashMessage("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int minizip_addfile(zipFile zf, const char *path, const char *filenameinzip, int level, const char *password)
{
    zip_fileinfo zi = { 0 };
    FILE *fin = NULL;
    int size_read = 0;
    int zip64 = 0;
    int err = ZIP_OK;
    char buf[UINT16_MAX];


    /* Get information about the file on disk so we can store it in zip */
    get_file_date(path, &zi.dos_date);

    zip64 = is_large_file(path);

    /* Add to zip file */
    err = zipOpenNewFileInZip3_64(zf, filenameinzip, &zi,
        NULL, 0, NULL, 0, NULL /* comment*/,
        (level != 0) ? Z_DEFLATED : 0, level, 0,
        -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
        password, 0, zip64);

    if (err != ZIP_OK)
    {
        nocashMessage("error in opening %s in zipfile (%d)\n", filenameinzip, err);
    }
    else
    {
        fin = fopen(path, "r");
        if (fin == NULL)
        {
            err = ZIP_ERRNO;
            nocashMessage("error in opening %s for reading\n", path);
        }
    }

    if (err == ZIP_OK)
    {
        /* Read contents of file and write it to zip */
        do
        {
            size_read = (int)fread(buf, 1, sizeof(buf), fin);
            if ((size_read < (int)sizeof(buf)) && (feof(fin) == 0))
            {
                nocashMessage("error in reading %s\n", filenameinzip);
                err = ZIP_ERRNO;
            }

            if (size_read > 0)
            {
                err = zipWriteInFileInZip(zf, buf, size_read);
                if (err < 0)
                    nocashMessage("error in writing %s in the zipfile (%d)\n", filenameinzip, err);
            }
        } while ((err == ZIP_OK) && (size_read > 0));
    }

    if (fin)
        fclose(fin);

    if (err < 0)
    {
        err = ZIP_ERRNO;
    }
    else
    {
        err = zipCloseFileInZip(zf);
        if (err != ZIP_OK)
            nocashMessage("error in closing %s in the zipfile (%d)\n", filenameinzip, err);
    }

    return err;
}

