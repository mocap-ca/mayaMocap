#include "listenCommand.h"

#include <maya/MGlobal.h>
#include <maya/MString.h>

ListenThread listener;

MSyntax ListenCommand::newSyntax() 
{
    MSyntax s;
    s.addFlag(portFlag,  portFlagLong , MSyntax::kUnsigned );
    s.addFlag(closeFlag, closeFlagLong );
    return s;

}


MStatus ListenCommand::doIt( const MArgList &args )
{
    MStatus status;

    MArgDatabase db(syntax(), args);

    port = 0;
    create = false;
    close = false;

    if( db.isFlagSet(portFlag) )
    {
        status = db.getFlagArgument( portFlag, 0, port );
        if( status ) { create=true; }
    }

    if( db.isFlagSet(closeFlag) )
    {
        close = true;
    }

    return this->redoIt();
}


MStatus ListenCommand::redoIt()
{
    if( create && close )
    {
        MGlobal::displayError("Create or close - not both");
        return MS::kFailure;
    }

    if(create && port > 1024)
    {
        if( listener.start(port) == 0 )
        {
            MGlobal::displayInfo("started");
        }
        else
        {
            MGlobal::displayError("could not start");
        }
    }

    if(close)
    {
        MGlobal::displayInfo("stopping");
        listener.stop();
    }
    return MS::kSuccess;
}
