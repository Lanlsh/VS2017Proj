// DCMTK_MySCU.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <string.h>

#include "LanStoreSCP.h"

LanStoreSCP* g_scp = NULL;

void StopSCP()
{
	Sleep(5000);
	if (g_scp)
	{
		g_scp->StopSCP();
	}
	
}

int main(/*int argc, char* argv[]*/)
{
	char aeTitle[256] = { 0 };
	const char* title = "LANDCMRECV";
	memcpy(aeTitle, title, strlen(title));

	char recvPath[256] = { 0 };
	const char* path = "E://DcmScuScp//DcmScp";
	memcpy(recvPath, path, strlen(path));

	LanStoreSCP scp(aeTitle, recvPath, 8888);
	g_scp = &scp;

	//test Exit is OK
	//std::thread td(&StopSCP);
	//td.detach();

	scp.StartSCP();

	return 0;
}
