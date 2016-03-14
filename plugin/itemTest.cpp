#include "item.h"

#include <vector>

int main(int argc, char *argv[])
{

	std::vector<Item> items;
	std::vector<Item> brundle;

	items.push_back(Item("test1", 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f));
	items.push_back(Item("test2", 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f));
	

	char buf[1024];

	size_t ret = serializeItems(items, buf, 1024);

	printf("Serialize returns: %d\n", ret);

	ret =parseItems(buf, ret, brundle);

	printf("Parse returns: %d\n", ret);


	for (std::vector<Item>::iterator i = brundle.begin(); i != brundle.end(); i++)
	{
		Item &it = *i;
		printf("Item: %s %f %f %f   %f %f %f %f\n", it.name, it.tx, it.ty, it.tz, it.rx, it.ry, it.rz, it.rw);

	}
}