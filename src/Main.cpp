/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#include "Common.h"
#include "Master.h"
#include "config.h"
#include "PosixDaemon.h"

#ifdef DEBUG_INFO_CALLGRIND
    void PorgramExitHandle(int exitCode)
    {
        LOG(ERROR)<<"program exit in DEBUG_INFO_CALLGRIND mode, exit: "<<exitCode;
        exit(exitCode);
    }
#endif

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

#ifdef DEBUG_INFO_CALLGRIND
    boost::asio::io_service iosv;
    boost::asio::deadline_timer timer(iosv, boost::posix_time::seconds(30));
    timer.async_wait(boost::bind(&PorgramExitHandle, 2));
    iosv.run();
#endif
    
    return sMaster.Run();
}
