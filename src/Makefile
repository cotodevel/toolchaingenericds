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

#toolchain version
export TOOLCHAIN_VERSION_HEADER =	0.1
export TOOLCHAIN_VERSION =	"$(TOOLCHAIN_VERSION_HEADER)"

# MISC/Dir Definitions
export DIR_ARM7 = arm7
export BUILD_ARM7	=	build

export DIR_ARM9 = arm9
export BUILD_ARM9	=	build


# Shared
export GCC_BUILD_ENV	= $(DEFAULT_GCC_PATH)
export GCC_BIN_PATH	= bin/
export ASGCC	=	arm-none-eabi-as.exe
export GCC_C++	=	arm-none-eabi-gcc.exe
export GCC_OBJCOPY	=	arm-none-eabi-objcopy.exe
export CC = $(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(GCC_C++)
export AS	= $(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(ASGCC)
export OBJCOPY	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(GCC_OBJCOPY)
export NDSTOOL	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)ndstool.exe
export AR	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)arm-none-eabi-ar.exe



#GCC Defs
export TARGET_LIBRARY_NAME_7 = toolchaingen7
export TARGET_LIBRARY_NAME_9 = toolchaingen9
export TOOLCHAIN_LIBRARY_SRCPATH = $(GCC_BUILD_ENV)arm-none-eabi
export LIBPATH	= 	$(TOOLCHAIN_LIBRARY_SRCPATH)/lib/
export DIRLIBS_SHARED =	-I	"$(TOOLCHAIN_LIBRARY_SRCPATH)/include"
export TARGET_LIBRARY_PATH =	$(TOOLCHAIN_LIBRARY_SRCPATH)/lib/toolchain_generic
export TARGET_LIBRARY_LINKER_SRC = /linkers
export TARGET_LIBRARY_LINKER_DEST = $(TARGET_LIBRARY_PATH)
export TARGET_LIBRARY_HEADERS_7 = $(TARGET_LIBRARY_PATH)/headers7
export TARGET_LIBRARY_HEADERS_9 = $(TARGET_LIBRARY_PATH)/headers9
export TARGET_LIBRARY_FILE_7	=	lib$(TARGET_LIBRARY_NAME_7).a
export TARGET_LIBRARY_FILE_9	=	lib$(TARGET_LIBRARY_NAME_9).a
export TARGET_LIBRARY_LIBNAME_7	=	l$(TARGET_LIBRARY_NAME_7)
export TARGET_LIBRARY_LIBNAME_9	=	l$(TARGET_LIBRARY_NAME_9)

#default built-in rules removal
MAKEFLAGS += --no-builtin-rules

.SUFFIXES:
.SUFFIXES: .all

#####################################################ARM7#####################################################
#the correct format is : ../include/sys/ otherwise recursive folders break in later bash scripts
export DIRS_ARM7_SRC = source/	\
			source/wireless/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/	\
			../common/interrupts/
			
export DIRS_ARM7_HEADER = source/	\
			source/wireless/	\
			include/	\
			../include/	\
			../include/netinet/	\
			../include/sys/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/	\
			../common/interrupts/
			
#####################################################ARM9#####################################################

export DIRS_ARM9_SRC = source/	\
			source/arm9_driver/	\
			source/arm9_driver/console/	\
			source/arm9_driver/console/gui_draw/source/	\
			source/arm9_driver/fatfs-newlib-nano-layer/	\
			source/arm9_driver/fatfs-newlib-nano-layer/dldi/	\
			source/arm9_driver/fatfs-newlib-nano-layer/fatfs/source/	\
			source/dswifi_arm9/	\
			source/wnifilib/	\
			../include/	\
			../include/netinet/	\
			../include/sys/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/	\
			../common/interrupts/
			
export DIRS_ARM9_HEADER = source/	\
			source/arm9_driver/	\
			source/arm9_driver/console/	\
			source/arm9_driver/console/gui_draw/include/	\
			source/arm9_driver/fatfs-newlib-nano-layer/	\
			source/arm9_driver/fatfs-newlib-nano-layer/dldi/	\
			source/arm9_driver/fatfs-newlib-nano-layer/fatfs/source/	\
			source/dswifi_arm9/	\
			source/wnifilib/	\
			../include/	\
			../include/netinet/	\
			../include/sys/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/	\
			../common/interrupts/

#cleanup default C/C++/ASM flags
export CFLAGS	=''
export CPPFLAGS	=''
export CPPFLAGS	=''
export ASFLAGS	=''

# Build Target(s)	(both processors here)
all: compilelib

#Make
compilelib	:
	$(MAKE)	-R	-C	$(DIR_ARM7)/
	$(MAKE)	-R	-C	$(DIR_ARM9)/
	-mkdir -p $(TARGET_LIBRARY_LINKER_DEST) && cp	-r $(CURDIR)$(TARGET_LIBRARY_LINKER_SRC) $(TARGET_LIBRARY_LINKER_DEST)
	
#---------------------------------------------------------------------------------
# Clean
	
clean:
	$(MAKE)	clean	-C	$(DIR_ARM7)/
	$(MAKE)	clean	-C	$(DIR_ARM9)/
	-@rm -rf $(TARGET_LIBRARY_PATH)