#include "ACPluginOutputInterfaceBase.h"

ACPluginOutputInterfaceBase::ACPluginOutputInterfaceBase()
{
    m_checkName = QString();
    m_bCheckProcessIsNormal = false;
    m_errorMsg = QString();
}

ACPluginOutputInterfaceBase::ACPluginOutputInterfaceBase(const ACPluginOutputInterfaceBase& base)
{
    this->SetCheckName(base.GetCheckName());
    this->SetCheckProcessIsNormal(base.GetCheckProcessIsNormal());
    this->SetErrorMsg(base.GetErrorMsg());
}

ACPluginOutputInterfaceBase::~ACPluginOutputInterfaceBase()
{
}
