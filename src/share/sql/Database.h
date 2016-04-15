/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/08/21
*/

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "Common.h"
#include "sql/SqlPreparedStatement.h"
#include "sql/SqlDelayThread.h"
#include "Threading.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

//#include <ext/hash_map>
//using namespace __gnu_cxx;

class QueryResult;
class QueryNamedResult;
class SqlResultQueue;
class SqlQueryHolder;
class SqlTransaction;


#define MAX_QUERY_LEN   (32*1024)


class SqlConnection
{
private:
    typedef boost::recursive_mutex LOCK_TYPE;
    LOCK_TYPE m_mutex;

    typedef std::vector<SqlPreparedStatement*>   StmtHolder;
    StmtHolder  m_holder;

protected:
    Database& m_db;
    SqlConnection(Database& db)
        : m_db(db)  
    {

    }

    virtual SqlPreparedStatement* CreateStatement(const std::string& fmt);

    //allocate prepared statement and return statement ID
    SqlPreparedStatement* GetStmt(uint32 nIndex);

    // free prepared statements objects    
    void FreePreparedStatements();

public:
    virtual ~SqlConnection() {}

    class Lock
    {
    private:
        SqlConnection* const m_pConn;

    public:
        Lock(SqlConnection *conn)
            : m_pConn(conn)
        {
            // recursive mutex
            m_pConn->m_mutex.lock();
        }

        ~Lock()
        {
            // release resursive mutex
            m_pConn->m_mutex.unlock();
        }

        // return SqlConnection *operator ->
        SqlConnection* operator->() const
        {
            return m_pConn;
        }
    };

    // method for initializing DB connection
    virtual bool Initialize(const char *infoString) = 0;

    // public methods for making queries
    virtual QueryResult* Query(const char *sql) = 0;

    virtual QueryNamedResult* QueryNamed(const char *sql) = 0;

    // public methods for making requests
    virtual bool Execute(const char *sql) = 0;

    virtual unsigned long escape_string(char* to, const char* from, unsigned long length)
    {
        strncpy(to, from, length);
        return length;
    }

    // nothing do if DB not support transactions
    virtual bool BeginTransaction() 
    {
        return true;
    }

    virtual bool CommitTransaction()    
    {
        return true;
    }

    // can't rollback without transaction support
    virtual bool RollbackTransaction()  
    {
        return true;
    }

    // methods to work with prepared statements
    bool ExecuteStmt(int nIndex, const SqlStmtParameters& id);

    Database& DB() 
    {
        return m_db;
    }
};


class Database
{
private:
    // connection helper 
    boost::atomic_int m_nQueryCounter;      /**< counter for connection selection */
    int m_nQueryConnPoolSize;               /**< current size of query connection pool */
    

    // lets use pool of connections for sync queries
    typedef std::vector<SqlConnection*> SqlConnectionContainer;
    SqlConnectionContainer m_pQueryConnections;

    // only one single DB connection for transactions
    SqlConnection*  m_pAsyncConn;

    SqlResultQueue*         m_pResultQueue;     /**< Transaction queues from diff. threads */
    SqlDelayThread*         m_threadBody;       /**< Pointer to delay sql executer (owned by m_delayThread) */
    Boost_Based::Thread*    m_delayThread;      /**< Pointer to executer thread */

    bool                    m_bAllowAsyncTransactions;  /**< flag which specifies if async transactions are enabled */


    // PREPARED STATEMENT REGISTRY

    typedef boost::mutex    LOCK_TYPE;
    typedef boost::unique_lock<LOCK_TYPE>   LOCK_GUARD;

    mutable LOCK_TYPE m_stmtGuard;

    //typedef hash_map<std::string, int> PreparedStmtRegistry;
    typedef std::map<std::string, int>  PreparedStmtRegistry;
    PreparedStmtRegistry    m_stmtRegistry;

    int m_iStmtIndex;

private:
    bool            m_logSQL;
    std::string     m_logsDir;
    uint32          m_pingIntervallms;

public:
    virtual ~Database();

    virtual bool Initialize(const char* infoString, int nCoons = 1);
    
    // start worker thread for async DB request execution
    virtual void InitDelayThread();

    // stop worker thread
    virtual void HaltDelayThread();

    // Synchronous DB queries
    inline QueryResult* Query(const char *sql)
    {
        SqlConnection::Lock guard(getQueryConnection());
        return guard->Query(sql);
    }

    inline QueryNamedResult* QueryNamed(const char* sql)
    {
        SqlConnection::Lock guard(getQueryConnection());
        return guard->QueryNamed(sql);
    }

    QueryResult* PQuery(const char* format, ...) ATTR_PRINTF(2, 3);

    QueryNamedResult* PQueryNamed(const char* format, ...) ATTR_PRINTF(2, 3);

    inline bool DirectExecute(const char* sql)
    {
        if(!m_pAsyncConn)
        {
            return false;
        }

        SqlConnection::Lock guard(m_pAsyncConn);
        return guard->Execute(sql);
    }

    bool DirectPExecute(const char* format, ...) ATTR_PRINTF(2, 3);


    /// Async queries and query holders, implemented in DatabaseImpl.h

    // Query / member
    template<class Class>
    bool AsyncQuery(Class* object, void(Class::*method)(QueryResult*), const char* sql);

    template<class Class, typename ParamType1>
    bool AsyncQuery(Class* object, void(Class::*method)(QueryResult*, ParamType1), ParamType1 param1, const char* sql);

    template<class Class, typename ParamType1, typename ParamType2>
    bool AsyncQuery(Class* object, void(Class::*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* sql);

    template<class Class, typename ParamType1, typename ParamType2, typename ParamType3>
    bool AsyncQuery(Class* object, void(Class::*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* sql);


    // Query / static
    template<typename ParamType1>
    bool AsyncQuery(void (*method)(QueryResult*, ParamType1), ParamType1 param1, const char* sql);

    template<typename ParamType1, typename ParamType2>
    bool AsyncQuery(void (*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* sql);

    template<typename ParamType1, typename ParamType2, typename ParamType3>
    bool AsyncQuery(void (*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* sql);


    // PQuery / member
    template<class Class>
    bool AsyncPQuery(Class* object, void (Class::*method)(QueryResult*), const char* format, ...) ATTR_PRINTF(4, 5);

    template<class Class, typename ParamType1>
    bool AsyncPQuery(Class* object, void (Class::*method)(QueryResult*, ParamType1), ParamType1 param1, const char* format, ...) ATTR_PRINTF(5, 6);

    template<class Class, typename ParamType1, typename ParamType2>
    bool AsyncPQuery(Class* object, void (Class::*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* format, ...) ATTR_PRINTF(6, 7);

    template<class Class, typename ParamType1, typename ParamType2, typename ParamType3>
    bool AsyncPQuery(Class* object, void (Class::*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* format, ...) ATTR_PRINTF(7, 8);


    // PQuery / static
    template<typename ParamType1>
    bool AsyncPQuery(void (*method)(QueryResult*, ParamType1), ParamType1 param1, const char* format, ...) ATTR_PRINTF(4, 5);

    template<typename ParamType1, typename ParamType2>
    bool AsyncPQuery(void (*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* format, ...) ATTR_PRINTF(5, 6);

    template<typename ParamType1, typename ParamType2, typename ParamType3>
    bool AsyncPQuery(void (*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* format, ...) ATTR_PRINTF(6, 7);


    // QueryHolder
    template<class Class>
    bool DelayQueryHolder(Class* object, void(Class::*method)(QueryResult*, SqlQueryHolder*), SqlQueryHolder* holder);

    template<class Class, typename ParamType1>
    bool DelayQueryHolder(Class* object, void(Class::*method)(QueryResult*, SqlQueryHolder*, ParamType1), SqlQueryHolder* holder, ParamType1 param1);


    
    bool Execute(const char* sql);

    bool PExecute(const char* format, ...) ATTR_PRINTF(2, 3);
    
    // Writes SQL commands to a LOG file (see mangosd.conf "LogSQL")
    bool PExecuteLog(const char* format, ...) ATTR_PRINTF(2, 3);

    bool BeginTransaction();

    bool CommitTransaction();

    bool RollbackTransaction();
    
    // for sync transaction execution
    bool CommitTransactionDirect();


    /// ---- PREPARED STATEMENT API ----

    // allocate index for prepared statement with SQL request 'fmt'
    SqlStatement CreateStatement(SqlStatementID& index, const char* fmt);

    // get prepared statement format string
    std::string GetStmtString(const int stmtId) const;

    operator bool () const {return m_pQueryConnections.size() && m_pAsyncConn != 0;}

    // escape string generation
    void escape_string(std::string& str);
    
    // must be called before first query in thread (one time for thread using one from existing Database objects)
    virtual void ThreadStart();
    
    // must be called before finish thread run (one time for thread using one from existing Database objects)
    virtual void ThreadEnd();

    // set database-wide result queue. also we should use object-bases and not thread-based result queues
    void ProcessResultQueue();
     
    bool CheckRequiredField(const char* table_name, const char* required_name);

    uint32 GetPingIntervall() {return m_pingIntervallms;}

    // function to ping database connections
    void Ping();

    /**
    * @brief set this to allow async transactions
    *
    * you should call it explicitly after your server successfully started
    * up.
    * NO ASYNC TRANSACTIONS DURING SERVER STARTUP - ONLY DURING RUNTIME!!!
    *
    */
    void AllowAsyncTransactions() {m_bAllowAsyncTransactions = true;}


protected:

    Database()
        : m_nQueryConnPoolSize(1)
        , m_pAsyncConn(NULL)
        , m_pResultQueue(NULL)
        , m_threadBody(NULL)
        , m_delayThread(NULL)
        , m_bAllowAsyncTransactions(false)
        , m_iStmtIndex(-1)
        , m_logSQL(false)
        , m_pingIntervallms(0)
    {
        m_nQueryCounter = -1;
    }

    void StopServer();

    // factory method to create SqlConnection objects
    virtual SqlConnection* CreateConnection() = 0;

    // factory method to create SqlDelayThread objects
    virtual SqlDelayThread* CreateDelayThread();



    class TransHelper
    {
    private:
        SqlTransaction* m_pTrans;

    public:
        TransHelper()
            : m_pTrans(NULL)    {}

        ~TransHelper();

        // initializes new SqlTransaction object
        SqlTransaction* init();

        // gets pointer on current transaction object. Returns NULL if transaction was not initiated
        SqlTransaction* get() const 
        {
            return m_pTrans;
        }

        /**
        * @brief detaches SqlTransaction object allocated by init() function
        *
        * next call to get() function will return NULL!
        * do not forget to destroy obtained SqlTransaction object!
        *
        * @return SqlTransaction
        */
        SqlTransaction* detach();

        // destroyes SqlTransaction allocated by init() function
        void reset();
    };

    /**
    * @brief per-thread based storage for SqlTransaction object initialization - no locking is required
    *
    */

    //Thread Local Store(TLS) OR Thread Specific Storage(TSS)
    typedef boost::thread_specific_ptr<Database::TransHelper>   DBTransHelperTss;
    Database::DBTransHelperTss  m_TransStorage;

    Database::DBTransHelperTss& getTransStorage()
    {
        if(m_TransStorage.get() == NULL)
        {
            m_TransStorage.reset(new Database::TransHelper);    // when delete this memery
        }
        
        return m_TransStorage;
    }

    // ---- DB connections ----
    
    // round-robin connection selection
    SqlConnection* getQueryConnection();

    // for now return one single connection for async requests
    SqlConnection* getAsyncConnection() const 
    {
        return  m_pAsyncConn;
    }


    // ---- PREPARED STATEMENT API ----

    friend class SqlStatement;

    // query function for prepared statements
    bool ExecuteStmt(const SqlStatementID& id, SqlStmtParameters* params);

    bool DirectExecuteStmt(const SqlStatementID& id, SqlStmtParameters* params);
};

#endif//_DATABASE_H_

