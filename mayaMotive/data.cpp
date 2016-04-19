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

#include "data.h"

#include <vector>


MotiveRigidbody::MotiveRigidbody(int id_, float tx_, float ty_, float tz_, float rx_, float ry_, float rz_)
	: id(id_)
	, tx(tx_), ty(ty_), tz(tz_)
	, rx(rx_), ry(ry_), rz(rz_)
{}

size_t MotiveRigidbody::serialize(char *buf, size_t buflen)
{
	char *ptr = buf;

	if (1 + sizeof(int) + sizeof(float) * 6 > buflen)
	{
		// Overflow
		return 0;
	}

	// Header
	*(ptr++) = ID_RIGIDBODY;

	// Segment Id
	int *id_ptr = (int*)ptr;
	*id_ptr = id;
	ptr += sizeof(id);

	// Position
	memcpy(ptr, &tx, sizeof(float) * 6);
	ptr += sizeof(float) * 6;

	return ptr - buf;

}

size_t MotiveRigidbody::parse(char *buf, size_t buflen)
{
	if (buflen == 0) return 0;
	if (buf[0] != ID_RIGIDBODY) return 0;
	if (buflen < 1 + sizeof(int) + sizeof(float) * 6) return 0;

	char *ptr = buf + 1;
	int *idptr = (int*)ptr;
	id = *idptr;
	ptr += sizeof(int);

	memcpy(&tx, ptr, sizeof(float) * 6);

	return  1 + sizeof(int) + sizeof(float) * 6;
}




MotiveSegment::MotiveSegment(int id_, float tx_, float ty_, float tz_, float rx_, float ry_, float rz_)
	: id(id_)
	, tx(tx_), ty(ty_), tz(tz_)
	, rx(rx_), ry(ry_), rz(rz_)
{}


size_t MotiveSegment::serialize(char *buf, size_t buflen)
{
	char *ptr = buf;

	if ( 1 + sizeof(int) + sizeof(float) * 6 > buflen)
	{
		// Overflow
		return 0;
	}

	// Header
	*(ptr++) = ID_SEGMENT;

	// Segment Id
	int *id_ptr = (int*)ptr;
	*id_ptr = id;
	ptr += sizeof(id);

	// Position
	memcpy(ptr, &tx, sizeof(float) * 6);
	ptr += sizeof(float) * 6;

	return ptr - buf;

}

size_t MotiveSegment::parse(char *buf, size_t buflen)
{
	if (buflen == 0) return 0;
	if (buf[0] != ID_SEGMENT) return 0;
	if (buflen < 1 + sizeof(int) + sizeof(float) * 6) return 0;

	char *ptr = buf + 1;
	int *idptr = (int*)ptr;
	id = *idptr;
	ptr += sizeof(int);

	memcpy(&tx, ptr, sizeof(float)*6);

	return  1 + sizeof(int) + sizeof(float) * 6;
}


MotiveMarker::MotiveMarker(int id_, float x_, float y_, float z_)
	: id(id_)
	, x(x_), y(y_), z(z_)
{}




size_t MotiveMarker::serialize(char *buf, size_t buflen)
{
	char *ptr = buf;

	if (1 + sizeof(int) + sizeof(float) * 3 > buflen)
	{
		// Overflow
		return 0;
	}

	// Header
	*(ptr++) = ID_MARKER;

	// Segment Id
	int *id_ptr = (int*)ptr;
	*id_ptr = id;
	ptr += sizeof(id);

	// Position
	memcpy(ptr, &x, sizeof(float) * 3);
	ptr += sizeof(float) * 3;

	return ptr - buf;
}

size_t MotiveMarker::parse(char *buf, size_t buflen)
{
	if (buflen == 0) return 0;
	if (buf[0] != ID_MARKER) return 0;
	if (buflen < 1 + sizeof(int) + sizeof(float) * 3) return 0;

	char *ptr = buf + 1;
	int *idptr = (int*)ptr;
	id = *idptr;
	ptr += sizeof(int);

	memcpy(&x, ptr, sizeof(float) * 3);

	return 1 + sizeof(int) + sizeof(float) * 3;
}


MotiveMessage::MotiveMessage(char *m)
{
	strcpy_s(message, 255, m);
}

size_t MotiveMessage::serialize(char *buf, size_t len)
{
	if (strlen(message) + 2 > len)
	{
		// Overflow
		return 0;
	}

	char *ptr = buf;
	*(ptr++) = ID_MESSAGE;
	strcpy_s(ptr, len-2, message);
	ptr += strlen(message) + 1;  // +1 end of string term

	return ptr - buf;

}

size_t MotiveMessage::parse(char *buf, size_t buflen)
{
	if (buflen == 0) return 0;
	if (buf[0] != ID_MESSAGE) return 0;

	char *ptr = buf + 1;
	size_t len = strlen(ptr);
	if (len >= buflen) return 0;
	if (len >= 255) return 0;

	strcpy_s(message, 255, ptr);

	return len + 2; // id + end of string term
}


size_t serializeItems(std::vector<MotiveItem*> &items, char *buffer, size_t buflen)
{
	char *ptr = buffer;

	*(ptr++) = 11;
	*(ptr++) = (unsigned char)items.size();

	size_t ret;
	size_t pos = 2;
	

	for (size_t i = 0; i < items.size(); i++)
	{

		MotiveItem *item = items[i];
		ret = item->serialize(buffer + pos, buflen - pos);
		pos += ret;	
	}

	return pos;
}

bool parseItems(std::vector<MotiveItem*> &items, char *buffer, size_t buflen)
{
	if (buflen < 3) return 0;
	if (buffer[0] != 11) return 0;

	unsigned char count = buffer[1];
	if (count == 0) return 0;

	char *ptr = buffer + 2;

	for (std::vector<MotiveItem*>::iterator i = items.begin(); i != items.end(); i++)
		delete (*i);

	items.clear();

	for (size_t i = 0; i < count && (ptr - buffer) < buflen; i++)
	{
		MotiveItem *item = NULL;
		if (ptr[0] == ID_MARKER)    item = new MotiveMarker();
		if (ptr[0] == ID_SEGMENT)   item = new MotiveSegment();
		if (ptr[0] == ID_RIGIDBODY) item = new MotiveRigidbody();
		if (ptr[0] == ID_MESSAGE)   item = new MotiveMessage();

		if (item == 0) return false;

		ptr += item->parse(ptr, buffer + buflen - ptr);
		items.push_back(item);
	}
		
	return true;

}