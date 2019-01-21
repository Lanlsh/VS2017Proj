#pragma once
#include "SCPBase.h"
#include "CProcessCSTORE.h"
#include "CProcessCMOVE.h"
#include "CProcessCFIND.h"
#include "CProcessCGET.h"
#include "StoreSCU.h"
#include "Manteia_dcmnet_define.h"
#include <vector>

#define ERROR_MSG_SIZE	1024

class MANTIA_DECL_EXPORT CManteiaSCP :
	public CSCPBase
{
public:
	CManteiaSCP(char* pAETitle, char* pStorageRecvFilePath, int port, CallbackFunForDB callbackFunForDB=NULL,
		CallbackFunForRQ callbackFunForRQ=NULL, CallbackFunForRecvFile callbackFunForRecvFile=NULL);
	~CManteiaSCP();

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

	//获取数据库回调函数指针
	CallbackFunForDB GetCallbackFunForDB();

	//获取CSTORE接收到文件时的回调函数的指针
	CallbackFunForRecvFile GetCalllbackForRecvFile();

	//C-STORE
	OFCondition DoReceiveSTORERequest(T_DIMSE_C_StoreRQ &reqMessage,
		const T_ASC_PresentationContextID presID,
		DcmDataset *&reqDataset);

	OFCondition DoReceiveSTORERequest(T_DIMSE_C_StoreRQ &reqMessage,
		const T_ASC_PresentationContextID presID,
		const OFString &filename);

	OFCondition DoSendSTOREResponse(const T_ASC_PresentationContextID presID,
		const T_DIMSE_C_StoreRQ &reqMessage,
		const Uint16 rspStatusCode);

	//本机执行	C-STORE服务
	bool ExecuteCSTOREService(char* pDesAETitle, char* pDesPeerHostName, int desPort, std::vector<std::string> inputFiles);

	//C-MOVE
	OFCondition DoReceiveMOVERequest(T_DIMSE_C_MoveRQ &reqMessage,
		const T_ASC_PresentationContextID presID,
		DcmDataset *&reqDataset,
		OFString &moveDest);

	OFCondition DoSendMOVEResponse(const T_ASC_PresentationContextID presID,
		const Uint16 messageID,
		const OFString &sopClassUID,
		DcmDataset *rspDataset,
		const Uint16 rspStatusCode,
		DcmDataset *statusDetail = NULL,
		const Uint16 numRemain = 0,
		const Uint16 numComplete = 0,
		const Uint16 numFail = 0,
		const Uint16 numWarn = 0);

	//OFCondition CheckIsExisitFileByFileName();

	//C-FIND
	OFCondition DoReceiveFINDRequest(T_DIMSE_C_FindRQ &reqMessage,
		const T_ASC_PresentationContextID presID,
		DcmDataset *&reqDataset);

	OFCondition DoSendFINDResponse(const T_ASC_PresentationContextID presID,
		const Uint16 messageID,
		const OFString &sopClassUID,
		DcmDataset *rspDataset,
		const Uint16 rspStatusCode,
		DcmDataset* statusDetail = NULL);

	//C-GET
	OFCondition DoReceiveGETRequest(T_DIMSE_C_GetRQ &reqMessage,
		const T_ASC_PresentationContextID presID,
		DcmDataset *&reqDataset);

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
	virtual void notifyReleaseRequest(T_DIMSE_Command CommandField);

	/** handler that is called for each incoming command message.  This handler supports
 *  C-ECHO and C-STORE requests.  All other messages will be reported as an error.
 *  @param  incomingMsg  pointer to data structure containing the DIMSE message
 *  @param  presInfo     additional information on the Presentation Context used
 *  @return status, EC_Normal if successful, an error code otherwise
 */
	virtual OFCondition handleIncomingCommand(T_DIMSE_Message *incomingMsg,
		const DcmPresentationContextInfo &presInfo);

private:
	CProcessCSTORE*	m_proCSTORE;	//处理C-STORE
	CProcessCMOVE*	m_proCMOVE;		//处理C-MOVE
	CProcessCFIND*  m_proCFIND;		//处理C-FIND
	CProcessCGET*	m_proCGET;		//处理C-GET
	char* m_pAETitle;				//本机AETitle
	CallbackFunForDB m_callbackFunForDB;	//数据库处理回调函数
	CallbackFunForRQ m_callbackFunForRQ;	//会话完成结束回调函数
	CallbackFunForRecvFile m_callbackFunForRecvFile;	//SCP接收到CSTORE文件时的回调函数
	OFCmdUnsignedInt m_port;		//SCP端口号
	char* m_pStorageRecvFilePath;	//存储接收到的文件的路径
	bool  m_bIsStopRun;				//是否停止
	char  m_chErrorMsg[ERROR_MSG_SIZE];		//当有错误时，保存最后的错误信息

	CStoreSCU* m_storeSCU;		//C-STORE SCU

};

