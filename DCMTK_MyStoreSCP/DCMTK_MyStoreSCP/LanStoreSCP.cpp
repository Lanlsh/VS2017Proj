#include "LanStoreSCP.h"
#include <assert.h>
#include "pch.h"

#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/ofstd.h"       /* for OFStandard functions */
#include "dcmtk/ofstd/ofconapp.h"    /* for OFConsoleApplication */
#include "dcmtk/ofstd/ofstream.h"    /* for OFStringStream et al. */
#include "dcmtk/dcmdata/dcdict.h"    /* for global data dictionary */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/cmdlnarg.h"  /* for prepareCmdLineArgs */

#define OFFIS_CONSOLE_APPLICATION "landcmrecv"

static OFLogger dcmrecvLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

//#define APPLICATIONTITLE "LANDCMRECV"

#define SHORTCOL 4
#define LONGCOL 21

/* helper macro for converting stream output to a string */
#define CONVERT_TO_STRING(output, string) \
    optStream.str(""); \
    optStream.clear(); \
    optStream << output << OFStringStream_ends; \
    OFSTRINGSTREAM_GETOFSTRING(optStream, string)

// network errors
#define EXITCODE_CANNOT_INITIALIZE_NETWORK       60      // placeholder, currently not used
#define EXITCODE_CANNOT_START_SCP_AND_LISTEN     64
#define EXITCODE_INVALID_ASSOCIATION_CONFIG      66

#define SCPDELETE(p) \
		if(p) delete p;\
		p = NULL;

LanStoreSCP::LanStoreSCP(char* pAETitle, char* pStorageRecvFilePath, int port)
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

	bool bInit = InitSCP();
	if (!bInit)
	{
		assert(false);
	}
}


LanStoreSCP::~LanStoreSCP()
{
	SCPDELETE(m_pAETitle);
	SCPDELETE(m_pStorageRecvFilePath);
}

bool LanStoreSCP::InitSCP()
{
	m_bIsStopRun = true;
	memset(m_chErrorMsg, 0, ERROR_MSG_SIZE);

	const char *opt_configFile = "E://lanliangsheng//dcmtk//dcmtk-3.6.2//dcmnet//etc//storescp.cfg";	//此到时路径设为相对路径！！！
	const char *opt_profileName = "AllDICOM";
	const char *opt_aeTitle = m_pAETitle;
	const char *opt_outputDirectory = m_pStorageRecvFilePath;
	const char *opt_filenameExtension = "";

	OFCmdUnsignedInt opt_port = m_port;
	OFCmdUnsignedInt opt_dimseTimeout = 60;	//此参数与“T_DIMSE_BlockingMode opt_blockingMode”对应！！！当其锁模式变化时，超时时间设置要做相应改变，否则将引起连接中断！！！
	OFCmdUnsignedInt opt_acseTimeout = 30;
	OFCmdUnsignedInt opt_maxPDULength = ASC_DEFAULTMAXPDU;
	T_DIMSE_BlockingMode opt_blockingMode = DIMSE_NONBLOCKING;// DIMSE_NONBLOCKING;//DIMSE_BLOCKING;

	OFBool opt_showPresentationContexts = OFFalse;  // default: do not show presentation contexts in verbose mode
	OFBool opt_useCalledAETitle = OFFalse;          // default: respond with specified application entity title
	OFBool opt_HostnameLookup = OFTrue;             // default: perform hostname lookup (for log output)

	DcmStorageSCP::E_DirectoryGenerationMode opt_directoryGeneration = DcmStorageSCP::DGM_NoSubdirectory;
	DcmStorageSCP::E_FilenameGenerationMode opt_filenameGeneration = DcmStorageSCP::FGM_SOPInstanceUID;
	DcmStorageSCP::E_DatasetStorageMode opt_datasetStorage = DcmStorageSCP::DGM_StoreToFile;

	/* make sure data dictionary is loaded */
	if (!dcmDataDict.isDictionaryLoaded())
	{
		OFLOG_WARN(dcmrecvLogger, "no data dictionary loaded, check environment variable: "
			<< DCM_DICT_ENVIRONMENT_VARIABLE);
	}

	/* start with the real work */
	OFCondition status;

	OFLOG_INFO(dcmrecvLogger, "configuring service class provider ...");

	/* set general network parameters */
	setPort(OFstatic_cast(Uint16, opt_port));
	setAETitle(opt_aeTitle);
	setMaxReceivePDULength(OFstatic_cast(Uint32, opt_maxPDULength));
	setACSETimeout(OFstatic_cast(Uint32, opt_acseTimeout));
	setDIMSETimeout(OFstatic_cast(Uint32, opt_dimseTimeout));

	setConnectionTimeout(OFstatic_cast(Uint32, 1));	//连接超时1s
	setConnectionBlockingMode(DUL_NOBLOCK);	//设置连接时的阻塞模式，此时，连接超时会返回	//不设置时，默认为阻塞模式，即不响应超时！！！

	setDIMSEBlockingMode(opt_blockingMode);
	setVerbosePCMode(opt_showPresentationContexts);
	setRespondWithCalledAETitle(opt_useCalledAETitle);
	setHostLookupEnabled(opt_HostnameLookup);
	setDirectoryGenerationMode(opt_directoryGeneration);
	setFilenameGenerationMode(opt_filenameGeneration);
	setFilenameExtension(opt_filenameExtension);
	setDatasetStorageMode(opt_datasetStorage);

	/* load association negotiation profile from configuration file (if specified) */
	if ((opt_configFile != NULL) && (opt_profileName != NULL))
	{
		status = loadAssociationConfiguration(opt_configFile, opt_profileName);
		if (status.bad())
		{
			//OFLOG_FATAL(dcmrecvLogger, "cannot load association configuration: " << status.text());
			InitialErrorMsg();
			sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "cannot load association configuration: %s\n", status.text());
			return false;
		}
	}
	else {
		/* report a warning message that the SCP will not accept any Storage SOP Classes */
		OFLOG_WARN(dcmrecvLogger, "no configuration file specified, SCP will only support the Verification SOP Class");
	}

	/* specify the output directory (also checks whether directory exists and is writable) */
	status = setOutputDirectory(opt_outputDirectory);
	if (status.bad())
	{
		//OFLOG_FATAL(dcmrecvLogger, "cannot specify output directory: " << status.text());
		InitialErrorMsg();
		sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "cannot specify output directory: %s\n", status.text());
		return false;
	}

	return true;
}

bool LanStoreSCP::StartSCP()
{
	m_bIsStopRun = false;

	OFLOG_INFO(dcmrecvLogger, "starting service class provider and listening ...");

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

void LanStoreSCP::StopSCP()
{
	m_bIsStopRun = true;
}

OFBool LanStoreSCP::stopAfterConnectionTimeout()
{
	if (m_bIsStopRun)
	{
		return OFTrue;
	}

	return OFFalse;
}

OFBool LanStoreSCP::stopAfterCurrentAssociation()
{
	if (m_bIsStopRun)
	{
		return OFTrue;
	}

	return OFFalse;
}

void LanStoreSCP::notifyReleaseRequest()
{
	//操作完成通知
	OFLOG_INFO(dcmrecvLogger, "over.................");
}

bool LanStoreSCP::SetAETitle(const char * pAETitle)
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

bool LanStoreSCP::SetRecvPath(const char * pRecvPath)
{
	assert(pRecvPath);

	SCPDELETE(m_pStorageRecvFilePath);

	//初始化自己的AETitle
	m_pStorageRecvFilePath = (char*)malloc(sizeof(char)* (strlen(pRecvPath) + 1));
	memset(m_pStorageRecvFilePath, 0, sizeof(strlen(pRecvPath) + 1));
	memcpy(m_pStorageRecvFilePath, pRecvPath, strlen(pRecvPath));
	
	/* specify the output directory (also checks whether directory exists and is writable) */
	OFCondition status = setOutputDirectory(m_pStorageRecvFilePath);
	if (status.bad())
	{
		InitialErrorMsg();
		sprintf_s(m_chErrorMsg, ERROR_MSG_SIZE, "cannot specify output directory: %s\n", status.text());
		//OFLOG_FATAL(dcmrecvLogger, "cannot specify output directory: " << status.text());
		return false;
	}

	return true;
}

bool LanStoreSCP::SetPort(int port)
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

void LanStoreSCP::InitialErrorMsg()
{
	memset(m_chErrorMsg, 0, ERROR_MSG_SIZE);
}
