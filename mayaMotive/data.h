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

#ifndef DATA_H
#define DATA_H

#define ID_RIGIDBODY 19
#define ID_MARKER    20
#define ID_SEGMENT   21
#define ID_MESSAGE   22

#include <vector>

class MotiveItem
{
public:
	virtual size_t serialize(char*, size_t) = 0;
	virtual size_t parse(char*, size_t) = 0;
	
};

class MotiveRigidbody : public MotiveItem
{
public:
	MotiveRigidbody() {};
	MotiveRigidbody(int id, float tx, float ty, float tz, float rx, float ry, float rz);
	size_t serialize(char *, size_t);
	size_t parse(char *, size_t);
	int id;
	float tx, ty, tz, rx, ry, rz;
};


class MotiveSegment : public MotiveItem
{
public:
	MotiveSegment() {};
	MotiveSegment(int id, float tx, float ty, float tz, float rx, float ry, float rz);
	size_t serialize(char *, size_t);
	size_t parse(char *, size_t);
	int id;
	float tx, ty, tz, rx, ry, rz;
};


class MotiveMarker : public MotiveItem
{
public:
	MotiveMarker() {};
	MotiveMarker(int id, float x, float y, float z);
	size_t serialize(char *, size_t);
	size_t parse(char *, size_t);
	int id;
	float x, y, z;
};

class MotiveMessage : public MotiveItem
{
public:
	MotiveMessage() { message[0] = 0; }
	MotiveMessage(char *msg);
	size_t serialize(char *, size_t);
	size_t parse(char *, size_t);
	char message[255];
};

size_t serializeItems(std::vector<MotiveItem*> &items, char *buffer, size_t buflen);
bool parseItems(std::vector<MotiveItem*> &items, char *buffer, size_t buflen);

#endif