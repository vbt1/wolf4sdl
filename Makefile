#
#   SEGA SATURN Graphic library make file for GNU
#
# slightly modified for ISO building, COFF toolchain

# specify on command line
# OBJFMT = coff
 OBJFMT = elf

# macro
AS = sh-$(OBJFMT)-as
CC = sh-$(OBJFMT)-g++
CXX = sh-$(OBJFMT)-g++
CONV = sh-$(OBJFMT)-objcopy
RM = rm

# directory
SGLDIR = C:/SaturnOrbit/SGL_302j
SGLIDR = $(SGLDIR)/inc
SGLLDR = $(SGLDIR)/lib_elf

CMNDIR = l:/saturn/dev/wolf3ddma/root # C:/SaturnOrbit/COMMON
OBJECTS = ./objects

# option
#CCFLAGS = -O2 -m2 -g -c -I$(SGLIDR)
# -fomit-frame-pointer -fsort-data 
#CCFLAGS = -O2 -m2 -Wno-narrowing -fuse-linker-plugin -fno-web -fno-gcse -fno-unit-at-a-time -fomit-frame-pointer -flto
#CCFLAGS = -O2 -m2 -Wno-narrowing -fno-web -fno-gcse -fno-unit-at-a-time -fomit-frame-pointer -fpermissive -fno-lto -fno-builtin
#CCFLAGS = -Os -m2 -Wno-conversion-null -Wno-narrowing -fuse-linker-plugin -fno-unit-at-a-time -fomit-frame-pointer -flto -fpermissive -fno-builtin
#CCFLAGS = -save-temps -Os -g3 -m2 -Wformat-diag -Wno-conversion-null -fno-lto -Wshadow -Wall -Wswitch -Wpadded -fno-common -Wextra -Wno-narrowing -fno-web -fno-builtin -funit-at-a-time -Wwrite-strings -Wl,--strip-all -Wl,--verbose -mno-fsrra -maccumulate-outgoing-args -fomit-frame-pointer -D_SH -DMODEL_S 
CCFLAGS =  -fpermissive -O2 -m2 -falign-functions=4 -Wformat-diag -Wno-conversion-null -fno-lto -Wshadow -Wall -Wswitch -Wpadded -fno-common -Wextra -Wno-narrowing -fno-web -fno-builtin -funit-at-a-time -Wwrite-strings -Wl,--strip-all -Wl,--verbose -mno-fsrra -maccumulate-outgoing-args -fomit-frame-pointer -D_SH -DMODEL_S -DUSE_SPRITES -DPONY -DUSE_ADX -DUSE_SLAVE -DEMBEDDED
#CCFLAGS = -O0 -m2 -Wno-write-strings -Wno-narrowing -fno-lto
CCFLAGS += $(CFLAGS)
#CCFLAGS += -std=gnu99
#CCFLAGS += -Werror-implicit-function-declaration
#CCFLAGS += -Wimplicit-int
CCFLAGS += -fno-rtti 
CCFLAGS += -fno-exceptions
#CCFLAGS += -Wsequence-point
#CCFLAGS += -c -lm -lc -lgcc -I$(SGLIDR) 
CCFLAGS += -c -I$(SGLIDR) 


#-DUSE_HWRAM_CHUNK
 

# -m2 must be specified in LDFLAGS so the linker will search the SH2 lib dirs
# Specify path of libsgl.a by using "-L" option

DFLAGS =
include $(OBJECTS)

#LDFLAGS = -O2 -m2  -fuse-linker-plugin  -Xlinker -n -Xlinker -flto -Xlinker -T$(LDFILE) -Xlinker -Map \
#          -Xlinker $(MPFILE) -Xlinker -e -Xlinker ___Start -Xlinker -S -nostartfiles 
LDFLAGS = -m2 -Xlinker -n -Xlinker -T$(LDFILE) -Xlinker -Map \
          -Xlinker $(MPFILE) -Xlinker -e -Xlinker ___Start -Xlinker -S -nostartfiles
		  # -fno-builtin 
#          -Xlinker $(MPFILE) -Xlinker -e -Xlinker ___Start -nostartfiles -LL:/GNUSHV12/sh-elf/sh-elf/lib/m2/libc.a -LC:/SaturnOrbit/SGL_302j/LIB_ELF/LIBSGL.A 
TARGET   = root/sl.coff
TARGET1  = root/sl.bin
MPFILE   = $(TARGET:.coff=.maps)
IPFILE   = $(CMNDIR)/IP.BIN
LDFILE   = root/SL.LNK
MAKEFILE = makefile


all: $(TARGET) $(TARGET1)

# Use gcc to link so it will automagically find correct libs directory

$(TARGET) : $(SYSOBJS) $(OBJS) $(MAKEFILE) $(LDFILE) #$(OBJECTS)
	$(CC) $(LDFLAGS) $(SYSOBJS) $(OBJS) $(LIBS) -o $@

$(TARGET1) : $(SYSOBJS) $(OBJS) $(MAKEFILE) $(LDFILE)
	$(CONV) -O binary $(TARGET) $(TARGET1)

#$(LDFILE) : $(MAKEFILE)
#	@echo Making $(LDFILE)
#	@echo SECTIONS {		> $@
#	@echo 	SLSTART 0x06004000 : {	>> $@
#	@echo 		___Start = .;	>> $@
#	@echo 		*(SLSTART)	>> $@
#	@echo 	}			>> $@
#	@echo 	.text ALIGN(0x20) :			>> $@
#	@echo 	{			>> $@
#	@echo 		* (.text)			>> $@
#	@echo 		*(.strings)			>> $@
#	@echo 		__etext = .;			>> $@
#	@echo 	}			>> $@
#	@echo 	SLPROG ALIGN(0x20): {	>> $@
#	@echo 		__slprog_start = .;	>> $@
#	@echo 		*(SLPROG)	>> $@
#	@echo 		__slprog_end = .;	>> $@
#	@echo 	}			>> $@
#	@echo 	.tors  ALIGN(0x10) :			>> $@
#	@echo 	{			>> $@
#	@echo 		___ctors = . ;			>> $@
#	@echo 		*(.ctors)			>> $@
#	@echo 		___ctors_end = . ;			>> $@
#	@echo 		___dtors = . ;			>> $@
#	@echo 		*(.dtors)			>> $@
#	@echo 		___dtors_end = . ;			>> $@
#	@echo 	}			>> $@
#	@echo 	.data ALIGN(0x10):			>> $@
#	@echo 	{			>> $@
#	@echo 		* (.data)			>> $@
#	@echo 		_edata = . ;			>> $@
#	@echo 	}			>> $@
#	@echo 	.bss ALIGN(0x10) (NOLOAD):			>> $@
#	@echo 	{			>> $@
#	@echo 		__bstart = . ;			>> $@
#	@echo 		*(.bss)			>> $@
#	@echo 		* ( COMMON )			>> $@
#	@echo 		__bend = . ;			>> $@
#	@echo 	_end = .;			>> $@
#	@echo 	}			>> $@
#	@echo }				>> $@

# suffix
.SUFFIXES: .asm

.s.o:
	$(AS) $< $(ASFLAGS) $(_ASFLAGS) -o $@

.c.o:
	$(CC) $< $(DFLAGS) $(CCFLAGS) $(_CCFLAGS) -o $@
.cpp.o:
	$(CXX) $< $(DFLAGS) $(CCFLAGS) $(_CCFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET) $(TARGET1) $(TARGET2) $(MPFILE) cd/0.bin



