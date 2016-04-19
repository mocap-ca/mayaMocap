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
#include <stdio.h>
#include <vector>

int main(int argc, char *argv[])
{

	std::vector<MotiveItem*> items;

	MotiveMarker *m1 = new MotiveMarker(0, 2.0f, 3.5f, 4.5f);
	MotiveMarker *m2 = new MotiveMarker(1, 3.0f, 14.1f, -14.0f);

	MotiveSegment *s1 = new MotiveSegment(10, 10.1f, 11.2f, 12.3f, 0.1f, 0.2, 0.3f);
	MotiveSegment *s2 = new MotiveSegment(11, 22.1f, 33.2f, 44.5f, 1.1f, 10.2, 20.3f);

	MotiveRigidbody *r1 = new MotiveRigidbody(12, 1.2f, 2.3f, 4.5f, 66.6f, 77.7f, 88.8f);
	MotiveRigidbody *r2 = new MotiveRigidbody(13, 11.2f, 12.3f, 14.5f, 166.6f, 277.7f, 388.8f);

	MotiveMessage *x1 = new MotiveMessage("TEST");
	MotiveMessage *x2 = new MotiveMessage("Hello there lovely day isn't it.");


	items.push_back(m1);
	items.push_back(m2);
	items.push_back(s1);
	items.push_back(s2);
	items.push_back(r1);
	items.push_back(r2);
	items.push_back(x1);
	items.push_back(x2);

	char buf[1024];
	size_t ret = serializeItems(items, buf, 1024);

	printf("Serial Data: %d\n", ret);

	std::vector<MotiveItem*> copied;
	parseItems(copied, buf, ret);


	for (std::vector<MotiveItem*>::iterator i = copied.begin(); i != copied.end(); i++)
	{
		MotiveSegment* segment = dynamic_cast<MotiveSegment*>(*i);
		if (segment != NULL)
		{
			printf("Segment: %d\n", segment->id);
			printf("   %f", segment->tx);
			printf("   %f", segment->ty);
			printf("   %f\n", segment->tz);
			printf("   %f", segment->rx);
			printf("   %f", segment->ry);
			printf("   %f\n", segment->rz);
		}

		MotiveRigidbody* rb = dynamic_cast<MotiveRigidbody*>(*i);
		if (rb != NULL)
		{
			printf("Rigidbody: %d\n", rb->id);
			printf("   %f", rb->tx);
			printf("   %f", rb->ty);
			printf("   %f\n", rb->tz);
			printf("   %f", rb->rx);
			printf("   %f", rb->ry);
			printf("   %f\n", rb->rz);
		}

		MotiveMarker *marker = dynamic_cast<MotiveMarker*>(*i);
		if (marker != NULL)
		{
			printf("Marker: %d\n", marker->id);
			printf("   %f", marker->x);
			printf("   %f", marker->y);
			printf("   %f\n", marker->z);
		}

		MotiveMessage *message = dynamic_cast<MotiveMessage*>(*i);
		if (message != NULL)
		{
			printf("Message:  %s\n", message->message);
		}

		delete (*i);
	}







}