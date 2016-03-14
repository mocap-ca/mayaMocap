#include "threadedDevice.h"


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



#include "item.h"
#include <vector>
#include <string>


MObject ThreadedDevice::mocap;
MObject ThreadedDevice::outputName;
MObject ThreadedDevice::outputTranslate;
MObject ThreadedDevice::outputRotate;

MObject ThreadedDevice::info;


#define MCONT(s, msg) if( s != MS::kSuccess) { s.perror(msg); continue; }
#define MCHECKERROR(x, msg) { if(x!=MS::kSuccess) { x.perror(msg); return x;} }



void ThreadedDevice::postConstructor()
{
    MObjectArray oArray;
    oArray.append( ThreadedDevice::mocap );
    setRefreshOutputAttributes(oArray);
    createMemoryPools(1, 1024, 1);
}

ThreadedDevice::~ThreadedDevice()
{
    destroyMemoryPools();
}


/*
 *  Main Thread - gets the data from the socket and saves it in the maya storage buffer
 */

void ThreadedDevice::sendData(const char *message, size_t msglen, const char *data, size_t datalen)
{
    // Send the message length and data lengths as two size_t's, then send the message and the data
    MStatus status;
    if(message != NULL && msglen == 0)
    {
        // c string was passed without length
         msglen = strlen(message);
    }

    MCharBuffer buffer;
    status = acquireDataStorage(buffer);
    if( status != MS::kSuccess )
    {
        printf("X");
        fflush(stdout);
        //status.perror("thread storage error");
        return;
    }

    // Position of data blocks
    size_t *header    = (size_t*) buffer.ptr();
    char   *msgblock  = buffer.ptr() + sizeof(size_t)*2;
    char   *datablock = msgblock + msglen;

    //printf("Sizes: %d %d\n", msglen, datalen);

    // thread safe copy data
    beginThreadLoop();
    {
        header[0] = msglen;
        header[1] = datalen;
        if(msglen > 0) memcpy(msgblock,  message, msglen);
        if(datalen > 0 ) memcpy(datablock, data, datalen);
        pushThreadData(buffer);
    }
    endThreadLoop();
}




void ThreadedDevice::threadHandler()
{
    MStatus status;
    setDone(false);

    printf("Threaded Device\n");

    int i;
    
    char receiveBuffer[1024];

    printf("Starting thread\n");

    
	char buffer[1024];


    while(!isDone())
    {
        if(!isLive()) continue;

		if (!this->connect()) 
			break;
           
	        while(!isDone() && isLive()  )
        {
			size_t sz = this->receiveData(buffer, 1024);
			if (sz == -1) break;
			if (sz == 0) continue;
			if(sz > 0) sendData( 0, 0, buffer, sz);
        }
    }

	disconnect();
	
	sendData("Stopped");

    printf("Thread Finished\n");


    setDone(true);
}

void ThreadedDevice::threadShutdownHandler()
{
    setDone(true);
}

MStatus ThreadedDevice::initialize()
{
        MStatus status;
        MFnNumericAttribute   numAttr;
        MFnCompoundAttribute  cAttr;
        MFnTypedAttribute     tAttr;
        MFnMessageAttribute   mAttr;

        // Info 
        info = tAttr.create("info", "ifo", MFnData::kString, MObject::kNullObj, &status);
        MCHECKERROR(status, "creating info");
        tAttr.setStorable(false);
        //tAttr.setWritable(false);
        //tAttr.setReadable(true);
        tAttr.setKeyable(false);
        //tAttr.setConnectable(false);
        status = addAttribute(info);
        MCHECKERROR(status, "add info");


        // Name 
        outputName = tAttr.create("name", "n", MFnData::kString, MObject::kNullObj, &status);
        status = addAttribute(outputName);

        // Translate 
        outputTranslate  = numAttr.create("outputTranslate", "ot", MFnNumericData::k3Float, 0.0, &status);
        numAttr.setWritable(false);
        status = addAttribute(outputTranslate);

        // Rotate 
        outputRotate  = numAttr.create("outputRotate", "orot", MFnNumericData::k3Float, 0.0, &status);
        numAttr.setWritable(false);
        status = addAttribute(outputRotate);

        // Parent "Mocap" attribute (array)
        mocap = cAttr.create("mocap", "mc", &status);
        cAttr.setArray(true);
        status = addAttribute(mocap);
        status = cAttr.addChild(outputName);
        status = cAttr.addChild( outputTranslate );
        status = cAttr.addChild( outputRotate );
        
        attributeAffects( live, mocap);
        attributeAffects( frameRate, mocap);

        attributeAffects( mocap, outputTranslate );
        attributeAffects( mocap, outputRotate );


        return MS::kSuccess;
}



MStatus ThreadedDevice::compute( const MPlug &plug, MDataBlock& block)
{
    MStatus status;

    printf("Compute: %s\n", plug.name().asChar() );

    MPlug pInfo( thisMObject(), info);
    pInfo.setLocked(true);

        
    MObject thisNode = thisMObject();


    // Get an entry
    MCharBuffer buffer;
    status = popThreadData(buffer);
    if(status != MS::kSuccess)
    {
        printf(".");
        fflush(stdout);
        return MS::kSuccess;
    }

    size_t *header = (size_t*)buffer.ptr();
    size_t msglen  = header[0];
    size_t datalen = header[1];

    const char *message = buffer.ptr() + sizeof(size_t) * 2;
    const char *data    = message + msglen;

    if( msglen != 0)
    {
        MDataHandle infoHandle = block.outputValue( info, &status );
        infoHandle.set( MString( message, msglen ) );
    }

    if( datalen != 0)
    {
        // Output
        MArrayDataHandle outHandle = block.outputArrayValue( mocap, &status );
        MCHECKERROR( status, "mocap handle");

        std::vector<Item> items;

        parseItems( data, datalen, items );
        
        MPlug tPlug(thisNode, outputTranslate);
        MPlug rPlug(thisNode, outputRotate);
        MPlug nPlug(thisNode, outputName);

        int arrayIndex;
        int id = 0;

        for(std::vector<Item>::iterator i = items.begin(); i != items.end(); i++, id++)
        {
            arrayIndex = id;
            if( strcmp( (*i).name, "VCAM") == 0 ) arrayIndex = 100;

            // Translation
            status = tPlug.selectAncestorLogicalIndex( arrayIndex, mocap );
            MCHECKERROR(status, "Selecting translate attribute");

            MDataHandle tHandle = tPlug.constructHandle(block);
            MCHECKERROR(status, "Creating translation handle");

            float3& otrans = tHandle.asFloat3();
            otrans[0] = (*i).tx;
            otrans[1] = (*i).ty;
            otrans[2] = (*i).tz;

            tPlug.setValue(tHandle);
            tPlug.destructHandle(tHandle);

            // Rotation 
            status = rPlug.selectAncestorLogicalIndex( arrayIndex, mocap );
            MCHECKERROR(status, "Selecting rotate attribute");

            MDataHandle rHandle = rPlug.constructHandle(block);
            MCHECKERROR( status, "adding rotation element" );

            float3& orot = rHandle.asFloat3();
            MQuaternion q( (*i).rx, (*i).ry, (*i).rz, (*i).rw); 
            MEulerRotation e = q.asEulerRotation();
            orot[0] = e.x * 57.2958f;
            orot[1] = e.y * 57.2958f;
            orot[2] = e.z * 57.2958f;

            rPlug.setValue(rHandle);
            rPlug.destructHandle(rHandle);

            // Name
            status = nPlug.selectAncestorLogicalIndex( arrayIndex, mocap );
            MCHECKERROR(status, "Selecting name attribute");

            MDataHandle nHandle = nPlug.constructHandle(block);
            MCHECKERROR( status, "adding name element" );
        
            nHandle.set(MString((*i).name));

            nPlug.setValue(nHandle);
            nPlug.destructHandle(nHandle);

        }
    }

    //block.setClean( plug );
    block.setClean( mocap );
    block.setClean( outputName );
    block.setClean( outputTranslate );
    block.setClean( outputRotate );
    //block.setClean( output );

    releaseDataStorage(buffer);

    return MS::kSuccess;
}

