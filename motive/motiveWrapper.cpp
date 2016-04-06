#include "motiveWrapper.h"

#include <string>
#include <sstream>

MotiveWrapper *gWrapper;

bool MotiveWrapper::Init(int mode, char *local, char *remote, int commandPort, int dataPort)
{
	natNetClient = new NatNetClient(0);
	natNetClient->Initialize(local, remote, commandPort, dataPort);
	natNetClient->SetMessageCallback(&::messageCallback);
	natNetClient->SetDataCallback(&::dataCallback, (void*)this);
}


void MotiveWrapper::getDescriptions()
{
	sDataDescriptions *desc;

	int ret = natNetClient->GetDataDescriptions(&desc);

	for (size_t i = 0; i < ret; i++)
	{
		if (desc->arrDataDescriptions[i].type == Descriptor_MarkerSet)
		{
			sMarkerSetDescription* md;

			md = desc->arrDataDescriptions[i].Data.MarkerSetDescription;

			char*ptr = *md->szMarkerNames;
			for (size_t j = 0; j < md->nMarkers; j++)
			{
				std::ostringstream oss;
				oss << md->szName << " " << ptr;
				mkrDesc.push_back(oss.str());
				ptr += strlen(ptr) + 1;

			}
		}

		if (desc->arrDataDescriptions[i].type == Descriptor_RigidBody)
		{
			sRigidBodyDescription *rbd;
			rbd = desc->arrDataDescriptions[i].Data.RigidBodyDescription;
			rbDesc[rbd->ID] = std::string(rbd->szName);
		}

		if (desc->arrDataDescriptions[i].type == Descriptor_Skeleton)
		{
			sSkeletonDescription *sd;
			sd = desc->arrDataDescriptions[i].Data.SkeletonDescription;
			int id = sd->skeletonID;

			skelNames[id] = sd->szName;

			for (size_t j = 0; j < sd->nRigidBodies; j++)
			{
				int rbid = sd->RigidBodies[j].ID | (sd->skeletonID << 16);
				skelDesc[rbid] = std::string(sd->RigidBodies[j].szName);
			}
		}
	}
}



void MotiveWrapper::dataCallback(sFrameOfMocapData *data)
{
	int          frame = data->iFrame;
	unsigned int tc = data->Timecode;
	unsigned int subf = data->TimecodeSubframe;

	int hour, min, sec, tframe, subframe;
	char buf[64];

	frameCount++;

	natNetClient->DecodeTimecode(tc, subf, &hour, &min, &sec, &tframe, &subframe);
	natNetClient->TimecodeStringify(tc, subf, buf, 64);

	// Add the data to rbData member	
	WaitForSingleObject(hMutex, INFINITE);
	rbData.clear();
	for (size_t i = 0; i < data->nRigidBodies; i++)
	{
		const sRigidBodyData &rbd = data->RigidBodies[i];
		rbData[rbd.ID] = RigidBody(rbd);
	}

	for(size_t i=0; i < data->nLabeledMarkers; i++)
	{
	const sMarker &mkr = data->LabeledMarkers[i];
	if( i >= mkrDesc.size() ) getDescriptions();
	if ( i < mkrDesc.size() )
		markers.push_back( new peel::Marker( mkrDesc[i].c_str(), mkr.x, mkr.y, mkr.z ));
	}

	for (size_t i = 0; i < data->nSkeletons; i++)
	{
		const sSkeletonData &skel = data->Skeletons[i];
		for (size_t j = 0; j < skel.nRigidBodies; j++)
		{
			const sRigidBodyData &rbd = skel.RigidBodyData[j];
			std::string name = skelDesc[rbd.ID];
			segments.push_back(new peel::Segment(name.c_str(), rbd.x, rbd.y, rbd.z, rbd.qx, rbd.qy, rbd.qz, rbd.qw));
		}
	}

	ReleaseMutex(hMutex);
	

}

void messageCallback(int id, char *msg)
{
	gMotive->messageCallback(id, msg);
}


void dataCallback(sFrameOfMocapData* data, void *ptr)
{
	Motive *m = (Motive*)ptr;
	m->dataCallback(data);
}