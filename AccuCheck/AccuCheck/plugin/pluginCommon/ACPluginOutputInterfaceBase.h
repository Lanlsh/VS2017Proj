#pragma once
/*
    Desc: 插件输出接口的基类
*/
#include "ACIOInterface.h"
#include <QString>

class ACPluginOutputInterfaceBase: public ACIOInterface
{
public:
    ACPluginOutputInterfaceBase();
    virtual ~ACPluginOutputInterfaceBase();

    /*
        Desc: 深拷贝函数
    */
    ACPluginOutputInterfaceBase(const ACPluginOutputInterfaceBase& base);

    /*
        Desc: 克隆模式
    */
    virtual ACIOInterface* Clone()
    {
        return new ACPluginOutputInterfaceBase(*this);
    }

    /*
        Desc: 设置检查类型的名字
        Param: name 检查类型的名字
        Return: void
    */
    void SetCheckName(QString name)
    {
        m_checkName = name;
    }

    /*
        Desc: 获取检查类型的名字
        Param: void
        Return: QString
    */
    QString GetCheckName() const
    {
        return m_checkName;
    }

    /*
        Desc: 设置检测过程是否正常
        Param: bIsOK 是否正常
        Return: void
    */
    void SetCheckProcessIsNormal(bool bIsOK)
    {
        m_bCheckProcessIsNormal = bIsOK;
    }

    /*
        Desc: 获取检测过程是否正常
        Param: void
        Return: bool 是否正常
    */
    bool GetCheckProcessIsNormal() const
    {
        return m_bCheckProcessIsNormal;
    }

    /*
        Desc: 设置错误信息
        Param: errStr 错误信息
        Return: void
    */
    void SetErrorMsg(QString errStr)
    {
        m_errorMsg = errStr;
    }

    /*
        Desc: 获取错误信息
        Param: void
        Return: QString 错误信息
    */
    QString GetErrorMsg() const
    {
        return m_errorMsg;
    }

private:
    QString m_checkName;    //对应的检查类型的名字
    bool m_bCheckProcessIsNormal;   //检测过程是否顺利
    QString m_errorMsg; //错误信息
};

