/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/14
*/

#include <sys/time.h>

#include "DatabaseMysql.h"
#include "Util.h"
#include "DatabaseEnv.h"

size_t DatabaseMysql::db_count = 0;

void DatabaseMysql::ThreadStart()
{
    mysql_thread_init();
}

void DatabaseMysql::ThreadEnd()
{
    mysql_thread_end();
}

DatabaseMysql::DatabaseMysql()
{
    if(db_count++ == 0)
    {
        mysql_library_init(-1, NULL, NULL);

        if(!mysql_thread_safe())
        {
            // TODO, "FATAL ERROR: Used MySQL library isn't thread-safe."
            LOG(FATAL)<<"FATAL ERROR: Used MySQL library isn't thread-safe";
            exit(1);
        }
    }
}

DatabaseMysql::~DatabaseMysql()
{
    StopServer();

    if(--db_count == 0)
    {
        mysql_library_end();
    }
}

SqlConnection* DatabaseMysql::CreateConnection()
{
    return new MySQLConnection(*this);
}

MySQLConnection::~MySQLConnection()
{
    FreePreparedStatements();
    mysql_close(m_mysql);
}

bool MySQLConnection::Initialize(const char *infoString)
{
    MYSQL* mysqlInit = mysql_init(NULL);
    if(!mysqlInit)
    {
        // TODO, "Could not initialize Mysql connection"
        LOG(ERROR)<<"Could not initialize Mysql connection";
        return false;
    }

    Tokens tokens = StrSplit(infoString, ";");

    Tokens::iterator iter;

    std::string host, port_or_socket, user, password, database;
    int port;
    const char* unix_socket;

    iter = tokens.begin();

    // host;port;user;password;database
    if(iter != tokens.end())
    {
        host = *iter++;
    }
    if(iter != tokens.end())
    {
        port_or_socket = *iter++;
    }
    if(iter != tokens.end())
    {
        user = *iter++;
    }
    if(iter != tokens.end())
    {
        password = *iter++;
    }
    if(iter != tokens.end())
    {
        database = *iter++;
    }

    mysql_options(mysqlInit, MYSQL_SET_CHARSET_NAME, "utf8");

    // system different
    if(host == ".")
    {
        unsigned int opt = MYSQL_PROTOCOL_SOCKET;
        mysql_options(mysqlInit, MYSQL_OPT_PROTOCOL, (const char*)&opt);
        host = "localhost";
        port = 0;
        unix_socket = port_or_socket.c_str();
    }
    else
    {
        port = atoi(port_or_socket.c_str());
        unix_socket = 0;
    }

    m_mysql = mysql_real_connect(mysqlInit, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, unix_socket, 0);
    if(!m_mysql)
    {
        // TODO, "Could not connect to MySQL database at %s: %s\n", host.c_str(), mysql_error(mysqlInit)
        LOG(ERROR)<<"Could not connect to MySQL database at "<<host.c_str()<<": "<<mysql_error(mysqlInit);
        mysql_close(mysqlInit);
        return false;
    }

    /*----------SET AUTOCOMMIT ON---------*/
    // It seems mysql 5.0.x have enabled this feature
    // by default. In crash case you can lose data!!!
    // So better to turn this off
    // ---
    // Evil use transactions,
    // autocommit is turned of during it.
    // Setting it to on makes atomic updates work
    // ---
    // LEAVE 'AUTOCOMMIT' MODE ALWAYS ENABLED!!!
    // W/O IT EVEN 'SELECT' QUERIES WOULD REQUIRE TO BE WRAPPED INTO 'START TRANSACTION'<>'COMMIT' CLAUSES!!!
    
    if (!mysql_autocommit(m_mysql, 1))
    {
        LOG(INFO)<<"AUTOCOMMIT SUCCESSFULLY SET TO 1";
    }
    else
    {
        LOG(INFO)<<"AUTOCOMMIT NOT SET TO 1";
    }
    /*-------------------------------------*/


    // set connection properties to UTF8 to properly handle locales for different
    // server configs - core sends data in UTF8, so MySQL must expect UTF8 too
    Execute("SET NAMES `utf8`");        // QUESTION, modify table name
    Execute("SET CHARACTER SET `utf8`");

    return true;
}

bool MySQLConnection::_Query(const char* sql, MYSQL_RES** pResult, MYSQL_FIELD** pFields, uint64* pRowCount, uint32* pFieldCount)
{
    if(!m_mysql)
    {
        return false;
    }

    timeval timeBegin, timeEnd;
    gettimeofday(&timeBegin, NULL);

    if(mysql_query(m_mysql, sql))
    {
        // TODO, "SQL: %s", sql
        LOG(ERROR)<<"SQL: "<<sql;
        // TODO, "query ERROR: %s", mysql_error(mMysql)
        LOG(ERROR)<<"query ERROR: "<<mysql_error(m_mysql);
        return false;
    }
    else
    {
        gettimeofday(&timeEnd, NULL);
        // TODO, "[%u ms] SQL: %s", (timeEnd.tv_sec - timeBegin.tv_sec) * 1000 + (timeEnd.tv_usec - timeBegin.tv_usec)/1000, sql
        LOG(INFO)<<"["<<(timeEnd.tv_sec - timeBegin.tv_sec) * 1000 + (timeEnd.tv_usec - timeBegin.tv_usec)/1000<<" ms] SQL: "<<sql;
    }

    *pResult = mysql_store_result(m_mysql);
    *pRowCount = mysql_affected_rows(m_mysql);
    *pFieldCount = mysql_field_count(m_mysql);

    if(!*pResult)
    {
        return false;
    }

    if(!*pRowCount)
    {
        mysql_free_result(*pResult);
        return false;
    }

    *pFields = mysql_fetch_fields(*pResult);
    return true;
}

QueryResult* MySQLConnection::Query(const char *sql)
{
    MYSQL_RES* result = NULL;
    MYSQL_FIELD* fields = NULL;
    uint64 rowCount = 0;
    uint32 fieldCount = 0;

    if(!_Query(sql, &result, &fields, &rowCount, &fieldCount))
    {
        return NULL;
    }

    QueryResultMysql* queryResult = new QueryResultMysql(result, fields, rowCount, fieldCount);

    queryResult->NextRow();
    return queryResult;
}

QueryNamedResult* MySQLConnection::QueryNamed(const char *sql)
{
    MYSQL_RES* result = NULL;
    MYSQL_FIELD* fields = NULL;
    uint64 rowCount = 0;
    uint32 fieldCount = 0;

    if(!_Query(sql, &result, &fields, &rowCount, &fieldCount))
    {
        return NULL;
    }

    QueryFieldNames names(fieldCount);
    for(uint32 i = 0; i < fieldCount; ++i)
    {
        names[i] = fields[i].name;
    }

    QueryResultMysql* queryResult = new QueryResultMysql(result, fields, rowCount, fieldCount);

    queryResult->NextRow();
    return new QueryNamedResult(queryResult, names);
}

bool MySQLConnection::Execute(const char *sql)
{
    if(!m_mysql)
    {
        return false;
    }

    {
        timeval timeBegin, timeEnd;
        gettimeofday(&timeBegin, NULL);

        if(mysql_query(m_mysql, sql))
        {
            // TODO, "SQL: %s", sql
            LOG(ERROR)<<"SQL: "<<sql;
            // TODO, "query ERROR: %s", mysql_error(mMysql)
            LOG(ERROR)<<"query ERROR: "<<mysql_error(m_mysql);
            return false;
        }
        else
        {
            gettimeofday(&timeEnd, NULL);
            // TODO, "[%u ms] SQL: %s", (timeEnd.tv_sec - timeBegin.tv_sec) * 1000 + (timeEnd.tv_usec - timeBegin.tv_usec)/1000, sql
            LOG(INFO)<<"["<<(timeEnd.tv_sec - timeBegin.tv_sec) * 1000 + (timeEnd.tv_usec - timeBegin.tv_usec)/1000<<" ms] SQL: "<<sql;
        }
    }

    return true;
}

bool MySQLConnection::_TransactionCmd(const char* sql)
{
    if(mysql_query(m_mysql, sql))
    {
        // TODO, "SQL: %s", sql
        LOG(ERROR)<<"SQL: "<<sql;
        // TODO, "query ERROR: %s", mysql_error(mMysql)
        LOG(ERROR)<<"query ERROR: "<<mysql_error(m_mysql);
        return false;
    }
    else
    {
        // TODO, "SQL: %s", sql
        LOG(ERROR)<<"SQL: "<<sql;
    }

    return true;
}

bool MySQLConnection::BeginTransaction()
{
    return _TransactionCmd("START TRANSACTION");
}

bool MySQLConnection::CommitTransaction()
{
    return _TransactionCmd("COMMIT");
}

bool MySQLConnection::RollbackTransaction()
{
    return _TransactionCmd("ROLLBACK");
}

unsigned long MySQLConnection::escape_string(char* to, const char* from, unsigned long length)
{
    if(!m_mysql || !to || ! from || !length)
    {
        return 0;
    }

    return mysql_real_escape_string(m_mysql, to, from, length);
}


///////////////////////////////////////////////////////////////////////

SqlPreparedStatement* MySQLConnection::CreateStatement(const std::string& fmt)
{
    return new MySqlPreparedStatement(fmt, *this, m_mysql);
}

///////////////////////////////////////////////////////////////////////

MySqlPreparedStatement::MySqlPreparedStatement(const std::string& fmt, SqlConnection& conn, MYSQL* mysql)
    : SqlPreparedStatement(fmt, conn)
    , m_pMySQLConn(mysql)
    , m_stmt(NULL)
    , m_pInputArgs(NULL)
    , m_pResult(NULL)
    , m_pResultMetadata(NULL)   {}

MySqlPreparedStatement::~MySqlPreparedStatement()
{
    RemoveBinds();
}

bool MySqlPreparedStatement::prepare()
{
    if(isPrepared())
    {
        return true;
    }

    // remove old binds
    RemoveBinds();

    // create statement object
    m_stmt = mysql_stmt_init(m_pMySQLConn);
    if(!m_stmt)
    {
        // TODO, "SQL: mysql_stmt_init() failed "
        LOG(ERROR)<<"SQL: mysql_stmt_init() failed ";
        return false;
    }

    // prepare statement
    if(mysql_stmt_prepare(m_stmt, m_szFmt.c_str(), m_szFmt.length()))
    {
        // TODO, "SQL: mysql_stmt_prepare() failed for '%s'", m_szFmt.c_str()
        LOG(ERROR)<<"SQL: mysql_stmt_prepare() failed for '"<<m_szFmt<<"'";
        // "SQL ERROR: %s", mysql_stmt_error(m_stmt)
        LOG(ERROR)<<"SQL ERROR: "<<mysql_stmt_error(m_stmt);
        return false;
    }

    // Get the parameter count from the statement
    m_params = mysql_stmt_param_count(m_stmt);

    // Fetch result set meta information
    m_pResultMetadata = mysql_stmt_result_metadata(m_stmt);
    // if we do not have result metadata
    if(!m_pResultMetadata && strncasecmp(m_szFmt.c_str(), "select", 6) == 0)
    {
        // TODO, "SQL: no meta information for '%s'", m_szFmt.c_str()
        LOG(ERROR)<<"SQL: no meta information for '"<<m_szFmt<<"'";
        // TODO, SQL ERROR: %s", mysql_stmt_error(m_stmt)
        LOG(ERROR)<<"SQL ERROR: "<<mysql_stmt_error(m_stmt);
        return false;
    }

    // bind input buffers
    if(m_params)
    {
        m_pInputArgs = new MYSQL_BIND[m_params];
        memset(m_pInputArgs, 0, sizeof(MYSQL_BIND) * m_params);
    }

    // check if wo have a statement which returns result sets
    if(m_pResultMetadata)
    {
        m_isQuery = true;
        m_columns = mysql_num_fields(m_pResultMetadata);
        // bind output buffers
    }

    m_prepared = true;
    return true;
}

void MySqlPreparedStatement::bind(const SqlStmtParameters& holder)
{
    if(!isPrepared())
    {
        Evil_ASSERT(false);
        return;
    }

    // finalize adding params
    if(!m_pInputArgs)
    {
        return;
    }

    // verify if we bound all needed input parameters
    if(m_params != holder.boundParams())
    {
        Evil_ASSERT(false);
        return;
    }

    unsigned int nIndex = 0;
    SqlStmtParameters::ParameterContainer const& _args = holder.params();

    SqlStmtParameters::ParameterContainer::const_iterator iter_last = _args.end();
    for(SqlStmtParameters::ParameterContainer::const_iterator iter = _args.begin(); iter != iter_last; ++iter)
    {
        // bind parameter
        addParam(nIndex++, (*iter));
    }

    // bind input arguments
    if(mysql_stmt_bind_param(m_stmt, m_pInputArgs))
    {
        // TODO, "SQL ERROR: mysql_stmt_bind_param() failed\n"
        LOG(ERROR)<<"SQL ERROR: mysql_stmt_bind_param() failed";
        // TODO, "SQL ERROR: %s", mysql_stmt_error(m_stmt)
        LOG(ERROR)<<"SQL ERROR: "<<mysql_stmt_error(m_stmt);
    }
}

void MySqlPreparedStatement::addParam(unsigned int nIndex, const SqlStmtFieldData& data)
{
    Evil_ASSERT(m_pInputArgs);
    Evil_ASSERT(nIndex < m_params);

    MYSQL_BIND& pData = m_pInputArgs[nIndex];

    my_bool bUnsigned = 0;
    enum_field_types dataType = ToMySQLType(data, bUnsigned);

    // setup MYSQL_BIND structure
    pData.buffer_type = dataType;
    pData.is_unsigned = bUnsigned;
    pData.buffer = data.buff();
    pData.length = 0;
    pData.buffer_length = data.type() == FIELD_STRING ? data.size() : 0;
}

void MySqlPreparedStatement::RemoveBinds()
{
    if(!m_stmt)
    {
        return;
    }

    SafeDeleteArray(m_pInputArgs);
    SafeDeleteArray(m_pResult);

    mysql_free_result(m_pResultMetadata);
    mysql_stmt_close(m_stmt);

    m_stmt = NULL;
    m_pResultMetadata = NULL;

    m_prepared = false;
}

bool MySqlPreparedStatement::execute()
{
    if(!isPrepared())
    {
        return false;
    }

    if(mysql_stmt_execute(m_stmt))
    {
        // TODO, "SQL: can not execute '%s'", m_szFmt.c_str()
        LOG(ERROR)<<"SQL: can not execute '"<<m_szFmt<<"'";
        // TODO, "SQL ERROR: %s", mysql_stmt_error(m_stmt)
        LOG(ERROR)<<"SQL ERROR: "<<mysql_stmt_error(m_stmt);
        return false;
    }

    return true;
}

enum_field_types MySqlPreparedStatement::ToMySQLType(const SqlStmtFieldData& data, my_bool& bUnsigned)
{
    bUnsigned = 0;
    enum_field_types dataType = MYSQL_TYPE_NULL;

    switch(data.type())
    {
    case FIELD_NONE:
        {
            dataType = MYSQL_TYPE_NULL;
            break;
        }
    case FIELD_BOOL:
    case FIELD_UI8:
        {
            dataType = MYSQL_TYPE_TINY;
            bUnsigned = 1;
            break;
        }
    case FIELD_I8:
        {
            dataType = MYSQL_TYPE_TINY;
            break;
        }
    case FIELD_UI16:
        {
            dataType = MYSQL_TYPE_SHORT;
            bUnsigned = 1;
            break;
        }
    case FIELD_I16:
        {
            dataType = MYSQL_TYPE_SHORT;
            break;
        }
    case FIELD_UI32:
        {
            dataType = MYSQL_TYPE_LONG;
            bUnsigned = 1;
            break;
        }
    case FIELD_I32:
        {
            dataType = MYSQL_TYPE_LONG;
            break;
        }
    case FIELD_UI64:
        {
            dataType = MYSQL_TYPE_LONGLONG;
            bUnsigned = 1;
            break;
        }
    case FIELD_I64:
        {
            dataType = MYSQL_TYPE_LONGLONG;
            break;
        }
    case FIELD_FLOAT:
        {
            dataType = MYSQL_TYPE_FLOAT;
            break;
        }
    case FIELD_DOUBLE:
        {
            dataType = MYSQL_TYPE_DOUBLE;
            break;
        }
    case FIELD_STRING:
        {
            dataType = MYSQL_TYPE_STRING;
            break;
        }
    default:
        break;
    }

    return dataType;
}

