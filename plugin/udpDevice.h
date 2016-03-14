#ifndef PEEL_UDP_DEVICE_H
#define PEEL_UDP_DEVICE_H

#include "threadedDevice.h"
#include "udpSocket.h"

class UdpDevice : public ThreadedDevice
{
public:
	UdpDevice() : iPort(-1) {};

	static void*        creator() { return new UdpDevice; }

	virtual MStatus     compute(const MPlug& plug, MDataBlock& data);

	virtual bool connect();
	virtual bool disconnect();
	size_t receiveData(char*, size_t);

	static MObject      port;

	int       iPort;
	UdpServer socket;

	static MStatus initialize();

	static MTypeId id;
};


#endif