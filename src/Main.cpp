/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#include "Common.h"
#include "Master.h"
#include "config.h"
#include "PosixDaemon.h"

extern int main(int argc, char** argv)
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

    return sMaster.Run();
}
