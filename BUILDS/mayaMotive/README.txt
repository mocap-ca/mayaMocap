Maya Motive Builds
Alastair Macleod - The Sawmill Studios Inc
4/18/2016

Built using Visual Studio 2015 and NatNetSdk 2.9.0

Windows Only.


*** INSTALLATION *** 

Copy the .mll file to a maya plugin location.

These may need to be installed on your system:

https://www.optitrack.com/downloads/developer-tools.html#natnet-sdk
https://www.microsoft.com/en-us/download/details.aspx?id=48145

If you get an error like "Specified component cannot be found" or similar,
it check that you the 64bit version of NatNetLib.dll in your path
(e.g. copy it to c:\windows) and the visual studio 2015 runtime
redistributable installed.

Please send support requests to this mailing list:

https://groups.google.com/d/forum/sawmillmocap


*** QUICKSTART *** 

1. Install the correct plugin

2. Copy/Clone the peelRealtime scripts from here:

      https://github.com/mocap-ca/mayaMocap/raw/master/BUILDS/mayaMotive/peelRealtimePython.zip

3. Run the following python code:

from peelRealtime import mocapNode, realtime
import maya.cmds as m
x = mocapNode.MotiveNode('Motive')
m.select(x.node) 

4. Adjust the settings on the Motive node as needed.

5. Check "live" on. 

6. Stream some data from motive (if nothing is moving in motive, nothing will happen in maya).

7. Ensure Motive is streaming FBX named bones.  Unicast may be more stable.

8. Run this python code in the same editor:

x.connectNodes()

9. Edit this line and run it:

realtime.create( 'MarkerPrefix_', 'SkeletonNamespace', 'HIK-Character name')

MarkerPrefix_ = the name of the actor (underscore after is important)
SkeletonNamespace = is the namespace the skeleton will be put in to.
HIK-Character = is the name of the HIK character that will be created.



*** SOURCE CODE ***

https://github.com/mocap-ca/mayaMocap/tree/master/mayaMotive


*** LICENSE *** 

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




