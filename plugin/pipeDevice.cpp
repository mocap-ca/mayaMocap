#include "pipeDevice.h"

#include <Windows.h>

#define PIPENAME "\\\\.\\pipe\\vrmocap"
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
		printf("Could not create pipe\n");
		return false;
	}
	return true;

}

bool PipeDevice::disconnect()
{
	if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
	hPipe = INVALID_HANDLE_VALUE;
	return true;
}


size_t PipeDevice::receiveData(char *data, size_t buflen)
{
	DWORD len = buflen;
	BOOL ret = ReadFile(hPipe, data, 1024, &len, NULL);
	if (ret) return len;

	DWORD err = GetLastError();

	if (err == ERROR_PIPE_LISTENING)
	{
		// Other end has not opened 
		return 0;
	}

	if (err == ERROR_BROKEN_PIPE)
	{
		hPipe = INVALID_HANDLE_VALUE;
		return 0;
	}

	TCHAR buf[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, buf, 1024 * sizeof(TCHAR), 0);
	printf("%s", buf);

	return -1;


}

