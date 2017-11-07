/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/11
*/

#include <stdarg.h>

#include "SqlOperations.h"
//#include "SqlConnection.h"
#include "SqlDelayThread.h"
#include "DatabaseEnv.h"

//#define LOCK_DB_CONN(conn) SqlConnection::Lock guard(conn)


/// ---- ASYNC STATEMENTS / TRANSACTIONS ----

bool SqlPlainRequest::Execute(SqlConnection* pConn)
{
    SqlConnection::Lock guard(pConn);
    return guard->Execute(m_sql);
}

SqlTransaction::~SqlTransaction()
{
    while(!m_queue.empty())
    {
        delete m_queue.back();
        m_queue.pop_back();
    }
}

bool SqlTransaction::Execute(SqlConnection* pConn)
{
    if(m_queue.empty())
    {
        return true;
    }

    SqlConnection::Lock guard(pConn);
    
    guard->BeginTransaction();

    const int nItems = m_queue.size();
    for(int i = 0; i < nItems; ++i)
    {
        SqlOperation* pStmt = m_queue[i];
        if(!pStmt->Execute(pConn))
        {
            guard->RollbackTransaction();
            return false;
        }
    }

    return guard->CommitTransaction();
}

SqlPreparedRequest::SqlPreparedRequest(int nIndex, SqlStmtParameters* arg)
    : m_nIndex(nIndex)
    , m_pParam(arg) {}

SqlPreparedRequest::~SqlPreparedRequest()
{
    delete m_pParam;
}

bool SqlPreparedRequest::Execute(SqlConnection* pConn)
{
    SqlConnection::Lock guard(pConn);
    return guard->ExecuteStmt(m_nIndex, *m_pParam);
}


/// ---- ASYNC QUERIES ----

bool SqlQuery::Execute(SqlConnection* pConn)
{
    if(!m_callback || !m_queue)
    {
        return false;
    }

    SqlConnection::Lock guard(pConn);
    /// execute the query and store the result in the callback
    m_callback->SetResult(guard->Query(m_sql));
    /// add the callback to the sql result queue of the thread it originated from
    m_queue->add(m_callback);

    return true;
}

void SqlResultQueue::Update()
{
    // execute the callbacks waiting in the synchronization queue
    Evil::IQueryCallback* pCallback = NULL;

    while(next(pCallback))
    {
        pCallback->Execute();
        SafeDelete(pCallback);
    }
}

bool SqlQueryHolder::Execute(Evil::IQueryCallback* callback, SqlDelayThread* thread, SqlResultQueue* queue)
{
    if(!callback || !thread || !queue)
    {
        return false;
    }

    /// delay the execution of the queries, sync them with the delay thread
    /// which will in turn resync on execution (via the queue) and call back
    SqlQueryHolderEx* holderEx = new SqlQueryHolderEx(this, callback, queue);
    thread->Delay(holderEx);

    return true;
}

bool SqlQueryHolder::SetQuery(uint32 index, const char* sql)
{
    if(m_queries.size() <= index)
    {
        // TODO, "Query index (" SIZEFMTD ") out of range (size: " SIZEFMTD ") for query: %s", index, m_queries.size(), sql
        LOG(ERROR)<<"Query index: "<<index<<"out of range (size: "<<m_queries.size()<<" for query: "<<sql;
        return false;
    }

    if(m_queries[index].first != NULL)
    {
        // TODO, "Attempt assign query to holder index (" SIZEFMTD ") where other query stored (Old: [%s] New: [%s])",
        // index, m_queries[index].first, sql
        LOG(ERROR)<<"Attempt assign query to holder index: "<<index<<" where other query stored (Old: ["<<m_queries[index].first<<"] New: ["<<sql<<"])";
        return false;
    }

    /// not executed yet, just stored (it's not called a holder for nothing)
    m_queries[index] = SqlResultPair(common_strdup(sql), NULL);
    return true;
}

bool SqlQueryHolder::SetPQuery(uint32 index, const char* format, ...)
{
    if(!format)
    {
        // TODO, "Query (index: " SIZEFMTD ") is empty.", index
        LOG(ERROR)<<"Query (index: "<<index<<") is empty.";
        return false;
    }

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if(-1 == res)
    {
        // TODO, "SQL Query truncated (and not execute) for format: %s", format
        LOG(ERROR)<<"SQL Query truncated (and not execute) for format: "<<format;
        return false;
    }

    return SetQuery(index, szQuery);
}

QueryResult* SqlQueryHolder::GetResult(uint32 index)
{
    if(index < m_queries.size())
    {
        /// the query strings are freed on the first GetResult or in the destructor
        if(m_queries[index].first != NULL)
        {
            delete [] (m_queries[index].first);
            m_queries[index].first = NULL;
        }

        /// when you get a result aways remember to delete it!
        return m_queries[index].second;
    }

    return NULL;
}

void SqlQueryHolder::SetResult(uint32 index, QueryResult* result)
{
    /// store the result in the holder
    if(index < m_queries.size())
    {
        m_queries[index].second = result;
    }
    else
    {
        LOG(ERROR)<<"set result error, input index is "<<index<<", "
            <<"but queries size is "<<m_queries.size();
    }
}

SqlQueryHolder::~SqlQueryHolder()
{
    for(size_t i = 0; i < m_queries.size(); ++i)
    {
        /// if the result was never used, free the resources
        /// results used already (getresult called) are expected to be deleted
        if(m_queries[i].first)
        {
            delete[](const_cast<char*>(m_queries[i].first));
            delete m_queries[i].second;
        }
    }
}

void SqlQueryHolder::SetSize(uint32 size)
{
    /// to optimize push_back, reserve the number of queries about to be executed
    m_queries.reserve(size);
}

bool SqlQueryHolderEx::Execute(SqlConnection* pConn)
{
    if(!m_holder || ! m_callback || !m_queue)
    {
        return false;
    }

    SqlConnection::Lock guard(pConn);

    std::vector<SqlQueryHolder::SqlResultPair>& queries = m_holder->m_queries;
    for(size_t i = 0; i < queries.size(); ++i)
    {
        const char* sql = queries[i].first;
        if(sql)
        {
            m_holder->SetResult(i, guard->Query(sql));
        }
    }

    /// sync with the caller thread
    m_queue->add(m_callback);

    return true;
}



