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


*** INSTRUCTIONS *** 

1. Load the plugin.
2. Run this python code:

import maya.cmds as m
x = m.createNode('peelMotive')
m.setAttr( x + ".motiveIp", "127.0.0.1", typ='string')

   - this will create a node that should recieve the streaming data.

3. Check all the settings are correct and change as needed
4. Click the "Live" checkbox on.

If you change any settings click live off and on again to reconnect.


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




