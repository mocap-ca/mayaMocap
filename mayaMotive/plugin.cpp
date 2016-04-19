/*

Copyright (c) 2016 Alastair Macleod, Sawmill Studios Inc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>
#include <maya/MPxThreadedDeviceNode.h>
#include <maya/MGlobal.h>

#include "mayaMotive.h"


MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, "MocapCA", "1.0", "Any");

	status = plugin.registerNode("peelMotive", MayaMotive::id, MayaMotive::creator, MayaMotive::initialize, MPxNode::kThreadedDeviceNode);
	if (!status) { status.perror("failed to registerNode peelMotive"); }

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(MayaMotive::id);
	if (!status) { status.perror("failed to deregisterNode peelMotive"); }

	return status;
}
