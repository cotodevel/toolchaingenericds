This is the Toolchain Generic DS:

For building the toolchain you need three steps:

- Recompile Newlib 2.1 for Nintendo DS (mandatory, not on this guide). See: https://bitbucket.org/Coto88/newlib-nds

- Recompile ToolchainGenericDS (this guide)

- Recompile project (optional, not on this guide)

After you followed "Recompile Newlib 2.1 for Nintendo DS" you need to follow the below steps.


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
	https://bitbucket.org/Coto88/newlib-nds/src/dfacc79d6c457ab0e3669569315ba6889e0e337f/newlib-nds/?at=master
	
1.2
Then:
 - make clean 
 - make



After building ToolchainGenericDS, then you are ready to recompile the project (not covered here, go to the specific project supporting this toolchain)

For Project specifics check out the specific project. Remember that such project must support this toolchain for it to work. 
For more technical informacion refer to SPECIFICS

Thanks to:
- Martin Korth extensive gba/nds docs (http://problemkaputt.de/gbatek.htm) most of any DS toolchains/emus out there would not exist without such docs.
- Darkfader for ndstool
