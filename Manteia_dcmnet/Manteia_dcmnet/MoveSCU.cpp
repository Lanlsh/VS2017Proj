#include "MoveSCU.h"
#include "dcmtk/ofstd/ofstd.h"       /* for OFStandard functions */
#include "dcmtk/dcmnet/cond.h"
#include "dcmtk/dcmnet/diutil.h"
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

CMoveSCU::CMoveSCU(char* pSelfAETitle, char* pDesAETitle, char* pDesPeerHostName, char* pOutputFileDirectory, int selfPort, int desPort)
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

	//初始化输出文件路径
	size_t outputFileDirectorySize = sizeof(char)* (strlen(pOutputFileDirectory) + 1);
	m_pOutputFileDirectory = (char*)malloc(outputFileDirectorySize);
	memset(m_pOutputFileDirectory, 0, outputFileDirectorySize);
	memcpy(m_pOutputFileDirectory, pOutputFileDirectory, strlen(pOutputFileDirectory));

	m_desPort = OFstatic_cast(Uint16, desPort);
	m_slefPort = OFstatic_cast(Uint16, selfPort);

	bool bInit = InitMoveSCU();
	if (!bInit)
	{
		assert(false);
	}
}


CMoveSCU::~CMoveSCU()
{
	SCUDELETE(m_pSelfAETitle);
	SCUDELETE(m_pDesAETitle);
	SCUDELETE(m_pDesPeerHostName);
	SCUDELETE(m_pOutputFileDirectory);
}

bool CMoveSCU::InitMoveSCU()
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

bool CMoveSCU::ExecuteCMOVE(std::map<DcmTagKey, std::string>& moveCondition)
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

	//status = addPresentationContext(querySyntax[opt_queryModel].findSyntax, transferSyntaxes);
	status = addPresentationContext(querySyntax[opt_queryModel].moveSyntax, transferSyntaxes);
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

	//发送C-MOVE
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

	OFList<RetrieveResponse*> lstResponses;
	//RetrieveResponse response;
	//lstResponses.push_back(&response);
	T_ASC_PresentationContextID presID = findPresentationContextID(querySyntax[opt_queryModel].moveSyntax, "");
	status = sendMOVERequest(presID, m_pDesAETitle, &queryData, &lstResponses);

	if (status.good())
	{
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

OFCondition CMoveSCU::handleMOVEResponse(const T_ASC_PresentationContextID presID, RetrieveResponse * response, OFBool & waitForNextResponse)
{
	// Do some basic validity checks
	if (!isConnected())
		return DIMSE_ILLEGALASSOCIATION;
	if (response == NULL)
		return DIMSE_NULLKEY;

	DCMNET_INFO("Handling C-MOVE Response");
	switch (response->m_status) {
	case STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass:
		waitForNextResponse = OFFalse;
		DCMNET_ERROR("Identifier does not match SOP class in C-MOVE response");
		break;
	case STATUS_MOVE_Failed_MoveDestinationUnknown:
		waitForNextResponse = OFFalse;
		DCMNET_ERROR("Move destination unknown");
		break;
	case STATUS_MOVE_Failed_UnableToProcess:
		waitForNextResponse = OFFalse;
		DCMNET_ERROR("Unable to process C-Move response");
		break;
	case STATUS_MOVE_Cancel_SubOperationsTerminatedDueToCancelIndication:
		waitForNextResponse = OFFalse;
		DCMNET_INFO("Suboperations canceled by server due to CANCEL indication");
		break;
	case STATUS_MOVE_Warning_SubOperationsCompleteOneOrMoreFailures:
		waitForNextResponse = OFFalse;
		DCMNET_WARN("Suboperations of C-MOVE completed with one or more failures");
		break;
	case STATUS_Pending:
		/* in this case the current C-MOVE-RSP indicates that */
		/* there will be some more results */
		waitForNextResponse = OFTrue;
		DCMNET_INFO("One or more pending C-MOVE responses");
		break;
	case STATUS_Success:
		/* in this case, we received the last C-MOVE-RSP so there */
		/* will be no other responses we have to wait for. */
		waitForNextResponse = OFFalse;
		DCMNET_INFO("Received final C-MOVE response, no more C-MOVE responses expected");
		break;
	default:
		/* in all other cases, don't expect further responses to come */
		waitForNextResponse = OFFalse;
		DCMNET_WARN("Status is 0x"
			<< STD_NAMESPACE hex << STD_NAMESPACE setfill('0') << STD_NAMESPACE setw(4)
			<< response->m_status << " (unknown)");
		DCMNET_WARN("Will not wait for further C-MOVE responses");
		break;
	} //switch

	return EC_Normal;
}

