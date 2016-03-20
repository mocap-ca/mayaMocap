#include "pipeDevice.h"

#include <Windows.h>
#include <tchar.h>

#define PIPENAME _T("\\\\.\\pipe\\vrmocap")
#define _CRT_SECURE_NO_WARNINGS

MTypeId PipeDevice::id(0x001126D2);

PipeDevice::PipeDevice()
	: hPipe(INVALID_HANDLE_VALUE)
{

}

bool PipeDevice::connect()
{
	if (hPipe != INVALID_HANDLE_VALUE) return true;

	hPipe = CreateNamedPipe(PIPENAME, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 1024, 50, NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		TCHAR buf[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buf, 1024 * sizeof(TCHAR), 0);
		sendData(buf);
		printf("Could not create pipe: %s", buf);
		return false;
	}
	return true;
}

bool PipeDevice::isConnected()
{
	return hPipe != INVALID_HANDLE_VALUE;
}

bool PipeDevice::disconnect()
{
	if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
	hPipe = INVALID_HANDLE_VALUE;
	return true;
}


size_t PipeDevice::receiveData(char *data, size_t buflen)
{
	// Overloaded method to get the data from the pipe.
	// Should send a message if return <= 0

	DWORD len = buflen;
	BOOL ret = ReadFile(hPipe, data, 1024, &len, NULL);
	if (ret) return len;

	DWORD err = GetLastError();

	if (err == ERROR_PIPE_LISTENING)
	{
		// Other end has not opened
		sendData("Listening");
		return 0;
	}

	if (err == ERROR_BROKEN_PIPE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
		sendData("No Connection");
		return 0;
	}

	TCHAR buf[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, buf, 1024 * sizeof(TCHAR), 0);
	printf("%s", buf);

	sendData(buf);

	return -1;


}

