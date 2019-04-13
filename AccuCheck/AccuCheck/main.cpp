#include <QtCore/QCoreApplication>
//#include "./plugin/ACPluginManager.h"
#include "./business/ACBusinessManager.h"
#include "./plugin/pluginCommon/ACPluginInputForCheckDose.h"
#include "./plugin/pluginCommon/ACPluginOutputForCheckDose.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    //ACPluginManager manager;
    //manager.LoadAllPlugins();
    ACBusinessManager manager;
    ACPluginInputForCheckDose inputData;
    inputData.SetCheckName("checkdose");
    inputData.SetPlanPath("E:/QA/MonacoPatient/MonacoPatient/1~L6MRLRANDO/plan/Prostate/plan");
    ACPluginOutputForCheckDose outputData;
    outputData.SetCheckName("checkdose");
    manager.DoCheck(&inputData, &outputData);
    return a.exec();
}
