
ifndef MAYA_RELEASE

all : 2014
	echo "Build done. You may want to build the installer now"

clean :
	echo Cleaning 2014
	export MAYA_RELEASE=2014; make clean

2014 :
	@echo "-----------------------------"
	@echo "building 2014"
	export MAYA_RELEASE=2014; make

else



# set to 1 for debug build
#DEBUG = 1

# determine os
UNAME    = $(shell uname)

###################
##  OSX OPTIONS  ##
###################

ifeq ($(UNAME), Darwin)

MAYADIR = /Applications/Autodesk/maya$(MAYA_RELEASE)
EXTENSION=bundle
PLATFORM_INCLUDES = -I/usr/include/c++/4.2.1
MY_LFLAGS = -F/System/Library/Frameworks -framework AGL -framework OpenGL  -stdlib=libstdc++ -lstdc++ 
MY_CFLAGS = -Wno-switch-enum -Wno-switch  -stdlib=libstdc++
MY_CPPFLAGS = $(MY_CFLAGS) 

endif

#####################
### LINUX OPTIONS ###
#####################

ifeq ($(UNAME), Linux)

ifeq ($(MAYA_RELEASE), 2016)
MAYADIR = /usr/autodesk/maya$(MAYA_RELEASE)
else
MAYADIR = /usr/autodesk/maya$(MAYA_RELEASE)-x64
endif

EXTENSION=so
PLATFORM_INCLUDES = -I/usr/local/lib/gcc/x86_64-unknown-linux-gnu/4.1.2/include/ssp
MY_LFLAGS = -L$(MAYADIR)/lib -lGL

endif




##################
## MAYA OPTIONS ##
##################

# import maya defines
SRCDIR=.
TOP=$(MAYADIR)/devkit/plug-ins/
include $(MAYADIR)/devkit/plug-ins/buildconfig
# above devines: CFLAGS C++FLAGS INCLUDES LFLAGS



##################
# post maya edits
#################

ifeq ($(UNAME), Darwin)
CC  = llvm-gcc  # gcc
C++ = llvm-g++  # g++
LD  = llvm-g++  # g++
LFLAGS :=$(subst -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk,, $(LFLAGS))
LFLAGS :=$(subst -isysroot /Developer/SDKs/MacOSX10.6.sdk,, $(LFLAGS))
LFLAGS :=$(subst -isysroot /Developer/SDKs/MacOSX10.8.sdk,, $(LFLAGS))
LFLAGS :=$(subst -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk,, $(LFLAGS))
endif

#######################
## DEBUGGING OPTIONS ##
#######################
DEBUGEXT =
ifeq ($(DEBUG), 1)
DEBUGEXT = d
CFLAGS := $(subst -O3, -O0 -g -DDEBUG -D_DEBUG, $(CFLAGS))
BUILDDIR := debugbuild$(MAYA_RELEASE)
else
BUILDDIR := build$(MAYA_RELEASE)
endif



##########
## OBJS ##
##########

PLUGIN       = $(PLUGIN_NAME)_$(PLUGIN_VERSION)_$(MAYA_RELEASE)$(DEBUGEXT).$(EXTENSION)
MY_CFLAGS   += -DPLUGIN_VERSION=$(PEELSOLVE_VERSION) 
MY_CPPFLAGS += -DPLUGIN_VERSION=$(PEELSOLVE_VERSION) 
PLUGIN_OBJ   = $(PLUGIN_SRC:%.cpp=$(BUILDDIR)/plugin/%.o)



###########
## Rules ##
###########

MY_LIBS = -lOpenMayaRender -lOpenMayaUI -lOpenMaya -lFoundation -lOpenMayaAnim

ALL_LFLAGS   = $(LFLAGS)   $(MY_LFLAGS)   $(LIBS)     $(MY_LIBS)
ALL_CPPFLAGS = $(INCLUDES) $(MY_INCLUDES) $(C++FLAGS) $(MY_CPPFLAGS)
ALL_CFLAGS   = $(INCLUDES) $(MY_INCLUDES) $(CFLAGS)   $(MY_CFLAGS)


$(PLUGIN) : directories $(PLUGIN_OBJ)  $(PLUGIN_SRC) $(HEADER_SRC)
	@echo "======================================="
	@echo Building Plugin 
	$(LD) -o $@ $(PLUGIN_OBJ) $(ALL_LFLAGS)


directories : $(BUILDDIR) $(BUILDDIR)/plugin

$(BUILDDIR) :
	mkdir $(BUILDDIR)

$(BUILDDIR)/plugin :
	mkdir $(BUILDDIR)/plugin

$(BUILDDIR)/plugin/%.o : %.cpp
	@echo -+-=_=+-+-_+--+-+
	@echo $(MAYADIR)
	@echo $(CFLAGS)
	@echo $(ALL_CPPFLAGS)
	@echo plugin$(MAYA_RELEASE): $@
	$(CXX) -o $@ -c $(ALL_CPPFLAGS) $<


# ends if/else maya version selection
endif
