
ifndef MAYAVER

start :
	make MAYAVER=2016_sp4/maya
	make MAYAVER=2014_ext2p05/maya2014-x64
	

endif


MAYA_LOCATION=/local/prod/autodesk/maya/$(MAYAVER)
REP=$(subst _, ,$(MAYAVER))
BV= $(firstword $(REP))

INC_DIR = $(MAYA_LOCATION)/include
LIB_DIR = $(MAYA_LOCATION)/lib
LDLIBS  = -lm

LIBS := -lOpenMaya -lOpenMayaAnim -lOpenMayaUI -lFoundation

CC  = cc
C++ = c++
RM  = rm -f

#CDEBUGFLAGS = -O3
CDEBUGFLAGS = -g -O0

INCLUDES_ALL = $(INCLUDES) -I$(INC_DIR)
DEFINES_ALL  = $(DEFINES) -DFUNCPROTO -DMAYA_MOCAP_DEVKIT
CFLAGS_ALL   = -fno-gnu-keywords $(CDEBUGFLAGS) $(CFLAGS)	\
		       $(INCLUDES_ALL) $(DEFINES_ALL) -DMOTIF \
			   -DXT_CODE -DCONTEXT_MACRO_ACCESS -DLINUX -DUNIX -DREQUIRE_IOSTREAM -D_BOOL -fPIC
LDFLAGS_ALL  = $(LDFLAGS)  $(CDEBUGFLAGS) -L$(LIB_DIR) $(LDOPTS) \
			   $(LDLIBS) $(LIBS)
C++FLAGS     = $(CFLAGS_ALL) 

OUT=ikaRealtimeMocap_$(BV).mll

all : $(OUT) listenTest sendTest listenTestTcp
	@echo Building $(OUT)

.c.o:
	$(CC) -c $(CFLAGS_ALL) -o $@ $<

.cpp.o:
	$(C++) -c $(C++FLAGS) -o $@ $<

plugin_$(BV).o : plugin.cpp 
	$(C++) -c $(C++FLAGS) -o $@ $<

listenCommand_$(BV).o : listenCommand.cpp 
	$(C++) -c $(C++FLAGS) -o $@ $<

SRCS = plugin.cpp
OBJS = plugin_$(BV).o listenCommand_$(BV).o

$(OUT) : $(OBJS) udpSocket.h baseSocket.h tcpListen.h listenCommand.h
	@echo --- PLUGIN $(BV) ---
	$(CC) -shared $(CFLAGS_ALL) $(OBJS) $(LDFLAGS_ALL) -o $@


listenTest : listenTest.cpp udpSocket.h baseSocket.h  item.h
	$(C++) -o $@ $(CFLAGS_ALL) $(LD_FLAGS_ALL) $<

listenTestTcp : listenTestTcp.cpp tcpSocket.h baseSocket.h  item.h
	$(C++) -o $@ $(CFLAGS_ALL) $(LD_FLAGS_ALL) $<

sendTest : sendTest.cpp udpSocket.h baseSocket.h item.h
	$(C++) -o $@ $(CFLAGS_ALL) $(LD_FLAGS_ALL) $<

clean :
	$(RM) *.mll
	$(RM) plugin.o
	$(RM) *.o
