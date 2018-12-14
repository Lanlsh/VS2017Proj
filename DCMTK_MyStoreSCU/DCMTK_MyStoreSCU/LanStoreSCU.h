#pragma once
#include "dcmtk/dcmnet/dstorscu.h"

class LanStoreSCU :public DcmStorageSCU
{
public:
	LanStoreSCU();
	~LanStoreSCU();
};

