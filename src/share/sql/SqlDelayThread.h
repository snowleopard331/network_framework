/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/07
*/
#ifndef _SQL_DELAY_THREAD_H_
#define _SQL_DELAY_THREAD_H_

#include <boost/thread/mutex.hpp>

#include "LockedQueue.h"
#include "Threading.h"

class Database;
class SqlOperation;
class SqlConnection;


class SqlDelayThread
    : public Boost_Based::Runnable
{
    typedef Boost_Based::LockedQueue<SqlOperation*, boost::mutex> SqlQueue;

private:
    SqlQueue        m_sqlQueue;             /**< Queue of SQL statements */
    Database*       m_pDbEngine;            /**< Pointer to used Database engine */
    SqlConnection*  m_pDbConnection;        /**< Pointer to DB connection */
    volatile bool   m_running;              /**< TODO */

    // process all enqueued requests
    void ProcessRequests();

public:
    SqlDelayThread(Database* pDb, SqlConnection* pConn);

    ~SqlDelayThread();

    // Put sql statement to delay queue
    bool Delay(SqlOperation* sql) 
    {
        m_sqlQueue.add(sql); 
        return true;
    }

    // Stop event
    virtual void Stop();

    // Main Thread loop
    virtual void run();
};

#endif//_SQL_DELAY_THREAD_H_