#include "CProcessCFIND.h"
#include "ManteiaSCP.h"
#include "dcmtk/dcmnet/diutil.h"
#include "StoreSCU.h"
#include "common.h"

CProcessCFIND::CProcessCFIND(CManteiaSCP* scp):
	CProcessCommandBase(scp)
{

}


CProcessCFIND::~CProcessCFIND()
{
}

OFCondition CProcessCFIND::handleIncomingCommand(T_DIMSE_Message * incomingMsg, const DcmPresentationContextInfo & presInfo)
{
	OFCondition status = EC_IllegalParameter;
	if (incomingMsg != NULL)
	{
		// check whether we've received a supported command
		if (incomingMsg->CommandField == DIMSE_C_FIND_RQ)
		{
			// handle incoming C-MOVE request
			T_DIMSE_C_FindRQ &storeReq = incomingMsg->msg.CFindRQ;
			Uint16 rspStatusCode = STATUS_FIND_Failed_UnableToProcess;

			DcmFileFormat fileformat;
			DcmDataset *reqDataset = fileformat.getDataset();
			status = GetSCPInstance()->DoReceiveFINDRequest(incomingMsg->msg.CFindRQ, presInfo.presentationContextID, reqDataset);
			if (status.good())
			{
				//1. 检查C-FIND数据是否正确
				//2. 收集C-FIND需要的返回的信息

				//通知接收到消息
				NotifyRecvCFINDRequest(incomingMsg->msg.CFindRQ);

				CallbackDataForDB callbackData;
				//初始化数据
				callbackData.type = OT_CFIND;
				int nAcceptableKeyLength = sizeof(COMMON::AcceptableDcmTagKey) / sizeof(COMMON::AcceptableDcmTagKey[0]);
				for (int i=0; i< nAcceptableKeyLength; i++)
				{
					OFString value = OFString();
					if (reqDataset->findAndGetOFStringArray(COMMON::AcceptableDcmTagKey[i], value) == EC_Normal)
					{
						if (!value.empty())
						{
							callbackData.mapInInfo[COMMON::AcceptableDcmTagKey[i]].push_back(value.data());
						}
					}
					
				}

				//数据库查询信息
				if (GetSCPInstance()->GetCallbackFunForDB())
				{
					//执行数据库回调函数
					GetSCPInstance()->GetCallbackFunForDB()(&callbackData);

					//取查询数据
					DcmDataset resData;
					std::map<DcmTagKey, std::vector<std::string > >::iterator iter = callbackData.mapOutInfo.begin();
					for (; iter!=callbackData.mapOutInfo.end(); iter++)
					{
						for (int j=0; j<iter->second.size(); j++)
						{
							resData.putAndInsertOFStringArray(iter->first, OFString((iter->second)[j].c_str()));
						}
					}

					//返回处理信息
					rspStatusCode = STATUS_Pending;
					status = GetSCPInstance()->DoSendFINDResponse(presInfo.presentationContextID, incomingMsg->msg.CFindRQ.MessageID,
						incomingMsg->msg.CFindRQ.AffectedSOPClassUID, &resData, rspStatusCode);
				}

				if (status.good())
				{
					//收尾数据（必须要有，让scu知道C-FIND信息接收结束的标志！！！）
					rspStatusCode = STATUS_Success;
					status = GetSCPInstance()->DoSendFINDResponse(presInfo.presentationContextID, incomingMsg->msg.CFindRQ.MessageID,
						incomingMsg->msg.CFindRQ.AffectedSOPClassUID, NULL, rspStatusCode);

				}
				else
				{
					//收尾数据（必须要有，让scu知道C-FIND信息接收结束的标志！！！）
					rspStatusCode = STATUS_FIND_Failed_UnableToProcess;
					status = GetSCPInstance()->DoSendFINDResponse(presInfo.presentationContextID, incomingMsg->msg.CFindRQ.MessageID,
						incomingMsg->msg.CFindRQ.AffectedSOPClassUID, NULL, rspStatusCode);
				}

			}


		}
	}

	return status;
}

void CProcessCFIND::NotifyRecvCFINDRequest(const T_DIMSE_C_FindRQ & findRQ)
{
	DCMNET_INFO("Receive C-FIND :AffectedSOPClassUID= " << findRQ.AffectedSOPClassUID);
}
