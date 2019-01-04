#pragma once
#include "SCUBase.h"
#include "dcmtk/dcmnet/scu.h"
//#include "stdafx.h"

class MANTIA_DECL_EXPORT CFindSCU :public CSCUBase
{
public:
	CFindSCU(char* pSelfAETitle, char* pDesAETitle, char* pDesPeerHostName, int selfPort, int desPort);
	~CFindSCU();

	//初始化
	bool InitFindSCU();

	//执行C-MOVE	//conditions: C-MOVE的检索条件	//第一个TagKey为类型， 第二个string为值
	bool ExecuteCFIND(std::map<DcmTagKey, std::string>& moveCondition);

	/** This is the standard handler for C-FIND message responses: It just adds up all responses
 *  it receives and prints a DEBUG message. Therefore, it is called for each response
 *  received in sendFINDRequest(). The idea is of course to overwrite this function in a
 *  derived, actual SCU implementation if required. Thus, after each response, the caller of
 *  sendFINDRequest() can decide on its own whether he wants to cancel the C-FIND session,
 *  terminate the association, do something useful or whatever. That way this is a more
 *  object-oriented kind of callback.
 *  @param presID              [in]  The presentation context ID where the response was
 *                                   received on.
 *  @param response            [in]  The C-FIND response received.
 *  @param waitForNextResponse [out] Denotes whether SCU should try to receive another
 *                                   response. If set to OFTrue, then sendFINDRequest()
 *                                   will continue waiting for responses. The current
 *                                   implementation does that for all responses do not have
 *                                   status SUCESSS. If set to OFFalse, sendFINDRequest()
 *                                   will return control to the caller.
 *  @return EC_Normal, if response could be handled. Error code otherwise.
 *          The current implementation always returns EC_Normal.
 */
	virtual OFCondition handleFINDResponse(const T_ASC_PresentationContextID presID,
		QRResponse *response,
		OFBool &waitForNextResponse);

private:
	char* m_pSelfAETitle;			//自己的AETitle
	char* m_pDesAETitle;			//SCP的AETitle
	char* m_pDesPeerHostName;		//SCP的主机名或IP
	OFCmdUnsignedInt m_slefPort;		//目标SCP端口号
	OFCmdUnsignedInt m_desPort;		//目标SCP端口号

};

