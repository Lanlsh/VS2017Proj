#include "pch.h"
#include "Daemon.h"
#include <assert.h>

CDaemon::CDaemon(char* pSeviceName, char* pServiceExePath, char* pLogFilePath)
{
	if ((pSeviceName && pServiceExePath) == false)
	{
		assert(pSeviceName && pServiceExePath);
		return;
	}

	//初始化m_pServiceName
	size_t serverNameSize = (sizeof(char) * strlen(pSeviceName) + 1);
	m_pServiceName = (char*)malloc(serverNameSize);
	memset(m_pServiceName, 0, serverNameSize);
	memcpy(m_pServiceName, pSeviceName, sizeof(char) * strlen(pSeviceName));

	//初始化m_pServiceExePath
	size_t serverExePathSize = (sizeof(char) * strlen(pServiceExePath) + 1);
	m_pServiceExePath = (char*)malloc(serverExePathSize);
	memset(m_pServiceExePath, 0, serverExePathSize);
	memcpy(m_pServiceExePath, pServiceExePath, sizeof(char) * strlen(pServiceExePath));

	//初始化m_pLogFilePath
	if (pLogFilePath == NULL)
	{
		m_pLogFilePath = NULL;

	}
	else
	{
		size_t logFilePathSize = (sizeof(char) * strlen(pLogFilePath) + 1);
		m_pLogFilePath = (char*)malloc(logFilePathSize);
		memset(m_pLogFilePath, 0, logFilePathSize);
		memcpy(m_pLogFilePath, pLogFilePath, sizeof(char) * strlen(pLogFilePath));
	}

}


CDaemon::~CDaemon()
{
	if (m_pServiceName)
	{
		free(m_pServiceName);
		m_pServiceName = NULL;
	}

	if (m_pServiceExePath)
	{
		free(m_pServiceExePath);
		m_pServiceExePath = NULL;
	}

	if (m_pLogFilePath)
	{
		free(m_pLogFilePath);
		m_pLogFilePath = NULL;
	}
}

void CDaemon::WriteToLog(char* str)
{
	if (m_pLogFilePath == NULL)
	{
		return;
	}

	if (str == NULL)
	{
		return;
	}

	FILE* log;
	errno_t error = fopen_s(&log, m_pLogFilePath, "a+");
	if (error == -1)
		return;
	fprintf(log, "%s ", str);
	fclose(log);
}

bool CDaemon::CreateService()
{
	return DoCreatService(m_pServiceName, m_pServiceExePath);
}

bool CDaemon::StartService()
{
	return DoStartService(m_pServiceName);
}

bool CDaemon::SetServiceStartTypeToAuto()
{
	return DoSetServiceConfig(SERVICE_AUTO_START, m_pServiceName);
}

bool CDaemon::SetServiceStartTypeToAutoByServerName(char * pServerName)
{
	if (pServerName == NULL)
	{
		return false;
	}

	return DoSetServiceConfig(SERVICE_AUTO_START, pServerName);
}

bool CDaemon::StopService()
{
	return DoStopService(m_pServiceName);
}

bool CDaemon::ReStartService()
{
	return DoReStartService(m_pServiceName);
}

bool CDaemon::CheckServiceIsExistence()
{
	return DoCheckServiceIsExistence(m_pServiceName);
}

void CDaemon::ChangePCharToWCHAR(WCHAR* des, char* source, int desLength, int sourceSize)
{
	MultiByteToWideChar(CP_ACP, 0, source, sourceSize + 1, des, desLength);
}

bool CDaemon::DoCreatService(char* serviceName, char* serviceExePath)
{
	char buffer[1024] = { 0 };
	if (serviceName == NULL)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoCreatService: serviceName is NULL failed\n");
		WriteToLog(buffer);
		return false;
	}

	//将char* 转为WCHAR
	CHAR wszClassName[1024];
	memset(wszClassName, 0, sizeof(wszClassName));
	memcpy(wszClassName, m_pServiceName, strlen(m_pServiceName));
	//ChangePCharToWCHAR(wszClassName, serviceName, 1024, strlen(serviceName));

	CHAR wszServiceExePath[1024];
	memset(wszServiceExePath, 0, sizeof(wszServiceExePath));
	memcpy(wszServiceExePath, m_pServiceExePath, strlen(m_pServiceExePath));
	//ChangePCharToWCHAR(wszServiceExePath, serviceExePath, 1024, strlen(serviceExePath));

	LPCSTR LPServiceName = wszClassName;
	LPCSTR LPServiceExePath = wszServiceExePath;

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	// Get a handle to the SCM database. 
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);// SC_MANAGER_CREATE_SERVICE);  // full access rights //记住： 不能为SC_MANAGER_ALL_ACCESS， 此只有管理员才会返回正常值！！

	if (NULL == schSCManager)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoCreatService: (%s): OpenSCManager failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		return false;
	}


	// Install the new service
	schService = ::CreateService(
		schSCManager,
		LPServiceName, // eg "beep_srv"
		LPServiceName, // eg "Beep Service"
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		LPServiceExePath, // eg "c:\winnt\xxx.exe"
		0,
		0,
		0,
		0,
		0);

	if (!schService)
	{
		if (ERROR_SERVICE_EXISTS == GetLastError())
		{
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoCreatService: (%s): The service has been Created!!\n", serviceName);
			WriteToLog(buffer);
			CloseServiceHandle(schSCManager);
			return true;
		}

		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoCreatService: (%s): In CreateService failed  error: (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		CloseServiceHandle(schSCManager);
		return false;
	}
	else
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoCreatService: (%s): Service installed\n", serviceName);
		WriteToLog(buffer);
	}

	// Get a handle to the service.
	schService = OpenService(
		schSCManager,            // SCM database 
		(LPSTR)LPServiceName,               // name of service 
		SERVICE_CHANGE_CONFIG
	);  // need change config access 

	if (schService == NULL)
	{
		ZeroMemory(buffer, 1024);

		switch (GetLastError())
		{
		case ERROR_ACCESS_DENIED:
			sprintf_s(buffer, 1024, "DoCreatService: (%s): ERROR_ACCESS_DENIED (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_HANDLE:
			sprintf_s(buffer, 1024, "DoCreatService: (%s): ERROR_INVALID_HANDLE (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_NAME:
			sprintf_s(buffer, 1024, "DoCreatService: (%s): ERROR_INVALID_NAME (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_SERVICE_DOES_NOT_EXIST:
			sprintf_s(buffer, 1024, "DoCreatService: (%s): ERROR_SERVICE_DOES_NOT_EXIST (%d)\n", serviceName, GetLastError());
			break;
		default:
			sprintf_s(buffer, 1024, "DoCreatService: (%s): OpenService failed (%d)\n", serviceName, GetLastError());
		}

		WriteToLog(buffer);
		CloseServiceHandle(schSCManager);
		return false;
	}

	//设置服务的启动类型为自动
	// Change the service start type.
	if (!ChangeServiceConfig(
		schService,            // handle of service 
		SERVICE_NO_CHANGE,     // service type: no change 
		SERVICE_AUTO_START,  // service start type 
		SERVICE_NO_CHANGE,     // error control: no change 
		NULL,                  // binary path: no change 
		NULL,                  // load order group: no change 
		NULL,                  // tag ID: no change 
		NULL,                  // dependencies: no change 
		NULL,                  // account name: no change 
		NULL,                  // password: no change 
		NULL))                // display name: no change
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoCreatService: (%s): ChangeServiceConfig failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);

		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}
	else
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoCreatService: (%s): ChangeServiceConfig successfully\n", serviceName);
		WriteToLog(buffer);

		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
	}

	ZeroMemory(buffer, 1024);
	sprintf_s(buffer, 1024, "DoCreatService: (%s): server creates successfully\n", serviceName);
	WriteToLog(buffer);
	return true;
}

bool CDaemon::DoStartService(char* serviceName)
{
	char buffer[1024] = { 0 };
	if (serviceName == NULL)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: serviceName is NULL failed\n");
		WriteToLog(buffer);
		return false;
	}

	//如果服务启动类型显示为禁用，则需要先将启动类型设置为非禁用，才能正常启动服务！！
	//所以，确保在该服务已注册的情况下，启动这个服务
	DoSetServiceConfig(SERVICE_AUTO_START, serviceName);

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): OpenSCManager failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		return false;
	}


	// Get a handle to the service.
	//将char* 转为WCHAR
	CHAR wszClassName[1024];
	memset(wszClassName, 0, sizeof(wszClassName));
	//ChangePCharToWCHAR(wszClassName, serviceName, 1024, strlen(serviceName) + 1);
	memcpy(wszClassName, m_pServiceName, strlen(m_pServiceName));

	LPCSTR LPServiceName = wszClassName;

	schService = OpenService(
		schSCManager,         // SCM database 
		LPServiceName,        // name of service 
		SERVICE_ALL_ACCESS &(~SERVICE_CHANGE_CONFIG));  // full access 

	if (schService == NULL)
	{
		ZeroMemory(buffer, 1024);

		switch (GetLastError())
		{
		case ERROR_ACCESS_DENIED:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_ACCESS_DENIED (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_HANDLE:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_INVALID_HANDLE (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_NAME:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_INVALID_NAME (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_SERVICE_DOES_NOT_EXIST:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_SERVICE_DOES_NOT_EXIST (%d)\n", serviceName, GetLastError());
			break;
		default:
			sprintf_s(buffer, 1024, "DoStartService: (%s): OpenService failed (%d)\n", serviceName, GetLastError());
		}

		WriteToLog(buffer);
		CloseServiceHandle(schSCManager);
		return false;
	}

	// Check the status in case the service is not stopped. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // size needed if buffer is too small
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): QueryServiceStatusEx failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);

		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): Cannot start the service because it is already running\n", serviceName);
		WriteToLog(buffer);

		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return true;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	// Wait for the service to stop before attempting to start it.

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status until the service is no longer stop pending. 

		if (!QueryServiceStatusEx(
			schService,                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // size needed if buffer is too small
		{
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoStartService: (%s): QueryServiceStatusEx failed (%d)\n", serviceName, GetLastError());
			WriteToLog(buffer);

			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return false;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				//printf("Timeout waiting for service to stop\n");
				ZeroMemory(buffer, 1024);
				sprintf_s(buffer, 1024, "DoStartService: (%s): Timeout waiting for service to stop\n", serviceName);
				WriteToLog(buffer);

				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return false;
			}
		}
	}

	// Attempt to start the service.
	if (!::StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL))      // no arguments 
	{
		//printf("StartService failed (%d)\n", GetLastError());
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): StartService failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);

		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}
	else
	{
		//printf("Service start pending...\n");
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): Service start pending...\n", serviceName);
		WriteToLog(buffer);
	}

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // if buffer too small
	{
		//printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): QueryServiceStatusEx failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status again. 

		if (!QueryServiceStatusEx(
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // if buffer too small
		{
			//printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoStartService: (%s): QueryServiceStatusEx failed (%d)\n", serviceName, GetLastError());
			WriteToLog(buffer);
			break;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	}

	// Determine whether the service is running.

	if (ssStatus.dwCurrentState != SERVICE_RUNNING)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): Service not started. Current State: %d  Exit Code: %d  Check Point: %d Wait Hint: %d \n",
			serviceName, ssStatus.dwCurrentState, ssStatus.dwWin32ExitCode, ssStatus.dwCheckPoint, ssStatus.dwWaitHint);
		WriteToLog(buffer);

		//printf("Service not started. \n");
		//printf("  Current State: %d\n", ssStatus.dwCurrentState);
		//printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
		//printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		//printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
	}

	ZeroMemory(buffer, 1024);
	sprintf_s(buffer, 1024, "DoStartService: (%s): Service started successfully.\n", serviceName);
	WriteToLog(buffer);

	//设置服务的启动类型为“自动”
	//DoSetServiceConfig(SERVICE_AUTO_START, serviceName);

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return true;
}

bool CDaemon::DoStopService(char* serviceName)
{
	char buffer[1024] = { 0 };
	if (serviceName == NULL)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStopService: serviceName is NULL failed\n");
		WriteToLog(buffer);
		return false;
	}

	SERVICE_STATUS_PROCESS ssp;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwWaitTime;

	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStopService: (%s): OpenSCManager failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		return false;
	}

	// Get a handle to the service.
	//将char* 转为WCHAR
	CHAR wszClassName[1024];
	memset(wszClassName, 0, sizeof(wszClassName));
	//ChangePCharToWCHAR(wszClassName, serviceName, 1024, strlen(serviceName) + 1);
	memcpy(wszClassName, m_pServiceName, strlen(m_pServiceName));

	LPCSTR LPServiceName = wszClassName;

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,         // SCM database 
		LPServiceName,            // name of service 
		SERVICE_STOP |
		SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS);

	if (schService == NULL)
	{
		ZeroMemory(buffer, 1024);

		switch (GetLastError())
		{
		case ERROR_ACCESS_DENIED:
			sprintf_s(buffer, 1024, "DoStopService: (%s): ERROR_ACCESS_DENIED (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_HANDLE:
			sprintf_s(buffer, 1024, "DoStopService: (%s): ERROR_INVALID_HANDLE (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_NAME:
			sprintf_s(buffer, 1024, "DoStopService: (%s): ERROR_INVALID_NAME (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_SERVICE_DOES_NOT_EXIST:
			sprintf_s(buffer, 1024, "DoStopService: (%s): ERROR_SERVICE_DOES_NOT_EXIST (%d)\n", serviceName, GetLastError());
			break;
		default:
			sprintf_s(buffer, 1024, "DoStopService: (%s): OpenService failed (%d)\n", serviceName, GetLastError());
		}

		WriteToLog(buffer);
		CloseServiceHandle(schSCManager);
		return false;
	}

	// Make sure the service is not already stopped.
	if (!QueryServiceStatusEx(
		schService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded))
	{
		//printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStopService: (%s): QueryServiceStatusEx failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		goto stop_cleanup;
	}

	if (ssp.dwCurrentState == SERVICE_STOPPED)
	{
		//printf("Service is already stopped.\n");
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStopService: (%s): Service is already stopped\n", serviceName);
		WriteToLog(buffer);
		goto stop_cleanup;
	}

	// If a stop is pending, wait for it.

	while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
	{
		//printf("Service stop pending...\n");
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStopService: (%s): Service stop pending...\n", serviceName);
		WriteToLog(buffer);

		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssp.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			//printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoStopService: (%s): QueryServiceStatusEx failed (%d)\n", serviceName, GetLastError());
			WriteToLog(buffer);
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
		{
			printf("Service stopped successfully.\n");
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoStopService: (%s): Service stopped successfully.\n", serviceName);
			WriteToLog(buffer);
			goto stop_cleanup;
		}

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			printf("Service stop timed out.\n");
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoStopService: (%s): Service stop timed out.\n", serviceName);
			WriteToLog(buffer);
			goto stop_cleanup;
		}
	}

	// If the service is running, dependencies must be stopped first.

	StopDependentServices(schService, schSCManager);

	// Send a stop code to the service.

	if (!ControlService(
		schService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&ssp))
	{
		//printf("ControlService failed (%d)\n", GetLastError());
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStopService: (%s): ControlService failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		goto stop_cleanup;
	}

	// Wait for the service to stop.
	while (ssp.dwCurrentState != SERVICE_STOPPED)
	{
		Sleep(ssp.dwWaitHint);
		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			//printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoStopService: (%s): QueryServiceStatusEx failed (%d)\n", serviceName, GetLastError());
			WriteToLog(buffer);
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
			break;

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			//printf("Wait timed out\n");
			ZeroMemory(buffer, 1024);
			sprintf_s(buffer, 1024, "DoStopService: (%s): Wait timed out\n", serviceName);
			WriteToLog(buffer);
			goto stop_cleanup;
		}
	}
	//printf("Service stopped successfully\n");
	ZeroMemory(buffer, 1024);
	sprintf_s(buffer, 1024, "DoStopService: (%s): Service stopped successfully\n", serviceName);
	WriteToLog(buffer);

stop_cleanup:
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return true;
}

bool CDaemon::StopDependentServices(const SC_HANDLE& schService, const SC_HANDLE& schSCManager)
{
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if (EnumDependentServices(schService, SERVICE_ACTIVE,
		lpDependencies, 0, &dwBytesNeeded, &dwCount))
	{
		// If the Enum call succeeds, then there are no dependent
		// services, so do nothing.
		return TRUE;
	}
	else
	{
		if (GetLastError() != ERROR_MORE_DATA)
			return FALSE; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		if (!lpDependencies)
			return FALSE;

		__try {
			// Enumerate the dependencies.
			if (!EnumDependentServices(schService, SERVICE_ACTIVE,
				lpDependencies, dwBytesNeeded, &dwBytesNeeded,
				&dwCount))
				return FALSE;

			for (i = 0; i < dwCount; i++)
			{
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService(schSCManager,
					ess.lpServiceName,
					SERVICE_STOP | SERVICE_QUERY_STATUS);

				if (!hDepService)
					return FALSE;

				__try {
					// Send a stop code.
					if (!ControlService(hDepService,
						SERVICE_CONTROL_STOP,
						(LPSERVICE_STATUS)&ssp))
						return FALSE;

					// Wait for the service to stop.
					while (ssp.dwCurrentState != SERVICE_STOPPED)
					{
						Sleep(ssp.dwWaitHint);
						if (!QueryServiceStatusEx(
							hDepService,
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ssp,
							sizeof(SERVICE_STATUS_PROCESS),
							&dwBytesNeeded))
							return FALSE;

						if (ssp.dwCurrentState == SERVICE_STOPPED)
							break;

						if (GetTickCount() - dwStartTime > dwTimeout)
							return FALSE;
					}
				}
				__finally
				{
					// Always release the service handle.
					CloseServiceHandle(hDepService);
				}
			}
		}
		__finally
		{
			// Always free the enumeration buffer.
			HeapFree(GetProcessHeap(), 0, lpDependencies);
		}
	}

	return true;
}

bool CDaemon::DoSetServiceConfig(DWORD type, char* serviceName)
{
	char buffer[1024] = { 0 };
	if (serviceName == NULL)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoSetServiceConfig: serviceName is NULL failed\n");
		WriteToLog(buffer);
		return false;
	}

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	//将char* 转为WCHAR
	CHAR wszClassName[1024];
	memset(wszClassName, 0, sizeof(wszClassName));
	//ChangePCharToWCHAR(wszClassName, serviceName, 1024, strlen(serviceName) + 1);
	memcpy(wszClassName, m_pServiceName, strlen(m_pServiceName));

	LPCSTR LPServiceName = wszClassName;

	// Get a handle to the SCM database. 
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);// SC_MANAGER_CREATE_SERVICE);  // full access rights //记住： 不能为SC_MANAGER_ALL_ACCESS， 此只有管理员才会返回正常值！！

	if (NULL == schSCManager)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): OpenSCManager failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		return false;
	}

	// Get a handle to the service.
	schService = OpenService(
		schSCManager,            // SCM database 
		(LPSTR)LPServiceName,               // name of service 
		SERVICE_CHANGE_CONFIG
	);  // need change config access 

	if (schService == NULL)
	{
		ZeroMemory(buffer, 1024);

		switch (GetLastError())
		{
		case ERROR_ACCESS_DENIED:
			sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): ERROR_ACCESS_DENIED (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_HANDLE:
			sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): ERROR_INVALID_HANDLE (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_NAME:
			sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): ERROR_INVALID_NAME (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_SERVICE_DOES_NOT_EXIST:
			sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): ERROR_SERVICE_DOES_NOT_EXIST (%d)\n", serviceName, GetLastError());
			break;
		default:
			sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): OpenService failed (%d)\n", serviceName, GetLastError());
		}

		WriteToLog(buffer);
		CloseServiceHandle(schSCManager);
		return false;
	}

	//设置服务的启动类型为自动
	// Change the service start type.
	if (!ChangeServiceConfig(
		schService,            // handle of service 
		SERVICE_NO_CHANGE,     // service type: no change 
		type,					//SERVICE_AUTO_START,  // service start type 
		SERVICE_NO_CHANGE,     // error control: no change 
		NULL,                  // binary path: no change 
		NULL,                  // load order group: no change 
		NULL,                  // tag ID: no change 
		NULL,                  // dependencies: no change 
		NULL,                  // account name: no change 
		NULL,                  // password: no change 
		NULL))                // display name: no change
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): ChangeServiceConfig failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);

		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}
	else
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoSetServiceConfig: (%s): ChangeServiceConfig successfully\n", serviceName);
		WriteToLog(buffer);

		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
	}

	return true;
}

bool CDaemon::DoReStartService(char* serviceName)
{
	char buffer[1024] = { 0 };
	if (serviceName == NULL)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoSetServiceConfig: serviceName is NULL failed\n");
		WriteToLog(buffer);
		return false;
	}

	ZeroMemory(buffer, 1024);
	sprintf_s(buffer, 1024, "DoReStartService: (%s): service is restarting......\n", serviceName);
	WriteToLog(buffer);

	bool bIsStopOK = DoStopService(serviceName);
	bool bIsStartOK = DoStartService(serviceName);
	if (bIsStopOK && bIsStartOK)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoReStartService: (%s): service restarts successfully\n", serviceName);
		WriteToLog(buffer);
		return true;
	}

	ZeroMemory(buffer, 1024);
	sprintf_s(buffer, 1024, "DoReStartService: (%s): service restarts failed\n", serviceName);
	WriteToLog(buffer);

	return false;
}

bool CDaemon::DoCheckServiceIsExistence(char * serviceName)
{
	char buffer[1024] = { 0 };
	if (serviceName == NULL)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoSetServiceConfig: serviceName is NULL failed\n");
		WriteToLog(buffer);
		return false;
	}

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		ZeroMemory(buffer, 1024);
		sprintf_s(buffer, 1024, "DoStartService: (%s): OpenSCManager failed (%d)\n", serviceName, GetLastError());
		WriteToLog(buffer);
		return false;
	}


	// Get a handle to the service.
	//将char* 转为WCHAR
	CHAR wszClassName[1024];
	memset(wszClassName, 0, sizeof(wszClassName));
	//ChangePCharToWCHAR(wszClassName, serviceName, 1024, strlen(serviceName) + 1);
	memcpy(wszClassName, m_pServiceName, strlen(m_pServiceName));

	LPCSTR LPServiceName = wszClassName;

	schService = OpenService(
		schSCManager,         // SCM database 
		LPServiceName,        // name of service 
		SERVICE_ALL_ACCESS &(~SERVICE_CHANGE_CONFIG));  // full access 

	if (schService == NULL)
	{
		ZeroMemory(buffer, 1024);

		switch (GetLastError())
		{
		case ERROR_ACCESS_DENIED:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_ACCESS_DENIED (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_HANDLE:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_INVALID_HANDLE (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_INVALID_NAME:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_INVALID_NAME (%d)\n", serviceName, GetLastError());
			break;
		case ERROR_SERVICE_DOES_NOT_EXIST:
			sprintf_s(buffer, 1024, "DoStartService: (%s): ERROR_SERVICE_DOES_NOT_EXIST (%d)\n", serviceName, GetLastError());
			break;
		default:
			sprintf_s(buffer, 1024, "DoStartService: (%s): OpenService failed (%d)\n", serviceName, GetLastError());
		}

		WriteToLog(buffer);
		CloseServiceHandle(schSCManager);
		return false;
	}

	return true;
}
