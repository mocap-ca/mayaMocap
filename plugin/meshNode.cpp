#include "meshNode.h"
#include "mesh.h"

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>

#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

MObject MocapMesh::in_inMesh;
MObject MocapMesh::out_message;

MocapMesh::MocapMesh()
	: value(0.0) {}

void* MocapMesh::creator() { return new MocapMesh(); }
MTypeId MocapMesh::id(0x001126D3);

MStatus MocapMesh::initialize()
{
	MStatus stat;

	

	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;

	in_inMesh  = tAttr.create("inMesh", "im",MFnData::kMesh);
	tAttr.setKeyable(true);
	tAttr.setStorable(true);
	tAttr.setReadable(true);
	tAttr.setWritable(true);
	stat = addAttribute(in_inMesh);
	if(!stat) { stat.perror("adding inMesh"); return stat; }

	out_message  = nAttr.create("test", "test", MFnNumericData::kDouble, 0.0);
	nAttr.setKeyable(false);
	nAttr.setStorable(false);
	nAttr.setWritable(false);
	nAttr.setReadable(true);
	stat = addAttribute(out_message);
	if(!stat) { stat.perror("adding test"); return stat; }

	stat = attributeAffects(in_inMesh,out_message);
	if(!stat) { stat.perror("attributeAffects"); return stat; }
	return MS::kSuccess;
}
MStatus MocapMesh::compute( const MPlug &plug, MDataBlock &data )
{
	MStatus stat;
	MDataHandle indata_inMesh = data.inputValue(in_inMesh, &stat);
	if(stat != MS::kSuccess) return stat;

	MDataHandle hMessage = data.outputValue( out_message, &stat );
	if(stat != MS::kSuccess) return stat;

	double &v = hMessage.asDouble();
	value += 0.001;
	v = value;
	

	MObject oMesh = indata_inMesh.asMesh();
	MFnMesh fnMesh( oMesh );

	MPointArray points;
	fnMesh.getPoints( points );

	MIntArray vertCount, vertList;
	fnMesh.getVertices( vertCount, vertList );

	FILE * fp = fopen( "c:\\users\\al\\mesh.txt", "w");
	if(fp)
	{
		int i;
		for( i = 0; i < points.length(); i++)
		{
			fprintf(fp,  "%f %f %f %f\n", points[i].x,  points[i].y,  points[i].z,  points[i].w );
		}

		for( i = 0; i < vertCount.length(); i++)
		{
			fprintf(fp, "%d ", vertCount[i] );
		}
		fprintf(fp, "\n");

		for( i = 0; i < vertList.length(); i++)
		{
			fprintf(fp, "%d ", vertList[i] );
		}
		fprintf(fp, "\n");

	}

	double *pointdata = & (points[0].x  );
	int * countdata   = & (vertCount[0] );
	int * vertdata    = & (vertList[0]  );



	

	MDataHandle outdata_message = data.outputValue(out_message, &stat);
	if(stat != MS::kSuccess) return stat;

	return MS::kSuccess;
}