/*----------------------------------------------------------------------------*/
/*--  lzss.c - LZSS coding for Nintendo GBA/DS                              --*/
/*--  Copyright (C) 2011 CUE                                                --*/
/*--                                                                        --*/
/*--  This program is free software: you can redistribute it and/or modify  --*/
/*--  it under the terms of the GNU General Public License as published by  --*/
/*--  the Free Software Foundation, either version 3 of the License, or     --*/
/*--  (at your option) any later version.                                   --*/
/*--                                                                        --*/
/*--  This program is distributed in the hope that it will be useful,       --*/
/*--  but WITHOUT ANY WARRANTY; without even the implied warranty of        --*/
/*--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          --*/
/*--  GNU General Public License for more details.                          --*/
/*--                                                                        --*/
/*--  You should have received a copy of the GNU General Public License     --*/
/*--  along with this program. If not, see <http://www.gnu.org/licenses/>.  --*/
/*----------------------------------------------------------------------------*/


#include "lzss9.h"
#include "biosTGDS.h"
#include "fatfslayerTGDS.h"
#include "posixHandleTGDS.h"
#include "debugNocash.h"

/*
int main(int argc, char **argv) {
  int cmd, mode;
  int arg;

  Title();

  if (argc < 2) Usage();
  if      (!strcmpi(argv[1], "-d"))   { cmd = CMD_DECODE; }
  else if (!strcmpi(argv[1], "-evn")) { cmd = CMD_CODE_10; mode = LZS_VRAM; }
  else if (!strcmpi(argv[1], "-ewn")) { cmd = CMD_CODE_10; mode = LZS_WRAM; }
  else if (!strcmpi(argv[1], "-evf")) { cmd = CMD_CODE_10; mode = LZS_VFAST; }
  else if (!strcmpi(argv[1], "-ewf")) { cmd = CMD_CODE_10; mode = LZS_WFAST; }
  else if (!strcmpi(argv[1], "-evo")) { cmd = CMD_CODE_10; mode = LZS_VBEST; }
  else if (!strcmpi(argv[1], "-ewo")) { cmd = CMD_CODE_10; mode = LZS_WBEST; }
  else                                  EXIT("Command not supported");
  if (argc < 3) EXIT("Filename not specified");

  switch (cmd) {
    case CMD_DECODE:
      for (arg = 2; arg < argc; arg++) LZS_Decode(argv[arg]);
      break;
    case CMD_CODE_10:
      for (arg = 2; arg < argc; arg++) LZS_Encode(argv[arg], mode);
      break;
    default:
      break;
  }

  printf("Done");

  return(0);
}
*/


unsigned char ring[LZS_N + LZS_F - 1];
int           dad[LZS_N + 1], lson[LZS_N + 1], rson[LZS_N + 1 + 256];
int           pos_ring, len_ring, lzs_vram;

void Title(void) {
  //printf(
  //  ""
  // "LZSS - (c) CUE 2011"
  // "LZSS coding for Nintendo GBA/DS"
  //  ""
  //); 
}

/*----------------------------------------------------------------------------*/
/*
void Usage(void) {
  EXIT(
    "Usage: LZSS command filename [filename [...]]"
    ""
    "command:"
    "  -d ..... decode 'filename'"
    "  -evn ... encode 'filename', VRAM compatible, normal mode (LZ10)"
    "  -ewn ... encode 'filename', WRAM compatible, normal mode"
    "  -evf ... encode 'filename', VRAM compatible, fast mode"
    "  -ewf ... encode 'filename', WRAM compatible, fast mode"
    "  -evo ... encode 'filename', VRAM compatible, optimal mode (LZ-CUE)"
    "  -ewo ... encode 'filename', WRAM compatible, optimal mode (LZ-CUE)"
    ""
    "* multiple filenames and wildcards are permitted"
    "* the original file is overwritten with the new file"
  );
}
*/

/*----------------------------------------------------------------------------*/
char *Load(char *filenameIn, int *length, int min, int max) {
  FILE *fp;
  int   fs;
  char *fb;

  if ((fp = fopen(filenameIn, "r")) == NULL) EXIT("File open error");
  
  int structFD = fileno(fp);
  struct fd * fdinst=getStructFD(structFD);
  fs = f_size(fdinst->filPtr);
  if ((fs < min) || (fs > max)) EXIT("File size error");
  fb = Memory(fs + 3, sizeof(char));
  if (fread(fb, 1, fs, fp) != fs) EXIT("File read error");
  if (fclose(fp) == EOF) EXIT("File close error");

  *length = fs;

  return(fb);
}

/*----------------------------------------------------------------------------*/
void Save(char *filenameOut, char *buffer, int length) {
  FILE *fp;

  if ((fp = fopen(filenameOut, "w+")) == NULL) EXIT("File create error");
  if (fwrite(buffer, 1, length, fp) != length) EXIT("File write error");
  if (fclose(fp) == EOF) EXIT("File close error");
}

/*----------------------------------------------------------------------------*/
char *Memory(int length, int size) {
  char *fb;

  fb = (char *) TGDSARM9Calloc(length * size, size);
  if (fb == NULL) EXIT("Memory error");

  return(fb);
}

/*----------------------------------------------------------------------------*/
bool LZS_Decode(const char *filenameIn, const  char *filenameOut) {
  char *pak_buffer, *raw_buffer;
  int   pak_len, raw_len;
  unsigned int header;

  //printf("- decompressing... '%s' ", filenameIn);

  pak_buffer = (char *)Load((char*)filenameIn, &pak_len, LZS_MINIM, LZS_MAXIM);

  header = *pak_buffer;
  if (header != CMD_CODE_10) {
    TGDSARM9Free(pak_buffer);
    //printf("ERROR: file is not LZSS encoded!");
	return false;
  }
  
  raw_len = *(unsigned int *)pak_buffer >> 8;
  raw_buffer = (char *) Memory(raw_len, sizeof(char));
  
  swiDecompressLZSSWram((void *)pak_buffer, (void *)raw_buffer);
  Save((char*)filenameOut, raw_buffer, raw_len);

  TGDSARM9Free(raw_buffer);
  TGDSARM9Free(pak_buffer);

  //printf("LZS_Decode() end.");
  return true;
}

/*----------------------------------------------------------------------------*/
void LZS_Encode(const char *filenameIn, const char *filenameOut) {
  int mode = LZS_WBEST;
  unsigned char *raw_buffer, *pak_buffer, *new_buffer;
  int   raw_len, pak_len, new_len;

  lzs_vram = mode & 0xF;

  //printf("- compressing... '%s' ", filenameIn);

  raw_buffer = (unsigned char *)Load((char*)filenameIn, &raw_len, RAW_MINIM, RAW_MAXIM);
  pak_buffer = NULL;
  pak_len = LZS_MAXIM + 1;

  if (!(mode & LZS_FAST)) {
    mode = mode & LZS_BEST ? 1 : 0;
    new_buffer = (unsigned char *)LZS_Code(raw_buffer, raw_len, &new_len, mode);
  } else {
    new_buffer = (unsigned char *)LZS_Fast(raw_buffer, raw_len, &new_len);
  }
  if (new_len < pak_len) {
    if (pak_buffer != NULL) TGDSARM9Free(pak_buffer);
    pak_buffer = new_buffer;
    pak_len = new_len;
  }

  Save((char*)filenameOut, (char *)pak_buffer, pak_len);

  TGDSARM9Free(pak_buffer);
  TGDSARM9Free(raw_buffer);

  //printf("LZS_Encode end.");
}

/*----------------------------------------------------------------------------*/
char *LZS_Code(unsigned char *raw_buffer, int raw_len, int *new_len, int best) {
  unsigned char *pak_buffer, *pak, *raw, *raw_end, *flg=NULL;
  unsigned int   pak_len, len, pos, len_best, pos_best;
  unsigned int   len_next, pos_next, len_post, pos_post;
  unsigned char  mask;
  (void)pos_post;
  (void)pos_next;
#define SEARCH(l,p) { \
  l = LZS_THRESHOLD;                                          \
                                                              \
  pos = raw - raw_buffer >= LZS_N ? LZS_N : raw - raw_buffer; \
  for ( ; pos > lzs_vram; pos--) {                            \
    for (len = 0; len < LZS_F; len++) {                       \
      if (raw + len == raw_end) break;                        \
      if (*(raw + len) != *(raw + len - pos)) break;          \
    }                                                         \
                                                              \
    if (len > l) {                                            \
      p = pos;                                                \
      if ((l = len) == LZS_F) break;                          \
    }                                                         \
  }                                                           \
}

  pak_len = 4 + raw_len + ((raw_len + 7) / 8);
  pak_buffer = (unsigned char *) Memory(pak_len, sizeof(char));

  *(unsigned int *)pak_buffer = CMD_CODE_10 | (raw_len << 8);

  pak = pak_buffer + 4;
  raw = raw_buffer;
  raw_end = raw_buffer + raw_len;

  mask = 0;

  while (raw < raw_end) {
    if (!(mask >>= LZS_SHIFT)) {
      *(flg = pak++) = 0;
      mask = LZS_MASK;
    }

    SEARCH(len_best, pos_best);

    // LZ-CUE optimization start
    if (best) {
      if (len_best > LZS_THRESHOLD) {
        if (raw + len_best < raw_end) {
          raw += len_best;
          SEARCH(len_next, pos_next);
          raw -= len_best - 1;
          SEARCH(len_post, pos_post);
          raw--;

          if (len_next <= LZS_THRESHOLD) len_next = 1;
          if (len_post <= LZS_THRESHOLD) len_post = 1;

          if (len_best + len_next <= 1 + len_post) len_best = 1;
        }
      }
    }
    // LZ-CUE optimization end

    if (len_best > LZS_THRESHOLD) {
      raw += len_best;
      *flg |= mask;
      *pak++ = ((len_best - (LZS_THRESHOLD + 1)) << 4) | ((pos_best - 1) >> 8);
      *pak++ = (pos_best - 1) & 0xFF;
    } else {
      *pak++ = *raw++;
    }
  }

  *new_len = pak - pak_buffer;

  return((char *)pak_buffer);
}

/*----------------------------------------------------------------------------*/
char *LZS_Fast(unsigned char *raw_buffer, int raw_len, int *new_len) {
  unsigned char *pak_buffer, *pak, *raw, *raw_end, *flg=NULL;
  unsigned int   pak_len, len, r, s, len_tmp, i;
  unsigned char  mask; 

  pak_len = 4 + raw_len + ((raw_len + 7) / 8);
  pak_buffer = (unsigned char *) Memory(pak_len, sizeof(char));

  *(unsigned int *)pak_buffer = CMD_CODE_10 | (raw_len << 8);

  pak = pak_buffer + 4;
  raw = raw_buffer;
  raw_end = raw_buffer + raw_len;

  LZS_InitTree();

  r = s = 0;

  len = raw_len < LZS_F ? raw_len : LZS_F;
  while (r < LZS_N - len) ring[r++] = 0;

  for (i = 0; i < len; i++) ring[r + i] = *raw++;

  LZS_InsertNode(r);

  mask = 0;

  while (len) {
    if (!(mask >>= LZS_SHIFT)) {
      *(flg = pak++) = 0;
      mask = LZS_MASK;
    }

    if (len_ring > len) len_ring = len;

    if (len_ring > LZS_THRESHOLD) {
      *flg |= mask;
      pos_ring = ((r - pos_ring) & (LZS_N - 1)) - 1;
      *pak++ = ((len_ring - LZS_THRESHOLD - 1) << 4) | (pos_ring >> 8);
      *pak++ = pos_ring & 0xFF;
    } else {
      len_ring = 1;
      *pak++ = ring[r];
    }

    len_tmp = len_ring;
    for (i = 0; i < len_tmp; i++) {
      if (raw == raw_end) break;
      LZS_DeleteNode(s);
      ring[s] = *raw++;
      if (s < LZS_F - 1) ring[s + LZS_N] = ring[s];
      s = (s + 1) & (LZS_N - 1);
      r = (r + 1) & (LZS_N - 1);
      LZS_InsertNode(r);
    }
    while (i++ < len_tmp) {
      LZS_DeleteNode(s);
      s = (s + 1) & (LZS_N - 1);
      r = (r + 1) & (LZS_N - 1);
      if (--len) LZS_InsertNode(r);
    }
  }

  *new_len = pak - pak_buffer;

  return((char*)pak_buffer);
}

/*----------------------------------------------------------------------------*/
void LZS_InitTree(void) {
  int i;

  for (i = LZS_N + 1; i <= LZS_N + 256; i++)
    rson[i] = LZS_NIL;

  for (i = 0; i < LZS_N; i++)
    dad[i] = LZS_NIL;
}

/*----------------------------------------------------------------------------*/
void LZS_InsertNode(int r) {
  unsigned char *key;
  int            i, p, cmp, prev;

  prev = (r - 1) & (LZS_N - 1);

  cmp = 1;
  len_ring = 0;

  key = &ring[r];
  p = LZS_N + 1 + key[0];

  rson[r] = lson[r] = LZS_NIL;

  for ( ; ; ) {
    if (cmp >= 0) {
      if (rson[p] != LZS_NIL) p = rson[p];
      else                  { rson[p] = r; dad[r] = p; return; }
    } else {
      if (lson[p] != LZS_NIL) p = lson[p];
      else                  { lson[p] = r; dad[r] = p; return; }
    }

    for (i = 1; i < LZS_F; i++)
      if ((cmp = key[i] - ring[p + i])) break;

    if (i > len_ring) {
      if (!lzs_vram || (p != prev)) {
        pos_ring = p;
        if ((len_ring = i) == LZS_F) break;
      }
    }
  }

  dad[r] = dad[p]; lson[r] = lson[p]; rson[r] = rson[p];

  dad[lson[p]] = r; dad[rson[p]] = r;

  if (rson[dad[p]] == p) rson[dad[p]] = r;
  else                   lson[dad[p]] = r;

  dad[p] = LZS_NIL;
}

/*----------------------------------------------------------------------------*/
void LZS_DeleteNode(int p) {
  int q;
  
  if (dad[p] == LZS_NIL) return;

  if (rson[p] == LZS_NIL) {
    q = lson[p];
  } else if (lson[p] == LZS_NIL) {
    q = rson[p];
  } else {
    q = lson[p];
    if (rson[q] != LZS_NIL) {
      do {
        q = rson[q];
      } while (rson[q] != LZS_NIL);

      rson[dad[q]] = lson[q]; dad[lson[q]] = dad[q];
      lson[q]      = lson[p]; dad[lson[p]] = q;
    }

    rson[q] = rson[p]; dad[rson[p]] = q;
  }

  dad[q] = dad[p];

  if (rson[dad[p]] == p) rson[dad[p]] = q;
  else                   lson[dad[p]] = q;

  dad[p] = LZS_NIL;
}

/*----------------------------------------------------------------------------*/
/*--  EOF                                           Copyright (C) 2011 CUE  --*/
/*----------------------------------------------------------------------------*/
