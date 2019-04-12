#pragma once
/*
    Desc: 插件的接口类(AccuCheck的插件的最基本的基类)
*/

#include <QObject>
class ACPluginInterface
{
public:
    ACPluginInterface();
    virtual ~ACPluginInterface();

    /*
        Desc: 进行检查
        Param: void
        Return: void
    */
    virtual bool DoCheck() = 0;

public:
    QString pluginName = QString(); //插件名字
};

//后期实现的所有插件，都必须继承自 ACPluginInterface，这样才会被认定是自己的插件，以防外部插件注入。
//使用 Q_DECLARE_INTERFACE 宏，将 Plugin 接口与标识符一起公开。
#define AccuCheckPluginInterface_id "Mantiea.AccuCheck.Plugins"
Q_DECLARE_INTERFACE(ACPluginInterface, AccuCheckPluginInterface_id)
