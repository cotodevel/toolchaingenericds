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

#Shared
include $(DEFAULT_GCC_PATH)Makefile.basenewlib

#Custom
export BINSTRIP_RULE_7 =	arm7.bin
export BINSTRIP_RULE_9 =	arm9.bin
export DIR_ARM7 = arm7
export BUILD_ARM7	=	build
export DIR_ARM9 = arm9
export BUILD_ARM9	=	build
export ELF_ARM7 = arm7.elf
export ELF_ARM9 = arm9.elf

export TARGET_LIBRARY_NAME_7 = toolchaingen7
export TARGET_LIBRARY_NAME_9 = toolchaingen9
export TARGET_LIBRARY_FILE_7	=	lib$(TARGET_LIBRARY_NAME_7).a
export TARGET_LIBRARY_FILE_9	=	lib$(TARGET_LIBRARY_NAME_9).a
export TARGET_LIBRARY_LIBNAME_7	=	l$(TARGET_LIBRARY_NAME_7)
export TARGET_LIBRARY_LIBNAME_9	=	l$(TARGET_LIBRARY_NAME_9)

.SUFFIXES: .all

#####################################################ARM7#####################################################
#the correct format is : ../include/sys/ otherwise recursive folders break in later bash scripts
export DIRS_ARM7_SRC = source/	\
			source/dswifi_arm7/	\
			source/xmalloc/	\
			source/TGDSARM7Logger/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/	\
			../templateCode/source/	\
			../templateCode/data/		\
			data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
export DIRS_ARM7_HEADER = source/	\
			source/dswifi_arm7/	\
			source/xmalloc/	\
			source/TGDSARM7Logger/	\
			include/	\
			../include/	\
			../include/netinet/	\
			../include/sys/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/hardware/	\
			../templateCode/source/	\
			../templateCode/data/		\
			data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
#####################################################ARM9#####################################################

export DIRS_ARM9_SRC = source/	\
			source/arm9_driver/	\
			source/arm9_driver/console/	\
			source/arm9_driver/console/gui_draw/	\
			source/arm9_driver/ToolchainGenericDSFS/	\
			source/arm9_driver/ToolchainGenericDSFS/fatfs/source/	\
			source/arm9_driver/zlib/	\
			source/arm9_driver/zip/	\
			source/arm9_driver/keyboard/	\
			source/dswifi_arm9/	\
			source/wnifilib/	\
			../include/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/dswifi/netinet/	\
			../common/dswifi/sys/	\
			../common/hardware/	\
			../common/hardware/dldi/	\
			../templateCode/source/	\
			../templateCode/data/		\
			data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
export DIRS_ARM9_HEADER = source/	\
			source/arm9_driver/	\
			source/arm9_driver/console/	\
			source/arm9_driver/console/gui_draw/	\
			source/arm9_driver/ToolchainGenericDSFS/	\
			source/arm9_driver/ToolchainGenericDSFS/fatfs/source/	\
			source/arm9_driver/zlib/	\
			source/arm9_driver/zip/	\
			source/arm9_driver/keyboard/	\
			source/dswifi_arm9/	\
			source/wnifilib/	\
			../include/	\
			../common/	\
			../common/arm_driver_shared/	\
			../common/dswifi/	\
			../common/dswifi/	\
			../common/dswifi/netinet/	\
			../common/dswifi/sys/	\
			../common/hardware/	\
			../templateCode/source/	\
			../templateCode/data/		\
			../common/hardware/dldi/	\
			data/	\
			build/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
# Build Target(s)	(both processors here)
all: compilelib

#Make
compilelib	:
	cp	-r	$(CURDIR)/templateCode/	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/
ifeq ($(SOURCE_MAKEFILE7),default)
	cp	-r	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/$(DIR_ARM7)/Makefile	$(CURDIR)/$(DIR_ARM7)
endif
	$(MAKE)	-R	-C	$(DIR_ARM7)/
ifeq ($(SOURCE_MAKEFILE9),default)
	cp	-r	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/$(DIR_ARM9)/Makefile	$(CURDIR)/$(DIR_ARM9)
endif
	$(MAKE)	-R	-C	$(DIR_ARM9)/

#---------------------------------------------------------------------------------
# Clean
clean:
	$(MAKE)	clean	-C	$(DIR_ARM7)/
ifeq ($(SOURCE_MAKEFILE7),default)
	-@rm -rf $(CURDIR)/$(DIR_ARM7)/Makefile
endif
	$(MAKE)	clean	-C	$(DIR_ARM9)/
ifeq ($(SOURCE_MAKEFILE9),default)
	-@rm -rf $(CURDIR)/$(DIR_ARM9)/Makefile
endif
	-@rm -fr $(EXECUTABLE_FNAME)
	-@rm -fr	$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/

commitChanges:
	-@git commit -a	-m '$(COMMITMSG)'
	-@git push origin HEAD