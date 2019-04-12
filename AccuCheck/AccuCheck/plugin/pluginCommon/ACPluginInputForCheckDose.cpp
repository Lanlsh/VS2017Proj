#include "ACPluginInputForCheckDose.h"



ACPluginInputForCheckDose::ACPluginInputForCheckDose()
{
}


ACPluginInputForCheckDose::ACPluginInputForCheckDose(const ACPluginInputForCheckDose& base)
{
    this->SetCheckName(base.GetCheckName());
    this->SetPlanPath(base.GetPlanPath());
}

ACPluginInputForCheckDose::~ACPluginInputForCheckDose()
{
}
