// DCMTK_MySCU.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

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

#include "LanStoreSCU.h"

#ifdef WITH_ZLIB
#include <zlib.h>                    /* for zlibVersion() */
#endif

#if defined (HAVE_WINDOWS_H) || defined(HAVE_FNMATCH_H)
#define PATTERN_MATCHING_AVAILABLE
#endif


/* general definitions */

#define OFFIS_CONSOLE_APPLICATION "landcmsend"

static OFLogger dcmsendLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
                      OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

/* default application entity titles */
#define APPLICATIONTITLE     "LANDCMSEND"
#define PEERAPPLICATIONTITLE "CONQUESTSRV2"//"LANDCMRECV"


/* exit codes for this command line tool */
/* (common codes are defined in "ofexit.h" included from "ofconapp.h") */

// output file errors
#define EXITCODE_CANNOT_WRITE_REPORT_FILE        43

// network errors
#define EXITCODE_CANNOT_INITIALIZE_NETWORK       60
#define EXITCODE_CANNOT_NEGOTIATE_ASSOCIATION    61
#define EXITCODE_CANNOT_SEND_REQUEST             62
#define EXITCODE_CANNOT_ADD_PRESENTATION_CONTEXT 65


/* helper macro for converting stream output to a string */
#define CONVERT_TO_STRING(output, string) \
    optStream.str(""); \
    optStream.clear(); \
    optStream << output << OFStringStream_ends; \
    OFSTRINGSTREAM_GETOFSTRING(optStream, string)


/* static helper functions */

// make sure that everything is cleaned up properly
static void cleanup()
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

#define SHORTCOL 4
#define LONGCOL 21

int TestStoreSCU(int argc, char* argv[])
{
    OFOStringStream optStream;
    const char* opt_peer = "lanliangsheng";
    const char* opt_peerTitle = PEERAPPLICATIONTITLE;
    const char* opt_ourTitle = APPLICATIONTITLE;
    E_FileReadMode opt_readMode = ERM_fileOnly;
    OFCmdUnsignedInt opt_port = 9999;//8888;
    OFCmdUnsignedInt opt_timeout = 0;
    OFCmdUnsignedInt opt_dimseTimeout = 0;  //此参数与“T_DIMSE_BlockingMode opt_blockingMode”对应！！！当其锁模式变化时，超时时间设置要做相应改变，否则将引起连接中断！！！
    OFCmdUnsignedInt opt_acseTimeout = 30;  //等待SCP回应处理的超时时间  0为无限等待  单位为秒
    OFCmdUnsignedInt opt_maxReceivePDULength = ASC_DEFAULTMAXPDU;
    OFCmdUnsignedInt opt_maxSendPDULength = 0;
    T_DIMSE_BlockingMode opt_blockMode = DIMSE_BLOCKING;// DIMSE_NONBLOCKING;// DIMSE_BLOCKING;
#ifdef WITH_ZLIB
    OFCmdUnsignedInt opt_compressionLevel = 0;
#endif
    OFBool opt_showPresentationContexts = OFFalse;
    OFBool opt_haltOnInvalidFile = OFTrue;
    OFBool opt_haltOnUnsuccessfulStore = OFTrue;
    OFBool opt_allowIllegalProposal = OFTrue;
    OFBool opt_checkUIDValues = OFTrue;
    OFBool opt_multipleAssociations = OFTrue;
    DcmStorageSCU::E_DecompressionMode opt_decompressionMode = DcmStorageSCU::DM_losslessOnly;
    OFBool opt_dicomDir = OFFalse;
    OFBool opt_scanDir = OFFalse;
    OFBool opt_recurse = OFFalse;
    const char* opt_scanPattern = "";
    const char* opt_reportFilename = "E://DcmScuScp//DcmScp//report.txt";
    // register JPEG decoder
    DJDecoderRegistration::registerCodecs();
    // register JPEG-LS decoder
    DJLSDecoderRegistration::registerCodecs();
    // register RLE decoder
    DcmRLEDecoderRegistration::registerCodecs();
    /* print resource identifier */
    OFLOG_DEBUG(dcmsendLogger, rcsid << OFendl);

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
        OFLOG_WARN(dcmsendLogger, "no data dictionary loaded, check environment variable: "
                   << DCM_DICT_ENVIRONMENT_VARIABLE);
    }

    /* start with the real work */
    if (opt_scanDir)
        OFLOG_INFO(dcmsendLogger, "determining input files ...");

    /* iterate over all input filenames/directories */
    OFList<OFString> inputFiles;
    //const char *paramString = "E://DcmScuScp//files";
    //for (int i = 0; i < 2; i++)
    //{
    //  /* search directory recursively (if required) */
    //  if (OFStandard::dirExists(paramString))
    //  {
    //      if (opt_scanDir)
    //          OFStandard::searchDirectoryRecursively(paramString, inputFiles, opt_scanPattern, "" /* dirPrefix */, opt_recurse);
    //      else
    //          OFLOG_WARN(dcmsendLogger, "ignoring directory because option --scan-directories is not set: " << paramString);
    //  }
    //  else
    //      inputFiles.push_back(paramString);
    //}
    OFString str_1 = "E://DcmScuScp//files//7777777.dcm";
    inputFiles.push_back(str_1);
    //str_1 = "E://DcmScuScp//files//1.dcm";
    //inputFiles.push_back(str_1);
    //
    //str_1 = "E://DcmScuScp//files//2.16.840.1.114362.1.6.4.0.14604.7389452627.362891768.298.7460.dcm";
    //inputFiles.push_back(str_1);
    //str_1 = "E://DcmScuScp//files//2.16.840.1.114362.1.6.4.0.14604.7389452627.362891768.303.7462.dcm";
    //inputFiles.push_back(str_1);
    //str_1 = "E://DcmScuScp//files//2.16.840.1.114362.1.6.4.0.14604.7389452627.362891768.306.7463.dcm";
    //inputFiles.push_back(str_1);
    //str_1 = "E://DcmScuScp//files//2.16.840.1.114362.1.6.4.0.14604.7389452627.362891768.308.7464.dcm";
    //inputFiles.push_back(str_1);
    //str_1 = "E://DcmScuScp//files//2.16.840.1.114362.1.6.4.0.14604.7389452627.362891768.310.7465.dcm";
    //inputFiles.push_back(str_1);

    //str_1 = "E://DcmScuScp//files//2.dcm";
    //inputFiles.push_back(str_1);
    //str_1 = "E://DcmScuScp//files//3.dcm";
    //inputFiles.push_back(str_1);

    /* check whether there are any input files at all */
    if (inputFiles.empty())
    {
        OFLOG_FATAL(dcmsendLogger, "no input files to be processed");
        cleanup();
        return EXITCODE_NO_INPUT_FILES;
    }

    //DcmStorageSCU storageSCU;
    LanStoreSCU storageSCU;
    OFCondition status;
    unsigned long numInvalidFiles = 0;
    /* set parameters used for processing the input files */
    storageSCU.setReadFromDICOMDIRMode(opt_dicomDir);
    storageSCU.setHaltOnInvalidFileMode(opt_haltOnInvalidFile); //指定在批处理期间是否遇到无效文件时停止（例如，从DICOMDIR添加SOP实例时）或是否继续下一个SOP实例。
    OFLOG_INFO(dcmsendLogger, "checking input files ...");
    /* iterate over all input filenames */
    const char* currentFilename = NULL;
    OFListIterator(OFString) if_iter = inputFiles.begin();
    OFListIterator(OFString) if_last = inputFiles.end();

    while (if_iter != if_last)
    {
        currentFilename = (*if_iter).c_str();
        /* and add them to the list of instances to be transmitted */
        status = storageSCU.addDicomFile(currentFilename, opt_readMode, opt_checkUIDValues);

        if (status.bad())
        {
            /* check for empty filename */
            if (strlen(currentFilename) == 0)
                currentFilename = "<empty string>";

            /* if something went wrong, we either terminate or ignore the file */
            if (opt_haltOnInvalidFile)
            {
                OFLOG_FATAL(dcmsendLogger, "bad DICOM file: " << currentFilename << ": " << status.text());
                cleanup();
                return EXITCODE_INVALID_INPUT_FILE;
            }
            else
            {
                OFLOG_WARN(dcmsendLogger, "bad DICOM file: " << currentFilename << ": " << status.text() << ", ignoring file");
            }

            ++numInvalidFiles;
        }

        ++if_iter;
    }

    /* check whether there are any valid input files */
    if (storageSCU.getNumberOfSOPInstances() == 0)
    {
        OFLOG_FATAL(dcmsendLogger, "no valid input files to be processed");
        cleanup();
        return EXITCODE_NO_VALID_INPUT_FILES;
    }
    else
    {
        OFLOG_DEBUG(dcmsendLogger, "in total, there are " << storageSCU.getNumberOfSOPInstances()
                    << " SOP instances to be sent, " << numInvalidFiles << " invalid files are ignored");
    }

    /* set network parameters */
    storageSCU.setPeerHostName(opt_peer);
    storageSCU.setPeerPort(OFstatic_cast(Uint16, opt_port));
    storageSCU.setPeerAETitle(opt_peerTitle);
    storageSCU.setAETitle(opt_ourTitle);
    storageSCU.setMaxReceivePDULength(OFstatic_cast(Uint32, opt_maxReceivePDULength));
    storageSCU.setACSETimeout(OFstatic_cast(Uint32, opt_acseTimeout));
    storageSCU.setDIMSETimeout(OFstatic_cast(Uint32, opt_dimseTimeout));
    storageSCU.setDIMSEBlockingMode(opt_blockMode);
    storageSCU.setVerbosePCMode(opt_showPresentationContexts);
    storageSCU.setDatasetConversionMode(opt_decompressionMode != DcmStorageSCU::DM_never);
    storageSCU.setDecompressionMode(opt_decompressionMode);
    storageSCU.setHaltOnUnsuccessfulStoreMode(opt_haltOnUnsuccessfulStore);
    storageSCU.setAllowIllegalProposalMode(opt_allowIllegalProposal);

    /* output information on the single/multiple associations setting */
    if (opt_multipleAssociations)
    {
        OFLOG_DEBUG(dcmsendLogger, "multiple associations allowed (option --multi-associations used)");
    }
    else
    {
        OFLOG_DEBUG(dcmsendLogger, "only a single associations allowed (option --single-association used)");
    }

    /* add presentation contexts to be negotiated (if there are still any) */
    while ((status = storageSCU.addPresentationContexts()).good())
    {
        if (opt_multipleAssociations)
        {
            /* output information on the start of the new association */
            if (dcmsendLogger.isEnabledFor(OFLogger::DEBUG_LOG_LEVEL))
            {
                OFLOG_DEBUG(dcmsendLogger, OFString(65, '-') << OFendl
                            << "starting association #" << (storageSCU.getAssociationCounter() + 1));
            }
            else
            {
                OFLOG_INFO(dcmsendLogger, "starting association #" << (storageSCU.getAssociationCounter() + 1));
            }
        }

        OFLOG_INFO(dcmsendLogger, "initializing network ...");
        /* initialize network */
        status = storageSCU.initNetwork();

        if (status.bad())
        {
            OFLOG_FATAL(dcmsendLogger, "cannot initialize network: " << status.text());
            cleanup();
            return EXITCODE_CANNOT_INITIALIZE_NETWORK;
        }

        OFLOG_INFO(dcmsendLogger, "negotiating network association ...");
        /* negotiate network association with peer */
        status = storageSCU.negotiateAssociation();

        if (status.bad())
        {
            // check whether we can continue with a new association
            if (status == NET_EC_NoAcceptablePresentationContexts)
            {
                OFLOG_ERROR(dcmsendLogger, "cannot negotiate network association: " << status.text());
                // check whether there are any SOP instances to be sent
                const size_t numToBeSent = storageSCU.getNumberOfSOPInstancesToBeSent();

                if (numToBeSent > 0)
                {
                    OFLOG_WARN(dcmsendLogger, "trying to continue with a new association "
                               << "in order to send the remaining " << numToBeSent << " SOP instances");
                }
            }
            else
            {
                OFLOG_FATAL(dcmsendLogger, "cannot negotiate network association: " << status.text());
                cleanup();
                return EXITCODE_CANNOT_NEGOTIATE_ASSOCIATION;
            }
        }

        if (status.good())
        {
            OFLOG_INFO(dcmsendLogger, "sending SOP instances ...");
            /* send SOP instances to be transferred */
            status = storageSCU.sendSOPInstances();

            if (status.bad())
            {
                OFLOG_FATAL(dcmsendLogger, "cannot send SOP instance: " << status.text());

                // handle certain error conditions (initiated by the communication peer)
                if (status == DUL_PEERREQUESTEDRELEASE)
                {
                    // peer requested release (aborting)
                    storageSCU.closeAssociation(DCMSCU_PEER_REQUESTED_RELEASE);
                }
                else if (status == DUL_PEERABORTEDASSOCIATION)
                {
                    // peer aborted the association
                    storageSCU.closeAssociation(DCMSCU_PEER_ABORTED_ASSOCIATION);
                }

                cleanup();
                return EXITCODE_CANNOT_SEND_REQUEST;
            }
        }

        /* close current network association */
        storageSCU.releaseAssociation();

        /* check whether multiple associations are permitted */
        if (!opt_multipleAssociations)
            break;
    }

    /* if anything went wrong, report it to the logger */
    if (status.bad() && (status != NET_EC_NoPresentationContextsDefined))
    {
        OFLOG_ERROR(dcmsendLogger, "cannot add presentation contexts: " << status.text());
        cleanup();
        return EXITCODE_CANNOT_ADD_PRESENTATION_CONTEXT;
    }

    /* create a detailed report on the transfer of instances ... */
    if ((opt_reportFilename != NULL) && (strlen(opt_reportFilename) > 0))
    {
        /* ... and write it to the specified text file */
        status = storageSCU.createReportFile(opt_reportFilename);

        if (status.bad())
        {
            cleanup();
            return EXITCODE_CANNOT_WRITE_REPORT_FILE; // TODO: do we really want to exit?
        }
    }

    /* output some status information on the overall sending process */
    if (dcmsendLogger.isEnabledFor(OFLogger::INFO_LOG_LEVEL))
    {
        OFString summaryText;
        storageSCU.getStatusSummary(summaryText);
        OFLOG_INFO(dcmsendLogger, OFendl << summaryText);
    }

    /* make sure that everything is cleaned up properly */
    cleanup();
    return EXITCODE_NO_ERROR;
}

int main(int argc, char* argv[])
{
    int error = TestStoreSCU(argc, argv);
    return error;
}
