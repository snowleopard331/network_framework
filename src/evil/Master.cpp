/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#include "Master.h"
#include "Common.h"
#include "worldSocketMgr.h"

// volatile uint32 Master::m_MasterLoopCounter = 0;

Master::Master()
{

}

Master::~Master()
{

}

// Main function
int Master::Run()
{
    // launch the world listener socket
    uint16 port = sConfig.getIntDefault("Network", "Port", 10301);
    // std::string addr = sConfig.getStringDefault("Network", "Ip", "127.0.0.1");

    if(sWorldSocketMgr.StartNetwork(port) == -1)
    {
        LOG(ERROR)<<"Failed to start network";
        return -1;
    }

    sWorldSocketMgr.Wait();

    return 0;
}