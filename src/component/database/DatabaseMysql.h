/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/14
*/

#ifndef _DATABASE_MYSQL_H_
#define _DATABASE_MYSQL_H_

#include <mysql.h>

#include "Database.h"

class MySqlPreparedStatement
    : public SqlPreparedStatement
{
private:
    MYSQL*          m_pMySQLConn;
    MYSQL_STMT*     m_stmt;
    MYSQL_BIND*     m_pInputArgs;
    MYSQL_BIND*     m_pResult;
    MYSQL_RES*      m_pResultMetadata;

public:
    MySqlPreparedStatement(const std::string& fmt, SqlConnection& conn, MYSQL* mysql);

    ~MySqlPreparedStatement();

    virtual bool prepare() override;

    virtual void bind(const SqlStmtParameters& holder) override;

    // execute DML statement
    virtual bool execute() override;

protected:
    void addParam(unsigned int nIndex, const SqlStmtFieldData& data);

    static enum_field_types ToMySQLType(const SqlStmtFieldData& data, my_bool& bUnsigned);

private:
    void RemoveBinds();
};

class MySQLConnection
    : public SqlConnection
{
private:
    MYSQL*  m_mysql;

public:
    MySQLConnection(Database& db)
        : SqlConnection(db)
        , m_mysql(NULL) {}

    ~MySQLConnection();

    /**
    * @brief Initializes Mysql and connects to a server.
    *
    * @param infoString infoString should be formated like hostname;username;password;database
    * @return bool
    */
    bool Initialize(const char *infoString) override;

    QueryResult* Query(const char *sql) override;

    QueryNamedResult* QueryNamed(const char *sql) override;

    bool Execute(const char *sql) override;

    unsigned long escape_string(char* to, const char* from, unsigned long length);

    bool BeginTransaction() override;

    bool CommitTransaction() override;

    bool RollbackTransaction() override;

protected:
    SqlPreparedStatement* CreateStatement(const std::string& fmt) override;

private:
    bool _TransactionCmd(const char* sql);

    bool _Query(const char* sql, MYSQL_RES** pResult, MYSQL_FIELD** pFields, uint64* pRowCount, uint32* pFieldCount);
};


class DatabaseMysql
    : public Database
{
private:
    static size_t db_count;

public:
    DatabaseMysql();
    ~DatabaseMysql();

    // must be call before first query in thread
    void ThreadStart() override;

    // must be call before finish thread run
    void ThreadEnd() override;

protected:
    virtual SqlConnection* CreateConnection() override;
};


#endif//_DATABASE_MYSQL_H_