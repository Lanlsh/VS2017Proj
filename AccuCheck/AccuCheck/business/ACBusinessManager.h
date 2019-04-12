#pragma once
/*
    Desc: AccuCheck的业务管理类
*/
#include <QObject>

class ACPluginManager;
class ACPluginInputInterfaceBase;
class ACPluginOutputInterfaceBase;
class ACPluginBase;
class ACBusinessManager: public QObject
{
    Q_OBJECT

public:
    ACBusinessManager();
    ~ACBusinessManager();

    /*
        Desc: 加载配置信息
    */
    void LoadConfigInfo();

    /*
        Desc: 检查是否支持输入的检查的名字
        Param: checkName 检查的名字
        return: bool   是否支持
    */
    bool CheckIsSupportForTheInputCheckName(QString checkName);

    /*
        Desc: DoCheck
        Param: inputData 输入接口信息
        Param:
        Param: void(*callback)(ACPluginOutputInterfaceBase*) 检测完的回调函数
    */
    void DoCheck(ACPluginInputInterfaceBase* inputData, void(*callback)(ACPluginOutputInterfaceBase*) );

protected:
    /*
        Desc: 加载相应的配置文件
    */
    void LoadConfigFile();

    /*
        Desc: 加载插件
    */
    void LoadPlugs();

    /*
        Desc: 获取需要使用的插件
        Param: name 需要检查的名字
        Return: ACPluginBase* 返回的插件
    */
    ACPluginBase* GetPluginByCheckName(QString name);

private:
    ACPluginManager* m_pluginManager;   //插件管理者
};

