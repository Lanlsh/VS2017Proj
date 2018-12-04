#include "pch.h"
#include "PipeServerByOL.h"
#include <assert.h>

const LPCSTR lpszPipename = TEXT("\\\\.\\pipe\\ACpipe");

PipeServerByOL::PipeServerByOL()
{
	bool bIsInitialOk = Initialize();
	if (!bIsInitialOk)
	{
		printf("初始化失败！！！\n");
		assert(bIsInitialOk);
	}
}


PipeServerByOL::~PipeServerByOL()
{
	for (int i=0; i<INSTANCES; i++)
	{
		if (m_hEvents[i])
		{
			CloseHandle(m_hEvents[i]);
		}

		if (m_pipe[i].hPipeInst)
		{
			CloseHandle(m_pipe[i].hPipeInst);
		}
	}
}

void PipeServerByOL::StartPipeServer()
{
	DWORD i, dwWait, cbRet, dwErr;
	BOOL fSuccess;
	i = 0;

	while (1)
	{
		// Wait for the event object to be signaled, indicating 
		// completion of an overlapped read, write, or 
		// connect operation. 
		//dwWait = WaitForMultipleObjects(
		//	INSTANCES,    // number of event objects 
		//	m_hEvents,      // array of event objects 
		//	FALSE,        // does not wait for all 
		//	INFINITE);    // waits indefinitely 
		//暂时只需要一个管道进行通讯，为了便于理解和排查问题，使用此函数！！
		dwWait = WaitForSingleObject(		
			m_hEvents[i],      // array of event objects 
			PIPE_TIMEOUT); //INFINITE   // waits indefinitely 

		//是否立即停止循环
		if (m_bStopLoop)
		{
			return;
		}
	 // // dwWait shows which pipe completed the operation. 

		//i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
		//if (i < 0 || i >(INSTANCES - 1))
		//{
		//	printf("Index out of range.\n");
		//	return ;
		//}
		switch (dwWait)
		{
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			m_bIsTimeOut = true;
			printf("TIMEOUT!!!!!!!!!!!!!!!!!!\n");	//说明客户端没有返回信息了
			return;
			//continue;
			break;
		//case WAIT_FAILED:
		//	printf("WAIT_FAILED!!!!!!!!!!!!!!!!!!\n");
		//	break;
		}

		// Get the result if the operation was pending. 

		if (m_pipe[i].fPendingIO)
		{
			fSuccess = GetOverlappedResult(
				m_pipe[i].hPipeInst, // handle to pipe 
				&m_pipe[i].oOverlap, // OVERLAPPED structure 
				&cbRet,            // bytes transferred 
				FALSE);            // do not wait 

			switch (m_pipe[i].dwState)
			{
				// Pending connect operation 
			case PIPE_CONNECTING_STATE:
				if (!fSuccess)
				{
					printf("Error %d.\n", GetLastError());
					return;
				}
				m_pipe[i].dwState = PIPE_WRITING_STATE;// PIPE_READING_STATE;
				break;

				// Pending read operation 
			case PIPE_READING_STATE:
				if (!fSuccess || cbRet == 0)
				{
					DisconnectAndReconnect(i);
					continue;
				}
				m_pipe[i].cbRead = cbRet;
				m_pipe[i].dwState = PIPE_WRITING_STATE;
				break;

				// Pending write operation 
			case PIPE_WRITING_STATE:
				if (!fSuccess || cbRet != m_pipe[i].cbToWrite)
				{
					DisconnectAndReconnect(i);
					continue;
				}
				m_pipe[i].dwState = PIPE_READING_STATE;
				break;

			default:
			{
				printf("Invalid pipe state.\n");
				return;
			}
			}
		}

		// The pipe state determines which operation to do next. 

		switch (m_pipe[i].dwState)
		{
			// PIPE_READING_STATE: 
			// The pipe instance is connected to the client 
			// and is ready to read a request from the client. 

		case PIPE_READING_STATE:
			fSuccess = ReadFile(
				m_pipe[i].hPipeInst,
				m_pipe[i].chRequest,
				BUFSIZE * sizeof(TCHAR),
				&m_pipe[i].cbRead,
				&m_pipe[i].oOverlap);

			// The read operation completed successfully. 

			if (fSuccess && m_pipe[i].cbRead != 0)
			{
				m_pipe[i].fPendingIO = FALSE;
				m_pipe[i].dwState = PIPE_WRITING_STATE;
				continue;
			}

			// The read operation is still pending. 

			dwErr = GetLastError();
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				m_pipe[i].fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i);
			break;

			// PIPE_WRITING_STATE: 
			// The request was successfully read from the client. 
			// Get the reply data and write it to the client. 

		case PIPE_WRITING_STATE:
			if (m_pipe[i].fPendingIO == FALSE)
			{
				Sleep(PIPE_TIMEOUT);	//每隔PIPE_TIMEOUT发送一条消息
			}

			GetAnswerToRequest(&m_pipe[i]);
			OVERLAPPED oOverlap = m_pipe[i].oOverlap;
			//oOverlap.Offset = 0;
			fSuccess = WriteFile(
				m_pipe[i].hPipeInst,
				m_pipe[i].chReply,
				m_pipe[i].cbToWrite,
				&cbRet,
				&oOverlap);

			// The write operation completed successfully. 

			if (fSuccess && cbRet == m_pipe[i].cbToWrite)
			{
				m_pipe[i].fPendingIO = FALSE;
				m_pipe[i].dwState = PIPE_READING_STATE;
				continue;
			}

			// The write operation is still pending. 

			dwErr = GetLastError();
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				m_pipe[i].fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i);
			break;

		default:
		{
			printf("Invalid pipe state.\n");
			return ;
		}
		}
	}
}

void PipeServerByOL::StopServer()
{
	m_bStopLoop = true;
	m_bIsTimeOut = false;
	if (INSTANCES > 0)
	{
		SetEvent(m_hEvents[0]);
	}
	
}

// DisconnectAndReconnect(DWORD) 
// This function is called when an error occurs or when the client 
// closes its handle to the pipe. Disconnect from this client, then 
// call ConnectNamedPipe to wait for another client to connect. 

bool PipeServerByOL::Initialize()
{
	// The initial loop creates several instances of a named pipe 
	// along with an event object for each instance.  An 
	// overlapped ConnectNamedPipe operation is started for 
	// each instance. 
	m_bStopLoop = false;
	for (int i = 0; i < INSTANCES; i++)
	{

		// Create an event object for this instance. 
		memset(&m_pipe[i], 0, sizeof(PIPEINST));
		m_hEvents[i] = CreateEvent(
			NULL,    // default security attribute 
			TRUE,    // manual-reset event 
			TRUE,    // initial state = signaled 
			NULL);   // unnamed event object 

		if (m_hEvents[i] == NULL)
		{
			printf("CreateEvent failed with %d.\n", GetLastError());
			return false;
		}

		m_pipe[i].oOverlap.hEvent = m_hEvents[i];

		m_pipe[i].hPipeInst = CreateNamedPipe(
			lpszPipename,            // pipe name 
			PIPE_ACCESS_DUPLEX |     // read/write access 
			FILE_FLAG_OVERLAPPED,    // overlapped mode 
			PIPE_TYPE_MESSAGE |      // message-type pipe 
			PIPE_READMODE_MESSAGE |  // message-read mode 
			PIPE_WAIT,               // blocking mode 
			INSTANCES,               // number of instances 
			BUFSIZE * sizeof(TCHAR),   // output buffer size 
			BUFSIZE * sizeof(TCHAR),   // input buffer size 
			PIPE_TIMEOUT,            // client time-out 
			NULL);                   // default security attributes 

		if (m_pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe failed with %d.\n", GetLastError());
			return false;
		}

		// Call the subroutine to connect to the new client

		m_pipe[i].fPendingIO = ConnectToNewClient(
			m_pipe[i].hPipeInst,
			&m_pipe[i].oOverlap);

		m_pipe[i].dwState = m_pipe[i].fPendingIO ?
			PIPE_CONNECTING_STATE : // still connecting 
			PIPE_READING_STATE;     // ready to read 
	}

	return true;
}

VOID PipeServerByOL::DisconnectAndReconnect(DWORD i)
{
	// Disconnect the pipe instance. 

	if (!DisconnectNamedPipe(m_pipe[i].hPipeInst))
	{
		printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Call a subroutine to connect to the new client. 

	m_pipe[i].fPendingIO = ConnectToNewClient(
		m_pipe[i].hPipeInst,
		&m_pipe[i].oOverlap);

	m_pipe[i].dwState = m_pipe[i].fPendingIO ?
		PIPE_CONNECTING_STATE : // still connecting 
		PIPE_READING_STATE;     // ready to read 
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED) 
// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 

BOOL PipeServerByOL::ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo);

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected)
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError())
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
	}

	return fPendingIO;
}

VOID PipeServerByOL::GetAnswerToRequest(LPPIPEINST pipe)
{
	memset(pipe->chReply, 0, BUFSIZE);
	_tprintf(TEXT("[%d] %s\n"), (int)pipe->hPipeInst, pipe->chRequest);
	StringCchCopy(pipe->chReply, BUFSIZE, TEXT("Default answer from server"));
	pipe->cbToWrite = (lstrlen(pipe->chReply) + 1) * sizeof(TCHAR);
}