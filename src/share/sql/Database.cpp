/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/09
*/

#include "sql/DatabaseEnv.h"
#include "sql/SqlOperations.h"

#include <ctime>
#include <fstream>
#include <stdarg.h>

#define MIN_CONNECTION_POOL_SIZE    1
#define MAX_CONNECTION_POOL_SIZE    16

SqlPreparedStatement* SqlConnection::CreateStatement(const std::string& fmt)
{
    return new SqlPlainPreparedStatement(fmt, *this);
}

void SqlConnection::FreePreparedStatements()
{
    SqlConnection::Lock guard(this);

    size_t nStmts = m_holder.size();
    for(size_t i = 0; i < nStmts; ++i)
    {
        delete m_holder[i];
    }

    m_holder.clear();
}

SqlPreparedStatement* SqlConnection::GetStmt(uint32 nIndex)
{
    // resize stmt container
    if(m_holder.size() <= nIndex)
    {
        m_holder.resize(nIndex + 1, NULL);
    }

    SqlPreparedStatement* pStmt = NULL;

    // create stmt if needed
    if(m_holder[nIndex] == NULL)
    {
        // obtain SQL request string
        std::string fmt = m_db.GetStmtString(nIndex);
        Jovi_ASSERT(fmt.length());

        // allocate SQlPreparedStatement object
        pStmt = CreateStatement(fmt);

        // prepare statement
        if(!pStmt->prepare())
        {
            // ´ýÍêÉÆ, ÈÕÖ¾ Unable to prepare SQL statement
            return NULL;
        }

        // save statement in internal registry
        m_holder[nIndex] = pStmt;
    }
    else
    {
        pStmt = m_holder[nIndex];
    }

    return pStmt;
}

bool SqlConnection::ExecuteStmt(int nIndex, const SqlStmtParameters& id)
{
    if(nIndex == -1)
    {
        return false;
    }

    // get prepared statement object
    SqlPreparedStatement* pStmt = GetStmt(nIndex);

    // bind parameters
    pStmt->bind(id);

    // execute statement
    return pStmt->execute();
}

///////////////////////////////////////////////////////////

Database::~Database()
{
    StopServer();
}

bool Database::Initialize(const char* infoString, int nCoons /* = 1 */)
{
    // Enable logging of SQL commands (usually only GM commands)
    // (See method: PExecuteLog)
    m_logSQL = sConfig.getBoolDefault("SQL", "LogSQL", false);
    m_logsDir = sConfig.getStringDefault("SQL", "LogDir", "");
    if(!m_logsDir.empty())
    {
        if((m_logsDir.at(m_logsDir.length() - 1) != '/') &&
            (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
        {
            m_logsDir.append("/");
        }
    }

    m_pingIntervallms = sConfig.getIntDefault("SQL", "MaxPingTime", 30) * MINUTE * 1000;

    /// create DB connections

    // setup connection pool size
    if(nCoons < MIN_CONNECTION_POOL_SIZE)
    {
        m_nQueryConnPoolSize = MIN_CONNECTION_POOL_SIZE;
    }
    else if(nCoons > MAX_CONNECTION_POOL_SIZE)
    {
        m_nQueryConnPoolSize = MAX_CONNECTION_POOL_SIZE;
    }
    else
    {
        m_nQueryConnPoolSize = nCoons;
    }

    // create connection pool for sync requests
    for(int i = 0; i < m_nQueryConnPoolSize; ++i)
    {
        SqlConnection* pConn = CreateConnection();
        if(!pConn->Initialize(infoString))
        {
            delete pConn;
            return false;
        }

        m_pQueryConnections.push_back(pConn);
    }

    // create and initialize connection for async requests
    m_pAsyncConn = CreateConnection();
    if(!m_pAsyncConn->Initialize(infoString))
    {
        return false;
    }

    m_pResultQueue = new SqlResultQueue;

    InitDelayThread();

    return true;
}

void Database::StopServer()
{
    HaltDelayThread();

    SafeDelete(m_pResultQueue);
    SafeDelete(m_pAsyncConn);

    for(size_t i = 0; i < m_pQueryConnections.size(); ++i)
    {
        delete m_pQueryConnections[i];
    }

    m_pQueryConnections.clear();
}

SqlDelayThread* Database::CreateDelayThread()
{
    Jovi_ASSERT(m_pAsyncConn);
    return new SqlDelayThread(this, m_pAsyncConn);
}

void Database::InitDelayThread()
{
    Jovi_ASSERT(!m_delayThread);

    // New delay thread for delay execute, will deleted at m_delayThread delete
    m_threadBody = CreateDelayThread();
    m_delayThread = new Boost_Based::Thread(m_threadBody);
}

void Database::HaltDelayThread()
{
    if(!m_threadBody || !m_delayThread)
    {
        return;
    }

    m_threadBody->Stop();           // stop event
    m_delayThread->wait();          // wait for flush to DB
    SafeDelete(m_delayThread);      // this also deletes m_threadBody
    m_threadBody = NULL;
}

void Database::ThreadStart()
{

}

void Database::ThreadEnd()
{

}

void Database::ProcessResultQueue()
{
    if(m_pResultQueue)
    {
        m_pResultQueue->Update();
    }
}

void Database::escape_string(std::string& str)
{
    if(str.empty())
    {
        return;
    }

    char* buf = new char[str.size() * 2 + 1];

    // we don't care what connection to use - escape string will be the same
    m_pQueryConnections[0]->escape_string(buf, str.c_str(), str.size());
    str = buf;
    SafeDeleteArray(buf);
}

SqlConnection* Database::getQueryConnection()
{
    int count = 0;

    if(m_nQueryCounter == long(1<<31))
    {
        m_nQueryCounter = 0;
    }
    else
    {
        count = ++m_nQueryCounter;
    }

    return m_pQueryConnections[count % m_nQueryConnPoolSize];
}

void Database::Ping()
{
    const char* sql = "SELECT 1";

    {
        SqlConnection::Lock guard(m_pAsyncConn);
        delete guard->Query(sql);
    }

    for(int i = 0; i < m_nQueryConnPoolSize; ++i)
    {
        SqlConnection::Lock guard(m_pQueryConnections[i]);
        delete guard->Query(sql);
    }
}

bool Database::PExecuteLog(const char* format, ...)
{
    if(!format)
    {
        return false;
    }

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int ret = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if(-1 == ret)
    {
        // TODO, log"SQL Query truncated (and not execute) for format: %s", format
        LOG(ERROR)<<"SQL Query truncated (and not execute) for format: "<<format;
        return false;
    }

    if(m_logSQL)
    {
        time_t curr;
        tm local;
        time(&curr);
        local = *(localtime(&curr));
        char fName[128];
        sprintf(fName, "%04d-%02d-%02d-logSQL.sql", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);

        FILE* log_file;
        std::string logDir_fname = m_logsDir + fName;
        log_file = fopen(logDir_fname.c_str(), "a");
        if(log_file)
        {
            fprintf(log_file, "%s;\n", szQuery);
            fclose(log_file);
        }
        else
        {
            // TODO, log"SQL-Logging is disabled - Log file for the SQL commands could not be openend: %s", fName
            LOG(ERROR)<<"SQL-Logging is disabled - Log file for the SQL commands could not be openend: "<<fName;
        }
    }

    return Execute(szQuery);
}

QueryResult* Database::PQuery(const char* format, ...)
{
    IF_NOT_RETURN_VALUE(format, NULL);

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int ret = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if(-1 == ret)
    {
        // TODO, log "SQL Query truncated (and not execute) for format: %s", format
        LOG(ERROR)<<"SQL Query truncated (and not execute) for format: "<<format;
        return NULL;
    }

    return Query(szQuery);
}

QueryNamedResult* Database::PQueryNamed(const char* format, ...)
{
    IF_NOT_RETURN_VALUE(format, NULL);

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int ret = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if(-1 == ret)
    {
        // TODO, log "SQL Query truncated (and not execute) for format: %s", format
        LOG(ERROR)<<"SQL Query truncated (and not execute) for format: "<<format;
        return NULL;
    }

    return QueryNamed(szQuery);
}

bool Database::Execute(const char* sql)
{
    IF_NOT_RETURN_FALSE(m_pAsyncConn);
    SqlTransaction* pTrans = getTransStorage()->get();
    if(pTrans)
    {
        // add SQL request to trans queue
        pTrans->DelayExecute(new SqlPlainRequest(sql));
    }
    else
    {
        if(!m_bAllowAsyncTransactions)
        {
            return DirectExecute(sql);
        }

        m_threadBody->Delay(new SqlPlainRequest(sql));
    }

    return true;
}

bool Database::PExecute(const char* format, ...)
{
    IF_NOT_RETURN_FALSE(format);

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int ret = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if(-1 == ret)
    {
        // TODO, log "SQL Query truncated (and not execute) for format: %s", format
        LOG(ERROR)<<"SQL Query truncated (and not execute) for format: "<<format;
        return false;
    }

    return Execute(szQuery);
}

bool Database::DirectPExecute(const char* format, ...)
{
    IF_NOT_RETURN_FALSE(format);

    va_list ap;
    char szQuery[MAX_QUERY_LEN];
    va_start(ap, format);
    int ret = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);
    va_end(ap);

    if(-1 == ret)
    {
        // TODO, log "SQL Query truncated (and not execute) for format: %s", format
        LOG(ERROR)<<"SQL Query truncated (and not execute) for format: "<<format;
        return false;
    }

    return DirectExecute(szQuery);
}

bool Database::BeginTransaction()
{
    IF_NOT_RETURN_FALSE(m_pAsyncConn);

    // initiate transaction on current thread
    // currently we do not support queued transactions
    getTransStorage()->init();
    return true;
}

bool Database::CommitTransaction()
{
    IF_NOT_RETURN_FALSE(m_pAsyncConn);

    // check if we have pending transaction
    IF_NOT_RETURN_FALSE(getTransStorage()->get());

    // if async execution is not available
    IF_NOT_RETURN_VALUE(m_bAllowAsyncTransactions, CommitTransactionDirect());

    // add SqlTransaction to the async queue
    m_threadBody->Delay(getTransStorage()->detach());
    return true;
}

bool Database::CommitTransactionDirect()
{
    IF_NOT_RETURN_FALSE(m_pAsyncConn);

    // check if we have pending transaction
    IF_NOT_RETURN_FALSE(getTransStorage()->get());

    // directly execute SqlTransaction
    SqlTransaction* pTrans = getTransStorage()->detach();
    pTrans->Execute(m_pAsyncConn);
    SafeDelete(pTrans);

    return true;
}

bool Database::RollbackTransaction()
{
    IF_NOT_RETURN_FALSE(m_pAsyncConn);

    IF_NOT_RETURN_FALSE(getTransStorage()->get());

    // remove scheduled transaction
    getTransStorage()->reset();

    return true;
}

bool Database::CheckRequiredField(const char* table_name, const char* required_name)
{
    // check required field
    QueryResult* pResult = PQuery("SELECT %s FROM %s LIMIT 1", required_name, table_name);
    if(pResult)
    {
        SafeDelete(pResult);
        return true;
    }

    // check fail, prepare readable error message

    // search current required_* field in DB
    //const char* db_name;
    //if(!strcmp(table_name, "db_version"))
    //{
    //    db_name = "WORLD";  // TODO
    //}
    //else if(!strcmp(table_name, "character_db_version"))
    //{
    //    db_name = "CHARACTER";
    //}
    //else if(!strcmp(table_name, "realmd_db_version"))
    //{
    //    db_name = "REALMD";
    //}
    //else
    //{
    //    db_name = "UNKNOWN";
    //}

    //const char* req_sql_update_name = required_name + strlen("required_");

    //QueryNamedResult* pResult2 = PQueryNamed("SELECT * FROM %s LIMIT 1", table_name);
    //if(pResult2)
    //{
    //    const QueryFieldNames& namesMap = pResult2->GetFieldNames();
    //    std::string reqName;
    //    for(QueryFieldNames::const_iterator iter = namesMap.begin(); iter != namesMap.end(); ++iter)
    //    {
    //        if(iter->substr(0, 9) == "required_")
    //        {
    //            reqName = *iter;
    //            break;
    //        }
    //    }

    //    SafeDelete(pResult2);

    //    std::string cur_sql_update_name = reqName.substr(strlen("required_"), reqName.npos);

    //    if(!reqName.empty())
    //    {
    //        // TODO, log
    //    }
    //    else
    //    {
    //        // TODO, log
    //    }
    //}
    //else
    //{
    //    // TODO, log
    //}

    return false;
}

bool Database::ExecuteStmt(const SqlStatementID& id, SqlStmtParameters* params)
{
    IF_NOT_RETURN_FALSE(m_pAsyncConn);

    SqlTransaction* pTrans = getTransStorage()->get();
    if(pTrans)
    {
        // add SQL request to trans queue
        pTrans->DelayExecute(new SqlPreparedRequest(id.ID(), params));
    }
    else
    {
        // if async execution is not available
        if(!m_bAllowAsyncTransactions)
        {
            return DirectExecuteStmt(id, params);
        }

        // Simple sql statement
        m_threadBody->Delay(new SqlPreparedRequest(id.ID(), params));
    }

    return true;
}

bool Database::DirectExecuteStmt(const SqlStatementID& id, SqlStmtParameters* params)
{
    Jovi_ASSERT(params);
    // QUESTION
    std::auto_ptr<SqlStmtParameters> p(params);     // what to do ????????????????
    // execute statement
    SqlConnection::Lock guard(getAsyncConnection());
    return guard->ExecuteStmt(id.ID(), *params);
}

SqlStatement Database::CreateStatement(SqlStatementID& index, const char* fmt)
{
    int nId = -1;

    if(!index.initialized())
    {
        std::string szFmt(fmt);

        int nParams = std::count(szFmt.begin(), szFmt.end(), '?');

        LOCK_GUARD  guard(m_stmtGuard);
        PreparedStmtRegistry::const_iterator iter = m_stmtRegistry.find(szFmt);
        if(iter == m_stmtRegistry.end())
        {
            nId = ++m_iStmtIndex;
            m_stmtRegistry[szFmt] = nId;
        }
        else
        {
            nId = iter->second;
        }

        index.init(nId, nParams);
    }

    return SqlStatement(index, *this);
}

std::string Database::GetStmtString(const int stmtId) const
{
    LOCK_GUARD guard(m_stmtGuard);

    if(stmtId == -1 || stmtId > m_iStmtIndex)
    {
        return std::string();
    }

    PreparedStmtRegistry::const_iterator iter_last = m_stmtRegistry.end();
    for(PreparedStmtRegistry::const_iterator iter = m_stmtRegistry.begin(); iter != iter_last; ++iter)
    {
        if(iter->second == stmtId)
        {
            return iter->first;
        }
    }

    return std::string();
}

Database::TransHelper::~TransHelper()
{
    reset();
}

SqlTransaction* Database::TransHelper::init()
{
    // if we will get a nested transaction request - we MUST fix code!!!
    Jovi_ASSERT(!m_pTrans);
    m_pTrans = new SqlTransaction;
    return m_pTrans;
}

SqlTransaction* Database::TransHelper::detach()
{
    SqlTransaction* pRes = m_pTrans;
    m_pTrans = NULL;
    return pRes;
}

void Database::TransHelper::reset()
{
    SafeDelete(m_pTrans);
}




