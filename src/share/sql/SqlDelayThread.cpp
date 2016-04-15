/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/11
*/

#include "SqlDelayThread.h"
#include "SqlOperations.h"
#include "DatabaseEnv.h"

SqlDelayThread::SqlDelayThread(Database* pDb, SqlConnection* pConn)
    : m_pDbEngine(pDb)
    , m_pDbConnection(pConn)
    , m_running(true)
{

}

SqlDelayThread::~SqlDelayThread()
{
    // process all requests which might have been queued while thread was stopping
    ProcessRequests();
}

void SqlDelayThread::run()
{
    mysql_thread_init();        // another database should switch this func

    const uint32 loopSleepms = 10;

    const uint32 pingEveryLoop = m_pDbEngine->GetPingIntervall() / loopSleepms;

    uint32 loopCounter = 0;
    while(m_running)
    {
        // if the running state gets turned off while sleeping
        // empty the queue before exiting
        Boost_Based::Thread::Sleep(loopSleepms);

        ProcessRequests();

        if((loopCounter++) >= pingEveryLoop)
        {
            loopCounter = 0;
            m_pDbEngine->Ping();
        }
    }

    mysql_thread_end();     // another database should switch this func
}

void SqlDelayThread::Stop()
{
    m_running = false;
}

void SqlDelayThread::ProcessRequests()
{
    SqlOperation* pOper = NULL;

    while(m_sqlQueue.next(pOper))
    {
        pOper->Execute(m_pDbConnection);
        SafeDelete(pOper);
    }
}