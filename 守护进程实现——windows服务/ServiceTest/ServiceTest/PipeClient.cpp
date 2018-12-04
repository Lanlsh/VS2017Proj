#include "pch.h"
#include "PipeClient.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#define BUFSIZE 512

LPCSTR lpvMessage = TEXT("Default message from client.");
LPCSTR lpszPipename = TEXT("\\\\.\\pipe\\ACpipe");

PipeClient::PipeClient()
{
	m_bIsStopLoop = false;
}


PipeClient::~PipeClient()
{
}

void PipeClient::StartPipeClient()
{
	m_bIsStopLoop = false;

	HANDLE hPipe = NULL;
	TCHAR  chBuf[BUFSIZE];
	BOOL   fSuccess = FALSE;
	DWORD  cbRead, cbToWrite, cbWritten;

	// Try to open a named pipe; wait for it, if necessary. 

reStart:
	while (!m_bIsStopLoop)	//此处的循环变量可以设置一个合理的值
	{
		hPipe = CreateFile(
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

	  // Break if the pipe handle is valid. 

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			continue;
		}

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			return;
		}
	}

	// The pipe connected; change to message-read mode. 

	//dwMode = PIPE_READMODE_MESSAGE;
	//fSuccess = SetNamedPipeHandleState(
	//	hPipe,    // pipe handle 
	//	&dwMode,  // new pipe mode 
	//	NULL,     // don't set maximum bytes 
	//	NULL);    // don't set maximum time 
	//if (!fSuccess)
	//{
	//	_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
	//	return -1;
	//}

	// Send a message to the pipe server. 

	cbToWrite = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR);
	_tprintf(TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage);

	bool bIsSendOk = false;
	bool bIsRecvOk = false;
	while (!m_bIsStopLoop)	//此处的循环变量可以设置一个合理的值
	{
		memset(chBuf, 0, BUFSIZE);
		do
		{
			// Read from the pipe. 
			bIsRecvOk = ReadFile(
				hPipe,    // pipe handle 
				chBuf,    // buffer to receive reply 
				BUFSIZE * sizeof(TCHAR),  // size of buffer 
				&cbRead,  // number of bytes read 
				NULL);    // not overlapped 

			if (!bIsRecvOk && GetLastError() != ERROR_MORE_DATA)
			{
				break;
			}


			_tprintf(TEXT("\"%s\"\n"), chBuf);
		} while (!bIsRecvOk);  // repeat loop if ERROR_MORE_DATA 

		if (!bIsRecvOk)
		{
			_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
			goto reStart;
			continue;
			//return -1;
		}

		bIsSendOk = WriteFile(
			hPipe,                  // pipe handle 
			lpvMessage,             // message 
			cbToWrite,              // message length 
			&cbWritten,             // bytes written 
			NULL);                  // not overlapped 

		if (!bIsSendOk)
		{
			_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
			goto reStart;
			continue;
			//return -1;
		}


	}

	printf("\n<End of message, press ENTER to terminate connection and exit>");
	//_getch();

	if (hPipe)
	{
		CloseHandle(hPipe);
	}
	
}
