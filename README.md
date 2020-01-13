![ToolchainGenericDS](img/TGDS-Logo.png)

This is the ToolchainGenericDS 1.5 library for NintendoDS:

[Building the devkit]:
- Recompile Newlib 2.1 for Nintendo DS (mandatory, not on this guide). See: https://bitbucket.org/Coto88/newlib-nds
- Recompile ToolchainGenericDS (this guide)
- Recompile ToolchainGenericDS Project (optional, not on this guide)

After you followed "Recompile Newlib 2.1 for Nintendo DS" you can follow the upcoming steps.


Build Steps:

[Windows]

1.1
-	Extract to some directory the ToolchainGenericDS sources, open msys.bat, and go to the desired directory (through msys)

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

Thanks to:
- Martin Korth extensive gba/nds docs (http://problemkaputt.de/gbatek.htm) most of any DS toolchains/emus out there would not exist without such docs.
- Darkfader for ndstool
- Archeide for NDS printf render code
- TricksterGuy: https://github.com/TricksterGuy/nin10kit (tool to convert ToolchainGenericDS Logo into DS BMP format)
- freelogodesign.org (ToolchainGenericDS Logo)

---------------------------------------------

Developers:

I have been working on this for at least 2 years, but development started circa 2014. I just needed the "motivation" to gather the pieces together, and I am glad I wrote this.
You are free to do almost whatever you want with it, just release the source codes so we all can benefit from it.

The idea is to have a toolchain that is GNU GPL v2+ (so you can either re-release your sources as GNU GPL v2, GNU GPL v2+ or GNU GPL v3).

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
	-WIP: 3D Support

The environment uses GCC 4.9.2 to build Newlib 2.1 for Nintendo DS, and ToolchainGenericDS.


How to code things:
-	Once you follow [Building the devkit], you can grab a TGDS Project template such as: https://bitbucket.org/Coto88/toolchaingenericds-template/ and rebuild it (a TGDS Project Makefile has some properties you can override).


Coto.
