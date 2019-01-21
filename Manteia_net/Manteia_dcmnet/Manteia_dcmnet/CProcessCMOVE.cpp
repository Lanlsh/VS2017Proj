#include "CProcessCMOVE.h"
#include "stdafx.h"
#include "ManteiaSCP.h"
#include "StoreSCU.h"
#include "dcmtk/dcmnet/diutil.h"
#include "common.h"

CProcessCMOVE::CProcessCMOVE(CManteiaSCP* scp):
	CProcessCommandBase(scp)
{

}


CProcessCMOVE::~CProcessCMOVE()
{
}


OFCondition CProcessCMOVE::handleIncomingCommand(T_DIMSE_Message *incomingMsg,
	const DcmPresentationContextInfo &presInfo)
{
	OFCondition status = EC_IllegalParameter;
	if (incomingMsg != NULL)
	{
		// check whether we've received a supported command
		if (incomingMsg->CommandField == DIMSE_C_MOVE_RQ)
		{
			// handle incoming C-MOVE request
			T_DIMSE_C_StoreRQ &storeReq = incomingMsg->msg.CStoreRQ;
			Uint16 rspStatusCode = STATUS_MOVE_Failed_UnableToProcess;

			DcmFileFormat fileformat;
			DcmDataset *reqDataset = fileformat.getDataset();
			OFString moveDest = OFString();
			status = GetSCPInstance()->DoReceiveMOVERequest(incomingMsg->msg.CMoveRQ, presInfo.presentationContextID, reqDataset, moveDest);
			if (status.good())
			{
				//1. 检查C-MOVE数据是否正确
				//2. 收集C-MOVE需要的返回的信息//此信息是不是根据move请求去加载相应的文件信息？

				//通知接收到消息
				NotifyRecvCMOVERequest(incomingMsg->msg.CMoveRQ.AffectedSOPClassUID, incomingMsg->msg.CMoveRQ.MoveDestination);

				CallbackDataForDB callbackData;
				//初始化数据
				callbackData.type = OT_CMOVE;
				int nAcceptableKeyLength = sizeof(COMMON::AcceptableDcmTagKey) / sizeof(COMMON::AcceptableDcmTagKey[0]);
				for (int i = 0; i < nAcceptableKeyLength; i++)
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

				bool bIsOk = false;
				//数据库查询信息
				if (GetSCPInstance()->GetCallbackFunForDB())
				{
					//执行数据库回调函数
					GetSCPInstance()->GetCallbackFunForDB()(&callbackData);

					//取查询数据
					std::vector<std::string> inputFiles;
					for (int j = 0; j < callbackData.vInputFile.size(); j++)
					{
						inputFiles.push_back(callbackData.vInputFile[j].c_str());
					}

					//发送符合的文件//注意：当局部变量SCU释放内存是，dcmtk的数据字典也会被释放，导致后续的scp提示“Data dictionary missing!!!”
					if (!inputFiles.empty())
					{
						bIsOk = GetSCPInstance()->ExecuteCSTOREService(incomingMsg->msg.CMoveRQ.MoveDestination, callbackData.pDesHostName, callbackData.nDesPort, inputFiles);
					}

				}

				DcmDataset resData;
				resData.insertEmptyElement(DCM_RETIRED_CommandLengthToEnd);
				if (bIsOk)
				{
					//3. status.good() 发送MoveResponse
					rspStatusCode = STATUS_Success;
					status = GetSCPInstance()->DoSendMOVEResponse(presInfo.presentationContextID, incomingMsg->msg.CMoveRQ.MessageID,
						incomingMsg->msg.CMoveRQ.AffectedSOPClassUID, &resData, rspStatusCode);
				}
				else
				{
					//3. status.good() 发送MoveResponse
					//resData.insertEmptyElement(DCM_RETIRED_CommandLengthToEnd);
					status = GetSCPInstance()->DoSendMOVEResponse(presInfo.presentationContextID, incomingMsg->msg.CMoveRQ.MessageID,
						incomingMsg->msg.CMoveRQ.AffectedSOPClassUID, reqDataset, rspStatusCode);
				}


			}

			
		}
	}

	return status;
}

void CProcessCMOVE::NotifyRecvCMOVERequest(DIC_UI AffectedSOPClassUID, DIC_AE MoveDestination)
{
	DCMNET_INFO("Receive C-MOVE :AffectedSOPClassUID= "<< AffectedSOPClassUID <<" MoveDestination: "<< MoveDestination);
}
