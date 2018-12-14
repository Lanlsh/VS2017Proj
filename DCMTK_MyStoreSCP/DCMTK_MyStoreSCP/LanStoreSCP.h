#pragma once
#include "dcmtk/dcmnet/dstorscp.h"

#define ERROR_MSG_SIZE	1024

class LanStoreSCP :public DcmStorageSCP
{
public:
	LanStoreSCP(char* pAETitle, char* pStorageRecvFilePath, int port);
	~LanStoreSCP();

	//初始化数据
	bool InitSCP();

	//启动SCP
	bool StartSCP();

	//停止SCP
	void StopSCP();

	//设置AETitle
	bool SetAETitle(const char* pAETitle);

	//设置存储接收到的文件的路径
	bool SetRecvPath(const char* pRecvPath);

	//设置端口号
	bool SetPort(int port);

protected:
	//初始化错误信息结构
	void InitialErrorMsg();

	//手动停止SCP
	virtual OFBool stopAfterConnectionTimeout();
	virtual OFBool stopAfterCurrentAssociation();

	//操作完通知函数
	/** Overwrite this function to be notified about an incoming association release request.
	 *  The standard handler only outputs some information to the logger.
	 */
	virtual void notifyReleaseRequest();

private:
	char* m_pAETitle;	//本机AETitle
	OFCmdUnsignedInt m_port;	//SCP端口号
	char* m_pStorageRecvFilePath;	//存储接收到的文件的路径
	bool  m_bIsStopRun;	//是否停止
	char  m_chErrorMsg[ERROR_MSG_SIZE];
};

