#ifndef LISTEN_COMMAND_H
#define LISTEN_COMMAND_H


#include <maya/MPxCommand.h>
#include "tcpListen.h"
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

static const char* portFlag      = "-p";
static const char* portFlagLong  = "-port";
static const char* closeFlag     = "-c";
static const char* closeFlagLong = "-close";
static const char* moduleFlag    = "-m";
static const char* moduleFlagLong = "-module";


class ListenCommand : public MPxCommand
{
public:
    ListenCommand() {}
    virtual ~ListenCommand() {}

    MStatus doIt( const MArgList &);
    MStatus redoIt();
    bool isUndoable() const { return false; }
    bool hasSyntax() const { return true; }
    static MSyntax newSyntax();

    static void* creator() { return new ListenCommand; }

private :
    int port;
    bool create;
    bool close;
	MString module;
};


#endif // LISTEN_COMMAND_H
