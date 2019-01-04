#include "StoreSCU.h"

#include "dcmtk/ofstd/ofstd.h"       /* for OFStandard functions */
#include "dcmtk/ofstd/ofconapp.h"    /* for OFConsoleApplication */
#include "dcmtk/ofstd/ofstream.h"    /* for OFStringStream et al. */
#include "dcmtk/dcmdata/dcdict.h"    /* for global data dictionary */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/cmdlnarg.h"  /* for prepareCmdLineArgs */
#include "dcmtk/dcmdata/dcostrmz.h"  /* for dcmZlibCompressionLevel */

#include "dcmtk/dcmjpeg/djdecode.h"  /* for JPEG decoders */
#include "dcmtk/dcmjpls/djdecode.h"  /* for JPEG-LS decoders */
#include "dcmtk/dcmdata/dcrledrg.h"  /* for RLE decoder */
#include "dcmtk/dcmjpeg/dipijpeg.h"  /* for dcmimage JPEG plugin */

#define OFFIS_CONSOLE_APPLICATION "STORESCU"

static OFLogger manteiaLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

#define SCUDELETE(p) \
		if(p) delete p;\
		p = NULL;

CStoreSCU::CStoreSCU(char * pOurAETitle) :
	m_pDesAETitle(NULL),
	m_desPort(0),
	m_pDesPeerHostName(NULL)
{
	assert(pOurAETitle);
	//初始化自己的AETitle
	size_t ourAETitleSize = (sizeof(char) * strlen(pOurAETitle) + 1);
	m_pOurAETitle = (char*)malloc(ourAETitleSize);
	memset(m_pOurAETitle, 0, ourAETitleSize);
	memcpy(m_pOurAETitle, pOurAETitle, strlen(pOurAETitle));
}

CStoreSCU::CStoreSCU(char* pOurAETitle, char* pDesAETitle, char* pDesPeerHostName, int port)
{
	assert(pOurAETitle);
	assert(pDesAETitle);
	assert(pDesPeerHostName);
	assert((port >= 0) && (port <= 65535));

	//初始化自己的AETitle
	size_t ourAETitleSize = (sizeof(char) * strlen(pOurAETitle) + 1);
	m_pOurAETitle = (char*)malloc(ourAETitleSize);
	memset(m_pOurAETitle, 0, ourAETitleSize);
	memcpy(m_pOurAETitle, pOurAETitle, strlen(pOurAETitle));

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

	m_desPort = OFstatic_cast(Uint16, port);

	// register JPEG decoder
	DJDecoderRegistration::registerCodecs();
	// register JPEG-LS decoder
	DJLSDecoderRegistration::registerCodecs();
	// register RLE decoder
	DcmRLEDecoderRegistration::registerCodecs();
}


CStoreSCU::~CStoreSCU()
{
	/* make sure that everything is cleaned up properly */
	CStoreSCU::cleanup();

	SCUDELETE(m_pOurAETitle);
	SCUDELETE(m_pDesAETitle);
	SCUDELETE(m_pDesPeerHostName);
}

bool CStoreSCU::InitStoreSCU()
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
	DcmStorageSCU::E_DecompressionMode opt_decompressionMode = DcmStorageSCU::DM_losslessOnly;


	OFBool opt_scanDir = OFFalse;
	OFBool opt_recurse = OFFalse;
	const char *opt_scanPattern = "";

	/* make sure data dictionary is loaded */
	if (!dcmDataDict.isDictionaryLoaded())
	{
		OFLOG_WARN(manteiaLogger, "no data dictionary loaded, check environment variable: "
			<< DCM_DICT_ENVIRONMENT_VARIABLE);
	}

	/* set network parameters */
	setPeerHostName(m_pDesPeerHostName);
	setPeerPort(OFstatic_cast(Uint16, m_desPort));
	setPeerAETitle(m_pDesAETitle);
	setAETitle(m_pOurAETitle);
	setMaxReceivePDULength(OFstatic_cast(Uint32, opt_maxReceivePDULength));
	setACSETimeout(OFstatic_cast(Uint32, opt_acseTimeout));
	setDIMSETimeout(OFstatic_cast(Uint32, opt_dimseTimeout));
	setDIMSEBlockingMode(opt_blockMode);
	setVerbosePCMode(opt_showPresentationContexts);
	setDatasetConversionMode(opt_decompressionMode != DcmStorageSCU::DM_never);
	setDecompressionMode(opt_decompressionMode);
	setHaltOnUnsuccessfulStoreMode(opt_haltOnUnsuccessfulStore);
	setAllowIllegalProposalMode(opt_allowIllegalProposal);

	return true;
}

void CStoreSCU::SetDesInfo(char * pDesAETitle, char * pDesPeerHostName, int desPort)
{
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
}

bool CStoreSCU::ExecuteCSTORE(std::vector<std::string> cstoreFiles)
{
	if (cstoreFiles.empty())
	{
		OFLOG_FATAL(manteiaLogger, "no input files to be processed");
		return false;
	}

	OFList<OFString> inputFiles;
	for (int i = 0; i < cstoreFiles.size(); i++)
	{
		OFString strFile(cstoreFiles[i].c_str());
		inputFiles.push_back(strFile);
	}

	/* check whether there are any input files at all */
	if (inputFiles.empty())
	{
		OFLOG_FATAL(manteiaLogger, "input files convert to OFString failed");
		return false;
	}

	bool bInit = InitStoreSCU();
	if (!bInit)
	{
		OFLOG_INFO(manteiaLogger, "InitStoreSCU() failed");
		return false;
	}

	OFCondition status;
	unsigned long numInvalidFiles = 0;
	E_FileReadMode opt_readMode = ERM_fileOnly;
	OFBool opt_dicomDir = OFFalse;
	OFBool opt_haltOnInvalidFile = OFFalse;
	OFBool opt_checkUIDValues = OFTrue;
	OFBool opt_multipleAssociations = OFTrue;

	/* set parameters used for processing the input files */

	setReadFromDICOMDIRMode(opt_dicomDir);
	setHaltOnInvalidFileMode(opt_haltOnInvalidFile);	//指定在批处理期间是否遇到无效文件时停止（例如，从DICOMDIR添加SOP实例时）或是否继续下一个SOP实例。

	OFLOG_INFO(manteiaLogger, "checking input files ...");
	/* iterate over all input filenames */
	const char *currentFilename = NULL;
	OFListIterator(OFString) if_iter = inputFiles.begin();
	OFListIterator(OFString) if_last = inputFiles.end();
	while (if_iter != if_last)
	{
		currentFilename = (*if_iter).c_str();
		/* and add them to the list of instances to be transmitted */
		status = addDicomFile(currentFilename, opt_readMode, opt_checkUIDValues);
		if (status.bad())
		{
			/* check for empty filename */
			if (strlen(currentFilename) == 0)
				currentFilename = "<empty string>";
			/* if something went wrong, we either terminate or ignore the file */
			if (opt_haltOnInvalidFile)
			{
				OFLOG_FATAL(manteiaLogger, "bad DICOM file: " << currentFilename << ": " << status.text());
				return false;
			}
			else {
				OFLOG_INFO(manteiaLogger, "bad DICOM file: " << currentFilename << ": " << status.text() << ", ignoring file");
			}
			++numInvalidFiles;
		}
		++if_iter;
	}

	/* check whether there are any valid input files */
	if (getNumberOfSOPInstances() == 0)
	{
		OFLOG_FATAL(manteiaLogger, "no valid input files to be processed");
		return false;
	}
	else {
		OFLOG_INFO(manteiaLogger, "in total, there are " << getNumberOfSOPInstances()
			<< " SOP instances to be sent, " << numInvalidFiles << " invalid files are ignored");
	}

	/* add presentation contexts to be negotiated (if there are still any) */
	while ((status = addPresentationContexts()).good())
	{
		if (opt_multipleAssociations)
		{
			/* output information on the start of the new association */
			if (manteiaLogger.isEnabledFor(OFLogger::DEBUG_LOG_LEVEL))
			{
				OFLOG_INFO(manteiaLogger, OFString(65, '-') << OFendl
					<< "starting association #" << (getAssociationCounter() + 1));
			}
			else {
				OFLOG_INFO(manteiaLogger, "starting association #" << (getAssociationCounter() + 1));
			}
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
			// check whether we can continue with a new association
			if (status == NET_EC_NoAcceptablePresentationContexts)
			{
				OFLOG_ERROR(manteiaLogger, "cannot negotiate network association: " << status.text());
				// check whether there are any SOP instances to be sent
				const size_t numToBeSent = getNumberOfSOPInstancesToBeSent();
				if (numToBeSent > 0)
				{
					OFLOG_WARN(manteiaLogger, "trying to continue with a new association "
						<< "in order to send the remaining " << numToBeSent << " SOP instances");
				}
			}
			else {
				OFLOG_FATAL(manteiaLogger, "cannot negotiate network association: " << status.text());
				return false;
			}
		}
		if (status.good())
		{
			OFLOG_INFO(manteiaLogger, "sending SOP instances ...");
			/* send SOP instances to be transferred */
			status = sendSOPInstances();
			if (status.bad())
			{
				OFLOG_FATAL(manteiaLogger, "cannot send SOP instance: " << status.text());
				// handle certain error conditions (initiated by the communication peer)
				if (status == DUL_PEERREQUESTEDRELEASE)
				{
					// peer requested release (aborting)
					closeAssociation(DCMSCU_PEER_REQUESTED_RELEASE);
				}
				else if (status == DUL_PEERABORTEDASSOCIATION)
				{
					// peer aborted the association
					closeAssociation(DCMSCU_PEER_ABORTED_ASSOCIATION);
				}
				return false;
			}
		}
		/* close current network association */
		releaseAssociation();
		/* check whether multiple associations are permitted */
		if (!opt_multipleAssociations)
			break;
	}

	/* if anything went wrong, report it to the logger */
	if (status.bad() && (status != NET_EC_NoPresentationContextsDefined))
	{
		OFLOG_ERROR(manteiaLogger, "cannot add presentation contexts: " << status.text());
		return false;
	}

	/* create a detailed report on the transfer of instances ... */
	const char *opt_reportFilename = "E://DcmScuScp//DcmScp//report.txt";	//当C-STORE成功时，记录相关信息
	if ((opt_reportFilename != NULL) && (strlen(opt_reportFilename) > 0))
	{
		/* ... and write it to the specified text file */
		status = createReportFile(opt_reportFilename);
		if (status.bad())
		{
			return false; // TODO: do we really want to exit?
		}
	}

	/* output some status information on the overall sending process */
	if (manteiaLogger.isEnabledFor(OFLogger::INFO_LOG_LEVEL))
	{
		OFString summaryText;
		getStatusSummary(summaryText);
		OFLOG_INFO(manteiaLogger, OFendl << summaryText);
	}

	return true;
}


void CStoreSCU::cleanup()
{
	// deregister JPEG decoder
	DJDecoderRegistration::cleanup();
	// deregister JPEG-LS decoder
	DJLSDecoderRegistration::cleanup();
	// deregister RLE decoder
	DcmRLEDecoderRegistration::cleanup();
#ifdef DEBUG
	/* useful for debugging with dmalloc */
	dcmDataDict.clear();
#endif
}
