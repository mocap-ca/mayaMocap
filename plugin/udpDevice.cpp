#include "udpDevice.h"
#include "udpSocket.h"
#include <iostream>

/*  
Usage:
import maya.cmds as m
m.loadPlugin('C:/cpp/git/github/mayaMocap/plugin/build_vs2015/x64/Debug/build_vs2015.mll')
x = m.createNode('peelRealtimeMocap')
loc = m.spaceLocator()
m.connectAttr( x + ".mocap[0].outputTranslate", loc[0] + ".t")
m.connectAttr( x + ".mocap[0].outputRotate", loc[0] + ".r")
*/

#define DEFAULT_PORT 9119
#define DEFAULT_CMDPORT 9120
#define _CRT_SECURE_NO_WARNINGS

#include <maya/MFnNumericAttribute.h>


MObject UdpDevice::port;

MTypeId UdpDevice::id(0x001126D1);

bool UdpDevice::connect()
{
	if (socket.isConnected()) return true;

	if (iPort < 1024)
	{
		fprintf(stderr, "Invalid port: %d\n", iPort);
		sendData("Invalid Port");
		return false;
	}

	if (!socket.bind(iPort))
	{
		fprintf(stderr, "Cannot connect\n");
		sendData("Cannot connect");
		return false;
	}

	return true;
}

bool UdpDevice::isConnected()
{
    return socket.isConnected();
}

bool UdpDevice::disconnect()
{
	socket.close();
	return true;
}

size_t UdpDevice::receiveData(char *data, size_t buflen)
{
	int ret = socket.receive();  // get the data, or timeout
								 //int ret = socket.stub();
	if (ret == -1) return -1;  // error
	if (ret == 0) return 0; // timeout

	char buf[128];
	//std::cout << "Listening on port: " << iPort;
	if (socket.buflen > buflen)
	{
		fprintf(stderr, "Buffer overflow\n");
		return -1;
	}
	memcpy(data, socket.readBuffer, socket.buflen);

	return socket.buflen;
}

MStatus UdpDevice::initialize()
{
	MFnNumericAttribute   numAttr;
	MStatus status;

	// port
	port = numAttr.create("port", "p", MFnNumericData::kInt, DEFAULT_PORT, &status);
	status = addAttribute(port);

	MStatus stat = ThreadedDevice::initialize();
	if (stat != MS::kSuccess) return stat;

	attributeAffects(port, mocap);

	return MS::kSuccess;
}


MStatus UdpDevice::compute(const MPlug &plug, MDataBlock& block)
{
	MStatus status;

	MDataHandle hPort = block.inputValue(port, &status);
	iPort = hPort.asInt();

	MStatus stat = ThreadedDevice::compute(plug, block);

	return MS::kSuccess;

}
