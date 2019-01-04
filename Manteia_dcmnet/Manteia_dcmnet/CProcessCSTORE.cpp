#include "CProcessCSTORE.h"
#include "stdafx.h"
#include "ManteiaSCP.h"
#include "dcmtk/dcmnet/diutil.h"

// constant definitions

const char *CProcessCSTORE::m_StandardSubdirectory = "data";
const char *CProcessCSTORE::m_UndefinedSubdirectory = "undef";
const char *CProcessCSTORE::m_FilenameExtension = "";


CProcessCSTORE::CProcessCSTORE(CManteiaSCP* scp):
	CProcessCommandBase(scp)
{
	OutputDirectory = OFString();
	StandardSubdirectory = OFString(m_StandardSubdirectory);
	UndefinedSubdirectory = OFString(m_UndefinedSubdirectory);
	FilenameExtension = OFString(m_FilenameExtension);
	DirectoryGeneration = DcmStorageSCP::DGM_Default;
	FilenameGeneration = DcmStorageSCP::FGM_Default;
	FilenameCreator = OFFilenameCreator();
	DatasetStorage = DcmStorageSCP::DSM_Default;

	const char *opt_filenameExtension = "";
	DcmStorageSCP::E_DirectoryGenerationMode opt_directoryGeneration = DcmStorageSCP::DGM_NoSubdirectory;
	DcmStorageSCP::E_FilenameGenerationMode opt_filenameGeneration = DcmStorageSCP::FGM_SOPInstanceUID;
	DcmStorageSCP::E_DatasetStorageMode opt_datasetStorage = DcmStorageSCP::DGM_StoreToFile;

	setDirectoryGenerationMode(opt_directoryGeneration);
	setFilenameGenerationMode(opt_filenameGeneration);
	setFilenameExtension(opt_filenameExtension);
	setDatasetStorageMode(opt_datasetStorage);
}


CProcessCSTORE::~CProcessCSTORE()
{
	// clear internal state
	clear();
}

void CProcessCSTORE::clear()
{
	// DcmSCP::clear();
	OutputDirectory.clear();
	StandardSubdirectory = m_StandardSubdirectory;
	UndefinedSubdirectory = m_UndefinedSubdirectory;
	FilenameExtension = m_FilenameExtension;
	DirectoryGeneration = DcmStorageSCP::DGM_Default;
	FilenameGeneration = DcmStorageSCP::FGM_Default;
	DatasetStorage = DcmStorageSCP::DSM_Default;
}


// get methods

const OFString &CProcessCSTORE::getOutputDirectory() const
{
	return OutputDirectory;
}


DcmStorageSCP::E_DirectoryGenerationMode CProcessCSTORE::CProcessCSTORE::getDirectoryGenerationMode() const
{
	return DirectoryGeneration;
}


DcmStorageSCP::E_FilenameGenerationMode CProcessCSTORE::CProcessCSTORE::getFilenameGenerationMode() const
{
	return FilenameGeneration;
}


const OFString &CProcessCSTORE::getFilenameExtension() const
{
	return FilenameExtension;
}


DcmStorageSCP::E_DatasetStorageMode CProcessCSTORE::CProcessCSTORE::getDatasetStorageMode() const
{
	return DatasetStorage;
}


// set methods

OFCondition CProcessCSTORE::setOutputDirectory(const OFString &directory)
{
	OFCondition status = EC_Normal;
	if (directory.empty())
	{
		// empty directory refers to the current directory
		if (OFStandard::isWriteable("."))
			OutputDirectory.clear();
		else
			status = EC_DirectoryNotWritable;
	}
	else {
		// check whether given directory exists and is writable
		if (OFStandard::dirExists(directory))
		{
			if (OFStandard::isWriteable(directory))
				OFStandard::normalizeDirName(OutputDirectory, directory);
			else
				status = EC_DirectoryNotWritable;
		}
		else
			status = EC_DirectoryDoesNotExist;
	}
	return status;
}


void CProcessCSTORE::setDirectoryGenerationMode(const DcmStorageSCP::E_DirectoryGenerationMode mode)
{
	DirectoryGeneration = mode;
}


void CProcessCSTORE::setFilenameGenerationMode(const DcmStorageSCP::E_FilenameGenerationMode mode)
{
	FilenameGeneration = mode;
}


void CProcessCSTORE::setFilenameExtension(const OFString &extension)
{
	FilenameExtension = extension;
}


void CProcessCSTORE::setDatasetStorageMode(const DcmStorageSCP::E_DatasetStorageMode mode)
{
	DatasetStorage = mode;
}

OFCondition CProcessCSTORE::handleIncomingCommand(T_DIMSE_Message *incomingMsg,
	const DcmPresentationContextInfo &presInfo)
{
	OFCondition status = EC_IllegalParameter;
	if (incomingMsg != NULL)
	{
		// check whether we've received a supported command
		if (incomingMsg->CommandField == DIMSE_C_STORE_RQ)
		{
			// handle incoming C-STORE request
			T_DIMSE_C_StoreRQ &storeReq = incomingMsg->msg.CStoreRQ;
			Uint16 rspStatusCode = STATUS_STORE_Error_CannotUnderstand;
			// special case: bit preserving mode
			if (DatasetStorage == DcmStorageSCP::DGM_StoreBitPreserving)
			{
				OFString filename;
				// generate filename with full path (and create subdirectories if needed)
				status = generateSTORERequestFilename(storeReq, filename);
				if (status.good())
				{
					if (OFStandard::fileExists(filename))
						DCMNET_WARN("file already exists, overwriting: " << filename);
					// receive dataset directly to file
					status = GetSCPInstance()->DoReceiveSTORERequest(storeReq, presInfo.presentationContextID, filename);
					if (status.good())
					{
						// call the notification handler (default implementation outputs to the logger)
						notifyInstanceStored(filename, storeReq.AffectedSOPClassUID, storeReq.AffectedSOPInstanceUID);
						rspStatusCode = STATUS_Success;
					}
				}
			}
			else {
				DcmFileFormat fileformat;
				DcmDataset *reqDataset = fileformat.getDataset();
				// receive dataset in memory
				status = GetSCPInstance()->DoReceiveSTORERequest(storeReq, presInfo.presentationContextID, reqDataset);
				if (status.good())
				{
					// do we need to store the received dataset at all?
					if (DatasetStorage == DcmStorageSCP::DSM_Ignore)
					{
						// output debug message that dataset is not stored
						DCMNET_DEBUG("received dataset is not stored since the storage mode is set to 'ignore'");
						rspStatusCode = STATUS_Success;
					}
					else {
						// check and process C-STORE request
						rspStatusCode = checkAndProcessSTORERequest(storeReq, fileformat);
					}
				}
			}
			// send C-STORE response (with DIMSE status code)
			if (status.good())
				status = GetSCPInstance()->DoSendSTOREResponse(presInfo.presentationContextID, storeReq, rspStatusCode);
			else if (status == DIMSE_OUTOFRESOURCES)
			{
				// do not overwrite the previous error status
				GetSCPInstance()->DoSendSTOREResponse(presInfo.presentationContextID, storeReq, STATUS_STORE_Refused_OutOfResources);
			}
		}
		else {
			// unsupported command
			OFString tempStr;
			DCMNET_ERROR("cannot handle this kind of DIMSE command (0x"
				<< STD_NAMESPACE hex << STD_NAMESPACE setfill('0') << STD_NAMESPACE setw(4)
				<< OFstatic_cast(unsigned int, incomingMsg->CommandField)
				<< "), we are a Storage SCP only");
			DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, *incomingMsg, DIMSE_INCOMING));
			// TODO: provide more information on this error?
			status = DIMSE_BADCOMMANDTYPE;
		}
	}
	return status;
}


Uint16 CProcessCSTORE::checkAndProcessSTORERequest(const T_DIMSE_C_StoreRQ &reqMessage,
	DcmFileFormat &fileformat)
{
	DCMNET_DEBUG("checking and processing C-STORE request");
	Uint16 statusCode = STATUS_STORE_Error_CannotUnderstand;
	DcmDataset *dataset = fileformat.getDataset();
	// perform some basic checks on the request dataset
	if ((dataset != NULL) && !dataset->isEmpty())
	{
		OFString filename;
		OFString directoryName;
		OFString sopClassUID = reqMessage.AffectedSOPClassUID;
		OFString sopInstanceUID = reqMessage.AffectedSOPInstanceUID;
		// generate filename with full path
		OFCondition status = generateDirAndFilename(filename, directoryName, sopClassUID, sopInstanceUID, dataset);
		if (status.good())
		{
			DCMNET_DEBUG("generated filename for received object: " << filename);
			// create the output directory (if needed)
			status = OFStandard::createDirectory(directoryName, OutputDirectory /* rootDir */);
			if (status.good())
			{
				if (OFStandard::fileExists(filename))
					DCMNET_WARN("file already exists, overwriting: " << filename);
				// store the received dataset to file (with default settings)
				status = fileformat.saveFile(filename);
				if (status.good())
				{
					// call the notification handler (default implementation outputs to the logger)
					notifyInstanceStored(filename, sopClassUID, sopInstanceUID, dataset);
					statusCode = STATUS_Success;
				}
				else {
					DCMNET_ERROR("cannot store received object: " << filename << ": " << status.text());
					statusCode = STATUS_STORE_Refused_OutOfResources;

					// delete incomplete file
					OFStandard::deleteFile(filename);
				}
			}
			else {
				DCMNET_ERROR("cannot create directory for received object: " << directoryName << ": " << status.text());
				statusCode = STATUS_STORE_Refused_OutOfResources;
			}
		}
		else
			DCMNET_ERROR("cannot generate directory or file name for received object: " << status.text());
	}
	return statusCode;
}


OFCondition CProcessCSTORE::generateSTORERequestFilename(const T_DIMSE_C_StoreRQ &reqMessage,
	OFString &filename)
{
	OFString directoryName;
	OFString sopClassUID = reqMessage.AffectedSOPClassUID;
	OFString sopInstanceUID = reqMessage.AffectedSOPInstanceUID;
	// generate filename (with full path)
	OFCondition status = generateDirAndFilename(filename, directoryName, sopClassUID, sopInstanceUID);
	if (status.good())
	{
		DCMNET_DEBUG("generated filename for object to be received: " << filename);
		// create the output directory (if needed)
		status = OFStandard::createDirectory(directoryName, OutputDirectory /* rootDir */);
		if (status.bad())
			DCMNET_ERROR("cannot create directory for object to be received: " << directoryName << ": " << status.text());
	}
	else
		DCMNET_ERROR("cannot generate directory or file name for object to be received: " << status.text());
	return status;
}


void CProcessCSTORE::notifyInstanceStored(const OFString &filename,
	const OFString & /*sopClassUID*/,
	const OFString & /*sopInstanceUID*/,
	DcmDataset * /*dataset*/)
{
	// by default, output some useful information
	DCMNET_INFO("Stored received object to file: " << filename);

	//接收到文件后回调
	if ( GetSCPInstance()->GetCalllbackForRecvFile() )
	{
		(GetSCPInstance()->GetCalllbackForRecvFile())(filename.data());
	}
}


OFCondition CProcessCSTORE::generateDirAndFilename(OFString &filename,
	OFString &directoryName,
	OFString &sopClassUID,
	OFString &sopInstanceUID,
	DcmDataset *dataset)
{
	OFCondition status = EC_Normal;
	// get SOP class and instance UID (if not yet known from the command set)
	if (dataset != NULL)
	{
		if (sopClassUID.empty())
			dataset->findAndGetOFString(DCM_SOPClassUID, sopClassUID);
		if (sopInstanceUID.empty())
			dataset->findAndGetOFString(DCM_SOPInstanceUID, sopInstanceUID);
	}
	// generate directory name
	OFString generatedDirName;
	switch (DirectoryGeneration)
	{
	case DcmStorageSCP::DGM_NoSubdirectory:
		// do nothing (default)
		break;
		// use series date (if available) for subdirectory structure
	case DcmStorageSCP::DGM_SeriesDate:
		if (dataset != NULL)
		{
			OFString seriesDate;
			DcmElement *element = NULL;
			// try to get the series date from the dataset
			if (dataset->findAndGetElement(DCM_SeriesDate, element).good() && (element->ident() == EVR_DA))
			{
				OFString dateValue;
				DcmDate *dateElement = OFstatic_cast(DcmDate *, element);
				// output ISO format is: YYYY-MM-DD
				if (dateElement->getISOFormattedDate(dateValue).good() && (dateValue.length() == 10))
				{
					OFOStringStream stream;
					stream << StandardSubdirectory << PATH_SEPARATOR
						<< dateValue.substr(0, 4) << PATH_SEPARATOR
						<< dateValue.substr(5, 2) << PATH_SEPARATOR
						<< dateValue.substr(8, 2) << OFStringStream_ends;
					OFSTRINGSTREAM_GETSTR(stream, tmpString)
						generatedDirName = tmpString;
					OFSTRINGSTREAM_FREESTR(tmpString);
				}
			}
			// alternatively, if that fails, use the current system date
			if (generatedDirName.empty())
			{
				OFString currentDate;
				status = DcmDate::getCurrentDate(currentDate);
				if (status.good())
				{
					OFOStringStream stream;
					stream << UndefinedSubdirectory << PATH_SEPARATOR
						<< currentDate << OFStringStream_ends;
					OFSTRINGSTREAM_GETSTR(stream, tmpString)
						generatedDirName = tmpString;
					OFSTRINGSTREAM_FREESTR(tmpString);
				}
			}
		}
		else {
			DCMNET_DEBUG("received dataset is not available in order to determine the SeriesDate "
				<< DCM_SeriesDate << ", are you using the bit preserving mode?");
			// no DICOM dataset given, so we cannot determine the series date
			status = EC_CouldNotGenerateDirectoryName;
		}
		break;
	}
	if (status.good())
	{
		// combine the generated directory name with the output directory
		OFStandard::combineDirAndFilename(directoryName, OutputDirectory, generatedDirName);
		// generate filename
		OFString generatedFileName;
		switch (FilenameGeneration)
		{
			// use modality prefix and SOP instance UID (default)
		case DcmStorageSCP::FGM_SOPInstanceUID:
		{
			if (sopClassUID.empty())
				status = NET_EC_InvalidSOPClassUID;
			else if (sopInstanceUID.empty())
				status = NET_EC_InvalidSOPInstanceUID;
			else {
				OFOStringStream stream;
				stream << dcmSOPClassUIDToModality(sopClassUID.c_str(), "UNKNOWN")
					<< '.' << sopInstanceUID << FilenameExtension << OFStringStream_ends;
				OFSTRINGSTREAM_GETSTR(stream, tmpString)
					generatedFileName = tmpString;
				OFSTRINGSTREAM_FREESTR(tmpString);
				// combine the generated file name with the directory name
				OFStandard::combineDirAndFilename(filename, directoryName, generatedFileName);
			}
			break;
		}
		// unique filename based on modality prefix and newly generated UID
		case DcmStorageSCP::FGM_UniqueFromNewUID:
		{
			char uidBuffer[70];
			dcmGenerateUniqueIdentifier(uidBuffer);
			OFOStringStream stream;
			stream << dcmSOPClassUIDToModality(sopClassUID.c_str(), "UNKNOWN")
				<< ".X." << uidBuffer << FilenameExtension << OFStringStream_ends;
			OFSTRINGSTREAM_GETSTR(stream, tmpString)
				generatedFileName = tmpString;
			OFSTRINGSTREAM_FREESTR(tmpString);
			// combine the generated file name with the directory name
			OFStandard::combineDirAndFilename(filename, directoryName, generatedFileName);
			break;
		}
		// unique pseudo-random filename (also checks for existing files, so we need some special handling)
		case DcmStorageSCP::FGM_ShortUniquePseudoRandom:
		{
			OFString prefix = dcmSOPClassUIDToModality(sopClassUID.c_str(), "XX");
			prefix += '_';
			// TODO: we might want to use a more appropriate seed value
			unsigned int seed = OFstatic_cast(unsigned int, time(NULL));
			if (!FilenameCreator.makeFilename(seed, directoryName.c_str(), prefix.c_str(), FilenameExtension.c_str(), filename))
				status = EC_CouldNotGenerateFilename;
			break;
		}
		// use current system time and modality suffix for filename
		case DcmStorageSCP::FGM_CurrentSystemTime:
		{
			OFString timeStamp;
			// get the date/time as: YYYYMMDDHHMMSS.FFFFFF
			if (DcmDateTime::getCurrentDateTime(timeStamp, OFTrue /* seconds */, OFTrue /* fraction */).good())
			{
				OFOStringStream stream;
				stream << timeStamp << '.' << dcmSOPClassUIDToModality(sopClassUID.c_str(), "UNKNOWN")
					<< FilenameExtension << OFStringStream_ends;
				OFSTRINGSTREAM_GETSTR(stream, tmpString)
					generatedFileName = tmpString;
				OFSTRINGSTREAM_FREESTR(tmpString);
				// combine the generated file name
				OFStandard::combineDirAndFilename(filename, directoryName, generatedFileName);
			}
			else
				status = EC_CouldNotGenerateFilename;
			break;
		}

		}
	}
	return status;
}
