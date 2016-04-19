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

/*
This class uses a MPxThreadedDeviceNode to recieve data from the NatNetSdk.

All data must be put in to a fixed length buffer, which is sized by the
PostConstructor method.  Currently if more than the buffer of data is sent
then the class will stop working.  It may be worth investigating other
methods of transfering data between the device and maya threads, such as 
passing a pointer.

It seems that the buffer is requested by maya once per frame, so all data
needs to be passed by a single buffer.
*/


#include "mayaMotive.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MString.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MThreadAsync.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MFnStringData.h>

#include <vector>

#include "interfaces.h"
#include <Windows.h>

#include "data.h"

MObject MayaMotive::oScale;
MObject MayaMotive::oMocap;
MObject MayaMotive::rigidbodies;
MObject MayaMotive::rigidbodyName;
MObject MayaMotive::rigidbodyTranslation;
MObject MayaMotive::rigidbodyRotation;

MObject MayaMotive::markers;
MObject MayaMotive::markerName;
MObject MayaMotive::markerTranslation;

MObject MayaMotive::segments;
MObject MayaMotive::segmentName;
MObject MayaMotive::segmentTranslation;
MObject MayaMotive::segmentRotation;

MObject MayaMotive::npMode;
MObject MayaMotive::npMulticastAddress;
MObject MayaMotive::npMotiveIp;
MObject MayaMotive::npLocalInterface;
MObject MayaMotive::npCommandPort;
MObject MayaMotive::npDataPort;

MObject MayaMotive::statusInfo;
MStringArray MayaMotive::interfaceList;

MTypeId MayaMotive::id(0x001126D4);

#define MCONT(s, msg) if( s != MS::kSuccess) { s.perror(msg); continue; }
#define MCHECKERROR(x, msg) { if(x!=MS::kSuccess) { x.perror(msg); return x;} }
#define MERR(x, msg) { status = x; MCHECKERROR(status, msg); }

#define BUFSIZE      1024 * 12
#define COMMAND_PORT 1510
#define DATA_PORT    1511	



void MayaMotive::postConstructor()
{
	// While this is defined as an array, only one attribute is supported.
	MObjectArray oArray;
	oArray.append(MayaMotive::oMocap);
	setRefreshOutputAttributes(oArray);
	createMemoryPools(1, BUFSIZE, 1);

	mConnected = false;
	hMutex = CreateMutex(0, false, 0);
	hEvent = CreateEvent(NULL, FALSE, FALSE, "FrameEvent");
}

MayaMotive::~MayaMotive()
{
	destroyMemoryPools();
	CloseHandle(hMutex);
}


MStatus MayaMotive::initialize()
{
	MStatus status;
	MFnNumericAttribute   numAttr;
	MFnCompoundAttribute  cAttr;
	MFnTypedAttribute     tAttr;
	MFnMessageAttribute   mAttr;
	MFnEnumAttribute      eAttr;

	// Command Port
	npCommandPort = numAttr.create("commandPort", "cmd", MFnNumericData::kInt, COMMAND_PORT, &status);
	MCHECKERROR(status, "Creating command port attribute");
	MERR( addAttribute(npCommandPort), "Adding command port");

	// Data Port
	npDataPort = numAttr.create("dataPort", "dp", MFnNumericData::kInt, DATA_PORT, &status);
	MCHECKERROR(status, "Creating data port attribute");
	MERR(addAttribute(npDataPort), "Adding data port");
	
	// Multicast 
	MFnStringData sdata;
	MObject ostring = sdata.create();
	sdata.set("239.255.42.99");
	npMulticastAddress = tAttr.create("multicastAddress", "mif", MFnData::kString, ostring, &status);
	MCHECKERROR(status, "creating multicast address");
	MERR( addAttribute(npMulticastAddress), "Adding multicast address attribute");

	// Motive IP
	npMotiveIp = tAttr.create("motiveIp", "mip", MFnData::kString, MObject::kNullObj, &status);
	MCHECKERROR(status, "creating motive ip attribute");
	MERR( addAttribute(npMotiveIp), "Adding motive ip attribute");

	// Local Interface
	npLocalInterface = eAttr.create("localInterface", "iface", 0, &status);
	MCHECKERROR(status, "creating motive local interface attribute");
	interfaceList = getInterfaces();
	for (int i = 0; i < interfaceList.length(); i++)
	{
		eAttr.addField(interfaceList[i], i);
	}
	MERR(addAttribute(npLocalInterface), "Adding motive local interface attribute");

	// Motive mode
	npMode = eAttr.create("motiveMode", "mode", 0, &status);
	MCHECKERROR(status, "creating motive mode");
	eAttr.addField("Multicast", 0);  // see NatNetClient constructor for name->int mapping
	eAttr.addField("Unicast", 1);
	MERR(addAttribute(npMode), "adding motive mode");

	// Info 
	statusInfo = tAttr.create("info", "ifo", MFnData::kString, MObject::kNullObj, &status);
	MCHECKERROR(status, "creating info");
	tAttr.setStorable(false);
	tAttr.setKeyable(false);
	tAttr.setConnectable(false);
	MERR(addAttribute(statusInfo), "add info");

	// Scale
	oScale = numAttr.create("scale", "sc", MFnNumericData::kFloat, 1.0, &status);
	MCHECKERROR(status, "creating scale");
	MERR(addAttribute(oScale), "adding scale");


	//////////////
	// Segment (Compound)
	segments = cAttr.create("segments", "seg", &status);
	MCHECKERROR(status, "creatoing rigidbody attribute");
	cAttr.setArray(true);

	// Segment -> Name 
	segmentName = tAttr.create("segmentName", "sn", MFnData::kString, MObject::kNullObj, &status);
	MCHECKERROR(status, "creating segment name");
	MERR(addAttribute(segmentName), "adding segment name");
	MERR(cAttr.addChild(segmentName), "adding segment name child attribute");

	//  Segment -> Translate 
	segmentTranslation = numAttr.create("segmentTranslate", "st", MFnNumericData::k3Float, 0.0, &status);
	MCHECKERROR(status, "creating segment translate");
	numAttr.setWritable(false);
	MERR(addAttribute(segmentTranslation), "adding segment translate");
	MERR(cAttr.addChild(segmentTranslation), "adding segment translation child attribute");

	// Segment -> Rotate 
	segmentRotation = numAttr.create("segmentRotate", "sr", MFnNumericData::k3Float, 0.0, &status);
	MCHECKERROR(status, "creating segment rotate");
	numAttr.setWritable(false);
	MERR(addAttribute(segmentRotation), "adding segment rotate");
	MERR(cAttr.addChild(segmentRotation), "adding segment rotate child attribute");

	MERR(addAttribute(segments), "adding segment attribute");


	//////////////
	// Rigidbody (Compound)
	rigidbodies = cAttr.create("rigidbodies", "rb", &status);
	MCHECKERROR(status, "creating rigidbody attribute");
	cAttr.setArray(true);
	
	// Rigidbody -> Name 
	rigidbodyName = tAttr.create("rigidbodyName", "rn", MFnData::kString, MObject::kNullObj, &status);
	MCHECKERROR(status, "creating rigidbody name");
	MERR(addAttribute(rigidbodyName), "adding rigidbody name");
	MERR(cAttr.addChild(rigidbodyName), "adding rigidbody name child attribute");

	//  Rigidbody -> Translate 
	rigidbodyTranslation = numAttr.create("rigidbodyTranslate", "rt", MFnNumericData::k3Float, 0.0, &status);
	MCHECKERROR(status, "creating rigidbody translate");
	numAttr.setWritable(false);
	MERR(addAttribute(rigidbodyTranslation), "adding rigidbody translate");
	MERR(cAttr.addChild(rigidbodyTranslation), "adding rigidbody translation child attribute");

	// Rigidbody -> Rotate 
	rigidbodyRotation = numAttr.create("rigidbodyRotate", "rr", MFnNumericData::k3Float, 0.0, &status);
	MCHECKERROR(status, "creating rigidbody rotate");
	numAttr.setWritable(false);
	MERR(addAttribute(rigidbodyRotation), "adding rigidbody rotate");
	MERR(cAttr.addChild(rigidbodyRotation), "adding rigidbody rotate child attribute");

	MERR(addAttribute(rigidbodies), "adding rigidbody attribute");


	//////////////
	// Marker (Compound)
	markers = cAttr.create("markers", "mkr", &status);
	MCHECKERROR(status, "creatoing markers attribute");
	cAttr.setArray(true);

	// Marker -> Name 
	markerName = tAttr.create("markerName", "mn", MFnData::kString, MObject::kNullObj, &status);
	MCHECKERROR(status, "creating marker name");
	MERR(addAttribute(markerName), "adding rigidbodyName");
	MERR(cAttr.addChild(markerName), "adding rigidbody name child attribute");

	//  Marker -> Translate 
	markerTranslation = numAttr.create("markerTranslate", "mt", MFnNumericData::k3Float, 0.0, &status);
	MCHECKERROR(status, "creating marker translate");
	numAttr.setWritable(false);
	MERR(addAttribute(markerTranslation),   "adding marker translate");
	MERR(cAttr.addChild(markerTranslation), "adding marker translation child attribute");

	MERR(addAttribute(markers), "adding markers attribute");

	//////////////////
	// Parent "Mocap" attribute (array)
	oMocap = numAttr.create("mocap", "mc", MFnNumericData::kInt, 0.0, &status);
	MCHECKERROR(status, "creating mocap attribute");
	MERR(addAttribute(oMocap), "adding mocap attribute");

	// add children to mocap
	//MERR(cAttr.addChild(rigidbodies), "adding mocap attribute as child");
	//MERR(cAttr.addChild(markers),     "adding markers attribute as child");
	//MERR(cAttr.addChild(segments),    "adding segments attribute as child");

	////////////////
	// Attribute Affects
	MERR( attributeAffects(npCommandPort, oMocap),        "command port affects mocap");
	MERR( attributeAffects(npDataPort, oMocap),           "data port affects mocap");
	MERR( attributeAffects(npMulticastAddress, oMocap),   "multicast address affects mocap");
	MERR( attributeAffects(npLocalInterface, oMocap),        "local interface affects mocap");
	MERR( attributeAffects(npMotiveIp, oMocap),           "motive ip affects mocap");
	MERR( attributeAffects(live, oMocap),                 "live affects mocap");
	MERR( attributeAffects(frameRate, oMocap),            "frame rate affects mocap");
	MERR( attributeAffects(oMocap, rigidbodies), "mocap affects rigidbodies");
	MERR( attributeAffects(oMocap, markers),     "mocap affects markers");
	MERR( attributeAffects(oMocap, segments),    "mocap affects segments");

	return MS::kSuccess;
}


MStatus MayaMotive::compute(const MPlug &plug, MDataBlock& block)
{
	MStatus status;
	MObject thisNode = thisMObject();

	// Command Port 
	MDataHandle hCommandPort = block.inputValue(npCommandPort, &status);
	MCHECKERROR(status, "getting command port value");
	commandPort = hCommandPort.asInt();

	// Data Port 
	MDataHandle hDataPort = block.inputValue(npDataPort, &status);
	MCHECKERROR(status, "getting data port value");
	dataPort = hDataPort.asInt();

	// Motive IP
	MDataHandle hMotiveIp = block.inputValue(npMotiveIp, &status);
	MCHECKERROR(status, "getting motive ip value");
	motiveIp = hMotiveIp.asString();

	// Local Interface
	MDataHandle hLocalInterface = block.inputValue(npLocalInterface, &status);
	MCHECKERROR(status, "getting motive local interface value");
	localInterface = interfaceList[hLocalInterface.asInt()];

	// Motive Multicast Interface
	MDataHandle hMulticastAddress = block.inputValue(npMulticastAddress, &status);
	MCHECKERROR(status, "getting motive multicast interface value");
	multicastAddress = hMulticastAddress.asString();

	// mode
	MDataHandle hMode = block.inputValue(npMode, &status);
	MCHECKERROR(status, "getting mode value");
	mode = hMode.asInt();

	// scale (float)
	MDataHandle hScale = block.inputValue(oScale, &status);
	MCHECKERROR(status, "getting scale value");
	float scale = 1.0f;
	if (status) scale = hScale.asFloat();
	

	/************ Get an entry ****************/

	MCharBuffer buffer;
	status = popThreadData(buffer);
	if (status != MS::kSuccess) 
		return MS::kSuccess;

	char *ptr = buffer.ptr();
	size_t *datalen = (size_t*)ptr;
	ptr += sizeof(size_t);

	if (*datalen == 0)
	{
		MDataHandle infoHandle = block.outputValue(statusInfo, &status);
		if (status) infoHandle.set("No data");
		return MS::kSuccess;
	}
	

	// Output
	MDataHandle outHandle = block.outputValue(oMocap, &status);
	MCHECKERROR(status, "mocap handle");

	std::vector<MotiveItem*> items;

	parseItems(items, ptr, *datalen);

	MPlug rbPlug(thisNode, rigidbodies);
	MPlug rbtPlug(thisNode, rigidbodyTranslation);
	MPlug rbrPlug(thisNode, rigidbodyRotation);
	MPlug rbnPlug(thisNode, rigidbodyName);

	MPlug segPlug(thisNode, segments);
	MPlug segtPlug(thisNode, segmentTranslation);
	MPlug segrPlug(thisNode, segmentRotation);
	MPlug segnPlug(thisNode, segmentName);

	MPlug mkrPlug(thisNode, markers);
	MPlug mkrtPlug(thisNode, markerTranslation);
	MPlug mkrnPlug(thisNode, markerName);

	float rad2deg = 57.2958f;

	for (std::vector<MotiveItem*>::iterator i = items.begin(); i != items.end(); i++)
	{
		MotiveRigidbody *rb = dynamic_cast<MotiveRigidbody*>(*i);
		if (rb != NULL)
		{
			int index = findNameId(rigidbodyNames, rb->id, rbPlug, rbnPlug, rigidbodies, block);
			setVector(rb->tx*scale,   rb->ty*scale,   rb->tz*scale,   index, rigidbodies, rbtPlug, block);
			setVector(rb->rx*rad2deg, rb->ry*rad2deg, rb->rz*rad2deg, index, rigidbodies, rbrPlug, block);
		}

		MotiveMarker *marker = dynamic_cast<MotiveMarker*>(*i);
		if (marker != NULL)
		{
			int index = findNameId(markerNames, marker->id, mkrPlug, mkrnPlug, markers, block);
			setVector(marker->x*scale, marker->y*scale, marker->z*scale, index, markers, mkrtPlug, block);
		}

		MotiveSegment *segment = dynamic_cast<MotiveSegment*>(*i);
		if (segment != NULL)
		{
			int index = findNameId(jointNames, segment->id, segPlug, segnPlug, segments, block);
			setVector(segment->tx*scale,   segment->ty*scale,   segment->tz*scale,   index, segments, segtPlug, block);
			setVector(segment->rx*rad2deg, segment->ry*rad2deg, segment->rz*rad2deg, index, segments, segrPlug, block);
		}

		MotiveMessage *message = dynamic_cast<MotiveMessage*>(*i);
		if (message != NULL)
		{
			MDataHandle infoHandle = block.outputValue(statusInfo, &status);
			if (status == MS::kSuccess) infoHandle.set( MString(message->message) );
		}

		delete *i;

	}

	//block.setClean( plug );
	block.setClean(oMocap);
	block.setClean(rigidbodyName);
	block.setClean(rigidbodyTranslation);
	block.setClean(rigidbodyRotation);
	block.setClean(segmentName);
	block.setClean(segmentTranslation);
	block.setClean(segmentRotation);
	block.setClean(markers);
	block.setClean(markerName);
	block.setClean(statusInfo);
	//block.setClean( output );

	releaseDataStorage(buffer);

	return MS::kSuccess;
}


int MayaMotive::findNameId(
	std::map<int, MString> &mapping, 
	int     nameId, 
	MPlug   groupPlug, 
	MPlug   namePlug,
	MObject groupObject, 
	MDataBlock& block)
{
	MStatus status;
	std::map<int, MString>::iterator i = mapping.find(nameId);
	if (i == mapping.end()) return -1;
	MString name = (*i).second;

	int arrayIndex = -1;
	for (unsigned int i = 0; i < groupPlug.numElements(); i++)
	{
		// Get the name
		MERR(namePlug.selectAncestorLogicalIndex(i, groupObject), "Selecting name attribute");
		MString a = namePlug.asString();
		MString b = MString(name);
		if ( a == b) return i;
	}
	if (arrayIndex == -1)
	{
		// Append
		arrayIndex = groupPlug.numElements();
		MERR(namePlug.selectAncestorLogicalIndex(arrayIndex, groupObject), "Selecting name attribute");
		MDataHandle nHandle = namePlug.constructHandle(block);
		nHandle.set(MString(name));
		namePlug.setValue(name);		
	}
	return arrayIndex;
}


MStatus MayaMotive::setVector(float x, float y, float z, int index, MObject &groupObject, MPlug &plug, MDataBlock &block )
{
	MStatus status;

	MERR(plug.selectAncestorLogicalIndex(index, groupObject), "Selecting attribute");
	MDataHandle tHandle = plug.constructHandle(block);

	float3& otrans = tHandle.asFloat3();

	otrans[0] = x;
	otrans[1] = y;
	otrans[2] = z;

	MERR(plug.setValue(tHandle), "Setting vector handle");
	plug.destructHandle(tHandle);

	return MS::kSuccess;

}


/*
*  Device Thread - gets the data from the socket and saves it in the maya storage buffer
*/

void MayaMotive::sendData(const char *data, size_t datalen)
{
	// Send the message length and data lengths as two size_t's, then send the message and the data

	MStatus status;

	if (datalen == 0) return;

	if (datalen + sizeof(size_t) >= BUFSIZE)
	{
		// Overflow
		return;
	}

	MCharBuffer buffer;
	status = acquireDataStorage(buffer);
	if (status != MS::kSuccess)
	{
		// Could not allocate memory - this can happen if data is coming too quickly
		// and generally can be safely ignored (frames skipped)
		return;
	}

	// Position of data blocks
	size_t *header = (size_t*)buffer.ptr();
	char   *datablock = buffer.ptr() + sizeof(size_t);

	// thread safe copy data
	beginThreadLoop();
	{
		*header = datalen;
		memcpy(datablock, data, datalen);
		pushThreadData(buffer);
	}
	endThreadLoop();
}


void MayaMotive::threadHandler()
{
	// Called by maya as the entry point to the device thread

	MStatus status;
	setDone(false);

	char buffer[BUFSIZE];

	while (!isDone())
	{
		if (!isLive())
		{
			if (this->isConnected()) this->disconnect();
			continue;
		}

		if (!this->isConnected())
		{
			this->connect();
			Sleep(500);
			continue;
		}

		while (!isDone() && isLive())
		{
			// Block till a frame arrives
			DWORD ret = WaitForSingleObject(hEvent, 200);
			if (ret == WAIT_OBJECT_0)
			{
				// get data (thread safe)
				WaitForSingleObject(hMutex, INFINITE);
				size_t ret = serializeItems(items, buffer, BUFSIZE);
				ReleaseMutex(hMutex);

				if (ret == 0) continue;
				sendData(buffer, ret);
			}
		}
	}

	disconnect();

	setDone(true);
}

void MayaMotive::threadShutdownHandler()
{
	setDone(true);
}

bool MayaMotive::isConnected()
{
	return natNetClient != NULL;
}

void MayaMotive::disconnect()
{
	natNetClient->Uninitialize();
	delete natNetClient;
	natNetClient = NULL;
}


bool MayaMotive::connect()
{
	if (natNetClient != NULL) disconnect();

	natNetClient = new NatNetClient(mode);

	char *mc = (char*)multicastAddress.asChar();
	natNetClient->SetMulticastAddress(mc);

	char *motive = (char*)motiveIp.asChar();
	char *iface = (char*)localInterface.asChar();
	int ret = natNetClient->Initialize(iface, motive, commandPort, dataPort);

	if (ret == 0)
	{
		mConnected = true;
		natNetClient->SetMessageCallback(&::messageCallback);
		natNetClient->SetDataCallback(&::dataCallback, (void*)this);
		getDescriptions();
		return true;
	}
	else
	{
		mConnected = false;
		MGlobal::displayError("Could not connect to Motive");
		return false;
	}
}


void MayaMotive::getDescriptions()
{

	WaitForSingleObject(hMutex, INFINITE);

	sDataDescriptions *desc;

	int ret = natNetClient->GetDataDescriptions(&desc);

	for (size_t i = 0; i < ret; i++)
	{
		if (desc->arrDataDescriptions[i].type == Descriptor_MarkerSet)
		{
			sMarkerSetDescription* md;

			md = desc->arrDataDescriptions[i].Data.MarkerSetDescription;

			char*ptr = *md->szMarkerNames;
			for (int j = 0; j < md->nMarkers; j++)
			{
				size_t len = strlen(ptr);
				MString name(md->szName);
				name += '_';
				name += MString(ptr);
				markerNames[j] = name;
				ptr += len + 1;
			}
		}

		if (desc->arrDataDescriptions[i].type == Descriptor_RigidBody)
		{
			sRigidBodyDescription *rbd;
			rbd = desc->arrDataDescriptions[i].Data.RigidBodyDescription;
			rigidbodyNames[rbd->ID] = MString(rbd->szName);
		}

		if (desc->arrDataDescriptions[i].type == Descriptor_Skeleton)
		{
			sSkeletonDescription *sd;
			sd = desc->arrDataDescriptions[i].Data.SkeletonDescription;
			int id = sd->skeletonID;

			subjectNames[id] = sd->szName;

			for (size_t j = 0; j < sd->nRigidBodies; j++)
			{
				int rbid = sd->RigidBodies[j].ID | (sd->skeletonID << 16);
				jointNames[rbid] = MString(sd->RigidBodies[j].szName);
			}
		}
	}

	ReleaseMutex(hMutex);
}


void dataCallback(sFrameOfMocapData *f, void *ptr)
{
	MayaMotive* m = (MayaMotive*)ptr;
	m->dataCallback(f);
}

void MayaMotive::dataCallback(sFrameOfMocapData *data)
{
	// Sync motive and maya sub-thread
	WaitForSingleObject(hMutex, INFINITE);

	for (size_t i = 0; i < data->nRigidBodies; i++)
	{
		const sRigidBodyData &rbd = data->RigidBodies[i];
		MQuaternion q(rbd.qx, rbd.qy, rbd.qz, rbd.qw);
		MEulerRotation e = q.asEulerRotation();
		items.push_back(new MotiveSegment(rbd.ID, rbd.x, rbd.y, rbd.z, e.x, e.y, e.z));
	}

	for (size_t i = 0; i < data->nSkeletons; i++)
	{
		const sSkeletonData &skel = data->Skeletons[i];
		for (size_t j = 0; j < skel.nRigidBodies; j++)
		{
			const sRigidBodyData &rbd = skel.RigidBodyData[j];
			MQuaternion q(rbd.qx, rbd.qy, rbd.qz, rbd.qw);
			MEulerRotation e = q.asEulerRotation();
			items.push_back(new MotiveSegment(rbd.ID, rbd.x, rbd.y, rbd.z, e.x, e.y, e.z));
		}
	}

	ReleaseMutex(hMutex);
	SetEvent(hEvent);

}


void messageCallback(int id, char *msg)
{
	MGlobal::displayInfo(msg);
}



