#include "unityPlugin.h"
#include "item.h"

using namespace peel;

#include <vector>
#include <Windows.h>

std::vector<Item*> items;

HANDLE hPipePos;
HANDLE hThread;
HANDLE hMutex;

char *points;
char *verts;
int pointsLen;
int vertsLen;

bool running;

char threadInfo[255];
char pointInfo[255];


DWORD WINAPI ThreadFunc(void* param)
{
	// Thread to read data from the vrmocapmesh pipe and threadsafe save a local copy

	DWORD ret;

	HANDLE hPipe = INVALID_HANDLE_VALUE;

	strcpy(threadInfo, "Init");
	
	while(running)
	{
		if (hPipe == INVALID_HANDLE_VALUE)
		{
			hPipe = CreateFile("\\\\.\\pipe\\vrmocapmesh", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (hPipe == INVALID_HANDLE_VALUE)
			{
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, threadInfo, 255 * sizeof(TCHAR), 0);
				printf("Error reading from to pipe: %s", threadInfo);
				Sleep(2000);
				continue;
			}
		}


		DWORD len;
		int header[3];
		if (!ReadFile(hPipe, (void*)header, sizeof(int) * 3, &len, NULL))
		{
			strcpy(threadInfo, "Error Reading from pipe");
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
			continue;
		}


		if(header[0] != 0x3223)
		{
			strcpy(threadInfo, "Invalid data");
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
			continue;
		}

		if (points == NULL)
		{
			points = (char*)malloc(header[1]);
		}
		else
		{
			if (header[0] != pointsLen) points = (char*) realloc(points, pointsLen);
		}

		if (verts == NULL)
		{
			verts = (char*)malloc(header[2]);
		}
		else
		{
			if (header[1] != vertsLen) verts = (char*)realloc(verts, vertsLen);
		}

		pointsLen = header[1];
		vertsLen = header[2];

		ret = WaitForSingleObject(hMutex, INFINITE);
		if (ret == WAIT_OBJECT_0)
		{
			ret = ReadFile(hPipe, (void*)points, pointsLen, &len, FALSE);
			ret = ReadFile(hPipe, (void*)verts, vertsLen, &len, FALSE);
		}
		ReleaseMutex(hMutex);

		strcpy(threadInfo, "Ok");

	}

	return 0;
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:  
		hPipePos  = INVALID_HANDLE_VALUE; 

		running = true;
		points = NULL;
		verts = NULL;
		hMutex = CreateMutex(NULL, FALSE, FALSE);

		threadInfo[0] = 0;
		pointInfo[0] = 0;

		hThread = CreateThread(NULL, 0, &ThreadFunc, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH:
		if (hPipePos != INVALID_HANDLE_VALUE) CloseHandle(hPipePos);
		if (hThread   != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		hPipePos  = INVALID_HANDLE_VALUE;
		hThread   = INVALID_HANDLE_VALUE;
		break;
	}

	return TRUE;
}


extern "C" {

	// Get get the mesh buffer provided by the thread
	int pointSize() { return pointsLen;  }
	int vertsSize() { return vertsLen;  }

	int getData(float *points_, int *verts_)
	{
		DWORD res = WaitForSingleObject(hMutex, INFINITE);
		if (res != WAIT_OBJECT_0) return 0;

		memcpy(points_, points, pointsLen);
		memcpy(verts_, verts, vertsLen);
		ReleaseMutex(hMutex);
	}

	
	// Add some data to be sent 

	void AddPosition(char *name, float tx, float ty, float tz, float rx, float ry, float rz, float rw)
	{
		items.push_back(new Segment(name, tx, ty, tz, rx, ry, rz, rw));
	}

	bool Send()
	{
		char buffer[1024];


		if (hPipePos == INVALID_HANDLE_VALUE)
		{
			// Create the pipe if it does not exist
			hPipePos = CreateFile("\\\\.\\pipe\\vrmocap", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

			if (hPipePos == INVALID_HANDLE_VALUE)
			{
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, pointInfo, 255 * sizeof(TCHAR), 0);
				return false;
			}
		}

		DWORD datalen = serializeItems(items, buffer, 1024);
		DWORD writelen;

		return WriteFile(hPipePos, buffer, datalen, &writelen, NULL) != 0;

	}


	void  Clear()
	{
		for (std::vector<Item*>::iterator i = items.begin(); i != items.end(); i++)
			delete (*i);

		items.clear();
	}

	void Close()
	{
		if (hPipePos != INVALID_HANDLE_VALUE) CloseHandle(hPipePos);
		hPipePos = INVALID_HANDLE_VALUE;
	}

	char* ThreadInfo()
	{
		return threadInfo;
	}

	char* PointInfo()
	{
		return pointInfo;
	}
}
