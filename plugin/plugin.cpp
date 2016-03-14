#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
//#include <api_macros.h>

#include "udpDevice.h"
#include "listenCommand.h"
#include "pingCommand.h"

#ifdef _WIN32
#include "pipeDevice.h"
#endif

MStatus initializePlugin( MObject obj )
{
	MStatus status;
	MFnPlugin plugin(obj, "MocapCA", "1.0", "Any");

	status = plugin.registerNode( "peelRealtimeMocap", UdpDevice::id, UdpDevice::creator, UdpDevice::initialize, MPxNode::kThreadedDeviceNode );
	if( !status ) { status.perror("failed to registerNode ThreadedDevice"); }

	status = plugin.registerCommand( "mocapListen", ListenCommand::creator, ListenCommand::newSyntax);
	if(!status) { status.perror("failed to register listen command"); }

	status = plugin.registerCommand( "mocapPing", PingCommand::creator, PingCommand::newSyntax);
	if(!status) { status.perror("failed to register ping command"); }

#ifdef _WIN32
	status = plugin.registerNode("peelRealtimeMocapPipe", PipeDevice::id, PipeDevice::creator, PipeDevice::initialize, MPxNode::kThreadedDeviceNode);
	if (!status) { status.perror("failed to registerNode ThreadedDevice"); }
#endif


	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode( UdpDevice::id );
	if( !status ) { status.perror("failed to deregisterNode UdpDevice"); }

	status = plugin.deregisterCommand( "mocapListen" );
	if( !status) { status.perror( "failed to deregister mocap listem command"); }

	status = plugin.deregisterCommand( "mocapPing" );
	if( !status) { status.perror( "failed to deregister mocap ping command"); }


	return status;
}
