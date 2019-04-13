#include "ACBusinessManager.h"
#include "./common/AccuCheckCommon.h"
#include "./plugin/ACPluginManager.h"
#include "./plugin/pluginCommon/ACPluginInputInterfaceBase.h"
#include "./plugin/pluginCommon/ACPluginOutputInterfaceBase.h"
#include "./plugin/pluginCommon/ACPluginInputForCheckDose.h"
#include "./plugin/pluginCommon/ACPluginOutputForCheckDose.h"
#include "./plugin/pluginCommon/ACPluginBase.h"
#include "./plugin/pluginCommon/ACPluginInterface.h"
#include <QVariant>
#include <QDebug>
#include <QPluginLoader>
#include <QList>
#include <functional>

ACBusinessManager::ACBusinessManager()
{
    m_pluginManager = new ACPluginManager();
    LoadConfigInfo();
}


ACBusinessManager::~ACBusinessManager()
{
    ACDelete(m_pluginManager);
}

void ACBusinessManager::LoadConfigInfo()
{
    LoadConfigFile();
    LoadPlugs();
}

bool ACBusinessManager::CheckIsSupportForTheInputCheckName(QString checkName)
{
    QList<QVariant> lstName = m_pluginManager->GetAllLoadedPluginsNames();

    if (lstName.empty())
    {
        return false;
    }

    QList<QVariant>::Iterator iter = lstName.begin();

    for (; iter != lstName.end(); iter++)
    {
        if (iter->toString() == checkName)
        {
            return true;
        }
    }

    return false;
}

void ACBusinessManager::DoCheck(ACPluginInputInterfaceBase* inputData, ACPluginOutputInterfaceBase* outputData)
{
    QString checkName = inputData->GetCheckName();

    if (CheckIsSupportForTheInputCheckName(checkName))
    {
        ACPluginBase* plugin = GetPluginByCheckName(checkName);

        if (!plugin)
        {
            qDebug() << "任务失败： " << inputData->GetCheckName() << "任务";
            return;
        }

        //设置输入数据接口
        ACIOInterface* newInputData = inputData->Clone();
        plugin->SetInputIterface(dynamic_cast<ACPluginInputInterfaceBase*>(newInputData));
        //设置输出数据接口
        ACIOInterface* newOutPutData = outputData->Clone();
        plugin->SetOutputIterface(dynamic_cast<ACPluginOutputInterfaceBase*>(newOutPutData));
        //设置检测回调函数
        CallbackFunForCheckResult callbackFun = std::bind(&ACBusinessManager::CallbackForCheckResult, this, std::placeholders::_1);
        plugin->SetResultCallback(callbackFun);
        //执行任务
        plugin->DoCheck();
    }
    else
    {
        qDebug() << "暂时不支持" << inputData->GetCheckName() << "功能";
    }
}

void ACBusinessManager::LoadConfigFile()
{
}

void ACBusinessManager::LoadPlugs()
{
    m_pluginManager->LoadAllPlugins();
}

ACPluginBase* ACBusinessManager::GetPluginByCheckName(QString name)
{
    return m_pluginManager->GetPluginByCheckName(name);
}

void ACBusinessManager::CallbackForCheckResult(ACPluginOutputInterfaceBase* ouputData)
{
    qDebug("检查结果： ");
    qDebug() << "检查类型名字： " << ouputData->GetCheckName();
    qDebug() << "检查过程中是否正常： " << ouputData->GetCheckProcessIsNormal();
    qDebug() << "检查过程中的错误信息： " << ouputData->GetErrorMsg();
    //将检查结果添加到队列
    m_mtxForCheckResultQueue.lock();
    m_queueForcheckResult.push_back( dynamic_cast<ACPluginOutputInterfaceBase*>(ouputData->Clone()) );
    m_mtxForCheckResultQueue.unlock();
}
