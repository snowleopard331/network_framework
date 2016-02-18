/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#include "Common.h"
#include "Master.h"
#include "config.h"

extern int main(int argc, char** argv)
{
    /// init log
    sLog;
    
    if(!sConfig.setSource("./config"))
    {
        LOG(ERROR)<<"Could not find configuration file config";
        return 1;
    }

    return sMaster.Run();
}
