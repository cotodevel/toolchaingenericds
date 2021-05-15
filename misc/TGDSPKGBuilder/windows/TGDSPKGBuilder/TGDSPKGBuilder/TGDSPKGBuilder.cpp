//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)

// color your text in Windows console mode
// colors are 0=black 1=blue 2=green and so on to 15=white
// colorattribute = foreground + background * 16
// to get red text on yellow use 4 + 14*16 = 228
// light red on yellow would be 12 + 14*16 = 236
// a Dev-C++ tested console application by vegaseat 07nov2004

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#ifdef _WIN32
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <windows.h> // WinApi header
using namespace std; // std::cout, std::cin
#include <vector>
#include <string>

#include "crc32Tool.h"

enum COLOR
 {
     // Text foreground colors
     // Standard text colors
     GRAY_TEXT=8,      BLUE_TEXT,       GREEN_TEXT,
     TEAL_TEXT,        RED_TEXT,        PINK_TEXT,
     YELLOW_TEXT,      WHITE_TEXT,
     // Faded text colors
     BLACK_TEXT=0,     BLUE_FADE_TEXT,  GREEN_FADE_TEXT,
     TEAL_FADE_TEXT,   RED_FADE_TEXT,   PINK_FADE_TEXT,
     YELLOW_FADE_TEXT, WHITE_FADE_TEXT,
     // Standard text background color
     GRAY_BACKGROUND=GRAY_TEXT<<4,     BLUE_BACKGROUND=BLUE_TEXT<<4,
     GREEN_BACKGROUND=GREEN_TEXT<<4,   TEAL_BACKGROUND=TEAL_TEXT<<4,
     RED_BACKGROUND=RED_TEXT<<4,       PINK_BACKGROUND=PINK_TEXT<<4,
     YELLOW_BACKGROUND=YELLOW_TEXT<<4, WHITE_BACKGROUND=WHITE_TEXT<<4,
     // Faded text background color
     BLACK_BACKGROUND=BLACK_TEXT<<4,           BLUE_FADE_BACKGROUND=BLUE_FADE_TEXT<<4,
     GREEN_FADE_BACKGROUND=GREEN_FADE_TEXT<<4, TEAL_FADE_BACKGROUND=TEAL_FADE_TEXT<<4,
     RED_FADE_BACKGROUND=RED_FADE_TEXT<<4,       PINK_FADE_BACKGROUND=PINK_FADE_TEXT<<4,
     YELLOW_FADE_BACKGROUND=YELLOW_FADE_TEXT<<4, WHITE_FADE_BACKGROUND=WHITE_FADE_TEXT<<4
 };

#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

#endif

// TGDSPKGBuilder.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include "tarball.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>

#ifdef _WIN32
static bool resetTextColor(HANDLE h)
{
    return SetConsoleTextAttribute(h, WHITE_FADE_TEXT);
}

static void TGDSPackageBuilderHelp(HANDLE winHandle){
	printf("Usage: \n");
	SetConsoleTextAttribute(winHandle, 31);
	printf("TGDSPKGBuilder TGDSProjectName [/baseTargetDecompressorDirectory] [/TGDSLibrarySourceDirectory] [/TGDSPKGOutDirectory] [/TGDSProjectSourceFolder] \n");
	SetConsoleTextAttribute(winHandle, 14);
}
#endif

std::vector<std::string>
list_directory(
    const std::string &directory)
{
    WIN32_FIND_DATAA findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    std::string full_path = directory + "\\*";
    std::vector<std::string> dir_list;

    hFind = FindFirstFileA(full_path.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Invalid handle value! Please check your path...");

    while (FindNextFileA(hFind, &findData) != 0)
    {
        dir_list.push_back(std::string(findData.cFileName));
    }

    FindClose(hFind);

    return dir_list;
}

#ifdef __cplusplus
extern "C" {
#endif
//Build the TAR
extern int file_compress(char  *file, char  *mode);
#ifdef __cplusplus
}
#endif

int main( int argc, char *argv[] )
{
	
	HANDLE hConsole;
	int k;
	
	//printf("DEBUGGER: argv 0: %s\n", argv[0]);	// C:\toolchain_generic\6.2_2016q4\bin\TGDSPKGBuilder.exe
	//printf("DEBUGGER: argv 1: %s\n", argv[1]);	// ToolchainGenericDS-template
	//printf("DEBUGGER: argv 2: %s\n", argv[2]);	// TGDSPKG_template
	//printf("DEBUGGER: argv 3: %s\n", argv[3]);	// c:/toolchain_generic/6.2_2016q4/arm-eabi/lib/
	//printf("DEBUGGER: argv 5: %s\n", argv[4]);	// /release/arm7dldi-ntr
	
	/* avoid end-of-line conversions */
    SET_BINARY_MODE(stdin);
    SET_BINARY_MODE(stdout);

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (argc < 3){
		TGDSPackageBuilderHelp(hConsole);
		return -1;
	}
#ifdef _WIN32
	/*
	// Console font color change
	for(k = 1; k < 255; k++)
	{
		// pick the colorattribute k you want
		SetConsoleTextAttribute(hConsole, k);
		cout << k << " COLOR TEST." << endl;
	}
	cin.get(); // wait
	*/
	SetConsoleTextAttribute(hConsole, 14);
#endif

	// "/TGDSLibrarySourceDirectory"
	char TGDSLibrarySourceDirectory[256+1];
	strcpy(TGDSLibrarySourceDirectory, argv[3]); //strcpy(TGDSLibrarySourceDirectory, "C:\\toolchain_generic\\6.2_2016q4\\arm-eabi\\lib\\newlib-nano-2.1-nds\\");

	// "/baseTargetDecompressorDirectory"
	char baseTargetDecompressorDirectory[256+1];
	if((argv[2] != NULL) && (strlen(argv[2]) > 1)){
		strcpy(baseTargetDecompressorDirectory, argv[2]); //strcpy(baseTargetDecompressorDirectory, "TGDSbaseTargetDecompressorDirectory/");
		strcat(baseTargetDecompressorDirectory, "/");
	}
	else{
		strcpy(baseTargetDecompressorDirectory, "");
	}
	
	char TGDSProjectName[256+1];
	strcpy(TGDSProjectName, argv[1]); //strcpy(TGDSProjectName, "ToolchainGenericDS-template");
	char TGDSMainApp[256+1];
	strcpy(TGDSMainApp, TGDSProjectName);
	strcat(TGDSMainApp, ".nds");
	
	//ok

	//Output Directory
	TCHAR Buffer[MAX_PATH];
	DWORD dwRet;
	dwRet = GetCurrentDirectory(MAX_PATH, Buffer);
	char converted[MAX_PATH];
	wcstombs(converted, Buffer, wcslen(Buffer) + 1);
	char outputPKGPath[256+1];
	strcpy(outputPKGPath, (string(converted) + string("\\")).c_str() ); //strcpy(outputPKGPath, (string(converted) + string("\\..\\Debug")).c_str() );
	
	//ok

	printf("Source files Directory: %s\n", argv[4]);
	std::vector<std::string> vec = list_directory(string(converted) + string(argv[4]));	
	printf("Source files Count: %d\n", vec.size());
	
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;
	int crc32mainApp =-1;
	int crc32TGDSSDK =-1;

	/* open file for writing */
	char fullPathOuttar[256+1];
		//strcpy(fullPathOuttar, "..\\Debug\\");
		//strcat(fullPathOuttar, (string(TGDSProjectName) + string(".tar")).c_str());
		
		memset(fullPathOuttar, 0, sizeof(fullPathOuttar));
		strcpy(fullPathOuttar, outputPKGPath);
		strcat(fullPathOuttar, (string(argv[1]) + string(".tar")).c_str());

	
	/* open file for writing */
	std::fstream out(fullPathOuttar,std::ios::out | std::ios::binary);
	printf("Tar Out: %s\n", fullPathOuttar);
	
	//ok

	if(!out.is_open())
		{
		std::cerr << "Cannot open out: " << string(fullPathOuttar) << std::endl;
		return EXIT_FAILURE;
		}
	/* create the tar file */
	lindenb::io::Tar tarball(out);
	
	for(int i = 0; i < vec.size(); i++){
		char * filename = (char*)vec.at(i).c_str();
		if( (string(filename) != "..") && ((string(TGDSProjectName) + string(".tar")) != string(filename)) && ((string(TGDSProjectName) + string(".tar.gz")) != string(filename)) ){
			
			/* add a file */
			char fullPathIn[256+1];
			//strcpy(fullPathIn, "..\\Debug\\");
			//strcat(fullPathIn, filename);

			strcpy(fullPathIn, outputPKGPath);
			strcat(fullPathIn, (argv[4] + 1));
			strcat(fullPathIn, filename);
			
			printf("TAR: Add File: %d: %s \n", i, fullPathIn);
			printf("into: [%s] \n", (string(baseTargetDecompressorDirectory) + string(filename)).c_str());
			
			tarball.putFile(fullPathIn, (string(baseTargetDecompressorDirectory) + string(filename)).c_str());
			
			//Found mainApp?
			if(string(TGDSMainApp) == string(filename)){
				//unsigned long crc32 = -1;
				FILE* TGDSLibraryFile = fopen(fullPathIn,"rb");
				int err = Crc32_ComputeFile(TGDSLibraryFile, (unsigned __int32*)&crc32mainApp);
				fclose(TGDSLibraryFile);

				printf("mainApp[%s] CRC32: %x\n", filename, crc32mainApp);
			}
			
		}
	}
	
	//ok
	
	//crc32TGDSSDK
	//libcnano7.a
	//libcnano9.a
	//libtoolchaingen7.a
	//libtoolchaingen9.a
	int crc32TGDSSDKlibcnano7 = -1;
	int crc32TGDSSDKlibcnano9 = -1;
	int crc32TGDSSDKlibtoolchaingen7 = -1;
	int crc32TGDSSDKlibtoolchaingen9 = -1;
	int err = 0;
	FILE* TGDSLibraryFile = NULL;
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libcnano7.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){
		err = Crc32_ComputeFile(TGDSLibraryFile, (unsigned __int32*)&crc32TGDSSDKlibcnano7);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libcnano7.a missing. Make sure you build newlib-nds first!");
		return -1;
	}

	
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libcnano9.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (unsigned __int32*)&crc32TGDSSDKlibcnano9);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libcnano9.a missing. Make sure you build newlib-nds first!");
		return -1;
	}

	
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libtoolchaingen7.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (unsigned __int32*)&crc32TGDSSDKlibtoolchaingen7);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libtoolchaingen7.a missing. Make sure you build ToolchainGenericDS first!");
		return -1;
	}

	
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libtoolchaingen9.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (unsigned __int32*)&crc32TGDSSDKlibtoolchaingen9);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libtoolchaingen9.a missing. Make sure you build ToolchainGenericDS first!");
		return -1;
	}
	//ok

	/* Write the descriptor */
	char TGDSDescriptorBuffer[256+1];
	sprintf(TGDSDescriptorBuffer, "[Global]\n\nmainApp = %s\n\nmainAppCRC32 = %x\n\nTGDSSdkCrc32 = %x\n\nbaseTargetPath = %s\n\n", TGDSMainApp, crc32mainApp, (crc32TGDSSDKlibcnano7 + crc32TGDSSDKlibcnano9 + crc32TGDSSDKlibtoolchaingen7 + crc32TGDSSDKlibtoolchaingen9), baseTargetDecompressorDirectory);
	
	try{
		tarball.put( (string("descriptor.txt")).c_str(), TGDSDescriptorBuffer);
	}
	catch(exception ex){
		printf("descriptor.txt creating fail");
		return -1;
	}

	/* finalize the tar file */
	tarball.finish();
	/* close the file */
	out.close();
	/* we're done */

	//ok

	//gz zip the tarball
	if(file_compress(fullPathOuttar, "w+b") == Z_OK){
		rename(fullPathOuttar, (string(fullPathOuttar)+string(".gz")).c_str());
		printf("TGDSPKG %s build OK \n", (string(fullPathOuttar)+string(".gz")).c_str());
	}
	else
	{
		printf("TGDSPKG %s build ERROR \n", (string(fullPathOuttar)+string(".gz")).c_str());
	}

	resetTextColor(hConsole);
    return 0;
}

