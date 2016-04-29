/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/07
*/

#ifndef _QUERY_NAMED_RESULT_H_
#define _QUERY_NAMED_RESULT_H_

#include "QueryResult.h"

typedef std::vector<std::string>    QueryFieldNames;

class QueryNamedResult
{
public:

    explicit QueryNamedResult(QueryResult* query, QueryFieldNames& names)
        : m_pQuery(query)
        , m_FieldNames(names) 
    {

    }

    ~QueryNamedResult() 
    {
        delete m_pQuery;
    }

    // compatible interface with QueryResult

    bool NextRow() 
    {
        return m_pQuery->NextRow();
    }

    Field* Fetch() const 
    {
        return m_pQuery->Fetch();
    }

    uint32 GetFieldCount() const 
    {
        return m_pQuery->GetFieldCount();
    }

    uint64 GetRowCount() const 
    {
        return m_pQuery->GetRowCount();
    }

    Field const& operator[](int index) const 
    {
        return (*m_pQuery)[index];
    }

    Field const& operator[](const std::string& name) const
    {
        return m_pQuery->Fetch()[GetFieldIndex(name)];
    }

    QueryFieldNames const& GetFieldNames() const 
    {
        return m_FieldNames;
    }

    uint32 GetFieldIndex(const std::string& name) const
    {
        for(uint32 index = 0; index < m_FieldNames.size(); ++index)
        {
            if(m_FieldNames[index] == name)
            {
                return index;
            }
        }

        // TODO, 日志, 退出程序
        Evil_ASSERT(false && "unknown field name");
        return uint32(-1);
    }

protected:
    QueryResult*        m_pQuery;
    QueryFieldNames     m_FieldNames;
};

#endif//_QUERY_NAMED_RESULT_H_