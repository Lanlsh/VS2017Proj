#pragma once
#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#define INSTANCES 1			//pipe实例数量
#define PIPE_TIMEOUT 10000	//pipe超时时间
#define BUFSIZE 512			//pipe通讯字节数

enum EM_PIPE_STATE
{
	PIPE_CONNECTING_STATE,
	PIPE_READING_STATE,
	PIPE_WRITING_STATE
};

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

class PipeServerByOL
{
public:
	PipeServerByOL();
	~PipeServerByOL();

	void StartPipeServer();
	void StopServer();
	bool GetIsTimeOut() { return m_bIsTimeOut; }

protected:
	bool Initialize();

	VOID DisconnectAndReconnect(DWORD);
	BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
	VOID GetAnswerToRequest(LPPIPEINST);

private:
	PIPEINST m_pipe[INSTANCES];
	HANDLE m_hEvents[INSTANCES];
	bool m_bStopLoop;
	bool m_bIsTimeOut;	//	是否超时
};

