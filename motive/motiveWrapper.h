#ifndef MOTIVE_WRAPPER_H
#define MOTIVE_WRAPPER_H

#include "NatNetTypes.h"
#include "NatNetClient.h"
#include "item.h"

#include <map>
#include <string>
#include <vector>

#include <Windows.h>

class RigidBody
{
public:
	RigidBody() {}

	RigidBody(sRigidBodyData data)
		: tx(data.x)
		, ty(data.y)
		, tz(data.z)
		, rx(data.qx)
		, ry(data.qy)
		, rz(data.qz)
		, rw(data.qw)
	{}

	float tx;
	float ty;
	float tz;
	float rx;
	float ry;
	float rz;
	float rw;

};


class MotiveWrapper
{
public:
	bool Init(int mode, char *local, char *remote, int commandPort, int dataPort);

	void getDescriptions();

	void dataCallback(sFrameOfMocapData *data);

	std::vector <std::string> mkrDesc;
	std::vector < peel::Marker* > markers;
	std::vector < peel::Segment* > segments;

	std::map <int, std::string>  rbDesc;
	std::map <int, RigidBody>    rbData;
	std::map <int, std::string>  skelNames;
	std::map <int, std::string > skelDesc;

	NatNetClient *natNetClient;

	int frameCount;

	HANDLE hMutex;
};

void dataCallback(sFrameOfMocapData *, void *);
void messageCallback(int, char*);


#endif