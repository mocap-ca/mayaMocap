#include "unityPlugin.h"
#include "item.h"

#include <vector>
#include <Windows.h>

std::vector<Item> items;

HANDLE hPipe;

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:  
		hPipe = INVALID_HANDLE_VALUE; 
		break;

	case DLL_PROCESS_DETACH:
		if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
		break;
	}

	return TRUE;
}


extern "C" {
	void AddPosition(char *name, float tx, float ty, float tz, float rx, float ry, float rz, float rw)
	{
		items.push_back(Item(name, tx, ty, tz, rx, ry, rz, rw));
	}

	bool Send()
	{
		char buffer[1024];


		if (hPipe == INVALID_HANDLE_VALUE)
		{
			hPipe = CreateFile("\\\\.\\pipe\\vrmocap", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

			if (hPipe == INVALID_HANDLE_VALUE) return false;
		}

		DWORD datalen = serializeItems(items, buffer, 1024);
		DWORD writelen;

		return WriteFile(hPipe, buffer, datalen, &writelen, NULL) != 0;

	}


	void  Clear()
	{
		items.clear();
	}

	void Close()
	{
		if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}