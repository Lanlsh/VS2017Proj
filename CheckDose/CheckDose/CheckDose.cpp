#include "CheckDose.h"
#include "ACPluginInputForCheckDose.h"
#include "ACPluginOutputForCheckDose.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QJsonObject>
#include <QDebug>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>

CheckDose::CheckDose(CallbackFunForCheckResult callback):
    ACPluginBase(callback)
{
}

CheckDose::~CheckDose()
{
}

bool CheckDose::DoCheck()
{
    //获取输入
    ACPluginInputForCheckDose* inputData = dynamic_cast<ACPluginInputForCheckDose*>(GetInputIterface());

    if (!inputData)
    {
        qDebug() << "为设置ACPluginInputForCheckDose对象： 该对象的NULL";
        return false;
    }

    QString planPath = inputData->GetPlanPath();
    //由于现在调用的黑盒exe是用c#写的，所以，路径需要做下更改
    planPath = "\"" + planPath + "\"";
    //QProcess process;
    QString nativeArguments = planPath;
    QString exePath = QCoreApplication::applicationDirPath() + "/Manteia.exe";  //暂时先写死
    qDebug() << "exePath: " << exePath;
    process.setNativeArguments(nativeArguments);
    bool debug = connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotContourProgress()));
    process.start(exePath);
    process.waitForFinished(-1);
    //记录检查结果
    RecordCheckResult();
    //执行检查结果回调函数
    DoResultCallbackFun();
    return true;
}

void CheckDose::slotContourProgress()
{
    QByteArray ba = process.readAllStandardOutput();

    if (ba.size() > 0)
    {
        QString percentStr = ba;
        int percent = percentStr.toInt();
        qDebug() << percentStr;
    }
}

void CheckDose::RecordCheckResult()
{
    QFile loadFile("C:/temp/MuInfo.json");

    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "could't open projects json";
        return;
    }

    QByteArray allData = loadFile.readAll();
    loadFile.close();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));

    if (json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "json error!";
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    //输出所有key，这一步是非必须的，放最后的话，你可能读不到任何key
    QStringList keys = rootObj.keys();

    for (int i = 0; i < keys.size(); i++)
    {
        qDebug() << "key" << i << " is:" << keys.at(i);
    }

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if (rootObj.contains("DoseInfo"))
    {
        qDebug() << "------------------DoseInfo-------------------------";
        QJsonObject subObj = rootObj.value("DoseInfo").toObject();
        qDebug() << "CalculationGrid is:" << subObj["CalculationGrid"].toString();
        qDebug() << "DoseCalculationMethod is:" << subObj["DoseCalculationMethod"].toString();
        qDebug() << "DoseReferencePoint is:" << subObj["DoseReferencePoint"].toString();
        qDebug() << "GammaCriteria is:" << subObj["GammaCriteria"].toString();
        qDebug() << "PassingPoints is:" << subObj["PassingPoints"].toString();
        qDebug() << "PassingRate is:" << subObj["PassingRate"].toString();
        qDebug() << "PointsCalculated is:" << subObj["PointsCalculated"].toString();
        qDebug() << "PointsaboveThreshold is:" << subObj["PointsaboveThreshold"].toString();
        qDebug() << "TPSDose is:" << subObj["TPSDose"].toString();
        qDebug() << "Threshold is:" << subObj["Threshold"].toString();
        qDebug() << "planId is:" << subObj["planId"].toString();
        qDebug() << "------------------DoseInfo-------------------------";
    }

    if (rootObj.contains("MuInfo"))
    {
        qDebug() << "------------------MuInfo-------------------------";
        QJsonArray subArray = rootObj.value("MuInfo").toArray();

        for (int i = 0; i < subArray.size(); i++)
        {
            qDebug() << i << " value is:" << subArray.at(i).toString();
        }

        qDebug() << "------------------MuInfo-------------------------";
    }

    if (rootObj.contains("PatientInfo"))
    {
        QJsonObject subObj = rootObj.value("PatientInfo").toObject();
        qDebug() << "patientDateOfBirth is:" << subObj.value("patientDateOfBirth").toString();
        qDebug() << "patientGender is:" << subObj.value("patientGender").toString();
        qDebug() << "patientID is:" << subObj.value("patientID").toString();
        qDebug() << "patientName is:" << subObj.value("patientName").toString();
    }

    if (rootObj.contains("Result"))
    {
        QJsonObject subObj = rootObj.value("Result").toObject();
        int execCode = subObj.value("code").toInt();
        qDebug() << "code is:" << execCode;
        ACPluginOutputForCheckDose* outputData = dynamic_cast<ACPluginOutputForCheckDose*>(GetOutputIterface());

        if (!outputData)
        {
            qDebug() << "为设置ACPluginOutputForCheckDose对象： 该对象的NULL";
            return;
        }

        if (execCode == 0)
        {
            outputData->SetCheckProcessIsNormal(true);
        }
        else
        {
            outputData->SetCheckProcessIsNormal(false);
        }

        qDebug() << "message is:" << subObj.value("message").toString();
        qDebug() << "stackTrace is:" << subObj.value("stackTrace").toString();
        outputData->SetErrorMsg(subObj.value("stackTrace").toString());
    }
}
