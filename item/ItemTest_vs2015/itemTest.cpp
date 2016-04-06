#include "item.h"

#include <vector>

int main(int argc, char *argv[])
{

	std::vector<peel::Item*> items;
	std::vector<peel::Item*> brundle;

	items.push_back(new peel::Segment("test1", 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f));
	items.push_back(new peel::Segment("test2", 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f));
	items.push_back(new peel::Marker("m1", 100.0, 200.2, 300.3));

	char buf[1024];

	size_t ret = serializeItems(items, buf, 1024);

	peel::dumpData(buf);

	printf("Serialize returns: %d\n", ret);

	ret =parseItems(buf, ret, &brundle);

	printf("Parse returns: %d\n", ret);

	peel::dumpItems(brundle);

}