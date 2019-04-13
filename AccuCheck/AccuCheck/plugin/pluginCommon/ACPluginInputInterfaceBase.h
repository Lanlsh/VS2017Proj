#pragma once
/*
    Desc: 插件输入接口的基类
*/
#include <QObject>
#include "ACIOInterface.h"

class ACPluginInputInterfaceBase: public ACIOInterface
{
public:
    ACPluginInputInterfaceBase();
    virtual ~ACPluginInputInterfaceBase();

    /*
        Desc: 深拷贝函数
    */
    ACPluginInputInterfaceBase(const ACPluginInputInterfaceBase& base);

    /*
        Desc: 克隆模式
    */
    virtual ACIOInterface* Clone()
    {
        return new ACPluginInputInterfaceBase(*this);
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

private:
    QString m_checkName = QString();    //对应的检查类型的名字
};

