#SNES9X_CPP=1
#NETPLAY=1
UNZIP=1
CHEATS=1
SDD1_DECOMP=1
#GLIDE=1
#OPENGL=1
#THREAD_SOUND=1

#-DNO_INLINE_SET_GET
#-D_ZAURUS
#-DDEBUGGER
#-DJOYSTICK_SUPPORT
#-DUSE_GLIDE
#-DUSE_OPENGL
#-Isdl
#-D__linux
#-DVAR_CYCLES
#-DCPU_SHUTDOWN
#-DSPC700_SHUTDOWN

SNES9X_INCS = -I. -I..

PS2SDK_FLAGS = -DPS2LINK -fssa -fshort-double -msingle-float -mhard-float -mfused-madd -ffast-math -mno-check-zero-division -mtune=r5900 -mno-mips16 -fno-exceptions
SNES9X_FLAGS = -DUSE_THREADS -DOPTI -D_ZAURUS -DRIGHSHIFT_IS_SAR -DVAR_CYCLES -DCPU_SHUTDOWN -DSPC700_SHUTDOWN

SNES9X_OBJS = \
    ../fxinst.o \
    ../fxemu.o

SNES9X_OBJS += \
    ../spc700.o \
    ../soundux.o \
    ../apu.o

SNES9X_OBJS += \
    ../cpuops.o \
    ../cpuexec.o \
    ../2xsai.o \
    ../c4.o \
    ../c4emu.o

SNES9X_OBJS += \
    ../srtc.o \
    ../cpu.o \
    ../tile.o \
    ../gfx.o \
    ../clip.o \
    ../memmap.o \
    ../ppu.o \
    ../dma.o \
    ../sdd1.o \
    ../dsp1.o \
    ../sa1.o \
    ../sa1cpu.o \
    ../obc1.o \
    ../snapshot.o \
    ../data.o \
    ../globals.o

#    ../snaporig.o

ifdef SNES9X_CPP
SNES9X_OBJS += \
    ../snes9x.o
endif

ifdef SDD1_DECOMP
SNES9X_OBJS += \
    ../sdd1emu.o
SNES9X_FLAGS += -DSDD1_DECOMP
endif

ifdef CHEATS
SNES9X_OBJS += \
    ../cheats.o \
    ../cheats2.o
endif

ifdef NETPLAY
SNES9X_OBJS += \
    ../netplay.o \
    ../server.o
SNES9X_FLAGS += -DNETPLAY_SUPPORT
endif

ifdef UNZIP
SNES9X_OBJS += \
    ../loadzip.o \
    ../unzip/unzip.o \
    ../unzip/explode.o \
    ../unzip/unreduce.o \
    ../unzip/unshrink.o
SNES9X_FLAGS += -DUNZIP_SUPPORT -DZLIB
SNES9X_INCS += -I../unzip
endif

ifdef THREAD_SOUND
CPUDEFINES += -DUSE_THREADS
EXTRALIBS += -lpthread
endif

ifdef GLIDE
GLIDEOBJS = unix/glide.o
GLIDEDEFINES = -DUSE_GLIDE
GLIDELIBS = -lglide2x
endif

ifdef OPENGL
OPENGLOBJS = unix/opengl.o
OPENGLDEFINES = -DUSE_OPENGL
OPENGLLIBS = -lGL -lGLU -ldl
endif

INTERFACE_OBJS = \
    interface/src/browser.o \
    interface/src/cfg.o \
    interface/src/gui.o \
    interface/src/gzip.o \
    interface/src/hdd.o \
    interface/src/init.o \
    interface/src/interface.o \
    interface/src/lists.o \
    interface/src/paths.o \
    interface/src/settings.o \
    interface/src/strkat.o \
    interface/src/tar.o \
    interface/src/video.o

INTERFACE_INCS = \
    -Iinterface/include

EE_BIN = snes9x.elf

EE_OBJS = ps2_snes9x.o modules_tgz.o skin_tgz.o ps2_controls.o ps2_video.o ps2_sound_driver_audsrv.o snes9x_cfg.o $(INTERFACE_OBJS) $(SNES9X_OBJS)

EE_CFLAGS = $(SNES9X_FLAGS)

EE_CXXFLAGS += $(SNES9X_FLAGS) -fno-rtti

EE_INCS = $(INTERFACE_INCS) $(SNES9X_INCS) -I$(PS2SDK)/ports/include
EE_LDFLAGS = -L$(PS2SDK)/ports/lib

EE_GFX_LIBS = -ldma -lpacket -lgraph -lfont -ldraw -limage
EE_FILE_LIBS = -lfileXio
EE_SYS_LIBS = -lSMSCDVD -laudsrv -linputx -lmc -lmtap -lpadx -lpoweroff -lpatches -lc -lmf -lm

EE_LIBS = $(EE_GFX_LIBS) -lconfig -lpng -lz $(EE_FILE_LIBS) $(EE_SYS_LIBS)

all: $(EE_BIN)

clean:
	rm -f $(EE_OBJS) $(EE_BIN)

%_tgz.s:
	bin2s $*.tgz $*_tgz.s $*_tgz

include Makefile.eeglobal
include Makefile.pref

