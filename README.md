This is the Toolchain Generic DS:

For building the toolchain you need three steps:

- Recompile Newlib 2.1 for Nintendo DS (mandatory, not on this guide). See: https://github.com/cotodevel/newlib-nds

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
	Note: EVERYTIME you open a new shell/Terminal to compile TGDS homebrew, you must follow the below steps (at least once, per shell/Terminal),
	If you already recompiled Newlib 2.1 for NDS in the same terminal and you didn't exit, you can skip the below step, and proceed to 1.2
	https://github.com/cotodevel/newlib-nds/tree/master/newlib-nds
	
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
