// ACDaemon.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "Daemon.h"
#include "PipeServerByOL.h"

#define SLEEP_TIME 10000

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);

//服务关闭或停止时，设置服务启动类型为auto
void SetServiceStartTypeToAuto();

//停止pipe服务器
void StopPipeServer();

//	主服务信息
char serviceName_Main[] = "MemoryStatus";
char serviceExePath_Main[] = "E:/lanliangsheng/VS2017Proj/ServiceTest/Debug/ServiceTest.exe";
char logPath[] = "C://Users//Public//Documents//ACDaemon_Log.txt";

CDaemon daemon(serviceName_Main, serviceExePath_Main, logPath);
PipeServerByOL* pipeServer = NULL;

//	自己服务信息
char serviceName[] = "ACDaemon";

int main(int argc, char* argv[])
{
	SERVICE_TABLE_ENTRY ServiceTable[2];

	CHAR wszClassName[1024];
	memset(wszClassName, 0, sizeof(wszClassName));
	memcpy(wszClassName, serviceName, strlen(serviceName));
	ServiceTable[0].lpServiceName = wszClassName;
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

	CHAR wszClassName[1024];
	memset(wszClassName, 0, sizeof(wszClassName));
	memcpy(wszClassName, serviceName, strlen(serviceName));

	//hStatus = RegisterServiceCtrlHandler(
	//	wszClassName,
	//	(LPHANDLER_FUNCTION)ControlHandler);

	//if (hStatus == (SERVICE_STATUS_HANDLE)0)
	//{
	//	// Registering Control Handler failed
	//	char ch[] = "Registering Control Handler failed.\n";
	//	daemon.WriteToLog(ch);
	//	return;
	//}

	// Initialize Service 

	//if (!error)
	//{
	//	// Initialization failed
	//	ServiceStatus.dwCurrentState =
	//		SERVICE_STOPPED;
	//	ServiceStatus.dwWin32ExitCode = -1;
	//	SetServiceStatus(hStatus, &ServiceStatus);
	//	return;
	//}

	//创建主服务
	if (!daemon.CreateService())
	{
		char ch[] = "CreateService failed.\n";
		daemon.WriteToLog(ch);
		return;
	}
	else
	{
		char ch[] = "CreateService success.\n";
		daemon.WriteToLog(ch);
	}

	//启动主服务
	if (!daemon.StartService())
	{
		char ch[] = "StartService started.\n";
		daemon.WriteToLog(ch);
		return;
	}
	else
	{
		char ch[] = "StartService success.\n";
		daemon.WriteToLog(ch);
	}

	char ch[] = "Monitoring started.\n";
	daemon.WriteToLog(ch);

	// We report the running status to SCM. 

	ServiceStatus.dwCurrentState =
		SERVICE_RUNNING;
	SetServiceStatus(hStatus, &ServiceStatus);

	//MEMORYSTATUS memory;
	// The worker loop of a service
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		//检测主服务的状态
		if (pipeServer)
		{
			delete pipeServer;
			pipeServer = NULL;
		}

		pipeServer = new PipeServerByOL();
		if (pipeServer)
		{
			char ch_1[] = "创建pipeServer成功！！\n";
			daemon.WriteToLog(ch_1);

			pipeServer->StartPipeServer();
			if (pipeServer->GetIsTimeOut())
			{
				//说明pipe客户端没有回复
				char ch_1[] = "pipe客户端没有回复，需要重启主服务！！\n";
				daemon.WriteToLog(ch_1);

				if (daemon.CheckServiceIsExistence())
				{
					//主服务存在，则重启
					daemon.ReStartService();
				}
				else
				{
					if (daemon.CreateService())
					{
						char ch_2[] = "CreateService success.\n";
						daemon.WriteToLog(ch_2);

						if (!daemon.StartService())
						{
							char ch_3[] = "StartService started.\n";
							daemon.WriteToLog(ch_3);
							return;
						}
						else
						{
							char ch_4[] = "StartService success.\n";
							daemon.WriteToLog(ch_4);
						}
					}
					else
					{
						char ch[] = "CreateService failed.\n";
						daemon.WriteToLog(ch);
					}
				}
			}
			else
			{
				//说明pipe服务端正常退出
				char ch_1[] = "说明pipe服务端正常退出！！\n";
				daemon.WriteToLog(ch_1);
			}

			if (pipeServer)
			{
				delete pipeServer;
				pipeServer = NULL;
			}
		}
		else
		{
			char ch_1[] = "创建pipeServer失败！！\n";
			daemon.WriteToLog(ch_1);
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		}

		
	}
}

void SetServiceStartTypeToAuto()
{
	//当守护服务每次退出时，将自己和主服务的启动类型设置为“自动”
	if (daemon.SetServiceStartTypeToAuto())
	{
		char ch[] = "The main service startup type is set successfully.\n";
		daemon.WriteToLog(ch);
	}
	else
	{
		char ch[] = "Primary service startup type setting failed.\n";
		daemon.WriteToLog(ch);
	}

	if (daemon.SetServiceStartTypeToAutoByServerName(serviceName))
	{
		char ch[] = "The daemon service startup type is set successfully.\n";
		daemon.WriteToLog(ch);
	}
	else
	{
		char ch[] = "daemon service startup type setting failed.\n";
		daemon.WriteToLog(ch);
	}
}

void StopPipeServer()
{
	if (pipeServer)
	{
		pipeServer->StartPipeServer();
	}
}

void ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
	{
		char ch_1[] = "Monitoring stopped.\n";
		daemon.WriteToLog(ch_1);
		ServiceStatus.dwWin32ExitCode = 0;
		SetServiceStartTypeToAuto();
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		StopPipeServer();

		break;
	}
	case SERVICE_CONTROL_SHUTDOWN:
	{
		char ch[] = "Monitoring shutdown.\n";
		daemon.WriteToLog(ch);
		ServiceStatus.dwWin32ExitCode = 0;
		SetServiceStartTypeToAuto();
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		StopPipeServer();
		
		break;
	}
	case SERVICE_CONTROL_INTERROGATE:
	{
		char ch[] = "SERVICE_CONTROL_INTERROGATE Monitoring shutdown.\n";
		daemon.WriteToLog(ch);
		ServiceStatus.dwWin32ExitCode = 0;
		SetServiceStartTypeToAuto();
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		StopPipeServer();
		
		break;
	}
	case SERVICE_CONTROL_PRESHUTDOWN:
	{
		char ch[] = "SERVICE_CONTROL_PRESHUTDOWN Monitoring shutdown.\n";
		daemon.WriteToLog(ch);
		ServiceStatus.dwWin32ExitCode = 0;
		SetServiceStartTypeToAuto();
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		StopPipeServer();
		
		break;
	}
	case SERVICE_CONTROL_PAUSE:
	{
		char ch[] = "SERVICE_CONTROL_PAUSE Monitoring shutdown.\n";
		daemon.WriteToLog(ch);
		ServiceStatus.dwWin32ExitCode = 0;
		SetServiceStartTypeToAuto();
		
		//ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		StopPipeServer();
		
		break;
	}
	//case SERVICE_CONTROL_SESSIONCHANGE:
	//{
	//	char ch[] = "SERVICE_CONTROL_SESSIONCHANGE Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_POWEREVENT:
	//{
	//	char ch[] = "SERVICE_CONTROL_POWEREVENT Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
	//{
	//	char ch[] = "SERVICE_CONTROL_HARDWAREPROFILECHANGE Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_DEVICEEVENT:
	//{
	//	char ch[] = "SERVICE_CONTROL_DEVICEEVENT Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_PARAMCHANGE:
	//{
	//	char ch[] = "SERVICE_CONTROL_PARAMCHANGE Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_TIMECHANGE:
	//{
	//	char ch[] = "SERVICE_CONTROL_TIMECHANGE Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_TRIGGEREVENT:
	//{
	//	char ch[] = "SERVICE_CONTROL_TRIGGEREVENT Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_LOWRESOURCES:
	//{
	//	char ch[] = "SERVICE_CONTROL_LOWRESOURCES Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}
	//case SERVICE_CONTROL_SYSTEMLOWRESOURCES:
	//{
	//	char ch[] = "SERVICE_CONTROL_SYSTEMLOWRESOURCES Monitoring shutdown.\n";
	//	daemon.WriteToLog(ch);
	//	ServiceStatus.dwWin32ExitCode = 0;
	//	SetServiceStartTypeToAuto();
	//	ServiceStatus.dwCurrentState = SERVICE_STOPPED;

	//	break;
	//}

	default:
		break;
	}

	// Report current status
	SetServiceStatus(hStatus, &ServiceStatus);
}
