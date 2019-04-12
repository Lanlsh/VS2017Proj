#include "ACPluginOutputInterfaceBase.h"

ACPluginOutputInterfaceBase::ACPluginOutputInterfaceBase(QString name)
{
    m_checkName = name;
    m_bCheckProcessIsNormal = false;
    m_errorMsg = QString();
}

ACPluginOutputInterfaceBase::~ACPluginOutputInterfaceBase()
{
}
