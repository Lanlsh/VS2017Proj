#pragma once
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/scp.h"
#include "CProcessCommandBase.h"

class CManteiaSCP;

class CProcessCMOVE: public CProcessCommandBase
{
public:
	CProcessCMOVE(CManteiaSCP* scp);
	~CProcessCMOVE();

	OFCondition handleIncomingCommand(T_DIMSE_Message *incomingMsg, 
		const DcmPresentationContextInfo &presInfo);

	void NotifyRecvCMOVERequest(DIC_UI AffectedSOPClassUID, DIC_AE MoveDestination);
};

