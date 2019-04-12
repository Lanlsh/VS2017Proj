#pragma once
/*
    Desc: 插件的基类
*/

#include <QObject>
#include "ACPluginInterface.h"

class ACPluginInputInterfaceBase;
class ACPluginOutputInterfaceBase;

typedef void(*CallbackFunForCheckResult)(ACPluginOutputInterfaceBase*);

class ACPluginBase: public QObject, public ACPluginInterface
{
    Q_OBJECT
    //Q_INTERFACES 宏用于告诉 Qt 该类实现的接口。
    Q_INTERFACES(ACPluginInterface)
    //Q_PLUGIN_METADATA宏用于描述插件元数据
    //Q_PLUGIN_METADATA(IID AccuCheckPluginInterface_id)
public:
    ACPluginBase(CallbackFunForCheckResult callback = nullptr);
    virtual ~ACPluginBase();

    /*
        Desc: 进行检查
        Param: void
        Return: void
    */
    virtual bool DoCheck()
    {
        return true;
    };

    /*
        Desc: 设置插件输入接口
        Param: inputData 输入数据
        Return: void
        Remarks: 设置输入接口对象时，此对象将由相应的插件类进行管理，防止内存泄漏
    */
    void SetInputIterface(ACPluginInputInterfaceBase* inputData);

    /*
        Desc: 获取插件输入接口
        Param: void
        Return: ACPluginInputInterfaceBase*
    */
    ACPluginInputInterfaceBase* GetInputIterface()
    {
        return m_plugInputData;
    }

    /*
        Desc: 释放输入接口内存
        Param: void
        Return: void
    */
    void FreeInputInterfaceObj()
    {
        if (m_plugInputData)
        {
            delete m_plugInputData;
            m_plugInputData = nullptr;
        }
    }

    /*
        Desc: 设置插件输出接口
        Param: inputData 输出数据
        Return: void
        Remarks: 设置输出接口对象时，此对象将由相应的插件类进行管理，防止内存泄漏
    */
    void SetOutputIterface(ACPluginOutputInterfaceBase* outputData);

    /*
        Desc: 获取插件输出接口
        Param: void
        Return: ACPluginOutputInterfaceBase*
    */
    ACPluginOutputInterfaceBase* GetOutputIterface()
    {
        return m_plugOutputData;
    }

    /*
        Desc: 释放输出接口内存
        Param: void
        Return: void
    */
    void FreeOutputInterfaceObj()
    {
        if (m_plugOutputData != nullptr)
        {
            delete m_plugOutputData;
            m_plugOutputData = nullptr;
        }
    }

    /*
        Desc: 设置检查结果的回调函数
        Param: callback 回调函数地址
        Return: void
    */
    void SetResultCallback(CallbackFunForCheckResult callback)
    {
        m_reultCallback = callback;
    }

    /*
        Desc: 执行检查结果回调函数
        Param: void
        Return: void
    */
    void DoResultCallbackFun();

private:
    QString m_errorStr = QString(); //错误信息
    ACPluginInputInterfaceBase* m_plugInputData = nullptr;    //插件输入接口信息
    ACPluginOutputInterfaceBase* m_plugOutputData = nullptr;  //插件输出接口信息
    CallbackFunForCheckResult m_reultCallback = nullptr;  //检测完的回调函数
};
