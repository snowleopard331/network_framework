/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/22
*/

#include "Master.h"
#include "Common.h"
#include "authSocketMgr.h"

Master::Master()
    : m_stopEvent(false)
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

    /// Loop
    while(!m_stopEvent)
    {

    }

    return 0;
}

bool Master::_startDB()
{
    std::string dbstring = sConfig.getStringDefault("Database", "LoginDatabaseInfo", "");

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

