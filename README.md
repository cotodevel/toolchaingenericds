![ToolchainGenericDS](img/TGDS-Logo.png)

This is the ToolchainGenericDS library for NintendoDS:
Environment: Windows GCC 4.9.3 ARM-EABI / Linux GCC 4.9.1 ARM-EABI to build Newlib 2.1 for Nintendo DS, and ToolchainGenericDS.

---------------------------------------------
[Building the devkit]:
- Recompile Newlib for Nintendo DS (mandatory, not on this guide). See: https://bitbucket.org/Coto88/newlib-nds
- Recompile ToolchainGenericDS (this guide)
- Recompile ToolchainGenericDS Project (optional, not on this guide)

After you followed "Recompile Newlib for Nintendo DS" you can follow the upcoming steps.

Build Steps:

[Windows]

1.1
-	Extract to some directory the ToolchainGenericDS sources, open msys.bat, and go to the desired directory (through msys2)

1.2
Then:
 - make clean 
 - make


[Linux]

1.1
-	Extract to some directory the ToolchainGenericDS sources, open the shell/Terminal, and go to the desired directory (through shell).
	[Now follow 1a) step in the below link]
	https://bitbucket.org/Coto88/newlib-nds/src/master/newlib-nds/
	
1.2
Then:
 - make clean 
 - make

Then, only after building ToolchainGenericDS, you will be able to build a TGDS project. (Not covered here, go to the specific project supporting this toolchain)

---------------------------------------------
Thanks to (in no particular order):
- Martin Korth extensive gba/nds docs (http://problemkaputt.de/gbatek.htm) most of any DS toolchains/emus out there would not exist without such docs.
- Darkfader for ndstool
- Archeide for NDS printf render code (console mode)
- TricksterGuy: https://github.com/TricksterGuy/nin10kit (tool to convert ToolchainGenericDS Logo into DS BMP format)
- freelogodesign.org (ToolchainGenericDS Logo)
- Juglak's XMEM part of libriyo (malloc implementation)
- ADPCM Decoder/ Audio Streamer Authors: DiscoStew
- Simian Zombie : WoopsiUI framework and their respective developers
https://forum.gbadev.org/viewtopic.php?f=18&t=16289
- Michael Noland (joat) & Jason Rogers (dovoto): NDSLIB VideoGL implementation and ARM9 Math hardware registers
- CUE : LZSS compression routines
TWL support: Normatt (SD dldi code) / fincs (codec) / WinterMute / others (updated NDSTools, TWL hardware, VideoGL 3D code related to Texture mapping when using Call Lists )
- lifehackerhansol: Testing and debugging a lot of TGDS compatibility across different NTR / TWL hardware.

---------------------------------------------
Guidelines:

- If TGDS homebrew doesn't boot, wait for an error screen to happen, if it does happen, notify me.
- If TGDS homebrew doesn't initialize filesystem, make sure you use FAT32 64K/32K/16K/4K, or FAT16 64K/32K/16K/4K, anything else is untested and unsupported.
- NTR Mode: Make sure to DLDI patch TGDS homebrew. For TGDS homebrew development, you can use TGDS-RAMDISK DLDI or others. TWL mode doesn't care because it maps the internal SD as filesystem.

---------------------------------------------
Development guidelines:

POSIX File Operation flags:
"r" = Read File
"w" = Write File (one way, must not exist, otherwise returns error)
"w+" = Force Write File (creates a blank file if not exists, if exists, it's wiped, then created as blank file again)
"r+" = Opens an existing file for read / write operations, file pointer MUST be manually set through fseek() right after. If not exists an error is returned.

POSIX File Operation misc:
- fwrite to SD/DSi SD MUST BE in chunks of 256 bytes, if it's a larger number doesn't matter, but MUST BE a multiple of 256. Same rules apply if you perform multiple fwrites.
- filename example: FILE * fileHandle = fopen("0:/somedir/somefile.ext", "r"); //same as "POSIX File Operation flags".

ARGV Support:
- If you call TGDS homebrew from libnds homebrew and send arguments, make sure the arguments use the standard libnds format: "fat:/somedir/somefile.ext" for BOTH NTR/TWL modes. 
	This way, TGDS homebrew understands ARGVs perfectly and converts it into TGDS argument format: ("0:/somedir/somefile.ext") on the fly.
	Even if you call libnds homebrew from TGDS, the same ARGV format is converted back to "fat:/somedir/somefile.ext" for BOTH NTR/TWL modes.

TGDS Project whitescreens:
- If for some reason, you're getting whitescreens on compiled TGDS homebrew, first make sure you are not corrupting homebrew itself. If you are sure about that, then possibly the homebrew got
  relocated incorrectly. An easy workaround is to build TGDS project once, open the TGDS binary on a debugger (like DesMuME), and see which core is executing garbage.
  Do NOT clean it, then refer to TGDS Project Makefile on project root folder. SOURCE_MAKEFILE7 or SOURCE_MAKEFILE9 (depending on which core is running garbage), needs to be changed from "default"
  to "custom". Then open the processor folder: /arm7 or /arm9 depending on the above SOURCE_MAKEFILE setting, open the Makefile inside it, and change CPU processor flags at CC_ASFLAGS between:
  -Os, -O1, -Ofast, -O2, then save Makefile, clean TGDS project and rebuild it again, until you have a working TGDS Project again. Repeat these steps to ensure compatibility in NTR and TWL modes!

---------------------------------------------
License guidelines:

Since I use GPLv2 (not GPLv2+) you can relicense your homebrew as GPLv2 or as GPLv2+ or as GPLv3, and if you want to use custom stuff and not add the source code, 
yet link against the library, it's possible to distribute TGDS through a LGPL license. All I can ask from you is to release the source code so people can learn and improve their coding as well, through education.

---------------------------------------------
Features:

- Linkers
- Default Makefiles per project, through template Makefiles. Or you add your own Makefile for each ARM7/9 Core.
- Fat16/32/exFat (fatfs layer that extends POSIX file calls), interrupts, exceptions, touchscreen, spi, clock (from inferno DS), etc
- Console render
- Newlib nano (featuring POSIX filesystem: https://bitbucket.org/Coto88/gccnewlibnano_to_fatfs/, this means UNIX file POSIX layer: fread/fwrite/fopen/fclose/fprintf/etc supports natively DLDI driver)
- Modified dswifi library so it supports valid frames sent/received between DS's:
	-udp nifi 	(like local but through internet, requires a server that forwards WANIP-> local IP).
	-localnifi	
	-GDB Debugger (allows to read NDS memory real-time through TCP!)
	-WIP other features.
- TGDS drivers: 
	- ARM9DLDI and ARM7DLDI (DSi), basic sound playback, FIFO, interrupt handlers, keypad, touchscreen through the TGDS driver (embedded in all TGDS Projects) so you can focus right into coding rather than NDS hardware.
	- TGDS Sound API: Mono / Stereo WAV 8-bit signed -- 16-bit signed -- 24-bit signed -- 32-bit signed sound streaming support + sound samples (sound samples should load onto memory, play them then be deallocated). 
	- TGDS Events: Allow to set several events (like screen power off, keypress or sound play samples, etc depending on scripted files). WIP: Add record event methods.
	- WIP: 3D Support
	- DLDI RAMDISK Support. DLDI in emulator environment.
		Notes:
		DLDI RAMDisk: Download http://memory.dataram.com/products-and-services/software/ramdisk#freeware, mount a RAMDisk, copy files to it. Then use Desmume, choose Slot-2 (Gba slot) -> GBA Cartridge, choose the RAMDisk!. Launch emulator, TGDS Project now works with DLDI (32MB @ 0x08000000)!
	- Optional VSCode IDE: See https://bitbucket.org/Coto88/toolchaingenericds-ide

---------------------------------------------
Changelog:

TGDS1.65: 
- Separate libraries into: ToolchainGenericDS having the core functionality for NTR/TWL Hardware, and libutils. Depending on the TGDS project layout, this allows to create smaller, more efficient binaries.
  Introduce callbacks for extended functionality, stub them if unused. Check out ipcfifoTGDSUser.c's in TGDS project for details.
- Re-introduce linux support (on Ubuntu / Xubuntu 18.04)
- Add better initialization routines. NTR / TWL firmware detection, DLDI startup and TGDS project boot status can now be seen as debug if it didn't boot properly.
- Merge 1.64 fixes and make everything work with default BIOS calls, which enables power saving stuff.
- Fix older TGDS homebrew which used to work... once again.
  
TGDS 1.64:
- TGDS TWL Support, which means TGDS projects now work on real DSi/N2DS/N3DS using TWL hardware through SD hardware, besides NTR mode.

TGDS 1.63.1:
- Bugfixes in audioplayback that emerged between TGDS1.62 and TGDS1.63

TGDS 1.63:
- newlib-nds and ToolchainGenericDS uses Clang (v8.0.1) Compiler (C/C++)! Generates better code also allows to automate unit tests!

TGDS 1.62:
- Restored dswifi 0.4.0 sources. Fixes synchronous socket connections. TGDS now supports async/sync socket operations.

TGDS 1.61:
- Fixed IRQs and audio playback. Audio hears nicely now, no sound clicking anymore!

TGDS 1.6:
- Improved hardware IRQs, improved compatibility with nearly 99% of cards, loaders, emulators, etc. TGDS really works now.
- Removed experimental ARM7 DLDI support because it needed a stable codebase first. This was mostly for DSi support. If someone else is willing to give proper DSi support, I can add such methods back.
- ARM9 payload lzss compression, TGDS Binaries should be about ~40% smaller, and when TGDS project runs, it is decompressed realtime.

TGDS 1.5:
- Added Custom memory allocators. Allows to set a custom memory allocator to ARM7 and ARM9 NDS ARM Core
- Added keyboard support, WAV sound streaming, sound effects playback, Event signaling (which allows to map keys, timeouts, sounds, etc)
- Added ARM7 Filesystem, allows to open a file handle from ARM9 and read/write to it... through interrupts! which takes very little power and is efficient.
- Thousands of micro updates which improve compatibility with legacy NDS homebrew
- Experimental ARM7 DLDI support

TGDS 1.4:
- Improved stability on some cards still broken on 60% of them
- Improved DSWNIFI / TGDS hardware driver support
- Added ToolchainGenericDS Filesystem driver

TGDS 1.3:
Rewrote TGDS hardware drivers / Malloc partially working
- Added DSWNIFI
- ZLIB (Deflate) support

TGDS 1.2:
- Rewrote TGDS hardware drivers

TGDS 1.1:
Implemented DLDI driver / Basic Console rendering support

TGDS 1.0:
- First release, very unstable NDS support.
- Makefiles / linkers / and newlib-nds support through a GPL v2 license
- Adds a template system, where TGDS projects hook to a standard TGDS codebase. Improves scalability from the start.

---------------------------------------------
How to code things:
-	Once you follow [Building the devkit], you can grab a TGDS Project template such as: https://bitbucket.org/Coto88/toolchaingenericds-template/ and rebuild it (a TGDS Project Makefile has some properties you can override).

Coto.
