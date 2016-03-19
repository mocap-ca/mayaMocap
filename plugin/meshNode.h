#ifndef MOCAP_MESH_H
#define MOCAP_MESH_H

#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>





class MocapMesh : public MPxNode
{
public:
	MocapMesh();

	MStatus               compute( const MPlug& plug, MDataBlock& data );
	static  void*    creator();
	static  MStatus  initialize();

	static  MTypeId  id;

	static MObject in_inMesh;
	static MObject out_message;

	double value;
};


#endif