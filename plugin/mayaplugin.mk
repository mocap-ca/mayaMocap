
ifndef MAYA_RELEASE

all : 2014 2016
	echo "Build done."

clean :
	echo Cleaning 2014
	export MAYA_RELEASE=2014; make clean

2014 :
	@echo "-----------------------------"
	@echo "building 2014"
	export MAYA_RELEASE=2014; make

2016 :
	@echo "-----------------------------"
	@echo "building 2016"
	export MAYA_RELEASE=2016; make


else


-include localsettings.mak

# set to 1 for debug build
DEBUG = 0

# determine os
UNAME    = $(shell uname)

###################
##  OSX OPTIONS  ##
###################

ifeq ($(UNAME), Darwin)

CC                      = gcc-4.2
C++                     = g++-4.2
LD                      = g++-4.2

ifndef MAYADIR
MAYADIR = /Applications/Autodesk/maya$(MAYA_RELEASE)
endif

EXTENSION=bundle
PLATFORM_INCLUDES = -I/usr/include/c++/4.2.1

MAYA_LFLAGS          =   -fno-gnu-keywords -fpascal-strings  \
                                -isysroot /Developer/SDKs/MacOSX10.6.sdk \
                                -headerpad_max_install_names \
                                -framework System -framework SystemConfiguration \
                                -framework CoreServices -framework Carbon \
                                -framework Cocoa -framework ApplicationServices \
                                -framework IOKit \
                                -bundle \
				-L$(MAYADIR)/Maya.app/Contents/MacOS

MAYA_CFLAGS          = -DCC_GNU_ -DOSMac_ -DOSMacOSX_ -DREQUIRE_IOSTREAM\
                                -DOSMac_MachO_ -O3 $(ARCH_FLAGS)  -D_LANGUAGE_C_PLUS_PLUS \
                                -include "$(MAYADIR)/devkit/include/maya/OpenMayaMac.h" \
				-I$(MAYADIR)/devkit/include

MAYA_C++FLAGS        = $(CFLAGS) $(WARNFLAGS) $(ERROR_FLAGS) -fno-gnu-keywords -fpascal-strings

endif
#### end osx

#####################
### LINUX OPTIONS ###
#####################

ifeq ($(UNAME), Linux)

# MAYA_DIR may be seting localsettings.mak
ifndef MAYADIR

ifeq ($(MAYA_RELEASE), 2016)
MAYADIR = /usr/autodesk/maya$(MAYA_RELEASE)
else
MAYADIR = /usr/autodesk/maya$(MAYA_RELEASE)-x64
endif

endif

MAYA_C++FLAGS = -fPIC -pthread -pipe -D_BOOL -DREQUIRE_IOSTREAM -DLINUX -fno-gnu-keywords -Wno-deprecated -I$(MAYADIR)/include
MAYA_LFLAGS  = -shared  -Wl,-Bsymbolic -L$(MAYADIR)/lib

MAYA_LIBS = -lFoundation -lOpenMaya -lOpenMayaAnim -lOpenMayaFX -lOpenMayaRender -lOpenMayaUI
MAYA_LIBS_DIR = $(MAYADIR)/lib


EXTENSION=mll
#PLATFORM_INCLUDES = -I/usr/local/lib/gcc/x86_64-unknown-linux-gnu/4.1.2/include/ssp
MY_LFLAGS = -L$(MAYADIR)/lib -lGL

endif
#### END LINUX

CC=gcc
C++=g++



#######################
## DEBUGGING OPTIONS ##
#######################
DEBUGEXT =
ifeq ($(DEBUG), 1)
DEBUGEXT = d
CFLAGS := $(subst -O3, -O0, $(CFLAGS))
CFLAGS := $(CFLAGS) -g -DDEBUG -D_DEBUG
BUILDDIR := debugbuild$(MAYA_RELEASE)
else
BUILDDIR := build$(MAYA_RELEASE)
endif



##########
## OBJS ##
##########

PLUGIN       = $(PLUGIN_NAME)_$(PLUGIN_VERSION)_$(MAYA_RELEASE)$(DEBUGEXT).$(EXTENSION)
PLUGIN_OBJ   = $(PLUGIN_SRC:%.cpp=$(BUILDDIR)/plugin/%.o)



###########
## Rules ##
###########


$(PLUGIN) : directories $(PLUGIN_OBJ)  $(PLUGIN_SRC) $(HEADER_SRC) 
	@echo "======================================="
	@echo Building Plugin 
	$(LD) -o $@ $(PLUGIN_OBJ) $(MAYA_LFLAGS) $(MAYA_LIBS) 


directories : $(BUILDDIR) $(BUILDDIR)/plugin

$(BUILDDIR) :
	mkdir $(BUILDDIR)

$(BUILDDIR)/plugin :
	mkdir $(BUILDDIR)/plugin

$(BUILDDIR)/plugin/%.o : %.cpp
	@echo -+-=_=+-+-_+--+-+
	@echo Maya Dir: $(MAYADIR)
	@echo CFLAGS: $(CFLAGS)
	@echo MAYA CFLAGS: $(MAYA_CFLAGS)
	@echo RELEASE: plugin$(MAYA_RELEASE): $@
	$(CXX) -o $@ -c $(MAYA_C++FLAGS) $(CFLAGS) $<


# ends if/else maya version selection
endif
