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


# relative path (if you are outside arm9 folder then $(DIR_ARM9), arm7 is $(DIR_ARM7))
RELATIVE_PATH =	

#Prepare ARM9 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM9_HEADER),-I "$(CURDIR)/$(RELATIVE_PATH)$(dirres)" )	$(DIRLIBS_ALLOWED)	$(TARGET_LIBRARY_HEADERS_9_INC)	-I	$(TARGET_LIBRARY_PATH)/includeWoopsi/	-I	$(TARGET_LIBRARY_PATH)/includeWoopsi/fonts
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM9_SRC),-I "$(CURDIR)/$(RELATIVE_PATH)$(dirres)" )

file_arm9_src_c=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_o=  $(foreach dir,$(BUILD), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.o))  )
file_arm9_src_o_out = $(filter-out $(TARGET_LIBRARY_CRT0_FILE_9).o, $(file_arm9_src_o))
file_arm9_src_s=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )
file_arm9_src_bin=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.bin))  )

# Object Target
objs_arm9:=		${file_arm9_src_bin:.bin=.o}	${file_arm9_src_c:.c=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}		${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions.	-fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
AS_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_AS_COMPILER_FLAGS_9)		$(GLOBAL_AS_COMPILER_FLAGS)
CC_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CC_COMPILER_FLAGS_9)		$(GLOBAL_CC_COMPILER_FLAGS)		-fPIE	-pie
CPP_FLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CXX_COMPILER_FLAGS_9)	$(GLOBAL_CXX_COMPILER_FLAGS)	-fPIE	-pie
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIRS_ARM9_SRC)

#--gc-sections: allows relocatable sections linked (already) to be included, otherwise unused sections are discarded.
#check arm9.map "Discarded input sections" to see --gc-sections in action.
LDFLAGS	=	-specs=$(TARGET_LIBRARY_LINKER_PATH_LD)/nds_arm9_ld.specs	-Wl,--no-gc-sections,-Map,$(MAPFILE),-z,defs
LIBRARIES =	-l$(TARGET_LIBRARY_NAME_9)	-lwoopsi	-lstdc++	-lcnano9	-lgcc

################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)	$(file_arm9_src_o_out)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(NONSTRIPELF_ARM9))

#------------------------------------------ CLANG START
LD2 := $(CC)
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
  -Oz	\
  -DARM9
  
CFLAGS = \
  -g3 \
  -nostdlib \
  -fshort-enums	\
  $(COMPILER_SPECIFIC_CFLAGS) \
  $(ARCH_CFLAGS) \
  $(NEWLIB_NANO_COMPILE_FLAGS_C_9)	\
  $(CLI_CFLAG_OVERRIDES)
	
#------------------------------------------ CLANG END

all: makebuild	$(TARGET_LIBRARY_CRT0_FILE_9).o	$(BINSTRIP_RULE_9)
#all: debug

makebuild:
	-mkdir -p $(OBJDIR)

#rule for reading *.bin in /data folder, converts to .C source code, which then gets compiled as object so it can be linked later
$(OBJDIR)/%.o : %.bin
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

#Coto: Special "exception" rule. Fixes tricky projects such as SnemulDS's opcodes2.s where GCC AS is used, while compatibility with .S is required for newer projects.
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

#Special rule: compile crt0 for arm9 entrypoint
$(TARGET_LIBRARY_CRT0_FILE_9).o:	$(TARGET_LIBRARY_LINKER_FILE_9)
	-@echo 'S Compile start: crt0s'
	rm -fr $(BUILD)/$(TARGET_LIBRARY_CRT0_FILE_9).o
	$(CPP)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $(OBJDIR)/$@ -c $<
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)nds_arm9_ld.ld $(CURDIR)
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)nds_arm9_ld.specs $(CURDIR)
	-@echo 'S Compile end: crt0s'

#Linkage. 
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Linking phase'
	$(LD2) $^	$(LDFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_SHARED)	-L	$(TARGET_LIBRARY_PATH)/	-L	$(TOOLCHAIN_PATH)/		$(LIBRARIES)	-L	$(TARGET_LIBRARY_LINKER_PATH_LD)	-o $@
	-@echo 'Leaving Linking phase. Output: $@'

#xxxx-nonstripped.elf -> xxxx.elf STRIP operation
$(ELF_ARM9): $(OBJPROG)
	-@echo '$@ Unstripped. STRIP Start!'
	$(OBJSTRIP)	-s -R .comment -R .gnu.version	$<
	-mv	$<	$@
	
$(BINSTRIP_RULE_9): $(ELF_ARM9)
	-@echo 'STRIP End: Remove ELF header and build baremetal Binary executable: $<	->	$@ !'
	$(OBJCOPY) 	-S -O binary	$<	$@
	-rm	$<
################################################################################################

clean:
	-@echo 'cleanup $(DIR_ARM9)'
	-@rm -fr $(BINSTRIP_RULE_9)	$(MAPFILE)	$(CURDIR)/nds_arm9_ld.specs	$(CURDIR)/nds_arm9_ld.ld