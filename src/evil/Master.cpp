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
    if(sWorldSocketMgr.StartNetwork() == -1)
    {
        LOG(ERROR)<<"Failed to start network";
        // go down and shutdown the server
    }

    sWorldSocketMgr.Wait();

    /// stop server option
    sWorldSocketMgr.StopNetwork();

    return 0;
}