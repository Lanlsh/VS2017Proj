#include "CProcessCommandBase.h"
#include "stdafx.h"
#include <assert.h>

CProcessCommandBase::CProcessCommandBase(CManteiaSCP* scp)
{
	assert(scp);
	m_scp = scp;
}


CProcessCommandBase::~CProcessCommandBase()
{
}

CManteiaSCP * CProcessCommandBase::GetSCPInstance()
{
	return m_scp;
}
