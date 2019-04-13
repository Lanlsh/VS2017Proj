#include "ACPluginOutputForCheckDose.h"



ACPluginOutputForCheckDose::ACPluginOutputForCheckDose()
{
}


ACPluginOutputForCheckDose::ACPluginOutputForCheckDose(const ACPluginOutputForCheckDose& base)
{
    this->SetCheckName(base.GetCheckName());
    this->SetCheckProcessIsNormal(base.GetCheckProcessIsNormal());
    this->SetErrorMsg(base.GetErrorMsg());
}

ACPluginOutputForCheckDose::~ACPluginOutputForCheckDose()
{
}
