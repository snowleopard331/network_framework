/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/21
*/

#include "Common.h"
#include "PosixDaemon.h"
#include "Master.h"


int main(int argc, char** argv)
{
    /// initialize log
    sLog.initialize(argv[0]);

    /// log argv
    for(int i = 0; i <= argc; ++i)
    {
        LOG(INFO)<<"argv["<<i<<"]: "<<argv[i];
    }

    /// initialize config
    if(!sConfig.setSource("./config"))
    {
        LOG(ERROR)<<"Could not find configuration file config";
        return 1;
    }

    startDaemon();

    return sMaster.run();
}