#pragma once
#include "dcmtk/ofstd/offname.h"    /* for OFFilenameCreator */
#include "dcmtk/dcmnet/dstorscp.h"	/* for DcmStorageSCP */
#include "CProcessCommandBase.h"

class CManteiaSCP;

class CProcessCSTORE: public CProcessCommandBase
{
public:
	CProcessCSTORE(CManteiaSCP* scp);
	~CProcessCSTORE();

	// --- public types ---

/** modes for generating subdirectories
 */
	//enum E_DirectoryGenerationMode
	//{
	//	/// do not generate any subdirectories
	//	DGM_NoSubdirectory,
	//	/// generated subdirectories based on the Series Date (0008,0021)
	//	DGM_SeriesDate,
	//	/// default value
	//	DGM_Default = DGM_NoSubdirectory
	//};

	/** modes for generating filenames
	 */
	//enum E_FilenameGenerationMode
	//{
	//	/// generate filename from SOP Instance UID (0008,0018)
	//	FGM_SOPInstanceUID,
	//	/// generate unique filename based on new UID
	//	FGM_UniqueFromNewUID,
	//	/// generate short pseudo-random unique filename
	//	FGM_ShortUniquePseudoRandom,
	//	/// generate filename from current system time
	//	FGM_CurrentSystemTime,
	//	/// default value
	//	FGM_Default = FGM_SOPInstanceUID
	//};

	///** modes specifying whether and how to store the received datasets
	// */
	//enum E_DatasetStorageMode
	//{
	//	/// receive dataset in memory, perform some conversions and store it to file
	//	DGM_StoreToFile,
	//	/// receive dataset directly to file, i.e. write data exactly as received
	//	DGM_StoreBitPreserving,
	//	/// receive dataset in memory, but do not store it to file
	//	DSM_Ignore,
	//	/// default value
	//	DSM_Default = DGM_StoreToFile
	//};

	// --- public methods ---

	/** clear the internal member variables, i.e.\ set them to their default values
	 */
	void clear();

	// get methods

	/** get the output directory to be used for the storage of the received DICOM
	 *  datasets.  Depending on the current mode for generating subdirectories (see
	 *  getDirectoryGenerationMode()), further substructures are created automatically.
	 *  @return name of the output directory that is used for storing the received
	 *    DICOM datasets
	 */
	const OFString &getOutputDirectory() const;

	/** get the mode for generating subdirectories used to store the received datasets
	 *  @return current mode for generating subdirectories
	 *    (see DcmStorageSCP::E_DirectoryGenerationMode)
	 */
	DcmStorageSCP::E_DirectoryGenerationMode getDirectoryGenerationMode() const;

	/** get the mode for generating filenames for the received datasets
	 *  @return current mode for generating filenames
	 *    (see DcmStorageSCP::E_FilenameGenerationMode)
	 */
	DcmStorageSCP::E_FilenameGenerationMode getFilenameGenerationMode() const;

	/** get the filename extension that is appended to the generated filenames
	 *  @return current filename extension that is appended to the generated filenames
	 */
	const OFString &getFilenameExtension() const;

	/** get the mode specifying whether and how to store the received datasets
	 *  @return current mode specifying whether and how to store the received datasets
	 *    (see DcmStorageSCP::E_DatasetStorageMode)
	 */
	DcmStorageSCP::E_DatasetStorageMode getDatasetStorageMode() const;

	// set methods

	/** specify the output directory to be used for the storage of the received DICOM
	 *  datasets.  Depending on the current mode for generating subdirectories (see
	 *  getDirectoryGenerationMode()), further substructures are created automatically.
	 *  Before setting the new directory name, it is checked whether the specified
	 *  directory exists and is writable.  By default, the current directory is used.
	 *  @param  directory  name of the output directory to be used
	 *  @return status, EC_Normal if successful, an error code otherwise
	 */
	OFCondition setOutputDirectory(const OFString &directory);

	/** set the mode for generating subdirectories used to store the received datasets.
	 *  These subdirectories are created below the specified output directory.
	 *  The default value is specified by DcmStorageSCP::DGM_Default.
	 *  @param  mode  mode to be set for generating subdirectories
	 *               (see DcmStorageSCP::E_DirectoryGenerationMode)
	 */
	void setDirectoryGenerationMode(const DcmStorageSCP::E_DirectoryGenerationMode mode);

	/** set the mode for generating filenames for the received datasets.
	 *  The default value is specified by DcmStorageSCP::FGM_Default.
	 *  @param  mode  mode to be set for generating filenames
	 *               (see DcmStorageSCP::E_FilenameGenerationMode)
	 */
	void setFilenameGenerationMode(const DcmStorageSCP::E_FilenameGenerationMode mode);

	/** specify the filename extension to be appended to the generated filenames.
	 *  The default value is specified by DcmStorageSCP::DEF_FilenameExtension
	 *  (empty string).  A typical non-empty value would be ".dcm" (i.e. that "."
	 *  has to be specified explicitly).
	 *  @param  extension  filename extension appended to the generated filenames
	 */
	void setFilenameExtension(const OFString &extension);

	/** set the mode specifying how to store the received datasets.  This mode also
	 *  allows for specifying whether to store the received datasets at all.
	 *  The default value is specified by DcmStorageSCP::DSM_Default.
	 *  @param  mode  mode to be set specifying whether and how to store the received
	 *                datasets (see DcmStorageSCP::E_DatasetStorageMode)
	 */
	void setDatasetStorageMode(const DcmStorageSCP::E_DatasetStorageMode mode);

	// other methods

	/** handler that is called for each incoming command message.  This handler supports
 *  C-STORE requests.  All other messages will be reported as an error.
 *  After a valid C-STORE request has been received, the request and associated
 *  dataset will be checked and further processed by checkAndProcessSTORERequest().
 *  @param  incomingMsg  pointer to data structure containing the DIMSE message
 *  @param  presInfo     additional information on the Presentation Context used
 *  @return status, EC_Normal if successful, an error code otherwise
 */
	OFCondition handleIncomingCommand(T_DIMSE_Message *incomingMsg,
		const DcmPresentationContextInfo &presInfo);

protected:

	/** check the given C-STORE request and dataset for validity.  This method is called
	 *  by handleIncomingCommand() before sending the response in order to determine the
	 *  DIMSE status code to be used for the response message.  If this check has been
	 *  passed successfully, the received dataset is stored as a DICOM file.
	 *  @param  reqMessage  C-STORE request message data structure to be checked and
	 *                      processed
	 *  @param  fileformat  DICOM fileformat structure containing the C-STORE request
	 *                      dataset to be checked and processed
	 *  @return DIMSE status code to be used for the C-STORE response
	 */
	Uint16 checkAndProcessSTORERequest(const T_DIMSE_C_StoreRQ &reqMessage,
		DcmFileFormat &fileformat);

	/** generate a directory and file name for a DICOM dataset that will be received.
	 *  The naming scheme can be specified by the methods setDirectoryGenerationMode(),
	 *  setFilenameGenerationMode() and setFilenameExtension().
	 *  Please note that this method also creates the directory structure (if needed).
	 *  @param  reqMessage  C-STORE request message data structure used to generate the
	 *                      filename (depending on the specified options)
	 *  @param  filename    reference to variable that will store the resulting filename
	 *  @return status, EC_Normal if successful, an error code otherwise
	 */
	OFCondition generateSTORERequestFilename(const T_DIMSE_C_StoreRQ &reqMessage,
		OFString &filename);

	/** notification handler that is called for each DICOM object that has been received
	 *  with a C-STORE request and stored as a DICOM file
	 *  @param  filename        filename (with full path) of the object stored
	 *  @param  sopClassUID     SOP Class UID of the object stored
	 *  @param  sopInstanceUID  SOP Instance UID of the object stored
	 *  @param  dataset         pointer to dataset of the object stored (or NULL if the
	 *                          dataset has been stored directly to file).
	 *                          Please note that this dataset will be deleted by the calling
	 *                          method, so do not store any references to it!
	 */
	void notifyInstanceStored(const OFString &filename,
		const OFString &sopClassUID,
		const OFString &sopInstanceUID,
		DcmDataset *dataset = NULL);

	/** generate a directory and file name for a DICOM dataset that has been received.
	 *  The naming scheme can be specified by the methods setDirectoryGenerationMode(),
	 *  setFilenameGenerationMode() and setFilenameExtension().
	 *  Please note that this method only generates the names but neither creates the
	 *  directory structure nor the DICOM file.
	 *  @param  filename        reference to variable that will store the resulting
	 *                          filename
	 *  @param  directoryName   reference to variable that will store the resulting
	 *                          directory name (including the main output directory)
	 *  @param  sopClassUID     SOP Class UID of the DICOM object.  This is both an
	 *                          input and output parameter.  If an empty value is passed
	 *                          to this method, the value of the data element SOP Class
	 *                          UID (0008,0016) is determined from the DICOM dataset.
	 *  @param  sopInstanceUID  SOP Instance UID of the DICOM object.  This is both an
	 *                          input and output parameter.  If an empty value is passed
	 *                          to this method, the value of the data element SOP
	 *                          Instance UID (0008,0018) is determined from the dataset.
	 *  @param  dataset         pointer to dataset for which the directory and file name
	 *                          is to be generated (optional)
	 *  @return status, EC_Normal if successful, an error code otherwise
	 */
	OFCondition generateDirAndFilename(OFString &filename,
		OFString &directoryName,
		OFString &sopClassUID,
		OFString &sopInstanceUID,
		DcmDataset *dataset = NULL);

	// --- public constants ---

	/// default value for the name of the subdirectory that might be used for the
	/// "normal" case
	static const char *m_StandardSubdirectory;
	/// default value for the name of the subdirectory that might be used for the
	/// "exceptional" case
	static const char *m_UndefinedSubdirectory;
	/// default value for the filename extension appended to the generated filenames
	static const char *m_FilenameExtension;


private:

	/// name of the output directory that is used to store the received datasets
	OFString OutputDirectory;
	/// name of the subdirectory that might be used for the "normal" case, i.e.\ if the
	/// name of the subdirectory could be generated according to the current mode
	OFString StandardSubdirectory;
	/// name of the subdirectory that might be used for the "exceptional" case, i.e.\ if
	/// the name of the subdirectory could not be generated according to the current mode
	OFString UndefinedSubdirectory;
	/// filename extension appended to the generated filenames
	OFString FilenameExtension;
	/// mode that is used to generate subdirectories to store the received datasets
	DcmStorageSCP::E_DirectoryGenerationMode DirectoryGeneration;
	/// mode that is used to generate filenames for the received datasets
	DcmStorageSCP::E_FilenameGenerationMode FilenameGeneration;
	/// unique pseudo-random filename creator, which also checks for existing files
	OFFilenameCreator FilenameCreator;
	/// mode specifying how to store the received datasets (also allows for skipping the storage)
	DcmStorageSCP::E_DatasetStorageMode DatasetStorage;

};

