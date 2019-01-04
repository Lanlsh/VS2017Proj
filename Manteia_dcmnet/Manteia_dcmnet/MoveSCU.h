#pragma once
#include "SCUBase.h"
#include "dcmtk/dcmnet/scu.h"
#include <map>

class MANTIA_DECL_EXPORT CMoveSCU :
	public CSCUBase
{
public:
	CMoveSCU(char* pSelfAETitle, char* pDesAETitle, char* pDesPeerHostName, char* pOutputFileDirectory, int selfPort, int desPort);
	~CMoveSCU();

	//初始化
	bool InitMoveSCU();

	//执行C-MOVE	//conditions: C-MOVE的检索条件	//第一个string为类型， 第二个string为值
	bool ExecuteCMOVE(std::map<DcmTagKey, std::string>& moveCondition);

protected:
	/** This is the standard handler for C-MOVE message responses: It just adds up all responses
 *  it receives and prints a DEBUG message. Therefore, it is called for each response
 *  received in sendMOVERequest(). The idea is of course to overwrite this function in a
 *  derived, actual SCU implementation if required. Thus, after each response, the caller of
 *  sendMOVERequest() can decide on its own whether he wants to cancel the C-MOVE session,
 *  terminate the association, do something useful or whatever. Thus this function is a more
 *  object-oriented kind of callback.
 *  @param presID              [in]  The presentation context ID where the response was
 *                                   received on.
 *  @param response            [in]  The C-MOVE response received.
 *  @param waitForNextResponse [out] Denotes whether SCU should try to receive another
 *                                   response. If set to OFTrue, then sendMOVERequest() will
 *                                   continue waiting for responses. The current
 *                                   implementation does that for all responses do not have
 *                                   status Failed, Warning, Success or unknown. If set to
 *                                   OFFalse, sendMOVERequest() will return control to the
 *                                   caller.
 *  @return EC_Normal, if response could be handled. Error code otherwise.
 *          The current implementation always returns EC_Normal.
 */
	virtual OFCondition handleMOVEResponse(const T_ASC_PresentationContextID presID,
		RetrieveResponse *response,
		OFBool &waitForNextResponse);

private:
	char* m_pSelfAETitle;			//自己的AETitle
	char* m_pDesAETitle;			//SCP的AETitle
	char* m_pDesPeerHostName;		//SCP的主机名或IP
	char* m_pOutputFileDirectory;	//输出文件的存放路径
	OFCmdUnsignedInt m_slefPort;		//目标SCP端口号
	OFCmdUnsignedInt m_desPort;		//目标SCP端口号

};

