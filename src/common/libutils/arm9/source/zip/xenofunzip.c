/*
 * xenofunzip - gzip/zip decompression interface for zlib, based on info-zip's funzip
 * note: xenogzip uses gzio but xenofunzip uses normal inflateInit2()/inflate().
 */

//Coto: rewritten compressor code so it's ZLIB standard through TGDS malloc/free memory allocator.
//Also use EWRAM as stack for giant compressed .zip / .gz files, see zipSwapStack.S for specifics.

#include "xenofunzip.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "consoleTGDS.h"
#include <stdlib.h>
#include "zlib.h"
#include "posixHandleTGDS.h"	//add Toolchain Generic DS Filesystem Support
#include "utilsTGDS.h"
#include "global_settings.h"

//zalloc / zfree (zlib) default function prototypes look like:
//void * 	zalloc (void *opaque, unsigned items, unsigned size)
//void 		zfree (void *opaque, void *ptr)
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int funzipstdio(char * inFname, char * outFname){
	
	int encrypted;
	ush n;
	uch h[LOCHDR];                // first local header (GZPHDR < LOCHDR)
	int g = 0;                    // true if gzip format
	unsigned int method = 0;      // initialized here to shut up gcc warning
	int size = -1;
	u8 fname[256+1];
	memset(fname, 0, sizeof(fname));
	
	FILE *in=fopen(inFname, "r");
	if(!in){
		return -1;
	}
	
	//info-zip's funzip stuff
	n = fgetc(in);  n |= fgetc(in) << 8;
	if (n == ZIPMAG){
		if (fread((char *)h, 1, LOCHDR, in) != LOCHDR || SH(h) != LOCREM){
			return -1; //err("invalid zipfile");
		}

		switch (method = SH(h + LOCHOW)) {
			case STORED:
			case DEFLATED:
				break;
			default:{
				return -1; //err("first entry not deflated or stored");
			}
			break;
		}
		int offst = 0;
		for (n = SH(h + LOCFIL); n--; ){ 
			g = fgetc(in);
			fname[offst] = g;
			offst++;
		}
		for (n = SH(h + LOCEXT); n--; ){ g = fgetc(in); }
		fname[offst] = g = 0;
		size = LG(h+LOCSIZ);
		encrypted = h[LOCFLG] & CRPFLG;
	}
	else if (n == GZPMAG){
		if (fread((char *)h, 1, GZPHDR, in) != GZPHDR){
			return -1; //err("invalid gzip file");
		}
		if ((method = h[GZPHOW]) != DEFLATED && method != ENHDEFLATED){
			return -1; //err("gzip file not deflated");
		}
		if (h[GZPFLG] & GZPMUL){
			return -1; //err("cannot handle multi-part gzip files");
		}
		if (h[GZPFLG] & GZPISX){
			n = fgetc(in);  n |= fgetc(in) << 8;
			while (n--) g = fgetc(in);
		}
		if (h[GZPFLG] & GZPISF){
			while ((g = fgetc(in)) != 0 && g != EOF) ;
		}
		if (h[GZPFLG] & GZPISC){
			while ((g = fgetc(in)) != 0 && g != EOF) ;
		}
		g = 1;
		encrypted = h[GZPFLG] & GZPISE;
	}
	else{
		return -1; //err("input not a zip or gzip file");
	}
	//now in points to deflated entry. let's just inflate it using zlib.

	//if entry encrypted, decrypt and validate encryption header
	if (encrypted){
		return -1; //err("encrypted zip unsupported");
	}
	
	//Generate full dir path for output: internal ZIP name
	strncpy(outFname, (const char *)fname, strlen((const char *)fname)+1);
	u8 newOutFname[256+1];
	memset(newOutFname, 0, sizeof(newOutFname));
	int lastDir = (strlen(inFname) - strlen(outFname));
	if(lastDir > 0){
		strncpy((char *)newOutFname, inFname, lastDir);
	}
	//identical name or error 
	else{
		strncpy((char *)newOutFname, inFname, strlen(inFname));
	}
	strcat((char *)newOutFname, outFname);
	strcpy(outFname, (char *)newOutFname);
	FILE *out=fopen(outFname,"w+");
	if(!out){
		fclose(in);
		return -1;
	}
	
	//decompress
	if (g || h[LOCHOW]){ //deflate
		Bytef *ibuffer, *obuffer;
		z_stream z;
		int result;
		
		//Use TGDS malloc/free directly, don't let ZLIB to create buffers internally
		z.zalloc = Z_NULL;
		z.zfree = Z_NULL;
		z.opaque = Z_NULL;
		
		result = inflateInit2( &z,-MAX_WBITS );
		if( result != Z_OK ) {
			char buf[200];
			sprintf(buf,"inflateInit2 failed:%d",result);
			err( buf );
		}
		
		ibuffer = (Bytef *)TGDSARM9Malloc(UNZIPBUFFER_SIZE);
		obuffer = (Bytef *)TGDSARM9Malloc(UNZIPBUFFER_SIZE);
		
		memset ( ibuffer, 0, UNZIPBUFFER_SIZE);
		memset ( obuffer, 0, UNZIPBUFFER_SIZE);
		
		z.next_in = (Bytef *)ibuffer;
		z.avail_in = 0;
		
		for(;;){
			
			z.next_out = (Bytef *)obuffer;
			z.avail_out = UNZIPBUFFER_SIZE;
		
			if( z.avail_in == 0 ){
				z.next_in = ibuffer;
				if(size>=0){
					if(size>0){
						z.avail_in = fread( ibuffer, 1, min(size,UNZIPBUFFER_SIZE), in );
						size-=min(size,UNZIPBUFFER_SIZE);
					}
				}else{
					z.avail_in = fread( ibuffer, 1, UNZIPBUFFER_SIZE, in );
				}
			}
			result = inflate( &z, Z_SYNC_FLUSH );
			if( result != Z_OK && result != Z_STREAM_END ) {
				inflateEnd( &z );
				char buf[200];
				sprintf(buf,"inflate error:%d",result);
				err(buf);
			}
			
			int wrote = fwrite( obuffer, 1, UNZIPBUFFER_SIZE - z.avail_out, out );
			if(wrote >= 0){
				sint32 FDToSync = fileno(out);
				fsync(FDToSync);
			}
			if(result==Z_STREAM_END){
				break;
			}
			
		}
		
		inflateEnd( &z );
		TGDSARM9Free(ibuffer);
		TGDSARM9Free(obuffer);
		
	}else{ //stored
		while (size--) {
			int c = fgetc(in);fputc(c,out);
		}
	}	
	fclose(in);
	fclose(out);
	//should check CRC32 but...
	return 0;
}


//UserCode: (char*)"path1ToFileZipped", (char*)Must be an EMPTY char[MAX_TGDSFILENAME_LENGTH+1] as it stores the Zipped internal filename.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int load_gz(char *fname, char *newtempfname)
{
	return funzipstdio(fname, newtempfname);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u8* xenoTGDSARM9Malloc(int size){
	return TGDSARM9Malloc(size);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void xenoTGDSARM9Free(void *ptr){
	TGDSARM9Free(ptr);
}

//void TGDSARM9Free(void *ptr)