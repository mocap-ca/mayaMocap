#ifndef PEEL_MIDI_DEVICE_H
#define PEEL_MIDI_DEVICE_H

#include <maya/MPxThreadedDeviceNode.h>
#include <maya/MStatus.h>
#include <maya/MStringArray.h>
#include <maya/MFloatArray.h>

#include <Windows.h>
#include <Mmsystem.h>



class MidiDevice : public MPxThreadedDeviceNode
{
public:
	MidiDevice();
	~MidiDevice();

	//void threadHandler(const char* serverName, const char *deviceName );
	void threadHandler();
	void threadShutdownHandler();
	void postConstructor();

	virtual MStatus     compute(const MPlug& plug, MDataBlock& data);

	static MStatus      initialize();

	static void*  creator() { return new MidiDevice(); }

	void connectDevice();

	void midi_data(unsigned char event,
		unsigned char channel,
		unsigned char note,
		unsigned char velocity,
		int t
	);

	static MObject      oDevice;    // enum (in)
	static MObject      oChannel;   // int (in)
	static MObject      oMidiOut;   // 127 floats  (out)
	static MObject      oTime;
	static MObject      oStep;
	static MObject      oFile;
	static MObject      oWrite;

	static MTypeId id;

	MStringArray names;
	HMIDIIN      mHandle;
	UINT         mDeviceId;
	char         mChannel;
	static UINT  mDevices;
	unsigned char  mValues[127];
	int            mTime;
	int            mStep;

	FILE *mFp;

	mutable CRITICAL_SECTION cs;
};


#endif //  PEEL_MIDI_DEVICE_H