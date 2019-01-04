#pragma once
#include "dcmtk/oflog/oflog.h"       /* for OFStandard functions */
#include "dcmtk/dcmdata/dctagkey.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include <string>

#define OFFIS_CONSOLE_APPLICATION "MANTEIA_SCU"
static OFLogger manteiaLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

#define SCUDELETE(p) \
		if(p) delete p;\
		p = NULL;

#define SCPDELETE(p) \
		if(p) delete p;\
		p = NULL;

namespace COMMON {
	enum EM_TagKey
	{
		TK_QueryRetrieveLevel,
		TK_PatientID,
		TK_PatientName,
		TK_StudyDate,
		TK_StudyID,
		TK_SeriesNumber,
		TK_SOPInstanceUID,
		TK_SOPClassUID,
		TK_StudyInstanceUID,
		TK_SeriesInstanceUID,
		TK_PatientBirthDate,
		TK_PatientSex,
		TK_SeriesDate,
		TK_SeriesTime,
		TK_StudyTime,
		TK_BodyPartExamined,
		TK_InstanceNumber,

		TK_END
	};

	//顺序对应于EM_TagKey(通用)
	static std::string TagKeyString[] = {
		"QueryRetrieveLevel",
		"PatientID",
		"PatientName",
		"StudyDate",
		"StudyID",
		"SeriesNumber",
		"SOPInstanceUID",
		"SOPClassUID",
		"StudyInstanceUID",
		"SeriesInstanceUID",
		"PatientBirthDate",
		"PatientSex",
		"SeriesDate",
		"SeriesTime",
		"StudyTime",
		"BodyPartExamined",
		"InstanceNumber"
	};

	//顺序对应于EM_TagKey（Manteia数据库字段）
	static std::string TagKeyStringForManteia[] = {
		"queryRetrieveLevel",	//现在版本没有此字段，默认按PaitientLevel来的
		"patientID",
		"patientName",
		"studyDate",
		"studyID",
		"seriesNumber",
		"sopInsUID",
		"sopClassUID",
		"studyUID",
		"seriesUID",
		//---------------------
		"patientDOB",
		"patientSex",
		"seriesDate",
		"seriesTime",
		"studyTime",
		"seriesBodyPart",
		"instanceNumber"
	};

	//先已支持的DcmTagKey(顺序对应于EM_TagKey)
	static DcmTagKey AcceptableDcmTagKey[] = {
		DCM_QueryRetrieveLevel,
		DCM_PatientID,
		DCM_PatientName,
		DCM_StudyDate,
		DCM_StudyID,
		DCM_SeriesNumber,
		DCM_SOPInstanceUID,
		DCM_SOPClassUID,
		DCM_StudyInstanceUID,
		DCM_SeriesInstanceUID,
		DCM_PatientBirthDate,
		DCM_PatientSex,
		DCM_SeriesDate,
		DCM_SeriesTime,
		DCM_StudyTime,
		DCM_BodyPartExamined,
		DCM_InstanceNumber,
		//----------
		DCM_StudyDescription,
		DCM_AccessionNumber,
		DCM_Modality,
		DCM_Rows,
		DCM_Columns,
		DCM_SliceThickness,
		DCM_Manufacturer,
		DCM_SeriesDescription,
		DCM_PixelSpacing
	};

	//获取DcmTagKey
	static DcmTagKey GetManteiaDcmTagKey(std::string str)
	{
		int idx = TK_END;
		for (int i = 0; i < sizeof(TagKeyString) / sizeof(TagKeyString[0]); i++)
		{
			if (str == TagKeyString[i])
			{
				idx = i;
				break;
			}
		}

		if (idx == TK_END)
		{
			for (int i = 0; i < sizeof(TagKeyStringForManteia) / sizeof(TagKeyStringForManteia[0]); i++)
			{
				if (str == TagKeyStringForManteia[i])
				{
					idx = i;
					break;
				}
			}
		}

		switch (idx)
		{
		case TK_QueryRetrieveLevel:
			return DCM_QueryRetrieveLevel;
			break;
		case TK_PatientID:
			return DCM_PatientID;
			break;
		case TK_PatientName:
			return DCM_PatientName;
			break;
		case TK_StudyDate:
			return DCM_StudyDate;
			break;
		case TK_StudyID:
			return DCM_StudyID;
			break;
		case TK_SeriesNumber:
			return DCM_SeriesNumber;
			break;
		case TK_SOPInstanceUID:
			return DCM_SOPInstanceUID;
			break;
		case TK_SOPClassUID:
			return DCM_SOPClassUID;
			break;
		case TK_StudyInstanceUID:
			return DCM_StudyInstanceUID;
			break;
		case TK_SeriesInstanceUID:
			return DCM_SeriesInstanceUID;
			break;
		case TK_PatientBirthDate:
			return DCM_PatientBirthDate;
			break;
		case TK_PatientSex:
			return DCM_PatientSex;
			break;
		case TK_SeriesDate:
			return DCM_SeriesDate;
			break;
		case TK_SeriesTime:
			return DCM_SeriesTime;
			break;
		case TK_StudyTime:
			return DCM_StudyTime;
			break;
		case TK_BodyPartExamined:
			return DCM_BodyPartExamined;
			break;
		case TK_InstanceNumber:
			return DCM_InstanceNumber;
			break;
			
		default:

			break;
		}

		return DcmTagKey();
	};
}


