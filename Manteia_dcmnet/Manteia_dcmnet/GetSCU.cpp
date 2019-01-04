#include "stdafx.h"
#include "GetSCU.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/ofstd/oflist.h"
#include "dcmtk/dcmnet/scu.h"
#include "dcmtk/dcmdata/dcuid.h"      /* for dcmtk version name */
#include "dcmtk/dcmdata/dcostrmz.h"   /* for dcmZlibCompressionLevel */
#include "dcmtk/dcmdata/dcpath.h"     /* for DcmPathProcessor */
#include "dcmtk/oflog/oflog.h"

#include "common.h"

static const char* querySyntax[3] = {
  UID_GETPatientRootQueryRetrieveInformationModel,
  UID_GETStudyRootQueryRetrieveInformationModel,
  UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel
};


CGetSCU::CGetSCU(char* pSelfAETitle, char* pDesAETitle, char* pDesPeerHostName, char* pOutputFileDirectory, int desPort)
{
	assert(pSelfAETitle);
	assert(pDesAETitle);
	assert(pDesPeerHostName);
	assert((desPort >= 0) && (desPort <= 65535));

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

	bool bInit = InitGETSCU();
	if (!bInit)
	{
		assert(false);
	}
}


CGetSCU::~CGetSCU()
{
	SCUDELETE(m_pSelfAETitle);
	SCUDELETE(m_pDesAETitle);
	SCUDELETE(m_pDesPeerHostName);
	SCUDELETE(m_pOutputFileDirectory);
}

bool CGetSCU::InitGETSCU()
{
	/* make sure output directory exists and is writeable */
	if (!OFStandard::dirExists(m_pOutputFileDirectory))
	{
		OFLOG_FATAL(manteiaLogger, "specified output directory does not exist");
		return 1;
	}
	else if (!OFStandard::isWriteable(m_pOutputFileDirectory))
	{
		OFLOG_FATAL(manteiaLogger, "specified output directory is not writeable");
		return 1;
	}

	OFCmdUnsignedInt        opt_maxPDU = ASC_DEFAULTMAXPDU;
	E_TransferSyntax        opt_store_networkTransferSyntax = EXS_Unknown;
	E_TransferSyntax        opt_get_networkTransferSyntax = EXS_Unknown;
	MyStorageMode           opt_storageMode = CSCUBase_STORAGE_DISK;
	OFBool                  opt_showPresentationContexts = OFFalse;
	OFBool                  opt_abortAssociation = OFTrue;//OFFalse;
	OFCmdUnsignedInt        opt_repeatCount = 1;
	QueryModel              m_queryModel = QMPatientRoot;
	T_DIMSE_BlockingMode    opt_blockMode = DIMSE_BLOCKING;
	int                     opt_dimse_timeout = 1000;//0
	int                     opt_acse_timeout = 1000;//30;

	/* setup SCU */
	OFList<OFString> syntaxes;
	PrepareTS(opt_get_networkTransferSyntax, syntaxes);
	setMaxReceivePDULength(opt_maxPDU);
	setACSETimeout(opt_acse_timeout);
	setDIMSEBlockingMode(opt_blockMode);
	setDIMSETimeout(opt_dimse_timeout);
	setAETitle(m_pSelfAETitle);
	setPeerHostName(m_pDesPeerHostName);
	setPeerPort(OFstatic_cast(Uint16, m_desPort));
	setPeerAETitle(m_pDesAETitle);
	setVerbosePCMode(opt_showPresentationContexts);

	/* add presentation contexts for get and find (we do not actually need find...)
 * (only uncompressed)
 */
	addPresentationContext(querySyntax[m_queryModel], syntaxes);

	/* add storage presentation contexts (long list of storage SOP classes, uncompressed) */
	//syntaxes.clear();
	//PrepareTS(opt_store_networkTransferSyntax, syntaxes);
	//for (Uint16 j = 0; j < numberOfDcmLongSCUStorageSOPClassUIDs; j++)
	//{
	//	addPresentationContext(dcmLongSCUStorageSOPClassUIDs[j], syntaxes, ASC_SC_ROLE_SCP);
	//}

	/* set the storage mode */
	setStorageMode(opt_storageMode);
	if (opt_storageMode != DCMSCU_STORAGE_IGNORE)
	{
		setStorageDir(m_pOutputFileDirectory);
	}

	return true;
}

bool CGetSCU::ExecuteCGET(std::map<DcmTagKey, std::string>& moveCondition)
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

	OFString temp_str;

	/* initialize network and negotiate association */
	OFCondition cond = initNetwork();
	if (cond.bad())
	{
		OFLOG_FATAL(manteiaLogger, DimseCondition::dump(temp_str, cond));
		return false;
	}
	cond = negotiateAssociation();
	if (cond.bad())
	{
		OFLOG_FATAL(manteiaLogger, "No Acceptable Presentation Contexts");
		return false;
	}
	cond = EC_Normal;
	T_ASC_PresentationContextID pcid = findPresentationContextID(querySyntax[m_queryModel], "");
	if (pcid == 0)
	{
		OFLOG_FATAL(manteiaLogger, "No adequate Presentation Contexts for sending C-GET");
		return false;
	}

	/* do the real work, i.e. send C-GET requests and receive objects */
	OFList<RetrieveResponse*> responses;

	DcmDataset dset;
	std::map<DcmTagKey, std::string>::iterator iter = moveCondition.begin();
	for (; iter != moveCondition.end(); iter++)
	{
		OFString str(iter->second.c_str(), iter->second.length());
		dset.putAndInsertOFStringArray(iter->first, str);
	}

	cond = sendCGETRequest(pcid, &dset, &responses);
	if (cond.bad())
	{
		return false;
	}

	if (!responses.empty())
	{
		/* output final status report */
		OFLOG_INFO(manteiaLogger, "Final status report from last C-GET message:");
		(*(--responses.end()))->print();
		/* delete responses */
		OFListIterator(RetrieveResponse*) iter = responses.begin();
		OFListConstIterator(RetrieveResponse*) last = responses.end();
		while (iter != last)
		{
			delete (*iter);
			iter = responses.erase(iter);
		}
	}

	/* tear down association */
	if (cond == EC_Normal)
	{
		closeAssociation(CSCUBase_RELEASE_ASSOCIATION);
		//closeAssociation(CSCUBase_PEER_ABORTED_ASSOCIATION);
		//if (GetT_ASC_Association())
		//{
		//	//释放会话
		//	cond = ASC_releaseAssociation(GetT_ASC_Association());
		//	//if (cond. == OFStatus)
		//	//{
		//	//}
		//	if (DUL_PEERABORTEDASSOCIATION == cond)
		//		abortAssociation();

		//	//释放网络
		//	freeNetwork();
		//}

	}
	else
	{
		if (cond == DUL_PEERREQUESTEDRELEASE)
			closeAssociation(CSCUBase_PEER_REQUESTED_RELEASE);
		else if (cond == DUL_PEERABORTEDASSOCIATION)
			closeAssociation(CSCUBase_PEER_ABORTED_ASSOCIATION);
		else
		{
			OFLOG_ERROR(manteiaLogger, "Get SCU Failed: " << DimseCondition::dump(temp_str, cond));
			abortAssociation();
		}

		return false;
	}

	return true;
}

void CGetSCU::PrepareTS(E_TransferSyntax ts, OFList<OFString>& syntaxes)
{
	/*
** We prefer to use Explicitly encoded transfer syntaxes.
** If we are running on a Little Endian machine we prefer
** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
** Some SCP implementations will just select the first transfer
** syntax they support (this is not part of the standard) so
** organize the proposed transfer syntaxes to take advantage
** of such behavior.
**
** The presentation contexts proposed here are only used for
** C-FIND and C-MOVE, so there is no need to support compressed
** transmission.
*/

	switch (ts)
	{
	case EXS_LittleEndianImplicit:
		/* we only support Little Endian Implicit */
		syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		break;
	case EXS_LittleEndianExplicit:
		/* we prefer Little Endian Explicit */
		syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
		syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
		break;
	case EXS_BigEndianExplicit:
		/* we prefer Big Endian Explicit */
		syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
		syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
		break;
#ifdef WITH_ZLIB
	case EXS_DeflatedLittleEndianExplicit:
		/* we prefer Deflated Little Endian Explicit */
		syntaxes.push_back(UID_DeflatedExplicitVRLittleEndianTransferSyntax);
		syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
		syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
		break;
#endif
	default:
		DcmXfer xfer(ts);
		if (xfer.isEncapsulated())
		{
			syntaxes.push_back(xfer.getXferID());
		}
		/* We prefer explicit transfer syntaxes.
		 * If we are running on a Little Endian machine we prefer
		 * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
		 */
		if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
		{
			syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
			syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
		}
		else
		{
			syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
			syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		}
		syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
		break;
	}
}
