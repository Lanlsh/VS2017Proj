#pragma once
/*
    Desc: CheckDose的输入接口类
*/

#include "ACPluginInputInterfaceBase.h"

class ACPluginInputForCheckDose: public ACPluginInputInterfaceBase
{
public:
    ACPluginInputForCheckDose();
    ~ACPluginInputForCheckDose();

    /*
        Desc: 深拷贝函数
    */
    ACPluginInputForCheckDose(const ACPluginInputForCheckDose& base);

    /*
        Desc: 克隆模式
    */
    ACPluginInputInterfaceBase* Clone()
    {
        return new ACPluginInputForCheckDose(*this);
    }

    /*
        Desc: 设置计划的路径
        Param: planPath 计划的路径
        return: void
    */
    void SetPlanPath(QString planPath)
    {
        m_planPath = planPath;
    }

    /*
        Desc: 获取计划的路径
        Param: void
        return: QString 当前计划的路径
    */
    QString GetPlanPath() const
    {
        return m_planPath;
    }

private:
    QString m_planPath = QString(); //计划的路径
};

