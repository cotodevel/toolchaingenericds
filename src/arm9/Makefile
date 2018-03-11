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
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM9_HEADER),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)/" )	$(DIRLIBS_ALLOWED)
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM9_SRC),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)/" )

dir_arm9_src_h=  $(foreach dir,$(DIRS_ARM9_HEADER), $(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)*.h))
file_arm9_src_c=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_s=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )

# Object Target
objs_arm9:=	${file_arm9_src_c:.c=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}		${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions. -fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
SHARED_FLAGS	=	-mcpu=arm946e-s	-mthumb-interwork
AS_ASFLAGS		=	$(SHARED_FLAGS)	-defsym debug=0	-defsym	readvertoolchain='$(TOOLCHAIN_VERSION)'	-defsym ASM_OPCODES='1'	-defsym	ARM9='1'
CC_ASFLAGS		=	$(SHARED_FLAGS)	-Wall	-DNDS -DASM_OPCODES -DARM9	-fno-common	-mtune=arm946e-s	-marm	-O2 -ffast-math -fomit-frame-pointer	-Ddebug=0	-Dreadvertoolchain='$(TOOLCHAIN_VERSION)'
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIRS_ARM9_SRC)

#--gc-sections: allows relocatable sections linked (already) to be included, otherwise unused sections are discarded.
#check arm9.map "Discarded input sections" to see --gc-sections in action.
LDFLAGS	=	-specs=$(LIBPATH)nds_arm9_ld.specs	-Wl,,--no-gc-sections,-Map,$(MAPFILE)	-nostartfiles
LIBRARIES =


################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(TARGET_LIBRARY_FILE_9))

all: $(OBJDIR)|$(TARGET_LIBRARY_FILE_9)
#all: debug

#Check $(OBJDIR) if not exists: create it
$(OBJDIR):
	@echo "Folder $(OBJDIR) does not exist. Creating"
	mkdir -p $@

#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC)	$(CC_ASFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-nostdinc	$(DIRLIBS_PROHIBITED)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Do C++ (G++) Compile now!
$(OBJDIR)/%.o : %.cpp
	-@echo 'C++ Compile start: $@'
	$(CXX)	$(CC_ASFLAGS)	$(CPP_FLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-nostdinc	$(DIRLIBS_PROHIBITED)	-o $@ -c $<
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
	$(CC)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $@ -c $<
	-@echo 'S Compile end: $@'

#Builder.
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Build phase'
	$(AR) rvs	$@	$^
	-@echo 'Leaving Linking phase. Output: $@'

#build *.o into $(TARGET_LIBRARY_FILE_9).a
$(TARGET_LIBRARY_FILE_9): $(OBJPROG)
ifeq ($(TGDS_ENV),windows)
	-mkdir -p $(TARGET_LIBRARY_PATH) && cp	$(OBJPROG)	$(TARGET_LIBRARY_PATH)
	-mkdir -p $(TARGET_LIBRARY_HEADERS_9) && cp $(dir_arm9_src_h) $(TARGET_LIBRARY_HEADERS_9)
endif
ifeq ($(TGDS_ENV),linux)
	-sudo mkdir -p $(TARGET_LIBRARY_PATH) && sudo cp	$(OBJPROG)	$(TARGET_LIBRARY_PATH)
	-sudo mkdir -p $(TARGET_LIBRARY_HEADERS_9) && sudo cp $(dir_arm9_src_h) $(TARGET_LIBRARY_HEADERS_9)
endif

	-@echo 'build done: $(TARGET_LIBRARY_FILE_9)'
	
################################################################################################
clean:
	-@echo 'cleanup $(TARGET_LIBRARY_FILE_9)'
ifeq ($(TGDS_ENV),windows)
	-@rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)
endif
ifeq ($(TGDS_ENV),linux)
	-@sudo rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)
endif

debug:
	-@echo ''