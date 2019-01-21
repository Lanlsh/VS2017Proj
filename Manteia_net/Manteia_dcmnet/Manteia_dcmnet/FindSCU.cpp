#include "stdafx.h"
#include "FindSCU.h"
#include "dcmtk/ofstd/ofstd.h"       /* for OFStandard functions */
#include "dcmtk/dcmnet/cond.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/ofstd/oflist.h"
#include "common.h"

typedef enum {
	QMPatientRoot = 0,
	QMStudyRoot = 1,
	QMPatientStudyOnly = 2
} QueryModel;

typedef struct {
	const char *findSyntax;
	const char *moveSyntax;
} QuerySyntax;

static QuerySyntax querySyntax[3] = {
	{ UID_FINDPatientRootQueryRetrieveInformationModel,
	  UID_MOVEPatientRootQueryRetrieveInformationModel },
	{ UID_FINDStudyRootQueryRetrieveInformationModel,
	  UID_MOVEStudyRootQueryRetrieveInformationModel },
	{ UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,
	  UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel }
};

CFindSCU::CFindSCU(char* pSelfAETitle, char* pDesAETitle, char* pDesPeerHostName, int selfPort, int desPort)
{
	assert(pSelfAETitle);
	assert(pDesAETitle);
	assert(pDesPeerHostName);
	assert((desPort >= 0) && (desPort <= 65535));
	assert((selfPort >= 0) && (selfPort <= 65535));

	//初始化自己的AETitle
	size_t selfAETitleSize = (sizeof(char) * strlen(pSelfAETitle) + 1);
	m_pSelfAETitle = (char*)malloc(selfAETitleSize);
	memset(m_pSelfAETitle, 0, selfAETitleSize);
	memcpy(m_pSelfAETitle, pSelfAETitle, strlen(pSelfAETitle));

	//初始化目的地AETitle
	size_t desAETitleSize = sizeof(char)* (strlen(pDesAETitle) + 1);
	m_pDesAETitle = (char*)malloc(desAETitleSize);
	memset(m_pDesAETitle, 0, desAETitleSize);
	memcpy(m_pDesAETitle, pDesAETitle, strlen(pDesAETitle));

	//初始化目的地PeerHostName
	size_t desPeerHostNameSize = sizeof(char)* (strlen(pDesPeerHostName) + 1);
	m_pDesPeerHostName = (char*)malloc(desPeerHostNameSize);
	memset(m_pDesPeerHostName, 0, desPeerHostNameSize);
	memcpy(m_pDesPeerHostName, pDesPeerHostName, strlen(pDesPeerHostName));

	m_desPort = OFstatic_cast(Uint16, desPort);
	m_slefPort = OFstatic_cast(Uint16, selfPort);

	bool bInit = InitFindSCU();
	if (!bInit)
	{
		assert(false);
	}

}


CFindSCU::~CFindSCU()
{
	SCUDELETE(m_pSelfAETitle);
	SCUDELETE(m_pDesAETitle);
	SCUDELETE(m_pDesPeerHostName);
}

bool CFindSCU::InitFindSCU()
{
	OFCmdUnsignedInt opt_timeout = 0;
	OFCmdUnsignedInt opt_dimseTimeout = 0;	//此参数与“T_DIMSE_BlockingMode opt_blockingMode”对应！！！当其锁模式变化时，超时时间设置要做相应改变，否则将引起连接中断！！！
	OFCmdUnsignedInt opt_acseTimeout = 30;	//等待SCP回应处理的超时时间  0为无限等待  单位为秒
	OFCmdUnsignedInt opt_maxReceivePDULength = ASC_DEFAULTMAXPDU;
	OFCmdUnsignedInt opt_maxSendPDULength = 0;
	T_DIMSE_BlockingMode opt_blockMode = DIMSE_BLOCKING;// DIMSE_NONBLOCKING;// DIMSE_BLOCKING;
#ifdef WITH_ZLIB
	OFCmdUnsignedInt opt_compressionLevel = 0;
#endif

	OFBool opt_showPresentationContexts = OFFalse;
	OFBool opt_haltOnUnsuccessfulStore = OFTrue;
	OFBool opt_allowIllegalProposal = OFTrue;


	OFBool opt_scanDir = OFFalse;
	OFBool opt_recurse = OFFalse;

	/* set network parameters */
	setPeerHostName(m_pDesPeerHostName);
	setPeerPort(OFstatic_cast(Uint16, m_desPort));
	setPeerAETitle(m_pDesAETitle);
	setAETitle(m_pSelfAETitle);
	setMaxReceivePDULength(OFstatic_cast(Uint32, opt_maxReceivePDULength));
	setACSETimeout(OFstatic_cast(Uint32, opt_acseTimeout));
	setDIMSETimeout(OFstatic_cast(Uint32, opt_dimseTimeout));
	setDIMSEBlockingMode(opt_blockMode);
	setVerbosePCMode(opt_showPresentationContexts);

	return true;
}

bool CFindSCU::ExecuteCFIND(std::map<DcmTagKey, std::string>& moveCondition)
{
	if (moveCondition.empty())
	{
		OFLOG_INFO(manteiaLogger, "moveCondition is empty");
		return false;
	}
	else
	{
		std::map<DcmTagKey, std::string>::iterator iter = moveCondition.begin();
		for (; iter != moveCondition.end(); iter++)
		{
			if (!iter->first.hasValidGroup())
			{
				OFLOG_INFO(manteiaLogger, "moveCondition map's first is a unhandled value: " << iter->first);
				return false;
			}
		}

	}

	/* make sure data dictionary is loaded */
	if (!dcmDataDict.isDictionaryLoaded())
	{
		OFLOG_WARN(manteiaLogger, "no data dictionary loaded, check environment variable: "
			<< DCM_DICT_ENVIRONMENT_VARIABLE);
	}

	OFCondition status;
	OFString temp_str;
	QueryModel opt_queryModel = QMPatientRoot;
	/* We prefer explicit transfer syntaxes.
	 * If we are running on a Little Endian machine we prefer
	 * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
	 */
	OFList<OFString> transferSyntaxes;
	if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
	{
		transferSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		transferSyntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
	}
	else {
		transferSyntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
		transferSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
	}
	transferSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);

	status = addPresentationContext(querySyntax[opt_queryModel].findSyntax, transferSyntaxes);
	if (status.bad()) {
		OFLOG_FATAL(manteiaLogger, DimseCondition::dump(temp_str, status));
		return false;
	}

	OFLOG_INFO(manteiaLogger, "initializing network ...");
	/* initialize network */
	status = initNetwork();
	if (status.bad())
	{
		OFLOG_FATAL(manteiaLogger, "cannot initialize network: " << status.text());
		return false;
	}
	OFLOG_INFO(manteiaLogger, "negotiating network association ...");
	/* negotiate network association with peer */
	status = negotiateAssociation();

	if (status.bad())
	{
		OFLOG_FATAL(manteiaLogger, "cannot negotiate network association: " << status.text());
		return false;
	}

	//发送C-FIND
	//do real work
	DcmDataset queryData;
	//queryData.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");
	//queryData.putAndInsertOFStringArray(DCM_PatientID, OFString("ANON98541"));
	std::map<DcmTagKey, std::string>::iterator iter = moveCondition.begin();
	for (; iter != moveCondition.end(); iter++)
	{
		OFString str(iter->second.c_str(), iter->second.length());
		queryData.putAndInsertOFStringArray(iter->first, str);
	}

	OFList<QRResponse*> lstResponses;
	T_ASC_PresentationContextID presID = findPresentationContextID(querySyntax[opt_queryModel].findSyntax, "");
	status = sendFINDRequest(presID, &queryData, &lstResponses);

	if (status.good())
	{
		OFListIterator(QRResponse*) iter = lstResponses.begin();
		for (; iter != lstResponses.end(); iter++)
		{
			if ((*iter)->m_status == STATUS_Pending)
			{
				DCMNET_INFO("m_affectedSOPClassUID: " << (*iter)->m_affectedSOPClassUID);
				DCMNET_INFO("m_dataset: " << (*iter)->m_dataset);
				if ((*iter)->m_dataset)
				{
					OFString patientID;
					(*iter)->m_dataset->findAndGetOFStringArray(DCM_PatientID, patientID);
					DCMNET_INFO("DCM_PatientID: " << patientID);
				}
			}

		}

		/* close current network association */
		releaseAssociation();
	}
	else
	{
		if (status == DUL_PEERREQUESTEDRELEASE)
		{
			// peer requested release (aborting)
			closeAssociation(CSCUBase_PEER_REQUESTED_RELEASE);
		}
		else if (status == DUL_PEERABORTEDASSOCIATION)
		{
			// peer aborted the association
			closeAssociation(CSCUBase_PEER_ABORTED_ASSOCIATION);
		}

		// destroy and free memory of internal association and network structures
		freeNetwork();

		return false;
	}

	// destroy and free memory of internal association and network structures
	//freeNetwork();

	return true;
}

OFCondition CFindSCU::handleFINDResponse(const T_ASC_PresentationContextID presID, QRResponse * response, OFBool & waitForNextResponse)
{
	if (!isConnected())
		return DIMSE_ILLEGALASSOCIATION;
	if (response == NULL)
		return DIMSE_NULLKEY;

	DCMNET_INFO("Handling C-FIND Response");
	switch (response->m_status) {
	case STATUS_Pending:
	case STATUS_FIND_Pending_WarningUnsupportedOptionalKeys:
		/* in this case the current C-FIND-RSP indicates that */
		/* there will be some more results */
		waitForNextResponse = OFTrue;
		DCMNET_INFO("One or more pending C-FIND responses");
		break;
	case STATUS_Success:
		/* in this case the current C-FIND-RSP indicates that */
		/* there are no more records that match the search mask */
		waitForNextResponse = OFFalse;
		DCMNET_INFO("Received final C-FIND response, no more C-FIND responses expected");
		break;
	case STATUS_FIND_Failed_UnableToProcess:
		/* in this case the current C-FIND-RSP indicates that */
		/* there are no more records that match the search mask */
		waitForNextResponse = OFFalse;
		DCMNET_INFO("Received final C-FIND response:  SCP process failed, STATUS_FIND_Failed_UnableToProcess");
		break;
	default:
		/* in all other cases, don't expect further responses to come */
		waitForNextResponse = OFFalse;
		DCMNET_INFO("Status tells not to wait for further C-FIND responses");
		break;
	} //switch
	return EC_Normal;
}
