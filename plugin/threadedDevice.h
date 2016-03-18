#ifndef PEEL_THREADED_DEVICE_H
#define PEEL_THREADED_DEVICE_H

#include <maya/MPxThreadedDeviceNode.h>
#include <maya/MStatus.h>
#include <maya/MStringArray.h>


class ThreadedDevice : public MPxThreadedDeviceNode
{
public:
    ThreadedDevice() {};
    ~ThreadedDevice() ;

    //void threadHandler(const char* serverName, const char *deviceName );
    void threadHandler();
    void threadShutdownHandler();
    void postConstructor();

    virtual MStatus     compute( const MPlug& plug, MDataBlock& data );

    static MStatus      initialize();

    //size_t              parse(const char *buffer, std::vector<Item> &items );
    void sendData(const char*message, size_t msglen=0, const char* data=NULL, size_t datalen = 0);

	virtual bool connect() = 0; 
    virtual bool isConnected() = 0;
	virtual bool disconnect() = 0;
	virtual size_t receiveData(char *, size_t) = 0;

    static MObject      mocap;
    static MObject      outputName;
    static MObject      outputTranslate;
    static MObject      outputRotate;

    static MObject      info;

    MStringArray names;

};
#endif // PEEL_THREADED_DEVICE_H
