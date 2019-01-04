// TestMyDLL.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <thread>
#include "ManteiaSCP.h"
#include "Manteia_dcmnet_define.h"

CManteiaSCP* g_scp = NULL;

void StopSCP()
{
	Sleep(5000);
	if (g_scp)
	{
		g_scp->StopSCP();
	}

}

void DBCallback(CallbackDataForDB* callbackData)
{
	switch (callbackData->type)
	{
	case OT_CFIND:
		callbackData->mapOutInfo[DCM_PatientID].push_back("No.1008611");
		break;
	case OT_CGET:
		callbackData->vInputFile.push_back("E://DcmScuScp//files//0.dcm");
		break;
	case OT_CMOVE:
		callbackData->vInputFile.push_back("E://DcmScuScp//files//0.dcm");
		callbackData->nDesPort = 9999;
		std::string str = "192.168.10.46";
		memcpy(callbackData->pDesHostName, str.c_str(), str.length());
		break;

	}

}

void RecvFileCallback(std::string str)
{
	printf("recvfile： %s\n", str.c_str());
}

void RQCallback(CallbackDataForRQ* data)
{
	printf("--------------------------: %d \n", data->type);
}

int main()
{
	char aeTitle[256] = { 0 };
	const char* title = "LANDCMRECV";
	memcpy(aeTitle, title, strlen(title));

	char recvPath[256] = { 0 };
	const char* path = "E://DcmScuScp//DcmScp";
	memcpy(recvPath, path, strlen(path));

	CallbackFunForDB cb = DBCallback;
	CallbackFunForRQ rq = RQCallback;
	CallbackFunForRecvFile	rf = RecvFileCallback;
	CManteiaSCP scp(aeTitle, recvPath, 8888, cb, rq, rf);
	
	//test if Exit is OK
	//g_scp = &scp;
	//std::thread td(&StopSCP);
	//td.detach();

	scp.StartSCP();
    std::cout << "Hello World!\n"; 
}
