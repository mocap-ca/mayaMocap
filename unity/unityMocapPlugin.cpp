#include "unityMocapPlugin.h"
#include "item.h"
#include "udpSocket.h"

/*
Gets mesh data from a named pipe and passes it to a unity script
*/

using namespace peel;

#include <vector>
#include <winsock2.h>

std::vector<Item*> items;

HANDLE hThread;
HANDLE hMutex;

bool running;

char threadInfo[255];

UdpServer *server;

int port;


DWORD WINAPI ThreadFunc(void* param)
{
	// Thread to read data from the vrmocapmesh pipe and threadsafe save a local copy

	DWORD ret, bytes;


	while (running)
	{
		if (port == 0)
		{
			strcpy(threadInfo, "Not Connected");
			if (server->isConnected()) server->close();
			Sleep(500);
			continue;
		}

		if (!server->isConnected())
		{
			if (!server->bind(port))
			{
				strcpy(threadInfo, "Bind Error");
				Sleep(500);
				continue;
			}
			else
			{
				strcpy(threadInfo, "Connected");
			}
		}

		bytes = server->receive();
		if (bytes == -1) break;
		if (bytes == 0) continue;

		ret = WaitForSingleObject(hMutex, INFINITE);
		if (ret == WAIT_OBJECT_0)
		{
			parseItems(server->readBuffer, bytes, &items);
		}
		ReleaseMutex(hMutex);
	}

	return 0;
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{

	WSADATA wsaData;
	DWORD wVersionRequested = MAKEWORD(2, 2);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		WSAStartup(wVersionRequested, &wsaData);
		server        = new UdpServer();
		running       = true;
		hMutex        = CreateMutex(NULL, FALSE, FALSE);
		threadInfo[0] = 0;
		hThread       = CreateThread(NULL, 0, &ThreadFunc, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH:
		if (hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (server->isConnected()) server->close();	
		delete server;
		hThread = INVALID_HANDLE_VALUE;
		WSACleanup();
		break;
	}

	return TRUE;
}


extern "C" {

	void mocapBind(int p) { port = p; }
	bool mocapBound() { return server->isConnected(); }

	int markers()
	{
		DWORD res = WaitForSingleObject(hMutex, INFINITE);
		if (res != WAIT_OBJECT_0) return 0;

		int c = 0;
		for (std::vector<peel::Item*>::iterator i = items.begin(); i != items.end(); i++)
		{
			if (dynamic_cast<peel::Marker*>(*i) != NULL) c++;
		}
		ReleaseMutex(hMutex);

		return c;
	}

	int segments()
	{
		DWORD res = WaitForSingleObject(hMutex, INFINITE);
		if (res != WAIT_OBJECT_0) return 0;

		int c = 0;
		for (std::vector<peel::Item*>::iterator i = items.begin(); i != items.end(); i++)
		{
			if (dynamic_cast<peel::Segment*>(*i) != NULL) c++;
		}
		ReleaseMutex(hMutex);

		return c;
	}

	char* ThreadInfo()
	{
		return threadInfo;
	}
}
