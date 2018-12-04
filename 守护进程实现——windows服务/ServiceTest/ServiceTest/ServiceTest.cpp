// ServiceTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include "PipeClient.h"

#pragma comment(lib, "Advapi32")
#define SLEEP_TIME 2000
#define LOGFILE "C://Users//Public//Documents//ACDicomService_Log.txt"
#define SERVICENAME TEXT("MemoryStatus")

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int InitService();
int WriteToLog(char* str);

PipeClient pipeClient;

int main(int argc, char* argv[])
{
	SERVICE_TABLE_ENTRY ServiceTable[2];
	
	ServiceTable[0].lpServiceName = (LPSTR)SERVICENAME;
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	//分派表的最后一项必须是服务名和服务主函数域的 NULL 指针
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	StartServiceCtrlDispatcher(ServiceTable);

	//ServiceMain(argc, argv);

	return 0;
}


void ServiceMain(int argc, char** argv)
{
	int error;


	ServiceStatus.dwServiceType =
		SERVICE_WIN32;
	ServiceStatus.dwCurrentState =
		SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted =
		SERVICE_ACCEPT_STOP |
		SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	hStatus = RegisterServiceCtrlHandler(
		(LPCSTR)"MemoryStatus",
		(LPHANDLER_FUNCTION)ControlHandler);

	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{
		// Registering Control Handler failed
		return;
	}

	// Initialize Service 
	error = InitService();
	if (!error)
	{
		// Initialization failed
		ServiceStatus.dwCurrentState =
			SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = -1;
		SetServiceStatus(hStatus, &ServiceStatus);
		return;
	}

	// We report the running status to SCM. 
	ServiceStatus.dwCurrentState =
		SERVICE_RUNNING;
	SetServiceStatus(hStatus, &ServiceStatus);

	//MEMORYSTATUS memory;
	// The worker loop of a service
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		char ch_2[] = "pipeClient start.\n";
		WriteToLog(ch_2);

		pipeClient.StartPipeClient();

		char ch_1[] = "pipeClient stopped.\n";
		WriteToLog(ch_1);
		//char buffer[16];
		//ZeroMemory(buffer, 16);
		//GlobalMemoryStatus(&memory);
		//sprintf_s(buffer, 16,"%d", memory.dwAvailPhys);
		//if (sizeof(memory.dwAvailPhys) < 16)
		//{
		//	buffer[sizeof(memory.dwAvailPhys)] = '\n';
		//}
		//else
		//{
		//	buffer[15] = '\n';
		//}
		//
		//int result = WriteToLog(buffer);
		//if (result)
		//{
		//	ServiceStatus.dwCurrentState =
		//		SERVICE_STOPPED;
		//	ServiceStatus.dwWin32ExitCode = -1;
		//	SetServiceStatus(hStatus,
		//		&ServiceStatus);
		//	return;
		//}

	}
	return;
}

void ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
	{
		char ch_1[] = "Monitoring stopped.\n";
		WriteToLog(ch_1);
		ServiceStatus.dwWin32ExitCode = 0;
		pipeClient.StopPipeClient();
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;
	}
	case SERVICE_CONTROL_SHUTDOWN:
	{
		char ch[] = "Monitoring shutdown.\n";
		WriteToLog(ch);
		ServiceStatus.dwWin32ExitCode = 0;
		pipeClient.StopPipeClient();
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;
	}

	default:
		break;
	}

	// Report current status
	SetServiceStatus(hStatus, &ServiceStatus);
}


int WriteToLog(char* str)
{
	FILE* log;
	errno_t error = fopen_s(&log, LOGFILE, "a+");
	if (error == -1)
		return -1;
	fprintf(log, "%s ", str);
	fclose(log);
	return 0;
}
int InitService() {
	char ch[] = "Monitoring started.\n";
	WriteToLog(ch);
	return true;
}