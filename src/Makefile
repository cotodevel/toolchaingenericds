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

#TGDS1.3 compatible Makefile

#ToolchainGenericDS specific: Use Makefiles from either TGDS, or custom
export SOURCE_MAKEFILE7 = custom
export SOURCE_MAKEFILE9 = custom

#ToolchainGenericDS version
export TOOLCHAIN_VERSION_HEADER =	1.3
export TOOLCHAIN_VERSION =	"$(TOOLCHAIN_VERSION_HEADER)"

# TGDS Generic start
#The ndstool I use requires to have the elf section removed, so these rules create elf headerless- binaries.
export BINSTRIP_RULE_7 =	arm7.bin
export BINSTRIP_RULE_9 =	arm9.bin
export DIR_ARM7 = arm7
export BUILD_ARM7	=	build
export DIR_ARM9 = arm9
export BUILD_ARM9	=	build
export ELF_ARM7 = arm7.elf
export ELF_ARM9 = arm9.elf

# Shared
export GCC_BUILD_ENV	= $(DEFAULT_GCC_PATH)
export GCC_BIN_PATH	= bin/
export ASGCC	=	arm-none-eabi-as.exe
export GCC_C++	=	arm-none-eabi-gcc.exe
export GCC_G++	=	arm-none-eabi-gcc.exe
export GCC_OBJCOPY	=	arm-none-eabi-objcopy.exe
export CC = $(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(GCC_C++)
export AS	= $(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(ASGCC)
export G++	= $(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(GCC_G++)
export OBJCOPY	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(GCC_OBJCOPY)
export NDSTOOL	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)ndstool.exe
export AR	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)arm-none-eabi-ar.exe
export LD	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)arm-none-eabi-ld.exe
export BIN2C	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)bin2c.exe

#GCC Defs
export TARGET_LIBRARY_NAME_7 = toolchaingen7
export TARGET_LIBRARY_NAME_9 = toolchaingen9
export TOOLCHAIN_LIBRARY_SRCPATH = $(GCC_BUILD_ENV)arm-none-eabi
export TOOLCHAIN_PATH	= 	$(TOOLCHAIN_LIBRARY_SRCPATH)/lib/
export LIBPATH	=	$(TOOLCHAIN_PATH)newlib-nano-2.1-nds
export DIRLIBS_PROHIBITED =	"$(TOOLCHAIN_LIBRARY_SRCPATH)/include"
export DIRLIBS_ALLOWED =	-I "$(TOOLCHAIN_PATH)newlib-nano-2.1-nds/include/"	-I "$(TOOLCHAIN_PATH)newlib-nano-2.1-nds/newlib-nds-headers/" -I "$(TOOLCHAIN_PATH)newlib-nano-2.1-nds/newlib-nds-headers/bits/" -I "$(TOOLCHAIN_PATH)newlib-nano-2.1-nds/newlib-nds-headers/machine/" -I "$(TOOLCHAIN_PATH)newlib-nano-2.1-nds/newlib-nds-headers/sys/"
export TARGET_LIBRARY_PATH =	$(TOOLCHAIN_LIBRARY_SRCPATH)/lib/newlib-nano-2.1-nds
export TARGET_LIBRARY_LINKER_SRC = /linkers
export TARGET_LIBRARY_LINKER_DEST = $(TARGET_LIBRARY_PATH)
export TARGET_LIBRARY_MAKEFILES_SRC = /TGDSMakefiles
export TARGET_LIBRARY_MAKEFILES_DEST = $(TARGET_LIBRARY_PATH)
export TARGET_LIBRARY_CRT0_FILE_7 = nds_arm_ld_crt0
export TARGET_LIBRARY_CRT0_FILE_9 = nds_arm_ld_crt0
export TARGET_LIBRARY_LINKER_FILE_7 = $(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_LINKER_SRC)/$(TARGET_LIBRARY_CRT0_FILE_7).S
export TARGET_LIBRARY_LINKER_FILE_9 = $(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_LINKER_SRC)/$(TARGET_LIBRARY_CRT0_FILE_9).S
export TARGET_LIBRARY_LINKER_PATH_LD = $(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_LINKER_SRC)/
export TARGET_LIBRARY_HEADERS_7 = $(TARGET_LIBRARY_PATH)/headers7
export TARGET_LIBRARY_HEADERS_9 = $(TARGET_LIBRARY_PATH)/headers9
export TARGET_LIBRARY_HEADERS_7_INC = -I	"$(TARGET_LIBRARY_PATH)/headers7"
export TARGET_LIBRARY_HEADERS_9_INC = -I	"$(TARGET_LIBRARY_PATH)/headers9"

export TARGET_LIBRARY_FILE_7	=	lib$(TARGET_LIBRARY_NAME_7).a
export TARGET_LIBRARY_FILE_9	=	lib$(TARGET_LIBRARY_NAME_9).a
export TARGET_LIBRARY_LIBNAME_7	=	l$(TARGET_LIBRARY_NAME_7)
export TARGET_LIBRARY_LIBNAME_9	=	l$(TARGET_LIBRARY_NAME_9)

#default built-in rules removal
MAKEFLAGS += --no-builtin-rules

#cleanup default C/C++/ASM flags
export CFLAGS	=''
export CPPFLAGS	=''
export ASFLAGS	=''
export VPATH 	=''
# TGDS Generic end


.SUFFIXES:
.SUFFIXES: .all

#####################################################ARM7#####################################################
#the correct format is : ../include/sys/ otherwise recursive folders break in later bash scripts
export DIRS_ARM7_SRC = source/	\
			source/wireless/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/
			
export DIRS_ARM7_HEADER = source/	\
			source/wireless/	\
			include/	\
			../include/	\
			../include/netinet/	\
			../include/sys/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/
			
#####################################################ARM9#####################################################

export DIRS_ARM9_SRC = source/	\
			source/arm9_driver/	\
			source/arm9_driver/console/	\
			source/arm9_driver/console/gui_draw/source/	\
			source/arm9_driver/fatfs-newlib-nano-layer/	\
			source/arm9_driver/fatfs-newlib-nano-layer/dldi/	\
			source/arm9_driver/fatfs-newlib-nano-layer/fatfs/source/	\
			source/arm9_driver/zlib/	\
			source/arm9_driver/zip/	\
			source/arm9_driver/	\
			source/dswifi_arm9/	\
			source/wnifilib/	\
			../include/	\
			../include/netinet/	\
			../include/sys/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/
			
export DIRS_ARM9_HEADER = source/	\
			source/arm9_driver/	\
			source/arm9_driver/console/	\
			source/arm9_driver/console/gui_draw/include/	\
			source/arm9_driver/fatfs-newlib-nano-layer/	\
			source/arm9_driver/fatfs-newlib-nano-layer/dldi/	\
			source/arm9_driver/fatfs-newlib-nano-layer/fatfs/source/	\
			source/arm9_driver/zlib/	\
			source/arm9_driver/zip/	\
			source/arm9_driver/	\
			source/dswifi_arm9/	\
			source/wnifilib/	\
			../include/	\
			../include/netinet/	\
			../include/sys/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/
			
# Build Target(s)	(both processors here)
all: compilelib

#Make
compilelib	:
	$(MAKE)	-R	-C	$(DIR_ARM7)/
	$(MAKE)	-R	-C	$(DIR_ARM9)/
	
	
#---------------------------------------------------------------------------------
# Clean
clean:
	$(MAKE)	clean	-C	$(DIR_ARM7)/
	$(MAKE)	clean	-C	$(DIR_ARM9)/