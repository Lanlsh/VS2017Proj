#pragma once
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/scp.h"
#include "CProcessCommandBase.h"

class CManteiaSCP;

class CProcessCFIND : public CProcessCommandBase
{
public:
	CProcessCFIND(CManteiaSCP* scp);
	~CProcessCFIND();

	OFCondition handleIncomingCommand(T_DIMSE_Message *incomingMsg,
		const DcmPresentationContextInfo &presInfo);

	void NotifyRecvCFINDRequest(const T_DIMSE_C_FindRQ& findRQ);
};

