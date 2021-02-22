vc++ TGDSPKGBuilder (windows):
TGDSPKGBuilder.exe Binfile.bin Binfile.c Binfile (optional)SectionName

Where Binfile.bin is a raw binary blob that turns into BinFile, a C char[] object which can be recompiled and linked later.
Optionally, if exists, SectionName is the name of the section the raw binary blob will be moved onto. 
If no argument is specified, it goes into "embeddedBinData" section.
