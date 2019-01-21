#pragma once
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/scp.h"
#include "CProcessCommandBase.h"
#include "CStoreSCUForCGet.h"

class CManteiaSCP;

class CProcessCGET : public CProcessCommandBase
{
public:
	CProcessCGET(CManteiaSCP* scp);
	~CProcessCGET();

	OFCondition handleIncomingCommand(T_DIMSE_Message *incomingMsg,
		const DcmPresentationContextInfo &presInfo);

	void NotifyRecvCFINDRequest(const T_DIMSE_C_GetRQ& getRQ);

private:
	CStoreSCUForCGet* m_storeSCU;
};

