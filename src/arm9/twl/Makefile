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

#Prepare ARM9 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM9_HEADER),-I "$(CURDIR)/$(dirres)" )	$(DIRLIBS_ALLOWED)
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM9_SRC),-I "$(CURDIR)/$(dirres)" )

dir_arm9_src_h=  $(foreach dir,$(DIRS_ARM9_HEADER), $(wildcard $(CURDIR)/$(RELATIVE_PATH)$(dir)*.h))
file_arm9_src_c=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_s=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )
file_arm9_src_bin=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.bin))  )
file_arm9_src_tga=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.tga))  )
file_arm9_src_pcx=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.pcx))  )

# Object Target
objs_arm9:=	${file_arm9_src_pcx:.pcx=.o}	${file_arm9_src_tga:.tga=.o}	${file_arm9_src_bin:.bin=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}	${file_arm9_src_c:.c=.o}	${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions. -fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
AS_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_AS_COMPILER_FLAGS_9)		$(GLOBAL_AS_COMPILER_FLAGS)
CC_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CC_COMPILER_FLAGS_9)		$(GLOBAL_CC_COMPILER_FLAGS)		-DASM_OPCODES	-fPIE	-std=c99	-DNOCRYPT
CPP_FLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CXX_COMPILER_FLAGS_9)	$(GLOBAL_CXX_COMPILER_FLAGS)	-DASM_OPCODES	-fPIE	-std=c99	-DNOCRYPT
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIRS_ARM9_SRC)
################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(TARGET_LIBRARY_FILE_9))

#------------------------------------------ CLANG START
CC := clang-8
AR2 := llvm-ar
DEP_DIR := $(BUILD_DIR)/dep
DEP_CFLAGS := -MT $@ -MMD -MP 

# CFLAGS which can provided from the CLI
# They will be appended last and will consequently override
# any pre-existing settings
CLI_CFLAG_OVERRIDES ?=
CLI_LDFLAG_OVERRIDES ?=

# Note: These arguments will be used by clang to select
# the specific architecture target when we use the
# generic --target=arm-none-eabi
ARCH_CFLAGS += \
  -marm 	\
  -mcpu=arm946e-s \
  -mfloat-abi=soft	\
  -mfpu=none	

COMPILER_SPECIFIC_CFLAGS += \
  --target=arm-none-eabi \
  --sysroot= \
  -Os \
  -DARM9	\
  -DTWLMODE
  
CFLAGS = \
  -g3 \
  -ffunction-sections -fdata-sections \
  -nostdlib \
  -fPIC	\
  -fshort-enums	\
  $(COMPILER_SPECIFIC_CFLAGS) \
  $(ARCH_CFLAGS) \
  $(NEWLIB_NANO_COMPILE_FLAGS_C_9)	\
  $(CLI_CFLAG_OVERRIDES)	\
  -DNOCRYPT
	
#------------------------------------------ CLANG END

all: $(OBJDIR)|forceDelete	$(OBJPROG)
#all: debug

forceDelete:
	-@rm -fr ./$(BUILD)/$(TARGET_LIBRARY_FILE_9)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)

#Check $(OBJDIR) if not exists: create it
$(OBJDIR):
	@echo "Folder $(OBJDIR) does not exist. Creating"
	mkdir -p $@

#rule for reading *.bin in /data folder, converts to .C source code, which then gets compiled as object so it can be linked later
$(OBJDIR)/%.o : %.bin
	-@echo 'trying to convert .BIN ($<) -> .C output: ${@:.o=.c} : arrayName: $(subst build/,,${@:.o=})'
	$(BIN2C)	$<	${@:.o=.c}	$(subst build/,,${@:.o=})
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ ${@:.o=.c}

#convert a TGA BMP16 into a native 3D texture unit format (GL_COLOR 16bpp)
$(OBJDIR)/%.o : %.tga
	-$(GRITTOOL) ${<:.o=.c} -ftc -o $@ -gu32 -gb -gB16
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ ${@:.o=.c}	

$(OBJDIR)/%.o : %.pcx	
	-@echo 'trying to convert .BIN ($<) -> .C output: ${@:.o=.c} : arrayName: $(subst build/,,${@:.o=})'
	$(BIN2C)	$<	${@:.o=.c}	$(subst build/,,${@:.o=})
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ ${@:.o=.c}

#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ $< 
	-@echo 'C Compile End: $@'

#Do C++ (CXX) Compile now!
$(OBJDIR)/%.o : %.cpp
	-@echo 'C++ Compile start: $@'
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ $< 
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
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Linking phase'
	$(AR2) rc	$@	$^
ifeq ($(TGDS_ENV),windows)
	-cp	$(OBJPROG)	$(TARGET_LIBRARY_PATH)
	-cp $(dir_arm9_src_h) $(TARGET_LIBRARY_HEADERS_9)
endif
ifeq ($(TGDS_ENV),linux)
	-sudo cp	$(OBJPROG)	$(TARGET_LIBRARY_PATH)
	-sudo cp $(dir_arm9_src_h) $(TARGET_LIBRARY_HEADERS_9)
endif
	-@echo 'Leaving Linking phase. $(OBJPROG)'
	
################################################################################################
clean:
	-@echo 'cleanup $(TARGET_LIBRARY_FILE_9)'
ifeq ($(TGDS_ENV),windows)
	-@rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)	$(TARGET_LIBRARY_HEADERS_9)
endif
ifeq ($(TGDS_ENV),linux)
	-@sudo rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)	$(TARGET_LIBRARY_HEADERS_9)
endif
