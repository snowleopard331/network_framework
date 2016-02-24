/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#include "Master.h"
#include "Common.h"
#include "worldSocketMgr.h"

volatile uint32 Master::m_MasterLoopCounter = 0;

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
    uint16 port = 10301;
    std::string addr = "192.168.195.131";

    if(sWorldSocketMgr.StartNetwork(port, addr) == -1)
    {
        LOG(ERROR)<<"Failed to start network";
        return -1;
    }

    sWorldSocketMgr.Wait();

    return 0;
}