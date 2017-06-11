PLUGIN_NAME=peelRealtimeMocap
PLUGIN_VERSION=1


PLUGIN_SRC = plugin.cpp listenCommand.cpp pingCommand.cpp threadedDevice.cpp memoryNode.cpp
HEADER_SRC = ../item/udpSocket.h ../item/baseSocket.h ../item/tcpListen.h listenCommand.h

ITEM_SRC = item.cpp 

OTHER_OBJ = $(BUILDDIR)/item.o

include mayaplugin.mk

LFLAGS += -pthread
CFLAGS += -I../item

$(BUILDDIR)/item.o : ../item/item.cpp
	$(CXX) -o $@ -c $(MAYA_C++FLAGS) $(CFLAGS) $<


#tests : listenTest sendTest listenTestTcp
#	@echo Building $(OUT)

#.cpp.o:
#	$(C++) -c $(C++FLAGS) -o $@ $<


listenTest : listenTest.cpp udpSocket.h baseSocket.h  item.h item.cpp
	$(C++) -O0 -g -o $@ $(CFLAGS_ALL) $(LD_FLAGS_ALL) $< item.cpp

#listenTestTcp : listenTestTcp.cpp tcpSocket.h baseSocket.h  item.h
#	$(C++) -o $@ $(CFLAGS_ALL) $(LD_FLAGS_ALL) $<

#sendTest : sendTest.cpp udpSocket.h baseSocket.h item.h
#	$(C++) -o $@ $(CFLAGS_ALL) $(LD_FLAGS_ALL) $<

sendArduino : sendArduino.cpp udpSocket.h baseSocket.h item.h
	$(C++) -o $@ $(CFLAGS_ALL) $(LD_FLAGS_ALL) $<

