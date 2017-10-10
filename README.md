This is the Toolchain Generic DS:

For building the toolchain you need three steps:

Recompile newlib-nano2.1 for Nintendo DS (mandatory, not on this guide)

Recompile ToolchainGenericDS (this guide)

Recompile project (optional, not on this guide)

After you followed "Recompile newlib-nano2.1 for Nintendo DS" you need to follow the below steps.


Build Steps:
1)

extract to some directory the ToolchainGenericDS sources, open msys.bat, and go to the desired directory (through msys)

once in that directory, simply:

make clean <enter>
make <enter>

After building ToolchainGenericDS, then you are ready to recompile the project (not covered here, go to the specific project supporting this toolchain)

For Project specifics check out the specific project. Remember that such project must support this toolchain for it to work. 
For more technical informacion refer to SPECIFICS

Thanks to:
- Martin Korth extensive gba/nds docs (http://problemkaputt.de/gbatek.htm) most of any DS toolchains/emus out there would not exist without such docs.
- Darkfader for ndstool
