This is the Toolchain Generic DS:

The structure I have for toolchain + projects is:

a) build toolchain (this is what is covered here)
b) build projects




a) build toolchain
Follow the steps at: https://github.com/cotodevel/ToolchainGenericDS to set up the Nintendo DS "toolchain generic" toolchain, GNU licensed. 

So all the code you write is at least compatible with it.
If you are unsure of the licensing of your modified sources, I honestly don't care what license you may prefer, 
just release your sources so we all can benefit from it. So pull requests for any of my projects are welcome.
(unless you are a condescending elitist dev, then you are not welcome here)

Requirements:
- windows 7 and higher (for now), linux support will come later.
- internet connection, just for the initial setup

Build Steps:
1)
create a folder called "toolchain_generic" without commas, in c:/

Head to
https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

Download and run: 
gcc-arm-none-eabi-6_2-2016q4-20161216-win32.exe 

And install inside the newly created folder:
c:/toolchain_generic/

If not available, you can find a copy of gcc-arm-none-eabi-6_2-2016q4-20161216-win32.exe in /misc folder


You will see "6.2 2016q4" right after choosing the new path, 
rename into "6.2_2016q4" (no whitespaces). Required!!!


2)
add these environment variables (I will assume you know what that is):

key:
DEFAULT_GCC_PATH

value:
/c/toolchain_generic/6.2_2016q4/


3)
run MSYS(MinGW) setup:
https://sourceforge.net/projects/mingw/files/

choose:
mingw-developer-toolkit

Click:
Installation> Apply Changes

When done:
Head to C:/MinGW/msys/1.0/

create a shortcut from msys.bat to your desired place, this is the minimal environment for building your projects!

4)Unzip/Make projects from within:
C:/MinGW/msys/1.0/home


5)ndstool.exe is required for building proper NDS binaries. (v1.31) I honestly wrote a generic NDS Header parser but I would rather use what is already available.

Head to https://www.darkfader.net/ds/ and download: https://www.darkfader.net/ds/files/ndstool.exe

If not available, you can find a copy of ndstool 1.31 in /misc folder

then, move binary to:	/c/toolchain_generic/6.2_2016q4/bin/ folder

6)Build toolchain:

- The toolchain folder is in /src
Run msys.bat, head to /src folder

Make (yes, write Make and build it like any other project).This is required only once, but if you do changes to toolchain, you MUST re-build toolchain.

7)
Head to: C:\toolchain_generic\6.2_2016q4\arm-none-eabi\lib\toolchain_generic

if the following files exist:
/headers7
/headers9
/linkers
libtoolchaingen7.a
libtoolchaingen9.a

Then toolchain setup is complete.

For Project specifics check out the specific project. Remember that such project must support this toolchain for it to work.
 
For more technical informacion refer to SPECIFICS

Thanks to:
- Martin Korth extensive gba/nds docs (http://problemkaputt.de/gbatek.htm) most of any DS toolchains/emus out there would not exist without such docs.
- Darkfader for ndstool
