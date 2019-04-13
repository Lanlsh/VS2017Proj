#pragma once
/*
    Desc: AccuCheck的业务管理类
*/
#include <QObject>
#include <QQueue>

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
        Param: outputData 输出接口信息
        Return: void
    */
    void DoCheck(ACPluginInputInterfaceBase* inputData, ACPluginOutputInterfaceBase* outputData);

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

    /*
        Desc: 检查结果回调函数
        Param: void
    */
    void CallbackForCheckResult(ACPluginOutputInterfaceBase* ouputData);

private:
    ACPluginManager* m_pluginManager;   //插件管理者
    QQueue<ACPluginOutputInterfaceBase* > m_queueForcheckResult;    //检查结果队列，此队列保存所有的输出结果
};

