#include "ACPluginManager.h"
#include "ACPluginManagerPrivate.h"
#include "./pluginCommon/ACPluginBase.h"
#include <QDir>
#include <QCoreApplication>
#include <QLibrary>

ACPluginManager::ACPluginManager()
{
    m_data = new ACPluginManagerPrivate();
}


ACPluginManager::~ACPluginManager()
{
    delete m_data;
    m_data;
}

void ACPluginManager::LoadAllPlugins()
{
    // 进入插件目录
    QDir path = QDir(QCoreApplication::applicationDirPath());
    bool bIsHavePluginFolder = path.cd("plugins");

    if (!bIsHavePluginFolder)
    {
        //如果插件文件夹不存在，则返回
        return;
    }

    // 初始化插件中的元数据
    QFileInfoList lstInfo = path.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (int i = 0; i < lstInfo.size(); i++)
    {
        QFileInfo info = lstInfo.at(i);
        QString pluginName = QString();
        ScanContainsPluginsFromISONFile(info.absoluteFilePath(), pluginName);
        LoadPlugin(info.absoluteFilePath(), pluginName);
    }
}

void ACPluginManager::LoadPlugin(const QString& filePath, const QString& pluginName)
{
    // 判断是否是库
    if (!QLibrary::isLibrary(filePath))
        return;

    // 检测插件依赖
    //if (!d->check(path))
    //  return;
    // 加载插件
    QPluginLoader* loader = new QPluginLoader(filePath);

    if (loader->load())
    {
        // 如果继承自 ACPluginInterface，则认为是自己的插件（防止外部插件注入）。
        ACPluginInterface* plugin = qobject_cast<ACPluginInterface*>(loader->instance());

        if (plugin)
        {
            plugin->pluginName = pluginName;
            m_data->m_loaders.insert(filePath, loader);
        }
        else
        {
            delete loader;
            loader = Q_NULLPTR;
        }
    }
}

void ACPluginManager::ScanContainsPluginsFromISONFile(const QString& filePath, QString& pluginName)
{
    /*****
     *  判断是否是库（后缀有效性） 检查的是动态库！！
     * Windows：.dll、.DLL
     * Unix/Linux：.so
    *****/
    if (!QLibrary::isLibrary(filePath))
        return;

    // 获取元数据（名称、版本、依赖）
    QPluginLoader* loader = new QPluginLoader(filePath);
    QJsonObject json = loader->metaData().value("MetaData").toObject();
    m_data->m_names.insert(filePath, json.value("name").toVariant());
    pluginName = json.value("name").toString();
    m_data->m_versions.insert(filePath, json.value("version").toVariant());
    //d->m_dependencies.insert(path, json.value("dependencies").toArray().toVariantList());
    delete loader;
    loader = Q_NULLPTR;
}

void ACPluginManager::UnloadAllPlugins()
{
    QList<QString> lstPathFile = m_data->m_loaders.keys();

    for (int i = 0; i < lstPathFile.size(); i++)
    {
        const QString& filePath = lstPathFile.at(i);
        UnloadPlugin(filePath);
    }
}

void ACPluginManager::UnloadPlugin(const QString& filePath)
{
    QPluginLoader* loader = m_data->m_loaders.value(filePath);

    // 卸载插件，并从内部数据结构中移除
    if (loader->unload())
    {
        m_data->m_loaders.remove(filePath);
        delete loader;
        loader = Q_NULLPTR;
    }
}

QList<QPluginLoader* > ACPluginManager::GetAllPlugins()
{
    return m_data->m_loaders.values();
}

QList<QVariant> ACPluginManager::GetAllLoadedPluginsNames()
{
    return m_data->m_names.values();
}

ACPluginBase* ACPluginManager::GetPluginByCheckName(QString name)
{
    if (m_data->m_names.key(name) != QString())
    {
        QPluginLoader* loader = m_data->m_loaders.value(m_data->m_names.key(name));
        return dynamic_cast<ACPluginBase*>(qobject_cast<ACPluginInterface*>(loader->instance()));
    }

    return nullptr;
}
