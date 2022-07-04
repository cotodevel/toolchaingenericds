/*
 * untgz.c -- Display contents and extract files from a gzip'd TAR file
 *
 * written by Pedro A. Aranda Gutierrez <paag@tid.es>
 * adaptation to Unix by Jean-loup Gailly <jloup@gzip.org>
 * various fixes by Cosmin Truta <cosmint@cs.ubbcluj.ro>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#pragma warning(disable:4703)
#include "zlib-1.2.11/zlib.h"
#include "utilities.h"
#include <direct.h>
#include <io.h>
#endif
#ifndef WIN32
#include "zlib.h"
#include "posixHandleTGDS.h"
#include "fatfslayerTGDS.h"
#include <unistd.h>
#include <utime.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define unix

#ifdef unix
#else
#  include <direct.h>
#  include <io.h>
#endif

#if !defined(unix)
#include <windows.h>
#  ifndef F_OK
#    define F_OK  0
#  endif
#  define mkdir(dirname,mode)   _mkdir(dirname)
#  ifdef _MSC_VER
#    define access(path,mode)   _access(path,mode)
#    define chmod(path,mode)    _chmod(path,mode)
#    define strdup(str)         _strdup(str)
#  endif
#else

#endif


/* values used in typeflag field */

#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

/* GNU tar extensions */

#define GNUTYPE_DUMPDIR  'D'    /* file names from dumped directory */
#define GNUTYPE_LONGLINK 'K'    /* long link name */
#define GNUTYPE_LONGNAME 'L'    /* long file name */
#define GNUTYPE_MULTIVOL 'M'    /* continuation of file from another volume */
#define GNUTYPE_NAMES    'N'    /* file name that does not fit into main hdr */
#define GNUTYPE_SPARSE   'S'    /* sparse file */
#define GNUTYPE_VOLHDR   'V'    /* tape/volume header */


/* tar header */

#define BLOCKSIZE     512
#define SHORTNAMESIZE 100

struct tar_header
{                               /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
};

union tar_buffer
{
  char               buffer[BLOCKSIZE];
  struct tar_header  header;
};

struct attr_item
{
  struct attr_item  *next;
  char              *fname;
  int                mode;
  time_t             time;
};

enum { TGZ_EXTRACT, TGZ_LIST, TGZ_INVALID };

char *TGZfname          (const char *);
void TGZnotfound        (const char *);

int getoct              (char *, int);
char *strtime           (time_t *);
int setfiletime         (char *, time_t);
void push_attr          (struct attr_item **, char *, int, time_t);
void restore_attr       (struct attr_item **);

int ExprMatch           (char *, char *);

int makedir             (char *);
int matchname           (int, int, char **, char *);

void error              (const char *);
int tar                 (gzFile, int, int, int, char **);

void help               (int);
int main                (int, char **);

#ifndef WIN32
static bool existFilePosix(char * file){
	if(FileExists(file) == FT_FILE){
		return true;
	}
	return false;
}
#endif

char *prog;

const char *TGZsuffix[] = { "\0", ".tar", ".tar.gz", ".taz", ".tgz", NULL };

/* return the file name of the TGZ archive */
/* or NULL if it does not exist */

char *TGZfname (const char *arcname)
{
  static char buffer[1024];
  int origlen,i;

  strcpy(buffer,arcname);
  origlen = strlen(buffer);

  for (i=0; TGZsuffix[i]; i++)
    {
       strcpy(buffer+origlen,TGZsuffix[i]);
       if (existFilePosix(buffer) == true){
         return buffer;
	   }
    }
  return NULL;
}


/* error message for the filename */

void TGZnotfound (const char *arcname){
  int i;
  printf("%s: Couldn't find ", prog);
  for (i=0;TGZsuffix[i];i++){
     printf((TGZsuffix[i+1]) ? "%s%s, " : "or %s%s", arcname, TGZsuffix[i]);
  }
}


/* convert octal digits to int */
/* on error return -1 */

int getoct (char *p,int width)
{
  int result = 0;
  char c;

  while (width--)
    {
      c = *p++;
      if (c == 0)
        break;
      if (c == ' ')
        continue;
      if (c < '0' || c > '7')
        return -1;
      result = result * 8 + (c - '0');
    }
  return result;
}


/* convert time_t to string */
/* use the "YYYY/MM/DD hh:mm:ss" format */

char *strtime (time_t *t)
{
  struct tm   *local;
  static char result[32];

  local = localtime(t);
  sprintf(result,"%4d/%02d/%02d %02d:%02d:%02d",
          local->tm_year+1900, local->tm_mon+1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);
  return result;
}


/* set file time */

int setfiletime (char *fname,time_t ftime)
{
	/*
#ifdef WIN32
  static int isWinNT = -1;
  SYSTEMTIME st;
  FILETIME locft, modft;
  struct tm *loctm;
  HANDLE hFile;
  int result;

  loctm = localtime(&ftime);
  if (loctm == NULL)
    return -1;

  st.wYear         = (WORD)loctm->tm_year + 1900;
  st.wMonth        = (WORD)loctm->tm_mon + 1;
  st.wDayOfWeek    = (WORD)loctm->tm_wday;
  st.wDay          = (WORD)loctm->tm_mday;
  st.wHour         = (WORD)loctm->tm_hour;
  st.wMinute       = (WORD)loctm->tm_min;
  st.wSecond       = (WORD)loctm->tm_sec;
  st.wMilliseconds = 0;
  if (!SystemTimeToFileTime(&st, &locft) ||
      !LocalFileTimeToFileTime(&locft, &modft))
    return -1;

  if (isWinNT < 0)
    isWinNT = (GetVersion() < 0x80000000) ? 1 : 0;
  hFile = CreateFile(fname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                     (isWinNT ? FILE_FLAG_BACKUP_SEMANTICS : 0),
                     NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return -1;
  result = SetFileTime(hFile, NULL, NULL, &modft) ? 0 : -1;
  CloseHandle(hFile);
  return result;
#else
  struct utimbuf settime;

  settime.actime = settime.modtime = ftime;
  return utime(fname,&settime);
#endif
  */



	return -1;
}


/* push file attributes */

void push_attr(struct attr_item **list,char *fname,int mode,time_t time)
{
  struct attr_item *item;

  item = (struct attr_item *)TGDSARM9Malloc(sizeof(struct attr_item));
  if (item == NULL){
    error("Out of memory");
  }
  item->fname = strdup(fname);
  item->mode  = mode;
  item->time  = time;
  item->next  = *list;
  *list       = item;
}


/* restore file attributes */
void restore_attr(struct attr_item **list)
{
  struct attr_item *item, *prev;

  for (item = *list; item != NULL; )
    {
      setfiletime(item->fname,item->time);
      chmod(item->fname,item->mode);
	  prev = item;
      item = item->next;
      TGDSARM9Free(prev);
    }
  *list = NULL;
}


/* match regular expression */

#define ISSPECIAL(c) (((c) == '*') || ((c) == '/'))

int ExprMatch (char *string,char *expr)
{
  while (1)
    {
      if (ISSPECIAL(*expr))
        {
          if (*expr == '/')
            {
              if (*string != '\\' && *string != '/')
                return 0;
              string ++; expr++;
            }
          else if (*expr == '*')
            {
              if (*expr ++ == 0)
                return 1;
              while (*++string != *expr)
                if (*string == 0)
                  return 0;
            }
        }
      else
        {
          if (*string != *expr)
            return 0;
          if (*expr++ == 0)
            return 1;
          string++;
        }
    }
}


/* recursive mkdir */
/* abort on ENOENT; ignore other errors like "directory already exists" */
/* return 1 if OK */
/*        0 on error */

int makedir (char *newdir)
{
  char *buffer = strdup(newdir);
  char *p;
  int  len = strlen(buffer);

  if (len <= 0) {
    TGDSARM9Free(buffer);
    return 0;
  }
  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
	#ifdef WIN32
	if (mkdir(buffer) == 0)
	#endif
	#ifndef WIN32
	if (mkdir(buffer, 0755) == 0)
	#endif
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
	
	#ifdef WIN32
	if (mkdir(buffer) == -1)
	#endif
    #ifndef WIN32
	if ((mkdir(buffer, 0755) == -1) && (errno == ENOENT))
	#endif
	  {
          printf("%s: Couldn't create directory %s",prog,buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}


int matchname (int arg,int argc,char **argv,char *fname)
{
  if (arg == argc)      /* no arguments given (untgz tgzarchive) */
    return 1;

  while (arg < argc)
    if (ExprMatch(fname,argv[arg++]))
      return 1;

  return 0; /* ignore this for the moment being */
}


/* tar file list or extract */

int tar (gzFile in,int action,int arg,int argc,char **argv)
{
  union  tar_buffer buffer;
  int    len;
  int    err;
  int    getheader = 1;
  int    remaining = 0;
  FILE   *outfile = NULL;
  char   fname[BLOCKSIZE];
  int    tarmode;
  time_t tartime;
  struct attr_item *attributes = NULL;

  if (action == TGZ_LIST){
    //printf("    date      time     size                       file"
    //       " ---------- -------- --------- -------------------------------------");
  }
  while (1)
    {
      len = gzread(in, &buffer, BLOCKSIZE);
      if (len < 0){
        error(gzerror(in, &err));
	  }
	   /*
       * Always expect complete blocks to process
       * the tar information.
       */
      if (len != BLOCKSIZE)
        {
          action = TGZ_INVALID; /* force error exit */
          remaining = 0;        /* force I/O cleanup */
        }

      /*
       * If we have to get a tar header
       */
      if (getheader >= 1)
        {
          /*
           * if we met the end of the tar
           * or the end-of-tar block,
           * we are done
           */
          if (len == 0 || buffer.header.name[0] == 0)
            break;

          tarmode = getoct(buffer.header.mode,8);
          tartime = (time_t)getoct(buffer.header.mtime,12);
          if (tarmode == -1 || tartime == (time_t)-1)
            {
              buffer.header.name[0] = 0;
              action = TGZ_INVALID;
            }

          if (getheader == 1)
            {
              strncpy(fname,buffer.header.name,SHORTNAMESIZE);
              if (fname[SHORTNAMESIZE-1] != 0)
                  fname[SHORTNAMESIZE] = 0;
            }
          else
            {
              /*
               * The file name is longer than SHORTNAMESIZE
               */
              if (strncmp(fname,buffer.header.name,SHORTNAMESIZE-1) != 0){
                  error("bad long name");
			  }
              getheader = 1;
            }

          /*
           * Act according to the type flag
           */
          switch (buffer.header.typeflag)
            {
            case DIRTYPE:
              if (action == TGZ_LIST){
                //printf(" %s     <dir> %s",strtime(&tartime),fname);
              }
              if (action == TGZ_EXTRACT)
                {
                  makedir(fname);
                  push_attr(&attributes,fname,tarmode,tartime);
                }
              break;
            case REGTYPE:
            case AREGTYPE:
              remaining = getoct(buffer.header.size,12);
              if (remaining == -1)
                {
                  action = TGZ_INVALID;
                  break;
                }
              if (action == TGZ_LIST){
                //printf(" %s %9d %s",strtime(&tartime),remaining,fname);
              }
              else if (action == TGZ_EXTRACT)
                {
                  if (matchname(arg,argc,argv,fname))
                    {
                      #if !defined(ARM9)
                      outfile = fopen(fname,"wb");
                      #endif
                      #if defined(ARM9)
					  char * filenam = &fname[0];
					  char newName[256];
					  memset(newName, 0, sizeof(newName));
					  strcpy(newName, "0:/");
					  strcat(newName, filenam);
                      strcpy(fname, (char*)&newName[0]); 
                      outfile = fopen(fname,"w+");
                      #endif
                      if (outfile == NULL) {
                        /* try creating directory */
                        char *p = strrchr(fname, '/');
                        if (p != NULL) {
                          *p = '\0';
                           makedir(fname);
                          *p = '/';
                           #if !defined(ARM9)
                           outfile = fopen(fname,"wb");
                           #endif
						   #if defined(ARM9)
						   outfile = fopen(fname,"w+");
                           #endif
                        }
                      }
                      if (outfile != NULL){
                        //printf("[Extracting %s]",fname);
					  }
					  else{
                        //printf("[%s: Couldn't create %s]",prog,fname);
					  }
                    }
                  else
                    outfile = NULL;
                }
              getheader = 0;
              break;
            case GNUTYPE_LONGLINK:
            case GNUTYPE_LONGNAME:
              remaining = getoct(buffer.header.size,12);
              if (remaining < 0 || remaining >= BLOCKSIZE)
                {
                  action = TGZ_INVALID;
                  break;
                }
              len = gzread(in, fname, BLOCKSIZE);
              if (len < 0){
                error(gzerror(in, &err));
			  }
			  if (fname[BLOCKSIZE-1] != 0 || (int)strlen(fname) > remaining)
                {
                  action = TGZ_INVALID;
                  break;
                }
              getheader = 2;
              break;
            default:
              if (action == TGZ_LIST){
                //printf(" %s     <---> %s",strtime(&tartime),fname);
			  }
			  break;
            }
        }
      else
        {
          unsigned int bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining;

          if (outfile != NULL)
            {
              if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes)
                {
				  printf("%s: Error writing %s -- skipping",prog,fname);
                  
				  fclose(outfile);
                  outfile = NULL;
                  remove(fname);
                }
            }
          remaining -= bytes;
        }

      if (remaining == 0)
        {
          getheader = 1;
          if (outfile != NULL)
            {
              fclose(outfile);
              outfile = NULL;
              if (action != TGZ_INVALID){
                push_attr(&attributes,fname,tarmode,tartime);
			  }
            }
        }

      /*
       * Abandon if errors are found
       */
      if (action == TGZ_INVALID){
          error("broken archive");
          break;
      }
    }

  /*
   * Restore file modes and time stamps
   */
  restore_attr(&attributes);

  if (gzclose(in) != Z_OK){
    error("failed gzclose");
  }
  return 0;
}


/* ============================================================ */

void help(int exitval)

{
  printf("untgz version 0.2.1"
         "  using zlib version %s",
         zlibVersion());
  printf("Usage: untgz file.tgz            extract all files"
         "       untgz file.tgz fname ...  extract selected files"
         "       untgz -l file.tgz         list archive contents"
         "       untgz -h                  display this help");
}

void error(const char *msg){
  printf("%s: %s", prog, msg);
  while(1==1){}
}


/* ============================================================ */

#if defined(WIN32) && defined(__GNUC__)
int _CRT_glob = 0;      /* disable argument globbing in MinGW */
#endif

int untgzmain(int argc,char **argv)
{
    int         action = TGZ_EXTRACT;
    int         arg = 1;
    char        *TGZfile;
    gzFile      *f;

    prog = strrchr(argv[0],'\\');
    if (prog == NULL)
      {
        prog = strrchr(argv[0],'/');
        if (prog == NULL)
          {
            prog = strrchr(argv[0],':');
            if (prog == NULL)
              prog = argv[0];
            else
              prog++;
          }
        else
          prog++;
      }
    else
      prog++;

    if (argc == 1)
      help(0);

    if (strcmp(argv[arg],"-l") == 0)
      {
        action = TGZ_LIST;
        if (argc == ++arg)
          help(0);
      }
    else if (strcmp(argv[arg],"-h") == 0)
      {
        help(0);
      }

    if ((TGZfile = TGZfname(argv[arg])) == NULL)
      TGZnotfound(argv[arg]);

    ++arg;
    if ((action == TGZ_LIST) && (arg != argc))
      help(1);

/*
 *  Process the TGZ file
 */
    switch(action)
      {
      case TGZ_LIST:
      case TGZ_EXTRACT:
		  f = (gzFile*)gzopen(TGZfile,"rb");
        if (f == NULL)
          {
            printf("%s: Couldn't gzopen %s",prog,TGZfile);
            return 1;
          }
        tar(f, action, arg, argc, argv);
      break;

      default:
		  {
			printf("Unknown option");
		  }
      }
    return 0;
}