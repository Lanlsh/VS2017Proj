#pragma once
/*
    Desc: AccuCheck的输入输出接口的基类,采用克隆模式
*/
class ACIOInterface
{
public:
    ACIOInterface();
    ~ACIOInterface();

    /*
        Desc: 克隆函数
    */
    virtual ACIOInterface* Clone() = 0;
};

