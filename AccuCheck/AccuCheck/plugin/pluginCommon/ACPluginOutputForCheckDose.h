#pragma once
/*
    Desc: CheckDose的输出接口类
*/

#include "ACPluginOutputInterfaceBase.h"

class ACPluginOutputForCheckDose: public ACPluginOutputInterfaceBase
{
public:
    ACPluginOutputForCheckDose();
    ~ACPluginOutputForCheckDose();

    /*
        Desc: 深拷贝函数
    */
    ACPluginOutputForCheckDose(const ACPluginOutputForCheckDose& base);

    /*
        Desc: 克隆模式
    */
    virtual ACIOInterface* Clone()
    {
        return new ACPluginOutputForCheckDose(*this);
    }

};

