#pragma once
/*
    Desc: 插件管理器类
*/
#include <QString>
#include <QPluginLoader>

class ACPluginManagerPrivate;
class ACPluginBase;
class ACPluginManager
{
public:
    ACPluginManager();
    virtual ~ACPluginManager();

    /*
        Desc: 加载所有的插件
        Param: void
        Return: void
    */
    void LoadAllPlugins();

    /*
        Desc: 加载插件
        Param: filePath JSON文件的完整路径
        Param: pluginName 插件名字
        Return: void
    */
    void LoadPlugin(const QString& filePath, const QString& pluginName);

    /*
        Desc: 扫描出JSON文件中包含的插件
        Param: filePath JSON文件的完整路径
        Param: pluginName 插件名字
        Return: void
    */
    void ScanContainsPluginsFromISONFile(const QString& filePath, QString& pluginName);

    /*
        Desc: 卸载所有插件
        Param: void
        Return: void
    */
    void UnloadAllPlugins();

    /*
        Desc: 卸载指定插件
        Param:
        Return: void
    */
    void UnloadPlugin(const QString& filePath);

    /*
        Desc: 获取所有插件
        Param: void
        Return: QList<QPluginLoader* >  存储所有插件的list
    */
    QList<QPluginLoader* > GetAllPlugins();

    /*
        Desc: 获取所有已加载的插件的名称
        Param: void
        Return: QList<QVariant> 所有已加载的插件的名称
    */
    QList<QVariant> GetAllLoadedPluginsNames();

    /*
        Desc: 获取需要使用的插件
        Param: name 需要检查的名字
        Return: ACPluginBase* 返回的插件
    */
    ACPluginBase* GetPluginByCheckName(QString name);

protected:

private:
    ACPluginManagerPrivate* m_data; //插件管理器类的数据
};

