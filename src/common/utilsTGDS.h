/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

#ifndef __utilsTGDS_h__
#define __utilsTGDS_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "limitsTGDS.h"


#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif

#define argvItems (10)

//Method Export
#define PLAINTEXT_METHOD_SEPARATOR	(uint32)(0xfedcba98)	//not valid ARM function
typedef	struct {
	uint32 * cback_address;
	int cback_size;
	char methodname[256];
}METHOD_DESCRIPTOR;
		
//Version Struct
#define PLAINTEXT_VERSION_SEPARATOR	(sint8*)("-")	//sint8 * is char *
typedef	struct {
	char app_version[256];	//"0.6a-mm/dd/yyyy" //generated when parsing config file, section [Version]
	char framework_version[256];	//"0.6a-mm/dd/yyyy" //generated when parsing config file, section [Version]
}VERSION_DESCRIPTOR;

//splitCustom logic
typedef void(*splitCustom_fn)(const char *, size_t, char * ,int indexToLeftOut, char * delim);

//ToolchainGenericDS-LinkedModule format: ver 0.2
#define TGDS_LINKEDMODULE_TOPSIZE	(int)((1024*1024*1) + (256*1024))	//sint8 * is char *
struct TGDS_Linked_Module {
	u32 DSARMRegs[16];
	int	TGDS_LM_Size;	//filled by the linker
	unsigned int	TGDS_LM_Entrypoint; //filled by the linker
	u32 returnAddressTGDSLinkedModule; //TGDS LM implementation called when LM exits (return from module). (1/2)
	u32 returnAddressTGDSMainApp; //TGDS LM implementation called returnAddressTGDSLinkedModule reloads from RAM rather than image. (2/2)
	//argv 
	char args[8][MAX_TGDSFILENAME_LENGTH];
	char *argvs[8];
	int argCount;
	
	//TGDS ARM9.bin defs
	char TGDSMainAppName[MAX_TGDSFILENAME_LENGTH]; //todo: save full name (relative path it was called from) when calling TGDS-LModules
};

//NTR/TWL Binary descriptors
#define isNDSBinaryV1 ((int)0)
#define isNDSBinaryV2 ((int)1)
#define isTWLBinary ((int)2)
#define notTWLOrNTRBinary ((int)-1)

//Interfaces / Callbacks to connect to libutils

//FIFO
typedef void(*HandleFifoNotEmptyWeakRefLibUtils_fn)(uint32 cmd1, uint32 cmd2);

//Wifi
typedef void(*wifiUpdateVBLANKARM7LibUtils_fn)();
typedef void(*wifiInterruptARM7LibUtils_fn)();
typedef void(*timerWifiInterruptARM9LibUtils_fn)();

// SoundStream
typedef void(*SoundStreamTimerHandlerARM7LibUtils_fn)();
typedef void(*SoundStreamStopSoundARM7LibUtils_fn)();
typedef void(*SoundStreamSetupSoundARM7LibUtils_fn)();

//ARM7 Malloc
typedef void(*initMallocARM7LibUtils_fn)(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern size_t ucs2tombs(uint8* dst, const unsigned short* src, size_t len);

#ifdef ARM9

//reserved for project appVersion
extern volatile char app_version_static[256];

//METHOD_HANDLERS
extern METHOD_DESCRIPTOR Methods[8];
extern METHOD_DESCRIPTOR * callback_append_signature(uint32 * func_addr, uint32 * func_addr_end, char * methodname,METHOD_DESCRIPTOR * method_inst);
extern sint32 cback_build();
extern void cback_build_end();
extern sint32 callback_export_buffer(METHOD_DESCRIPTOR * method_inst, uint8 * buf_out);
extern sint32 callback_export_file(char * filename,METHOD_DESCRIPTOR * method_inst);

//VERSION_HANDLERS
extern volatile VERSION_DESCRIPTOR Version[1];

//Apps should update this at bootup
extern sint32 addAppVersiontoCompiledCode(VERSION_DESCRIPTOR * versionInst,char * appVersion,int appVersionCharsize);

//Framework sets this by default
extern sint32 updateVersionfromCompiledCode(VERSION_DESCRIPTOR * versionInst);

extern sint32 updateAssemblyParamsConfig(VERSION_DESCRIPTOR * versionInst);
extern sint32 glueARMHandlerConfig(VERSION_DESCRIPTOR * versionInst,METHOD_DESCRIPTOR * method_inst);

//misc
extern int	split (const sint8 *txt, sint8 delim, sint8 ***tokens);

#ifdef ARM9
extern int	FS_loadFile(sint8 *filename, sint8 *buf, int size);
extern int	FS_saveFile(sint8 *filename, sint8 *buf, int size,bool force_file_creation);
extern int	FS_getFileSize(sint8 *filename);

extern int		setBacklight(int flags);
extern int		FS_extram_init();
extern void		FS_lock();
extern void		FS_unlock();
extern char 	* str_replace (char *string, const char *substr, const char *replacement);
extern sint8 * print_ip(uint32 ip, char * outBuf);

//FileSystem utils
extern sint8 	*_FS_getFileExtension(sint8 *filename);
extern sint8 	*FS_getFileName(sint8 *filename);
extern int		FS_chdir(const sint8 *path);
extern int getLastDirFromPath(char * stream, char * haystack, char * outBuf);

//splitCustom string by delimiter implementation in C, that does not use malloc/calloc and re-uses callbacks
extern int count_substr(const char *str, const char* substr, bool overlap);
extern void splitCustom(const char *str, char sep, splitCustom_fn fun, char * outBuf, int indexToLeftOut);
extern void buildPath(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim);
extern int getLastDirFromPath(char * stream, char * haystack, char * outBuf);
extern int str_split(char * stream, char * haystack, char * outBuf, int itemSize, int blockSize);
extern int inet_pton(int af, const char *src, void *dst);
extern int	setBacklight(int flags);
extern void RenderTGDSLogoMainEngine(u8 * compressedLZSSBMP, int compressedLZSSBMPSize);

//ARGV 
extern int thisArgc;
extern char thisArgv[argvItems][MAX_TGDSFILENAME_LENGTH];
extern void handleARGV();

#endif

#endif

extern bool __dsimode;

extern u32 getValueSafe(u32 * buf);
extern void setValueSafe(u32 * buf, u32 val);
extern int getValueSafeInt(u32 * buf);
extern void setValueSafeInt(u32 * buf, int val);

#ifdef ARM7
extern void sdmmcMsgHandler(int bytes, void *user_data);
extern int sdmmcValueHandler(u32 value, void* user_data);
extern int PowerChip_ReadWrite(int cmd, int data);
extern int led_state;
extern void SetLedState(int state);

extern s16 *strpcmL0;
extern s16 *strpcmL1;
extern s16 *strpcmR0;
extern s16 *strpcmR1;

extern int lastL;
extern int lastR;
extern int multRate;
extern int pollCount;
extern u32 sndCursor;
extern u32 micBufLoc;
extern u32 sampleLen;
extern int sndRate;
#endif


//TWL Bits
extern void twlEnableSlot1();
extern void twlDisableSlot1();
extern void disableSlot1();
extern void enableSlot1();

extern bool sleepIsEnabled;
extern void shutdownNDSHardware();
extern bool isDSiMode();

extern int enterCriticalSection();
extern void leaveCriticalSection(int oldIME);

extern void TurnOnScreens();
extern void TurnOffScreens();

//Interfaces / Callbacks to connect to libutils

//FIFO
extern HandleFifoNotEmptyWeakRefLibUtils_fn libutilisFifoNotEmptyCallback;

//Wifi
extern wifiUpdateVBLANKARM7LibUtils_fn wifiUpdateVBLANKARM7LibUtilsCallback;
extern wifiInterruptARM7LibUtils_fn wifiInterruptARM7LibUtilsCallback;
extern timerWifiInterruptARM9LibUtils_fn timerWifiInterruptARM9LibUtilsCallback;

//SS
extern SoundStreamTimerHandlerARM7LibUtils_fn SoundStreamTimerHandlerARM7LibUtilsCallback;
extern SoundStreamStopSoundARM7LibUtils_fn SoundStreamStopSoundARM7LibUtilsCallback;
extern SoundStreamSetupSoundARM7LibUtils_fn SoundStreamSetupSoundARM7LibUtilsCallback;

//ARM7 custom malloc
extern initMallocARM7LibUtils_fn initMallocARM7LibUtilsCallback;

#ifdef ARM7
extern void initializeLibUtils7(
	HandleFifoNotEmptyWeakRefLibUtils_fn HandleFifoNotEmptyWeakRefLibUtilsCall, //ARM7 & ARM9
	wifiUpdateVBLANKARM7LibUtils_fn wifiUpdateVBLANKARM7LibUtilsCall, //ARM7
	wifiInterruptARM7LibUtils_fn wifiInterruptARM7LibUtilsCall,  //ARM7
	SoundStreamTimerHandlerARM7LibUtils_fn SoundStreamTimerHandlerARM7LibUtilsCall, //ARM7: void TIMER1Handler()
	SoundStreamStopSoundARM7LibUtils_fn SoundStreamStopSoundARM7LibUtilsCall, 	//ARM7: void stopSound()
	SoundStreamSetupSoundARM7LibUtils_fn SoundStreamSetupSoundARM7LibUtilsCall,	//ARM7: void setupSound()
	initMallocARM7LibUtils_fn initMallocARM7LibUtilsCall	//ARM7: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);
);
#endif

#ifdef ARM9
extern struct soundPlayerContext soundData;
extern u32 reloadStatus;
extern bool updateRequested;
#endif

extern bool TGDSMultibootRunNDSPayload(char * filename);
extern char * TGDSPayloadMode;
extern void reportTGDSPayloadMode(u32 bufferSource);

#ifdef ARM9
extern char bufModeARM7[256];
extern void addARGV(int argc, char *argv);
extern int isNTROrTWLBinary(char * filename);
#endif

extern int TGDSProjectReturnFromLinkedModule();	//resides in TGDS App caller address
	extern void TGDSProjectReturnFromLinkedModuleDeciderStub();	//resides in TGDSLinkedModule address. Called when TGDS-LM binary exceeds 1.25M

extern void TGDSProjectRunLinkedModule(char * TGDSLinkedModuleFilename, int argc, char **argv, char* TGDSProjectName);
extern void initSound();
extern void setupLibUtils();

extern void setGlobalArgc(int argcVal);
extern int getGlobalArgc();
extern void setGlobalArgv(char** argvVal);
extern char** getGlobalArgv();

extern int globalArgc;
extern char **globalArgv;

#ifdef __cplusplus
}
#endif