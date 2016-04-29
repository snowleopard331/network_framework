/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/15
*/

#include "DatabaseEnv.h"

QueryResultMysql::QueryResultMysql(MYSQL_RES* result, MYSQL_FIELD* fields, uint64 rowCount, uint32 fieldCount)
    : QueryResult(rowCount, fieldCount)
    , m_result(result)
{
    m_pCurrentRow = new Field[m_FieldCount];
    Evil_ASSERT(m_pCurrentRow);

    for(uint32 i = 0; i < m_FieldCount; ++i)
    {
        m_pCurrentRow[i].SetType(ConverNativeType(fields[i].type));
    }
}

QueryResultMysql::~QueryResultMysql()
{
    EndQuery();
}

bool QueryResultMysql::NextRow()
{
    MYSQL_ROW row;

    if(!m_result)
    {
        return false;
    }

    row = mysql_fetch_row(m_result);
    if(!row)
    {
        EndQuery();
        return false;
    }

    for(uint32 i = 0; i < m_FieldCount; ++i)
    {
        m_pCurrentRow[i].SetValue(row[i]);
    }

    return true;
}

void QueryResultMysql::EndQuery()
{
    SafeDeleteArray(m_pCurrentRow);

    if(m_result)
    {
        mysql_free_result(m_result);
        m_result = NULL;
    }
}

enum Field::DataTypes QueryResultMysql::ConverNativeType(enum_field_types mysqlType) const
{
    switch(mysqlType)
    {
    case FIELD_TYPE_TIMESTAMP:
    case FIELD_TYPE_DATE:
    case FIELD_TYPE_DATETIME:
    case FIELD_TYPE_TIME:
    case FIELD_TYPE_YEAR:
    case FIELD_TYPE_STRING:
    case FIELD_TYPE_VAR_STRING:
    case FIELD_TYPE_BLOB:
    case FIELD_TYPE_SET:
    case FIELD_TYPE_NULL:
        return Field::DB_TYPE_STRING;
    case FIELD_TYPE_TINY:
    case FIELD_TYPE_SHORT:
    case FIELD_TYPE_LONG:
    case FIELD_TYPE_INT24:
    case FIELD_TYPE_LONGLONG:
    case FIELD_TYPE_ENUM:
        return Field::DB_TYPE_INTEGER;
    case FIELD_TYPE_DECIMAL:
    case FIELD_TYPE_FLOAT:
    case FIELD_TYPE_DOUBLE:
        return Field::DB_TYPE_FLOAT;
    default:
        return Field::DB_TYPE_UNKNOWN;
    }
}
