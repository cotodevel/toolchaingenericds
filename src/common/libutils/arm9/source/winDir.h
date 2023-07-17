#include "TGDSTypes.h"

#ifdef __cplusplus

#ifndef __winDir__h_
#define __winDir__h_

//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)

#ifdef _WIN32
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <windows.h> // WinApi header
using namespace std; // std::cout, std::cin
#include <vector>
#include <string>

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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#endif
#endif
#endif

#ifdef __cplusplus
extern std::vector<std::string> list_directory(const std::string &directory);
extern std::vector<std::string> findFiles(const std::string &directory, const std::string &extension);
extern std::string getFileName(std::string filePath, bool withExtension);
extern std::string getFileNameNoExtension(std::string filename);
#endif

#ifdef __cplusplus
//wrapper for CRC16 NDS Bios call
extern "C"{
#endif

extern void getCWDWin(char * outPath, char* pathToNavigate);
uint16 swiCRC16(uint16 crc, void * data, uint32 size);

#ifdef __cplusplus
}
#endif

