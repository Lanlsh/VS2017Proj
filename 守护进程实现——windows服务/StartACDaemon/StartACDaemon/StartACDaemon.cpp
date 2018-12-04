// StartACDaemon.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "Daemon.h"

int main()
{
	char serviceName[] = "ACDaemon";
	char serviceExePath[] = "E:/lanliangsheng/VS2017Proj/ACDaemon/Debug/ACDaemon.exe";
	char logPath[] = "C://Users//Public//Documents//ACDaemon_Log.txt";
	CDaemon daemon(serviceName, serviceExePath, logPath);

	//创建守护服务
	if (!daemon.CreateService())
	{
		char ch[] = "Create daemon service failed.\n";
		daemon.WriteToLog(ch);
	}
	else
	{
		char ch[] = "Create daemon service success.\n";
		daemon.WriteToLog(ch);
	}

	//运行守护服务
	if (!daemon.StartService())
	{
		char ch[] = "Start daemon service failed.\n";
		daemon.WriteToLog(ch);
	}
	else
	{
		char ch[] = "Start daemon service success.\n";
		daemon.WriteToLog(ch);
	}
	
	return 0;
}
