#pragma once
#include "pch.h"

class CDaemon
{
public:
	CDaemon(char* pSeviceName, char* pServiceExePath, char* pLogFilePath = NULL);
	~CDaemon();

	/*
	Des: 写log日志
	Param: str	需要输入到日志的字符串
	*/
	void WriteToLog(char* str);

	/*
	Des: 创建服务
	return: bool  是否操作成功
	*/
	bool CreateService();

	/*
	Des: 启动服务
	return: bool  是否操作成功
	*/
	bool StartService();

	/*
	Des: 设置服务的启动类型为自动
	return: bool		是否操作成功
	*/
	bool SetServiceStartTypeToAuto();

	/*
	Des: 设置服务的启动类型为自动
	return: bool		是否操作成功
	*/
	bool SetServiceStartTypeToAutoByServerName(char* pServerName);

	/*
	Des: 停止服务
	return: bool	是否操作成功
	*/
	bool StopService();

	/*
	Des: 重启服务（服务存在的情况下）
	return: bool  是否操作成功
	*/
	bool ReStartService();

	/*
	Des: 检测服务是否存在
	return: bool  存在未true  不存在为false
	*/
	bool CheckServiceIsExistence();

protected:
	/*
	Des: 创建一个服务
	Param: serviceName  服务的名字
	Param: desService   服务的exe所在的路径
	return: bool  是否操作成功
	*/
	bool DoCreatService(char* serviceName, char* desService);

	/*
		Des: 启动一个服务
		Param: serviceName  服务的名字
		return: bool  是否操作成功
	*/
	bool DoStartService(char* serviceName);

	/*
		Des: 修改服务的属性
		Param: type			需要设置的属性的类型值
		Param: serviceName  服务的名字
		return: bool		是否操作成功
	*/
	bool DoSetServiceConfig(DWORD type, char* serviceName);

	/*
		Des: 停止一个服务
		Param: serviceName  服务的名字
		return: bool		是否操作成功
	*/
	bool DoStopService(char* serviceName);

	/*
		Des: 停止schService的依赖服务
		Param: schService		需要停止依赖服务的service的handle
		Param: schSCManager		service控制管理器的handle
		return: bool			是否操作成功
	*/
	bool StopDependentServices(const SC_HANDLE& schService, const SC_HANDLE& schSCManager);

	/*
		Des: 重启一个服务
		Param: serviceName  服务的名字
		return: bool  是否操作成功
	*/
	bool DoReStartService(char* serviceName);

	/*
	Des: 检测服务是否存在
	Param: serviceName  服务的名字
	return: bool  存在未true  不存在为false
	*/
	bool DoCheckServiceIsExistence(char* serviceName);

	/*
		Des: 将char* 转为WCHAR
		Param: des		转换后存储的地方
		Param: source	需要转换的字符串
		Param: desLength	des的大小
		Param: sourceSize	source的大小
	*/
	void ChangePCharToWCHAR(WCHAR* des, char* source, int desLength, int sourceSize);

private:
	char* m_pServiceName;	//	service名称
	char* m_pServiceExePath;	//	service的exe文件路径（包含xx.exe）
	char* m_pLogFilePath;	//日志文件路径（包含日志文件名）
};

