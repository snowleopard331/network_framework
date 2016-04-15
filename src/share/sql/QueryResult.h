/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/07
*/
#ifndef _QUERY_H_
#define _QUERY_H_

#include "Common.h"
#include "Field.h"

class QueryResult
{
public:
    QueryResult(uint64 rowCount, uint32 fieldCount)
        : m_FieldCount(fieldCount)
        , m_RowCount(rowCount) 
    {

    }

    virtual ~QueryResult() 
    {

    }

    virtual bool NextRow() = 0;

    Field* Fetch() const 
    {
        return m_pCurrentRow;
    }

    const Field& operator[](int index) const 
    {
        return m_pCurrentRow[index];
    }

    uint32 GetFieldCount() const 
    {
        return m_FieldCount;
    }

    uint64 GetRowCount() const 
    {
        return m_RowCount;
    }


protected:
    Field*      m_pCurrentRow;
    uint32      m_FieldCount;
    uint64      m_RowCount;
};

#endif//_QUERY_H_