#
#			Copyright (C) 2017  Coto
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful, but
#WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
#USA
#

#TGDS1.6 compatible Makefile

#ToolchainGenericDS specific: Use Makefiles from either TGDS, or custom
export SOURCE_MAKEFILE7 = custom
export SOURCE_MAKEFILE9 = custom

#Translate paths to windows with forward slashes
cpath := $(shell pwd)
ifeq ($(shell uname -o), Cygwin)
    CURDIR := '$(shell cygpath -w -p ${cpath})'
endif

#Shared
include $(DEFAULT_GCC_PATH_WIN)/Makefile.basenewlib

#Custom
export BINSTRIP_RULE_7 =	arm7.bin
export BINSTRIP_RULE_9 =	arm9.bin
export DIR_ARM7 = arm7
export BUILD_ARM7	=	build
export DIR_ARM9 = arm9
export BUILD_ARM9	=	build
export ELF_ARM7 = arm7.elf
export ELF_ARM9 = arm9.elf
#TWL TGDS7 & TGDS9
export TARGET_LIBRARY_NAME_7 = toolchaingen7i
export TARGET_LIBRARY_NAME_9 = toolchaingen9i
export TARGET_LIBRARY_FILE_7	=	lib$(TARGET_LIBRARY_NAME_7).a
export TARGET_LIBRARY_FILE_9	=	lib$(TARGET_LIBRARY_NAME_9).a
export TARGET_LIBRARY_LIBNAME_7	=	l$(TARGET_LIBRARY_NAME_7)
export TARGET_LIBRARY_LIBNAME_9	=	l$(TARGET_LIBRARY_NAME_9)

.SUFFIXES: .all
#####################################################ARM7#####################################################
#the correct format is : ../include/sys/ otherwise recursive folders break in later bash scripts
export DIRS_ARM7_SRC = ../source/	\
			../source/dswifi_arm7/	\
			../source/xmalloc/	\
			../source/twl/	\
			../../common/	\
			../../common/twl/	\
			../../common/hardware/	\
			../../common/hardware/dldi/	\
			../../templateCode/source/	\
			../../templateCode/data/arm7/		\
			../data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
export DIRS_ARM7_HEADER = ../source/	\
			../source/dswifi_arm7/	\
			../source/xmalloc/	\
			../source/twl/	\
			../include/	\
			../../common/	\
			../../common/libhardware/arm7twl/	\
			../../common/libhardware/	\
			../../common/twl/	\
			../../common/hardware/	\
			../../common/hardware/dldi/	\
			../../templateCode/source/	\
			../../templateCode/data/arm7/		\
			../data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
#####################################################ARM9#####################################################

export DIRS_ARM9_SRC = ../source/	\
			../source/console/	\
			../source/console/gui_draw/	\
			../source/imadecoder/	\
			../include/	\
			../../common/	\
			../../common/twl/	\
			../../common/hardware/	\
			../../common/hardware/dldi/	\
			../../templateCode/source/	\
			../../templateCode/data/arm9/		\
			../data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
export DIRS_ARM9_HEADER = ../source/	\
			../source/console/	\
			../source/console/gui_draw/	\
			../source/ToolchainGenericDSFS/	\
			../source/ToolchainGenericDSFS/fatfs/source/	\
			../source/imadecoder/	\
			../include/	\
			../../common/	\
			../../common/libhardware/	\
			../../common/twl/	\
			../../common/hardware/	\
			../../common/hardware/dldi/	\
			../../templateCode/source/	\
			../../templateCode/data/arm9/		\
			../data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			

# Build Target(s)	(both processors here)
all: compilelib	

#Make
compilelib	:
	cp	-r	$(CURDIR)/../src/templateCode/	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/
ifeq ($(SOURCE_MAKEFILE7),default)
	cp	-r	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/$(DIR_ARM7)/Makefile	../src/arm7/twl/
endif
	$(MAKE)	-R	-C	../src/common/libhardware/twl7
	$(MAKE)	-R	-C	../src/arm7/twl/
ifeq ($(SOURCE_MAKEFILE9),default)
	cp	-r	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/$(DIR_ARM9)/Makefile	../src/arm9/twl/
endif
	$(MAKE)	-R	-C	../src/common/libhardware/twl9
	$(MAKE)	-R	-C	../src/arm9/source/ToolchainGenericDSFS/twl
	$(MAKE)	-R	-C	../src/arm9/twl/

#---------------------------------------------------------------------------------
# Clean
clean:
	$(MAKE)	clean	-C	../src/common/libhardware/twl7
	$(MAKE)	clean	-C	../src/arm7/twl/
ifeq ($(SOURCE_MAKEFILE7),default)
	-@rm -rf ../src/arm7/twl/Makefile
endif
	$(MAKE)	clean	-C	../src/common/libhardware/twl9
	$(MAKE)	clean	-C	../src/arm9/source/ToolchainGenericDSFS/twl
	$(MAKE)	clean	-C	../src/arm9/twl/
ifeq ($(SOURCE_MAKEFILE9),default)
	-@rm -rf ../src/arm9/twl/Makefile
endif
	-@rm -fr $(EXECUTABLE_FNAME)
	-@rm -fr	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
#---------------------------------------------------------------------------------
