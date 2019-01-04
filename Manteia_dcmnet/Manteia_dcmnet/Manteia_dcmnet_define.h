#pragma once
#include <map>
#include <vector>
#include <string>

//设置导入导出宏
#define MANTEIA_DCMNET_EXEPORT _declspec(dllexport)
#define MANTEIA_DCMNET_IMPORT _declspec(dllimport)

//根据预编译宏MANTEIA_DCMNET_DLLEXPORTS来判断此项目是进行导入还是导出
#ifdef MANTEIA_DCMNET_DLLEXPORTS
#define MANTIA_DECL_EXPORT MANTEIA_DCMNET_EXEPORT
#else
#define MANTIA_DECL_EXPORT MANTEIA_DCMNET_IMPORT
#endif

enum EM_OperationTpye
{
	OT_CGET,
	OT_CFIND,
	OT_CMOVE,
	OT_CSTORE,
	OT_CECHO,

	OT_UNKNOWN
};

//给数据库处理的回调函数的形参结构
typedef struct ST_CallbackDataForDB
{
	char pDesAETitle[1024];		//目的AETitle
	char pDesHostName[1024];	//目的IP地址或计算机名字
	int  nDesPort;				//目的AETitle的端口号
	EM_OperationTpye type;		//接收到的SCU操作类型

	std::vector <std::string > vInputFile;	//要发送的文件名
	std::map<DcmTagKey, std::vector<std::string > >	mapInInfo;	//SCP存储接收到SCU消息后解析后的信息
	std::map<DcmTagKey, std::vector<std::string > >	mapOutInfo;	//数据库处理的回调函数返回的信息

	ST_CallbackDataForDB()
	{
		memset(pDesAETitle, 0, 1024);
		memset(pDesHostName, 0, 1024);
		nDesPort = 0;
		type = OT_UNKNOWN;
		vInputFile.clear();	//vector 一定不能使用ZeroMemory去初始化，否则会引起内存泄漏！！
		mapInInfo.clear();
		mapOutInfo.clear();
	}
}CallbackDataForDB;

//给数据库处理的回调函数定义
typedef void(*CallbackFunForDB)(CallbackDataForDB* callbackData);

//给会话完成时的回调函数的形参结构
typedef struct ST_CallbackDataForRQ
{
	EM_OperationTpye type;		//接收到的SCU操作类型

	ST_CallbackDataForRQ()
	{
		type = OT_UNKNOWN;
	}
}CallbackDataForRQ;

//会话完成结束回调函数定义
typedef void(*CallbackFunForRQ)(CallbackDataForRQ* callbackData);

//SCP接收到CSTORE文件时的回调函数定义
typedef void(*CallbackFunForRecvFile)(std::string str);