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

#Prepare ARM7 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM7_HEADER),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)/" )	$(DIRLIBS_SHARED)
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM7_SRC),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)/" )	$(DIRLIBS_SHARED)

dir_arm7_src_h=  $(foreach dir,$(DIRS_ARM7_HEADER), $(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)*.h))

file_arm7_src_c=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm7_src_s=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm7_src_SUPP=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )

# Object Target
objs_arm7:=	${file_arm7_src_c:.c=.o}	${file_arm7_src_s:.s=.o}	${file_arm7_src_SUPP:.S=.o}
d_objs_arm7:=	${file_arm7_src_c:.c=.d}	${file_arm7_src_s:.s=.d}	${file_arm7_src_SUPP:.S=.d}

#ARM7 specific definitions.
MAPFILE = $(DIR_ARM7).map
SHARED_FLAGS	=	-mcpu=arm7tdmi
AS_ASFLAGS		=	$(SHARED_FLAGS)	-defsym debug=0	-defsym	readvertoolchain='$(TOOLCHAIN_VERSION)'
CC_ASFLAGS		=	$(SHARED_FLAGS)	-Wall	-O2	-DASM_OPCODES -DARM7 -mtune=arm7tdmi	-marm -fomit-frame-pointer	-ffast-math	-Ddebug=0	-Dreadvertoolchain='$(TOOLCHAIN_VERSION)'
BUILD			= 	$(BUILD_ARM7)
DIR_SRC			= 	$(DIRS_ARM7_SRC)

#--gc-sections: allows relocatable sections linked (already) to be included, otherwise unused sections are discarded.
#check arm9.map "Discarded input sections" to see --gc-sections in action.
LDFLAGS	=	-specs=$(LIBPATH)nds_arm7_ld.specs	-Wl,--gc-sections,-Map,$(MAPFILE)	-nostartfiles
LIBRARIES =	-lc_nano


################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS =	$(objs_arm7)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG =	$(addprefix $(OBJDIR)/, $(TARGET_LIBRARY_FILE_7))

all: $(OBJDIR)|$(TARGET_LIBRARY_FILE_7)
#all: debug

#Check $(OBJDIR) if not exists: create it
$(OBJDIR):
	@echo "Folder $(OBJDIR) does not exist. Creating"
	mkdir -p $@

#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC)	$(CC_ASFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-o $@ -c $<
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

#build *.o into $(TARGET_LIBRARY_FILE_7)
$(TARGET_LIBRARY_FILE_7): $(OBJPROG)
	-mkdir -p $(TARGET_LIBRARY_PATH) && cp	$(OBJPROG)	$(TARGET_LIBRARY_PATH)
	-mkdir -p $(TARGET_LIBRARY_HEADERS_7) && cp	$(dir_arm7_src_h)	$(TARGET_LIBRARY_HEADERS_7)
	-@echo 'build done: $(TARGET_LIBRARY_FILE_7)'
	
################################################################################################
clean:
	-@echo 'cleanup $(TARGET_LIBRARY_FILE_7)'
	-@rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_7)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_7)