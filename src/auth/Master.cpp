/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/22
*/

#include "Master.h"
#include "Common.h"
#include "authSocketMgr.h"

#define AUTH_LOOP_INTERVAL      100000 // microsec

Master::Master()
    : m_stopEvent(false)
    , m_loopCounter(0)
    , m_numLoops(0)
{

}

Master::~Master()
{
    
}

int Master::run()
{
    /// Initialize the database connection
    if(!_startDB())
    {
        LOG(ERROR)<<"start database failed";
        return -1;
    }

    /// Initialize network
    if(sAuthSockMgr.startNetwork() == -1)
    {
        LOG(ERROR)<<"start network failed";
        return -1;
    }

    Proactor* pProactor = sAuthSockMgr.proactor();
    if(pProactor == nullptr)
    {
        LOG(ERROR)<<"get Proactor is nullptr";
        return -1;
    }

    /// Loop
    m_numLoops = sConfig.getIntDefault("SQL", "MaxPingTime", 30) * (MINUTE * 1000000 / AUTH_LOOP_INTERVAL);
    if(!m_stopEvent)
    {
        boost::asio::deadline_timer timer((*pProactor), boost::posix_time::microsec(AUTH_LOOP_INTERVAL));
        timer.async_wait(boost::bind(&Master::_masterLoop, this, boost::ref(timer)));
    }

    Proactor::work work(*pProactor);

    /// blocking
    pProactor->run();

    return 0;
}

void Master::stop()
{
    // Wait for the delay thread to exit
    m_loginDatabase.HaltDelayThread();

    // stop network
    sAuthSockMgr.stopNetwork();

    LOG(INFO)<<"Halting process...";

    return;
}

void Master::_masterLoop(boost::asio::deadline_timer &timer)
{
    if(sAuthSockMgr.proactor() && sAuthSockMgr.proactor()->stopped())
    {
        LOG(ERROR)<<"[Auth] proactor stopped";
        return;
    }

    if(++m_loopCounter == m_numLoops)
    {
        m_loopCounter = 0;
        LOG(INFO)<<"Ping MySQL to keep connection alive";
        m_loginDatabase.Ping();
    }

    if(!m_stopEvent)
    {
        timer.expires_from_now(boost::posix_time::microsec(AUTH_LOOP_INTERVAL));
        timer.async_wait(boost::bind(&Master::_masterLoop, this, boost::ref(timer)));
    }
    else
    {
        this->stop();
    }
}

bool Master::_startDB()
{
    std::string dbstring = sConfig.getStringDefault("SQL", "LoginDatabaseInfo", "");

    if(dbstring.empty())
    {
        LOG(ERROR)<<"Database not specified";
        return false;
    }

    if(!m_loginDatabase.Initialize(dbstring.c_str()))
    {
        LOG(ERROR)<<"can not connect to database";
        return false;
    }

    if(!m_loginDatabase.CheckRequiredField("account", "username"))
    {
        m_loginDatabase.HaltDelayThread();
        return false;
    }

    // server has started up successfully => enable async DB requests
    m_loginDatabase.AllowAsyncTransactions();

    return true;
}

