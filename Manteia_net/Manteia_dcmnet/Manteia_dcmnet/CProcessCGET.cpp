#include "CProcessCGET.h"

#include "ManteiaSCP.h"
#include "dcmtk/dcmnet/diutil.h"
#include "CStoreSCUForCGet.h"
#include "dcmtk/dcmnet/dstorSCU.h"	//用于发送C-STORE rq
#include "common.h"

CProcessCGET::CProcessCGET(CManteiaSCP* scp):
	CProcessCommandBase(scp),
	m_storeSCU(NULL)
{
	m_storeSCU = new CStoreSCUForCGet();
}


CProcessCGET::~CProcessCGET()
{
	if (m_storeSCU)
	{
		//只用C-GET时才需要设置NULL！！(此处额外设置NULL非常重要！！！)
		if (m_storeSCU->GetT_ASC_Association())
		{
			m_storeSCU->SetT_ASC_Association(NULL);
		}

		delete m_storeSCU;
		m_storeSCU = NULL;
	}
}

OFCondition CProcessCGET::handleIncomingCommand(T_DIMSE_Message* incomingMsg, const DcmPresentationContextInfo & presInfo)
{
	OFCondition status = EC_IllegalParameter;
	if (incomingMsg != NULL)
	{
		// check whether we've received a supported command
		if (incomingMsg->CommandField == DIMSE_C_GET_RQ)
		{
			//接收消息
			DcmFileFormat fileformat;
			DcmDataset *reqDataset = fileformat.getDataset();
			status = GetSCPInstance()->DoReceiveGETRequest(incomingMsg->msg.CGetRQ, presInfo.presentationContextID, reqDataset);

			if (status.good())
			{
				//通知接收到消息
				NotifyRecvCFINDRequest(incomingMsg->msg.CGetRQ);
				//符合条件就发送DIMSE_C_STORE_RQ回复(参考C-STORE SCU)
				OFList<OFString> inputFiles;

				CallbackDataForDB callbackData;
				//初始化数据
				callbackData.type = OT_CGET;
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

				//数据库查询信息
				if (GetSCPInstance()->GetCallbackFunForDB())
				{
					//执行数据库回调函数
					GetSCPInstance()->GetCallbackFunForDB()(&callbackData);

					//取查询数据
					for (int j=0; j<callbackData.vInputFile.size(); j++)
					{
						inputFiles.push_back(OFString(callbackData.vInputFile[j].c_str()));
					}

				}

				//根据数据库查询，符合条件就发送DIMSE_C_STORE_RQ回复，否则就发送DIMSE_C_GET_RSP
				if (!inputFiles.empty())
				{
					//清空之前数据
					m_storeSCU->clear();

					m_storeSCU->SetT_ASC_Association(GetSCPInstance()->GetAssociationInstance());

					m_storeSCU->setReadFromDICOMDIRMode(OFFalse);
					m_storeSCU->setHaltOnInvalidFileMode(OFFalse);	//指定在批处理期间是否遇到无效文件时停止（例如，从DICOMDIR添加SOP实例时）或是否继续下一个SOP实例。

					//"checking input files ..."
					/* iterate over all input filenames */
					const char *currentFilename = NULL;
					OFListIterator(OFString) if_iter = inputFiles.begin();
					OFListIterator(OFString) if_last = inputFiles.end();
					unsigned long numInvalidFiles = 0;
					while (if_iter != if_last)
					{
						currentFilename = (*if_iter).c_str();
						/* and add them to the list of instances to be transmitted */
						status = m_storeSCU->addDicomFile(currentFilename, ERM_fileOnly, OFTrue);
						if (status.bad())
						{
							/* check for empty filename */
							if (strlen(currentFilename) == 0)
								currentFilename = "<empty string>";
							/* if something went wrong, we either terminate or ignore the file */
							if (OFFalse)
							{
								DCMNET_INFO("bad DICOM file: " << currentFilename << ": " << status.text());
								break;
							}
							else {
								DCMNET_INFO("bad DICOM file: " << currentFilename << ": " << status.text() << ", ignoring file");
							}
							++numInvalidFiles;
						}
						++if_iter;
					}

					/* check whether there are any valid input files */
					if (m_storeSCU->getNumberOfSOPInstances() == 0)
					{
						DCMNET_INFO("no valid input files to be processed");
						//	//还原临时数据
						//if (GetSCPInstance()->GetAssociationInstance())
						//{
						//	GetSCPInstance()->GetAssociationInstance()->DULassociation = NULL;
						//	GetSCPInstance()->GetAssociationInstance()->params = NULL;
						//	GetSCPInstance()->GetAssociationInstance()->sendPDVBuffer = NULL;
						//}
						return EC_CouldNotGenerateFilename;
					}
					else {
						DCMNET_INFO("in total, there are " << m_storeSCU->getNumberOfSOPInstances()
							<< " SOP instances to be sent, " << numInvalidFiles << " invalid files are ignored");
					}

					while ((status = m_storeSCU->addPresentationContexts()).good())
					{
						DCMNET_INFO("sending SOP instances ...");
						/* send SOP instances to be transferred */
						status = m_storeSCU->sendSOPInstances();
						if (status.bad())
						{
							DCMNET_FATAL("cannot send SOP instance: " << status.text());
							//还原临时数据
							//if (GetSCPInstance()->GetAssociationInstance())
							//{
							//	GetSCPInstance()->GetAssociationInstance()->DULassociation = NULL;
							//	GetSCPInstance()->GetAssociationInstance()->params = NULL;
							//	GetSCPInstance()->GetAssociationInstance()->sendPDVBuffer = NULL;
							//}
							return status;
						}
					}

					//还原临时数据
					//if (GetSCPInstance()->GetAssociationInstance())
					//{
					//	GetSCPInstance()->GetAssociationInstance()->DULassociation = NULL;
					//	GetSCPInstance()->GetAssociationInstance()->params = NULL;
					//	GetSCPInstance()->GetAssociationInstance()->sendPDVBuffer = NULL;
					//}

					if (status.bad() && (status == NET_EC_NoPresentationContextsDefined))
					{
						status = EC_Normal;
					}

				}
				else
				{
					//发送DIMSE_C_GET_RSP
					//参照C-ECHO写下范例
					T_DIMSE_Message rsp;
					bzero((char*)&rsp, sizeof(rsp));

					rsp.CommandField = DIMSE_C_GET_RSP;
					rsp.msg.CGetRSP.MessageIDBeingRespondedTo = incomingMsg->msg.CGetRQ.MessageID;
					OFStandard::strlcpy(rsp.msg.CGetRSP.AffectedSOPClassUID,
						incomingMsg->msg.CGetRQ.AffectedSOPClassUID, sizeof(rsp.msg.CGetRSP.AffectedSOPClassUID));
					rsp.msg.CEchoRSP.opts = O_GET_NUMBEROFCOMPLETEDSUBOPERATIONS;
					rsp.msg.CEchoRSP.DataSetType = DIMSE_DATASET_NULL;
					rsp.msg.CEchoRSP.DimseStatus = STATUS_Success;

					status = DIMSE_sendMessageUsingMemoryData(GetSCPInstance()->GetAssociationInstance(), presInfo.presentationContextID, &rsp, NULL, NULL, NULL, NULL);

				}

			}

			if (status.bad())
			{
				DCMNET_INFO(" Sending C-GET Response (failed)");
			}
			else
			{
				DCMNET_INFO(" Sending C-GET Response (Success)");
			}
		}
	}

	return status;
}

void CProcessCGET::NotifyRecvCFINDRequest(const T_DIMSE_C_GetRQ & getRQ)
{
	DCMNET_INFO("Receive C-GET :AffectedSOPClassUID= " << getRQ.AffectedSOPClassUID);
}
