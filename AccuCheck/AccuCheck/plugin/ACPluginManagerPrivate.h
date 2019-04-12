#pragma once
/*
    Desc: ACPluginManager的数据层
*/

#include <QHash>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QPluginLoader>

class ACPluginManagerPrivate
{
public:
    ACPluginManagerPrivate();
    ~ACPluginManagerPrivate();

    QHash<QString, QVariant> m_names;  // 插件路径 - 名称
    QHash<QString, QVariant> m_versions;  // 插件路径 - 版本
    //QHash<QString, QVariantList> m_dependencies;  // 插件路径 - 其额外依赖的插件
    QHash<QString, QPluginLoader*> m_loaders;   // 插件路径 - QPluginLoader 实例
};

