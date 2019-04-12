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

void ACBusinessManager::DoCheck(ACPluginInputInterfaceBase* inputData, void(*callback)(ACPluginOutputInterfaceBase*))
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
        ACPluginInputInterfaceBase* newInputData = inputData->Clone();
        plugin->SetInputIterface(newInputData);
        //设置输出数据接口
        ACPluginOutputForCheckDose* outPutData = new ACPluginOutputForCheckDose(checkName);
        plugin->SetOutputIterface(outPutData);
        //设置检测回调函数
        plugin->SetResultCallback(callback);
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
