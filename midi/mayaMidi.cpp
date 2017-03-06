#include "mayaMidi.h"

#define BUFSIZE 256

MObject MidiDevice::oDevice;
MObject MidiDevice::oChannel;
MObject MidiDevice::oMidiOut;
MObject MidiDevice::oTime;
MObject MidiDevice::oStep;
UINT MidiDevice::mDevices;

#include <maya/MFnPlugin.h>
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

MTypeId MidiDevice::id(0x001126D7);

void MidiDevice::postConstructor()
{
	MObjectArray oArray;
	oArray.append(MidiDevice::oStep);
	setRefreshOutputAttributes(oArray);
	createMemoryPools(1, BUFSIZE, 1);
	InitializeCriticalSection(&cs);
	mTime = 0;
	mStep = 0;

}

MidiDevice::~MidiDevice()
{
	destroyMemoryPools();
}


void CALLBACK callback(
	HMIDIIN   hMidiIn,
	UINT      wMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam,
	DWORD_PTR timeStamp
)
{
	WORD p0 = HIWORD(dwParam);
	WORD p1 = LOWORD(dwParam);

	unsigned char note = HIBYTE(p1);
	unsigned char vel = LOBYTE(p0);

	WORD lw = LOWORD(wMsg);
	WORD hw = HIWORD(wMsg);

	unsigned char byte1 = HIBYTE(lw);
	unsigned char byte2 = LOBYTE(hw);

	unsigned event = (LOBYTE(lw) & 0xf0);
	unsigned channel = (LOBYTE(lw) & 0x0f);

	MidiDevice *ptr = (MidiDevice*)dwInstance;
	ptr->midi_data(event, channel, note, vel, timeStamp);
}

void MidiDevice::midi_data(unsigned char event, 
	unsigned char channel, 
	unsigned char note, 
	unsigned char velocity,
	int t)
{
	if (channel != mChannel) return;
	if (event != 0xc0) return;
	if (note >= 127) return;
	EnterCriticalSection(&cs);
	mValues[note] = velocity;
	mTime = t;
	LeaveCriticalSection(&cs);
}

void MidiDevice::connectDevice()
{
	MMRESULT res;

	memset(mValues, 0, 127);
	mTime = mTime + 1; // force refresh

	if (mHandle != NULL)
	{
		midiInClose(mHandle);
		midiInStop(mHandle);
	}

	res = midiInOpen(&mHandle, mDeviceId, (DWORD_PTR)&callback, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if (res != MMSYSERR_NOERROR)
	{
		mHandle = NULL;
		printf("Error opening midi device: %d\n", res);
		return;
	}

	res = midiInStart(mHandle);
	if (res != MMSYSERR_NOERROR)
	{
		mHandle = NULL;
		printf("Could not start midi\n");
	}
}


void MidiDevice::threadHandler()
{
	// Called by maya as the entry point to the device thread

	MStatus status;
	setDone(false);

	MCharBuffer buffer;

	while (!isDone())
	{
		if (!isLive()) continue;

		if (mHandle == NULL)
		{
			this->connectDevice();
			Sleep(500);
			continue;
		}

		while (!isDone() && isLive())
		{
			status = acquireDataStorage(buffer);
			if (status != MS::kSuccess)
				continue;

			unsigned char tmp[BUFSIZE];

			EnterCriticalSection(&cs);
			mStep++;
			memset(tmp, 0, BUFSIZE);
			memcpy(tmp, mValues, 127);
			memcpy(tmp + 127, &mTime, sizeof(unsigned int));
			memcpy(tmp + 127 + sizeof(unsigned int), &mStep, sizeof(unsigned int));
			LeaveCriticalSection(&cs);

			beginThreadLoop();
			{
				memcpy(buffer.ptr(), tmp, BUFSIZE);
				pushThreadData(buffer);
			}
			endThreadLoop();


		}
	}

	if (mHandle != NULL)
	{
		midiInClose(mHandle);
		midiInStop(mHandle);
		mHandle = NULL;
	}

	setDone(true);
}

void MidiDevice::threadShutdownHandler()
{
	setDone(true);
}


#define MCONT(s, msg) if( s != MS::kSuccess) { s.perror(msg); continue; }
#define MCHECKERROR(x, msg) { if(x!=MS::kSuccess) { x.perror(msg); return x;} }


MStatus MidiDevice::initialize()
{
	MStatus status;
	MFnNumericAttribute   numAttr;
	MFnCompoundAttribute  cAttr;
	MFnTypedAttribute     tAttr;
	MFnMessageAttribute   mAttr;
	MFnEnumAttribute      eAttr;


	MMRESULT res;

	mDevices = midiInGetNumDevs();


	// device names - string list 
	oDevice = eAttr.create("device", "dn", MFnData::kString, &status);
	eAttr.setReadable(true);

	for (UINT i = 0; i < mDevices; i++)
	{
		MIDIINCAPS caps;
		res = midiInGetDevCaps((UINT_PTR)i, &caps, sizeof(MIDIINCAPS));
		if (res == MMSYSERR_NOERROR)
			eAttr.addField(MString(caps.szPname), i);
		else
			printf("Error: %d", res);
	}
	status = addAttribute(oDevice);
	attributeAffects(oDevice, oMidiOut);

	// Channel - int
	oChannel = numAttr.create("channel", "c", MFnNumericData::kInt, 0, &status);
	addAttribute(oChannel);
	attributeAffects(oChannel, oMidiOut);
	attributeAffects(oChannel, oStep);

	// Time - int
	oTime = numAttr.create("time", "t", MFnNumericData::kInt, 0, &status);
	numAttr.setWritable(false);
	addAttribute(oTime);
	attributeAffects(oTime, oMidiOut);
	attributeAffects(oTime, oStep);

	// Step - int
	oStep = numAttr.create("step", "s", MFnNumericData::kInt, 0, &status);
	numAttr.setWritable(false);
	addAttribute(oStep);
	//attributeAffects(oStep, );

	// MidiOut - 127 channels, float array
	oMidiOut = numAttr.create("midi", "m", MFnNumericData::kFloat, 0, &status);
	numAttr.setReadable(true);
	numAttr.setUsesArrayDataBuilder(true);
	numAttr.setArray(true);
	addAttribute(oMidiOut);
	attributeAffects(oStep, oMidiOut);

	attributeAffects(live, oMidiOut);
	attributeAffects(frameRate, oMidiOut);

	return MS::kSuccess;
}





MStatus MidiDevice::compute(const MPlug &plug, MDataBlock& block)
{
	MStatus status;
	unsigned int i;

	MObject thisNode = thisMObject();

#ifdef _DEBUG
	printf("Compute: %s\n", plug.name().asChar());
#endif

	// Current Device 
	MDataHandle hCurrentDevice = block.inputValue(oDevice, &status);
	MCHECKERROR(status, "getting current device data");
	mDeviceId = hCurrentDevice.asInt();

	MDataHandle hChannel = block.inputValue(oChannel, &status);
	MCHECKERROR(status, "getting channel data");
	mChannel = hChannel.asInt();

	// Get an entry
	MCharBuffer buffer;
	status = popThreadData(buffer);
	if (status != MS::kSuccess)
	{
		printf(".");
		fflush(stdout);
		return MS::kSuccess;
	}


	MPlug pValues(thisNode, oMidiOut);

	MDataHandle handle = pValues.constructHandle(block);
	MArrayDataHandle hValues(handle, &status);
	MCHECKERROR(status, "getting output array handle");

	MArrayDataBuilder builderValues = hValues.builder(&status);
	MCHECKERROR(status, "getting midi array out handle");

	unsigned char *data = (unsigned char*)buffer.ptr();

	for (i = 0; i < 127; i++)
	{
		MDataHandle hValue = builderValues.addElement(i, &status);
		MCHECKERROR(status, "setting midi out");
		hValue.set(float(data[i]) / 127.0f );
	}

	// Time
	MDataHandle hTime = block.outputValue(oTime, &status);
	MCHECKERROR(status, "getting time handle");
	int *tval = (int*)(data + 127);
	hTime.set(*tval);

	// Step
	MDataHandle hStep = block.outputValue(oStep, &status);
	MCHECKERROR(status, "getting step handle");
	int *sval = (int*)(data + 127 + sizeof(unsigned int));
	hStep.set(*sval);


	status = hValues.set(builderValues);
	pValues.setValue(handle);
	pValues.destructHandle(handle);


	block.setClean(oMidiOut);
	block.setClean(oTime);
	block.setClean(oStep);

	releaseDataStorage(buffer);

	return MS::kSuccess;
}



MStatus initializePlugin(MObject obj)
{
	MStatus result;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");
	result = plugin.registerNode("peelMidi",
		MidiDevice::id,
		MidiDevice::creator,
		MidiDevice::initialize,
		MPxNode::kThreadedDeviceNode);

	return result;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus result;
	MFnPlugin plugin(obj);
	result = plugin.deregisterNode(MidiDevice::id);
	return result;
}