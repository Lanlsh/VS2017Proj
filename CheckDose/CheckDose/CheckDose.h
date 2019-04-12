#pragma once

#include "checkdose_global.h"
#include "ACPluginBase.h"
#include <QObject>
#include <QProcess>

class CHECKDOSE_EXPORT CheckDose: public ACPluginBase
{
    Q_OBJECT
    //Q_INTERFACES 宏用于告诉 Qt 该类实现的接口。
    Q_INTERFACES(ACPluginInterface)
    //Q_PLUGIN_METADATA宏用于描述插件元数据
    Q_PLUGIN_METADATA(IID AccuCheckPluginInterface_id FILE "checkdose.json")

public:
    CheckDose(CallbackFunForCheckResult callback = nullptr);
    virtual ~CheckDose();

    /*
        Desc: 进行检查
        Param: void
        Return: void
    */
    virtual bool DoCheck();

public slots:
    void slotContourProgress();

protected:
    /*
        Desc: 记录检查结果
        Param: void
        Return: void
    */
    void RecordCheckResult();

private:
    QProcess process;
};


