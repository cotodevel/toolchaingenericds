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

export TARGET_LIBRARY_NAME_7 = toolchaingen7
export TARGET_LIBRARY_NAME_9 = toolchaingen9
export TARGET_LIBRARY_FILE_7	=	lib$(TARGET_LIBRARY_NAME_7).a
export TARGET_LIBRARY_FILE_9	=	lib$(TARGET_LIBRARY_NAME_9).a
export TARGET_LIBRARY_LIBNAME_7	=	l$(TARGET_LIBRARY_NAME_7)
export TARGET_LIBRARY_LIBNAME_9	=	l$(TARGET_LIBRARY_NAME_9)


DIR_LIB_SRC_CWD := 	src/	\
					src/fonts
DIR_LIB_HDR_CWD := 	includeWoopsi/	\
					includeWoopsi/fonts	\
					build 	\
					lib/libfreetype/include	\
					lib/libfreetype/include/freetype	\
					lib/libfreetype/include/freetype/config	\
					lib/libfreetype/include/freetype/internal	\
					lib/libfreetype/include/freetype/internal/services	\
					
LIB_NAME := libwoopsi

#Prepare ARM9 dirs

DIRLIBS_HDR = $(foreach dirres,$(DIR_LIB_HDR_CWD),-I "$(dirres)" )	$(DIRLIBS_ALLOWED)	$(DIRLIBS_PROHIBITED)	$(TARGET_LIBRARY_HEADERS_9_INC)
DIRLIBS_SRC = $(foreach dirres,$(DIR_LIB_SRC_CWD),-I "$(dirres)" )

dir_arm9_src_h=  $(foreach dir,$(DIR_LIB_HDR_CWD), $(wildcard $(RELATIVE_PATH)$(dir)*.h))	$(foreach dir,$(DIRS_ARM9_HEADER), $(wildcard $(CURDIR)/$(RELATIVE_PATH)$(dir)*.h))


file_arm9_src_c=  $(foreach dir,$(DIR_LIB_SRC_CWD), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_s=  $(foreach dir,$(DIR_LIB_SRC_CWD), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIR_LIB_SRC_CWD), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIR_LIB_SRC_CWD), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )
file_arm9_src_bin=  $(foreach dir,$(DIR_LIB_SRC_CWD), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.bin))  )

# Object Target
objs_arm9:=	${file_arm9_src_bin:.bin=.o}	${file_arm9_src_c:.c=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}		${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions. -fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
AS_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_AS_COMPILER_FLAGS_9)		$(GLOBAL_AS_COMPILER_FLAGS)
CC_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CC_COMPILER_FLAGS_9)		$(GLOBAL_CC_COMPILER_FLAGS)
CPP_FLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CXX_COMPILER_FLAGS_9)	$(GLOBAL_CXX_COMPILER_FLAGS)
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIR_LIB_SRC_CWD)
################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(LIB_NAME))

all: $(OBJDIR)|$(OBJPROG).a
#all: debug

#Check $(OBJDIR) if not exists: create it
$(OBJDIR):
	@echo "Folder $(OBJDIR) does not exist. Creating"
	mkdir -p $@

#rule for reading *.bin in /data folder, converts to .C source code, which then gets compiled as object so it can be linked later
$(OBJDIR)/%.o : %.bin
	-@echo 'trying to convert .BIN ($<) -> .C output: ${@:.o=.c} : arrayName: $(subst build/,,${@:.o=})'
	$(BIN2C)	$<	${@:.o=.c}	$(subst build/,,${@:.o=})
	$(CC)	$(CC_ASFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c ${@:.o=.c}

#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC)	$(CC_ASFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Do C++ (CXX) Compile now!
$(OBJDIR)/%.o : %.cpp
	-@echo 'C++ Compile start: $@'
	$(CPP)	$(CPP_FLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Coto: Special "exception" rule. Assembler (.s) support ie: .ifdef/.endif (which differs from CPP-like syntax Assembler: ie: #ifdef/#endif by .S)
#.S is GCC CC/CPP for compiling assembler with cpp headers
#.s is GCC AS Assembler for compiling assembler the usual way

#Do s (GCC AS) Compile now!
$(OBJDIR)/%.o : %.s
	-@echo 's Compile start: $@'
	$(AS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(AS_ASFLAGS)	-o $@ -c $<
	-@echo 's Compile end: $@'

#Do S (GCC CC) Compile now!
$(OBJDIR)/%.o : %.S
	-@echo 'S Compile start: $@'
	$(CPP)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $@ -c $<
	-@echo 'S Compile end: $@'

#Builder.
$(OBJPROG).a: $(addprefix $(OBJDIR)/, $(OBJECTS))
	$(AR) rvs	$@	$^
ifeq ($(TGDS_ENV),windows)
	-mv	$@	$(TARGET_LIBRARY_PATH)/$(LIB_NAME).a
	-cp -R includeWoopsi/	$(TARGET_LIBRARY_PATH)
endif
ifeq ($(TGDS_ENV),linux)
	-sudo 	mv	$@	$(TARGET_LIBRARY_PATH)/$(LIB_NAME).a
	-sudo	cp -R includeWoopsi/	$(TARGET_LIBRARY_PATH)
endif
	
################################################################################################
clean:
	-@echo 'cleanup $(OBJPROG).a'
ifeq ($(TGDS_ENV),windows)
	-@rm -fr $(OBJDIR)	$(OBJPROG).a	$(TARGET_LIBRARY_PATH)/$(LIB_NAME).a	$(TARGET_LIBRARY_PATH)/includeWoopsi/
endif
ifeq ($(TGDS_ENV),linux)
	-@sudo -@rm -fr $(OBJDIR)	$(OBJPROG).a	$(TARGET_LIBRARY_PATH)/$(LIB_NAME).a	$(TARGET_LIBRARY_PATH)/includeWoopsi/
endif

debug:
	-@echo 'files: $(objs_arm9)'
	