/*

Copyright (c) 2016 Alastair Macleod, Sawmill Studios Inc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef MAYA_MOTIVE
#define MAYA_MOTIVE

#include <maya/MPxThreadedDeviceNode.h>
#include <maya/MStatus.h>
#include <maya/MStringArray.h>

#include "NatNetTypes.h"
#include "NatNetClient.h"

#include <map>
#include "data.h"
#include <Windows.h>

class MayaMotive : public MPxThreadedDeviceNode
{
public:
	MayaMotive() {};
	~MayaMotive();

	void threadHandler();
	void threadShutdownHandler();
	void postConstructor();

	void sendData( const char *data, size_t datalen);

	virtual MStatus     compute(const MPlug& plug, MDataBlock& data);

	static MStatus      initialize();

	static void*  creator() { return new MayaMotive(); }

	virtual bool connect();
	virtual bool isConnected();
	virtual void disconnect();

	static MObject      oMocap;

	static MObject      oScale;
	static MObject      rigidbodies;
	static MObject      rigidbodyName;
	static MObject      rigidbodyTranslation;
	static MObject      rigidbodyRotation;

	static MObject      markers;
	static MObject      markerName;
	static MObject      markerTranslation;

	static MObject      segments;
	static MObject      segmentName;
	static MObject      segmentTranslation;
	static MObject      segmentRotation;

	static MObject      npMode;
	static MObject      npMulticastAddress;
	static MObject      npMotiveIp;
	static MObject      npLocalInterface;
	static MObject      npCommandPort;
	static MObject      npDataPort;

	static MObject      statusInfo;
	static MTypeId id;

	void getDescriptions();
	void dataCallback(sFrameOfMocapData*);

	int findNameId(std::map<int, MString> &mapping,
		int     nameId,
		MPlug   &groupPlug,
		MPlug   &namePlug,
		MObject &groupObject,
		MDataBlock& block);

	MStatus setVector(float x, float y, float z, int index, MObject &groupObject, MPlug &plug, MDataBlock &block);

	std::map<int, MString> markerNames;
	std::map<int, MString> rigidbodyNames;
	std::map<int, MString> subjectNames;
	std::map<int, MString> jointNames;

	float     scale;
	int       commandPort;
	int       dataPort;
	MString   motiveIp;
	MString   localInterface;
	MString   multicastAddress;
	int       mode;
	bool      mConnected;

	NatNetClient *natNetClient;

	static MStringArray interfaceList;

	std::vector<MotiveItem*> items;
	HANDLE hMutex;
	HANDLE hEvent;
};


void dataCallback(sFrameOfMocapData *, void *);
void messageCallback(int, char*);


#endif