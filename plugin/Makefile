PLUGIN_NAME=peelRealtimeMocap
PLUGIN_VERSION=1

PLUGIN_SRC = plugin.cpp listenCommand.cpp pingCommand.cpp item.cpp threadedDevice.cpp memoryNode.cpp
HEADER_SRC = udpSocket.h baseSocket.h tcpListen.h listenCommand.h


include mayaplugin.mk

ifndef C++
C++ = g++ -O0 -g
endif

#OUT=peelRealtimeMocap_$(BV).mll

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

