#pragma once

class CManteiaSCP;

class CProcessCommandBase
{
public:
	CProcessCommandBase(CManteiaSCP* scp);
	virtual ~CProcessCommandBase();

	CManteiaSCP* GetSCPInstance();
private:
	CManteiaSCP* m_scp;
};

