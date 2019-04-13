#include "ACPluginInputInterfaceBase.h"

ACPluginInputInterfaceBase::ACPluginInputInterfaceBase()
{
}

ACPluginInputInterfaceBase::ACPluginInputInterfaceBase(const ACPluginInputInterfaceBase& base)
{
    this->SetCheckName(base.GetCheckName());
}

ACPluginInputInterfaceBase::~ACPluginInputInterfaceBase()
{
}

