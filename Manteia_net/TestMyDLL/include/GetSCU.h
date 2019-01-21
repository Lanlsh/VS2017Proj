#pragma once
#include "SCUBase.h"
#include "dcmtk/dcmnet/scu.h"

typedef enum {
	QMPatientRoot = 0,
	QMStudyRoot = 1,
	QMPatientStudyOnly = 2
} QueryModel;

class MANTIA_DECL_EXPORT CGetSCU :
	public CSCUBase
{
public:
	CGetSCU(char* pSelfAETitle, char* pDesAETitle, char* pDesPeerHostName, char* pOutputFileDirectory, int desPort);
	~CGetSCU();

	//初始化
	bool InitGETSCU();

	//执行C-MOVE	//conditions: C-MOVE的检索条件	//第一个string为类型， 第二个string为值
	bool ExecuteCGET(std::map<DcmTagKey, std::string>& moveCondition);

protected:
	void PrepareTS(E_TransferSyntax ts, OFList<OFString>& syntaxes);

private:
	char* m_pSelfAETitle;			//自己的AETitle
	char* m_pDesAETitle;			//SCP的AETitle
	char* m_pDesPeerHostName;		//SCP的主机名或IP
	char* m_pOutputFileDirectory;	//输出文件的存放路径
	OFCmdUnsignedInt m_slefPort;	//目标SCP端口号
	OFCmdUnsignedInt m_desPort;		//目标SCP端口号

	QueryModel	m_queryModel;
};

