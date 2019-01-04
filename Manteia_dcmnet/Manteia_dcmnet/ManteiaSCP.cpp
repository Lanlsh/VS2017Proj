#include "ManteiaSCP.h"
#include <assert.h>
#include "common.h"
#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/ofstd.h"       /* for OFStandard functions */
#include "dcmtk/ofstd/ofconapp.h"    /* for OFConsoleApplication */
#include "dcmtk/ofstd/ofstream.h"    /* for OFStringStream et al. */
#include "dcmtk/dcmdata/dcdict.h"    /* for global data dictionary */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/cmdlnarg.h"  /* for prepareCmdLineArgs */

CManteiaSCP::CManteiaSCP(char* pAETitle, char* pStorageRecvFilePath, int port, CallbackFunForDB callbackFunForDB,
	CallbackFunForRQ callbackFunForRQ, CallbackFunForRecvFile callbackFunForRecvFile)
	: m_proCSTORE(NULL),
	m_proCMOVE(NULL),
	m_proCFIND(NULL),
	m_proCGET(NULL),
	m_storeSCU(NULL),
	m_callbackFunForDB(NULL),
	m_callbackFunForRQ(NULL),
	m_callbackFunForRecvFile(NULL)
{
	assert(pAETitle);
	assert(pStorageRecvFilePath);
	assert((port >= 0) && (port <= 65535));

	//初始化自己的AETitle
	size_t AETitleSize = (sizeof(char) * strlen(pAETitle) + 1);
	m_pAETitle = (char*)malloc(AETitleSize);
	memset(m_pAETitle, 0, AETitleSize);
	memcpy(m_pAETitle, pAETitle, strlen(pAETitle));

	//初始化存储接收文件的路径
	size_t recvPathSize = sizeof(char)* (strlen(pStorageRecvFilePath) + 1);
	m_pStorageRecvFilePath = (char*)malloc(recvPathSize);
	memset(m_pStorageRecvFilePath, 0, recvPathSize);
	memcpy(m_pStorageRecvFilePath, pStorageRecvFilePath, strlen(pStorageRecvFilePath));

	m_port = OFstatic_cast(Uint16, port);

	if (callbackFunForDB)
	{
		m_callbackFunForDB = callbackFunForDB;
	}

	if (callbackFunForRQ)
	{
		m_callbackFunForRQ = callbackFunForRQ;
	}

	if (callbackFunForRecvFile)
	{
		m_callbackFunForRecvFile = callbackFunForRecvFile;
	}

	bool bInit = InitSCP();
	if (!bInit)
	{
		assert(false);
	}
}


CManteiaSCP::~CManteiaSCP()
{
	SCPDELETE(m_pAETitle);
	SCPDELETE(m_pStorageRecvFilePath);
	SCPDELETE(m_proCSTORE);
	SCPDELETE(m_proCMOVE);
	SCPDELETE(m_proCFIND);
	SCPDELETE(m_proCGET);
	SCPDELETE(m_storeSCU);
}

bool CManteiaSCP::InitSCP()
{
	m_storeSCU = new CStoreSCU(m_pAETitle);

	// make sure that the SCP at least supports C-ECHO with default transfer syntax
	OFList<OFString> transferSyntaxes;
	transferSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
	addPresentationContext(UID_VerificationSOPClass, transferSyntaxes);

	m_proCSTORE = new CProcessCSTORE(this);
	m_proCMOVE = new CProcessCMOVE(this);
	m_proCFIND = new CProcessCFIND(this);
	m_proCGET = new CProcessCGET(this);

	m_bIsStopRun = true;
	memset(m_chErrorMsg, 0, ERROR_MSG_SIZE);

	const char *opt_configFile = "E://lanliangsheng//dcmtk//dcmtk-3.6.2//dcmnet//etc//storescp.cfg";	//此到时路径设为相对路径！！！
	const char *opt_profileName = "AllDICOM";
	const char *opt_aeTitle = m_pAETitle;
	const char *opt_outputDirectory = m_pStorageRecvFilePath;


	OFCmdUnsignedInt opt_port = m_port;
	OFCmdUnsignedInt opt_dimseTimeout = 60;//60;	//此参数与“T_DIMSE_BlockingMode opt_blockingMode”对应！！！当其锁模式变化时，超时时间设置要做相应改变，否则将引起连接中断！！！
	OFCmdUnsignedInt opt_acseTimeout = 30;
	OFCmdUnsignedInt opt_maxPDULength = ASC_DEFAULTMAXPDU;
	T_DIMSE_BlockingMode opt_blockingMode = DIMSE_NONBLOCKING;// DIMSE_NONBLOCKING;//DIMSE_BLOCKING;

	OFBool opt_showPresentationContexts = OFFalse;  // default: do not show presentation contexts in verbose mode
	OFBool opt_useCalledAETitle = OFFalse;          // default: respond with specified application entity title
	OFBool opt_HostnameLookup = OFTrue;             // default: perform hostname lookup (for log output)



	/* make sure data dictionary is loaded */
	if (!dcmDataDict.isDictionaryLoaded())
	{
		OFLOG_WARN(manteiaLogger, "no data dictionary loaded, check environment variable: "
			<< DCM_DICT_ENVIRONMENT_VARIABLE);
	}

	/* start with the real work */
	OFCondition status;

	OFLOG_INFO(manteiaLogger, "configuring service class provider ...");

	/* set general network parameters */
	setPort(OFstatic_cast(Uint16, opt_port));
	setAETitle(opt_aeTitle);
	setMaxReceivePDULength(OFstatic_cast(Uint32, opt_maxPDULength));
	setACSETimeout(OFstatic_cast(Uint32, opt_acseTimeout));
	setDIMSETimeout(OFstatic_cast(Uint32, opt_dimseTimeout));

	setConnectionTimeout(OFstatic_cast(Uint32, 30));	//连接超时30s
	setConnectionBlockingMode(DUL_NOBLOCK);	//设置连接时的阻塞模式，此时，连接超时会返回	//不设置时，默认为阻塞模式，即不响应超时！！！

	setDIMSEBlockingMode(opt_blockingMode);
	setVerbosePCMode(opt_showPresentationContexts);
	setRespondWithCalledAETitle(opt_useCalledAETitle);
	setHostLookupEnabled(opt_HostnameLookup);


	/* load association negotiation profile from configuration file (if specified) */
	if ((opt_configFile != NULL) && (opt_profileName != NULL))
	{
		// first, try to load the configuration file
		status = loadAssociationCfgFile(opt_configFile);
		// and then, try to select the desired profile
		if (status.good())
			status = setAndCheckAssociationProfile(opt_profileName);

		if (status.bad())
		{
			//OFLOG_FATAL(manteiaLogger, "cannot load association configuration: " << status.text());
			InitialErrorMsg();
			sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "cannot load association configuration: %s\n", status.text());
			return false;
		}
	}
	else {
		/* report a warning message that the SCP will not accept any Storage SOP Classes */
		OFLOG_WARN(manteiaLogger, "no configuration file specified, SCP will only support the Verification SOP Class");
	}

	/* specify the output directory (also checks whether directory exists and is writable) */
	status = m_proCSTORE->setOutputDirectory(opt_outputDirectory);
	if (status.bad())
	{
		//OFLOG_FATAL(manteiaLogger, "cannot specify output directory: " << status.text());
		InitialErrorMsg();
		sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "cannot specify output directory: %s\n", status.text());
		return false;
	}

	return true;
}

bool CManteiaSCP::StartSCP()
{
	m_bIsStopRun = false;

	OFLOG_INFO(manteiaLogger, "starting service class provider and listening ...");

	/* start SCP and listen on the specified port */
	OFCondition status = listen();
	if (status.bad())
	{
		InitialErrorMsg();
		sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "cannot start SCP and listen on port %d %s\n", m_port, status.text());
		return false;
	}

	/* make sure that everything is cleaned up properly */
#ifdef DEBUG
	/* useful for debugging with dmalloc */
	dcmDataDict.clear();
#endif

	return true;
}

void CManteiaSCP::StopSCP()
{
	m_bIsStopRun = true;
}

OFBool CManteiaSCP::stopAfterConnectionTimeout()
{
	if (m_bIsStopRun)
	{
		return OFTrue;
	}

	return OFFalse;
}

OFBool CManteiaSCP::stopAfterCurrentAssociation()
{
	if (m_bIsStopRun)
	{
		return OFTrue;
	}

	return OFFalse;
}

void CManteiaSCP::notifyReleaseRequest(T_DIMSE_Command CommandField)
{
	//操作完成通知
	OFLOG_INFO(manteiaLogger, "over.................");
	if (m_callbackFunForRQ)
	{
		CallbackDataForRQ data;
		switch (CommandField)
		{
		case DIMSE_C_STORE_RQ:
			data.type = OT_CSTORE;
			break;
		case DIMSE_C_GET_RQ:
			data.type = OT_CGET;
			break;
		case DIMSE_C_MOVE_RQ:
			data.type = OT_CMOVE;
			break;
		case DIMSE_C_FIND_RQ:
			data.type = OT_CFIND;
			break;
		case DIMSE_C_ECHO_RQ:
			data.type = OT_CECHO;
			break;
		default:
			data.type = OT_UNKNOWN;
			break;
		}

		m_callbackFunForRQ(&data);
	}
}

OFCondition CManteiaSCP::handleIncomingCommand(T_DIMSE_Message * incomingMsg, const DcmPresentationContextInfo & presInfo)
{
	OFCondition status = EC_IllegalParameter;

	if (incomingMsg != NULL)
	{
		// check whether we've received a supported command
		switch (incomingMsg->CommandField)
		{
		case DIMSE_C_ECHO_RQ:
			// handle incoming C-ECHO request
			status = handleECHORequest(incomingMsg->msg.CEchoRQ, presInfo.presentationContextID);
			break;
		case DIMSE_C_STORE_RQ:
			// handle incoming C-STORE request
			status = m_proCSTORE->handleIncomingCommand(incomingMsg, presInfo);
			break;
		case DIMSE_C_FIND_RQ:
			// handle incoming C-FIND_RQ request
			status = m_proCFIND->handleIncomingCommand(incomingMsg, presInfo);
			break;
		case DIMSE_C_MOVE_RQ:
			// handle incoming C-MOVE request
			status = m_proCMOVE->handleIncomingCommand(incomingMsg, presInfo);
			break;
		case DIMSE_C_GET_RQ:
			//handle incoming C-GET request
			status = m_proCGET->handleIncomingCommand(incomingMsg, presInfo);
			break;
		default:
			break;
		}
	}

	return status;
}

bool CManteiaSCP::SetAETitle(const char * pAETitle)
{
	assert(pAETitle);

	SCPDELETE(m_pAETitle);

	//初始化自己的AETitle
	m_pAETitle = (char*)malloc(sizeof(char)* (strlen(pAETitle) + 1));
	memset(m_pAETitle, 0, sizeof(strlen(pAETitle) + 1));
	memcpy(m_pAETitle, pAETitle, strlen(pAETitle));
	setAETitle(m_pAETitle);

	return true;
}

bool CManteiaSCP::SetRecvPath(const char * pRecvPath)
{
	assert(pRecvPath);

	SCPDELETE(m_pStorageRecvFilePath);

	//初始化自己的AETitle
	m_pStorageRecvFilePath = (char*)malloc(sizeof(char)* (strlen(pRecvPath) + 1));
	memset(m_pStorageRecvFilePath, 0, sizeof(strlen(pRecvPath) + 1));
	memcpy(m_pStorageRecvFilePath, pRecvPath, strlen(pRecvPath));

	/* specify the output directory (also checks whether directory exists and is writable) */
	OFCondition status = m_proCSTORE->setOutputDirectory(m_pStorageRecvFilePath);
	if (status.bad())
	{
		InitialErrorMsg();
		sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "cannot specify output directory: %s\n", status.text());
		//OFLOG_FATAL(manteiaLogger, "cannot specify output directory: " << status.text());
		return false;
	}

	return true;
}

bool CManteiaSCP::SetPort(int port)
{
	if ((port < 0) || (port > 65535))
	{
		InitialErrorMsg();
		sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "SetPort error: port value is a error value!!\n");
		return false;
	}

	setPort(OFstatic_cast(Uint16, port));
	return true;
}

CallbackFunForDB CManteiaSCP::GetCallbackFunForDB()
{
	return m_callbackFunForDB;
}

CallbackFunForRecvFile CManteiaSCP::GetCalllbackForRecvFile()
{
	return m_callbackFunForRecvFile;
}

OFCondition CManteiaSCP::DoReceiveSTORERequest(T_DIMSE_C_StoreRQ & reqMessage, const T_ASC_PresentationContextID presID, DcmDataset *& reqDataset)
{
	return receiveSTORERequest(reqMessage, presID, reqDataset);
}

OFCondition CManteiaSCP::DoReceiveSTORERequest(T_DIMSE_C_StoreRQ &reqMessage,
	const T_ASC_PresentationContextID presID,
	const OFString &filename)
{
	return receiveSTORERequest(reqMessage, presID, filename);
}

OFCondition CManteiaSCP::DoSendSTOREResponse(const T_ASC_PresentationContextID presID, const T_DIMSE_C_StoreRQ & reqMessage, const Uint16 rspStatusCode)
{
	return sendSTOREResponse(presID, reqMessage, rspStatusCode);
}

bool CManteiaSCP::ExecuteCSTOREService(char * pDesAETitle, char * pDesPeerHostName, int desPort, std::vector<std::string> inputFiles)
{
	m_storeSCU->SetDesInfo(pDesAETitle, pDesPeerHostName, desPort);
	return m_storeSCU->ExecuteCSTORE(inputFiles);
}

OFCondition CManteiaSCP::DoReceiveMOVERequest(T_DIMSE_C_MoveRQ & reqMessage, const T_ASC_PresentationContextID presID, DcmDataset *& reqDataset, OFString & moveDest)
{
	return receiveMOVERequest(reqMessage, presID, reqDataset, moveDest);
}

OFCondition CManteiaSCP::DoSendMOVEResponse(const T_ASC_PresentationContextID presID, const Uint16 messageID, const OFString & sopClassUID, DcmDataset * rspDataset, const Uint16 rspStatusCode, DcmDataset * statusDetail, const Uint16 numRemain, const Uint16 numComplete, const Uint16 numFail, const Uint16 numWarn)
{
	return sendMOVEResponse(presID, messageID, sopClassUID, rspDataset, rspStatusCode, statusDetail,
		numRemain, numComplete, numFail, numWarn);
}

OFCondition CManteiaSCP::DoReceiveFINDRequest(T_DIMSE_C_FindRQ & reqMessage, const T_ASC_PresentationContextID presID, DcmDataset *& reqDataset)
{
	return receiveFINDRequest(reqMessage, presID, reqDataset);
}

OFCondition CManteiaSCP::DoSendFINDResponse(const T_ASC_PresentationContextID presID, const Uint16 messageID, const OFString & sopClassUID, DcmDataset * rspDataset, const Uint16 rspStatusCode, DcmDataset * statusDetail)
{
	return sendFINDResponse(presID, messageID, sopClassUID, rspDataset, rspStatusCode, statusDetail);
}

OFCondition CManteiaSCP::DoReceiveGETRequest(T_DIMSE_C_GetRQ & reqMessage, const T_ASC_PresentationContextID presID, DcmDataset *& reqDataset)
{
	// Do some basic validity checks
	if (GetAssociationInstance() == NULL)
		return DIMSE_ILLEGALASSOCIATION;

	OFCondition cond;
	OFString tempStr;
	T_ASC_PresentationContextID presIDdset;
	DcmDataset *dataset = NULL;

	// Dump debug information
	if (DCM_dcmnetLogger.isEnabledFor(OFLogger::DEBUG_LOG_LEVEL))
		DCMNET_INFO("Received C-GET Request");
	else
		DCMNET_INFO("Received C-GET Request (MsgID " << reqMessage.MessageID << ")");

	// Check if dataset is announced correctly
	if (reqMessage.DataSetType == DIMSE_DATASET_NULL)
	{
		DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, reqMessage, DIMSE_INCOMING, NULL, presID));
		DCMNET_ERROR("Received C-GET request but no dataset announced, aborting");
		return DIMSE_BADMESSAGE;
	}

	// Receive dataset
	cond = receiveDIMSEDataset(&presIDdset, &dataset);
	if (cond.bad())
	{
		DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, reqMessage, DIMSE_INCOMING, NULL, presID));
		DCMNET_ERROR("Unable to receive C-GET dataset on presentation context " << OFstatic_cast(unsigned int, presID));
		return DIMSE_BADDATA;
	}

	// Output request message only if trace level is enabled
	if (DCM_dcmnetLogger.isEnabledFor(OFLogger::TRACE_LOG_LEVEL))
		DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, reqMessage, DIMSE_INCOMING, dataset, presID));
	else
		DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, reqMessage, DIMSE_INCOMING, NULL, presID));

	// Compare presentation context ID of command and data set
	if (presIDdset != presID)
	{
		DCMNET_ERROR("Presentation Context ID of command (" << OFstatic_cast(unsigned int, presID)
			<< ") and data set (" << OFstatic_cast(unsigned int, presIDdset) << ") differs");
		delete dataset;
		return makeDcmnetCondition(DIMSEC_INVALIDPRESENTATIONCONTEXTID, OF_error,
			"DIMSE: Presentation Contexts of Command and Data Set differ");
	}

	// Set return value
	reqDataset = dataset;

	return cond;
}

void CManteiaSCP::InitialErrorMsg()
{
	memset(m_chErrorMsg, 0, ERROR_MSG_SIZE);
}

