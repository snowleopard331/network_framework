/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/15
*/

#ifndef _QUERYRESULT_MYSQL_H_
#define _QUERYRESULT_MYSQL_H_

#include <mysql.h>

#include "Common.h"

class QueryResultMysql
    : public QueryResult
{
public:
    QueryResultMysql(MYSQL_RES* result, MYSQL_FIELD* fields, uint64 rowCount, uint32 fieldCount);

    ~QueryResultMysql();

    bool NextRow() override;

private:
    enum Field::DataTypes ConverNativeType(enum_field_types mysqlType) const;

    void EndQuery();

    MYSQL_RES* m_result;
};

#endif//_QUERYRESULT_MYSQL_H_