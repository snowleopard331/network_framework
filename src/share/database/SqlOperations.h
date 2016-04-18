/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/08
*/

#ifndef _SQL_OPERATIONS_H_
#define _SQL_OPERATIONS_H_

#include <boost/thread/mutex.hpp>

#include "Common.h"
#include "LockedQueue.h"
#include "Callback.h"


class SqlStmtParameters;

/// ---- BASE ----

class SqlConnection;
class SqlDelayThread;

class SqlOperation
{
public:
    virtual void OnRemove() 
    {
        delete this;
    }

    virtual bool Execute(SqlConnection* pConn) = 0;

    virtual ~SqlOperation() 
    {

    }
};


/// ---- ASYNC STATEMENTS / TRANSACTIONS ----

class SqlPlainRequest
    : public SqlOperation
{
private:
    const char* m_sql;

public:
    SqlPlainRequest(const char* sql)
        : m_sql(common_strdup(sql)) 
    {

    }

    ~SqlPlainRequest()
    {
        char* toFree = const_cast<char*>(m_sql);
        SafeDeleteArray(toFree);
    }

    bool Execute(SqlConnection* pConn) override;
};


class SqlTransaction
    : public SqlOperation
{
private:
    std::vector<SqlOperation*> m_queue;

public:
    SqlTransaction() 
    {

    }

    ~SqlTransaction();


    void DelayExecute(SqlOperation* sql) 
    {
        m_queue.push_back(sql);
    }

    bool Execute(SqlConnection* pConn) override;
};

class SqlPreparedRequest
    : public SqlOperation
{
private:
    const int m_nIndex;
    SqlStmtParameters* m_pParam;

public:
    SqlPreparedRequest(int nIndex, SqlStmtParameters* arg);
    ~SqlPreparedRequest();

    bool Execute(SqlConnection* pConn) override;
};


/// ---- ASYNC QUERIES ----

class SqlQuery;
class QueryResult;
class QueryResultQueue;
class SqlQueryHolder;
class SqlQueryHolderEx;


class SqlResultQueue
    : public Boost_Based::LockedQueue<Evil::IQueryCallback*, boost::mutex>
{
public:
    SqlResultQueue()    
    {

    }

    void Update();
};


class SqlQuery
    : public SqlOperation
{
private:
    const char*             m_sql;
    Evil::IQueryCallback*   m_callback;
    SqlResultQueue*         m_queue;

public:
    SqlQuery(const char* sql, Evil::IQueryCallback* callback, SqlResultQueue* queue)
        : m_sql(sql)
        , m_callback(callback)
        , m_queue(queue)    
    {

    }

    ~SqlQuery() 
    {
        char* tofree = const_cast<char*>(m_sql);
        delete[] tofree;
    }

    bool Execute(SqlConnection* pConn) override;
};


class SqlQueryHolder
{
    friend class SqlQueryHolderEx;

private:

    typedef std::pair<const char*, QueryResult*> SqlResultPair;
    std::vector<SqlResultPair>  m_queries;

public:

    SqlQueryHolder()    
    {

    }

    ~SqlQueryHolder();

    bool SetQuery(uint32 index, const char* sql);

    bool SetPQuery(uint32 index, const char* format, ...) ATTR_PRINTF(3, 4);

    void SetSize(uint32 size);

    QueryResult* GetResult(uint32 index);

    void SetResult(uint32 index, QueryResult* result);

    bool Execute(Evil::IQueryCallback* callback, SqlDelayThread* thread, SqlResultQueue* queue);
};


class SqlQueryHolderEx
    : public SqlOperation
{
private:
    SqlQueryHolder*         m_holder;
    Evil::IQueryCallback*   m_callback;
    SqlResultQueue*         m_queue;

public:
    SqlQueryHolderEx(SqlQueryHolder* holder, Evil::IQueryCallback* callback, SqlResultQueue* queue)
        : m_holder(holder)
        , m_callback(callback)
        , m_queue(queue)    
    {

    }

    bool Execute(SqlConnection* pConn) override;
};

#endif//_SQL_OPERATIONS_H_
