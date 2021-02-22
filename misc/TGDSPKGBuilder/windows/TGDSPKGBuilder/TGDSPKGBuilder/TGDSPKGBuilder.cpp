// TGDSPKGBuilder.cpp : Defines the entry point for the console application.

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>

//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)

static bool cv_snprintf(char* buf, int len, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int res = vsnprintf((char *)buf, len, fmt, va);
    va_end(va);
#if defined _MSC_VER
    // maybe truncation maybe error 
    if(res < 0)
        //check for last errno 
    res = len -1;
    // ensure null terminating on VS2013	
#if def_MSC_VER<=1800
    buf[res] = 0; 
#endif
#endif
    return res >= 0 && res < len;
}

int main( int argc, char *argv[] )
{
	char *buf;
    char* ident;
    unsigned int i, file_size, need_comma;

    FILE *f_input, *f_output;

    if (argc < 4) {
        fprintf(stderr, "Usage: %s binary_file output_file array_name (optional)SectionName \n", argv[0]);
        return -1;
    }

    f_input = fopen(argv[1], "rb");
    if (f_input == NULL) {
        fprintf(stderr, "%s: can't open %s for reading\n", argv[0], argv[1]);
        return -1;
    }

    // Get the file length
    fseek(f_input, 0, SEEK_END);
    file_size = ftell(f_input);
    fseek(f_input, 0, SEEK_SET);

    buf = (char *)malloc(file_size);
    assert(buf);   

    fread(buf, file_size, 1, f_input);
    fclose(f_input);

	// *.c
    f_output = fopen(argv[2], "w");
    if (f_output == NULL)
    {
        fprintf(stderr, "%s: can't open %s for writing\n", argv[0], argv[1]);
        return -1;
    }

    ident = argv[3];
    
    need_comma = 0;

	char sectionName[256];
	memset(sectionName, 0, sizeof(sectionName));
	if (argv[4] != NULL) {
		strcpy(sectionName, argv[4]);
	}
	else {
		strcpy(sectionName, "embeddedBinData");
	}

    fprintf (f_output, "__attribute__((section(\".%s\"))) \n unsigned char %s[%i] = {", sectionName, ident, file_size);
    for (i = 0; i < file_size; ++i)
    {
        if (need_comma) fprintf(f_output, ", ");
        else need_comma = 1;
        if (( i % 11 ) == 0) fprintf(f_output, "\n\t");
        fprintf(f_output, "0x%.2x", buf[i] & 0xff);
    }
    fprintf(f_output, "\n};\n\n");

	char tempName[512];
	
	int offset = strlen((char*)argv[2]);
    
	#ifdef _MSC_VER
	if(offset>1){
		offset = offset - 1;
	}
	cv_snprintf(tempName, offset, "%s", (char*)argv[2]);	//ignore ending ."c"
	#endif

	#ifndef _MSC_VER
	snprintf(tempName, offset ,"%s",(char*)argv[2]);		//ignore ending ."c"
	#endif
	
	char * charEnd = (char*)(&tempName[offset-2]);
    if( *charEnd != (char)'.'){
		strcat(tempName,".");
	}
	strcat(tempName,"h");
	fprintf(f_output, "__attribute__((section(\".%s\"))) \n int %s_size = sizeof(%s);\n",sectionName,ident,ident);
    fclose(f_output);

	//*.h
	f_output = fopen(tempName, "w");
    
	if (f_output == NULL)
    {
        fprintf(stderr, "can't open %s ", tempName);
        return -1;
    }
	
	fprintf(f_output, "#ifdef __cplusplus \nextern \"C\" {\n#endif\n");
	fprintf(f_output, "	extern unsigned char %s[%i];\n", ident, file_size);
	fprintf(f_output, "	extern int %s_size;\n",ident);
	fprintf(f_output, "#ifdef __cplusplus \n} \n#endif \n");
    fclose(f_output);

	return 0;
}

