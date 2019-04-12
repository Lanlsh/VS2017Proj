#include "ACPluginBase.h"
#include "ACPluginInputInterfaceBase.h"
#include "ACPluginOutputInterfaceBase.h"

ACPluginBase::ACPluginBase(CallbackFunForCheckResult callback)
{
    m_reultCallback = callback;
}

ACPluginBase::~ACPluginBase()
{
    if (m_plugInputData)
    {
        delete m_plugInputData;
        m_plugInputData = nullptr;
    }

    if (m_plugOutputData)
    {
        delete m_plugOutputData;
        m_plugOutputData = nullptr;
    }
}

void ACPluginBase::SetInputIterface(ACPluginInputInterfaceBase* inputData)
{
    if (inputData)
    {
        FreeInputInterfaceObj();
        m_plugInputData = inputData;
    }
}

void ACPluginBase::SetOutputIterface(ACPluginOutputInterfaceBase* outputData)
{
    if (outputData)
    {
        FreeOutputInterfaceObj();
        m_plugOutputData = outputData;
    }
}

void ACPluginBase::DoResultCallbackFun()
{
    if (m_reultCallback)
    {
        m_reultCallback(m_plugOutputData);
    }
}

