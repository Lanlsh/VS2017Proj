#pragma once
#include "dcmtk/dcmnet/dstorscu.h"
#include "Manteia_dcmnet_define.h"
#include <vector>

class MANTIA_DECL_EXPORT CStoreSCU :public DcmStorageSCU
{
public:
	CStoreSCU(char* pOurAETitle);
	CStoreSCU(char* pOurAETitle, char* pDesAETitle, char* pDesPeerHostName, int desPort);
	~CStoreSCU();

	//初始化数据
	bool InitStoreSCU();

	//设置发送目标信息
	void SetDesInfo(char* pDesAETitle, char* pDesPeerHostName, int desPort);

	//执行C-STORE	//cstoreFiles: 需要C-STORE的文件的集合
	bool ExecuteCSTORE(std::vector<std::string> cstoreFiles);

	// make sure that everything is cleaned up properly
	static void cleanup();

private:
	char* m_pOurAETitle;			//自己的AETitle
	char* m_pDesAETitle;			//SCP的AETitle
	char* m_pDesPeerHostName;		//SCP的主机名或IP
	OFCmdUnsignedInt m_desPort;		//目标SCP端口号
};

