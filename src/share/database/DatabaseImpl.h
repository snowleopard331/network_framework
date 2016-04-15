/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/14
*/

#ifndef _DATABASE_IMPL_H_
#define _DATABASE_IMPL_H_

#include "DatabaseEnv.h"
#include "SqlOperations.h"

#define ASYNC_QUERY_BODY(sql) (if(!sql || !m_pResultQueue) return false;)
#define ASYNC_DELAYHOLDER_BODY(holder) (if(!holder || !m_pResultQueue) return false;)

#define ASYNC_PQUERY_BODY(format, szQuery)  \
    if(!format) \
    {   \
        return false;   \
    }   \
    char szQuery[MAX_QUERY_LEN];    \
    {   \
        va_list ap; \
        va_start(ap, format);   \
        int res = vsnprintf(szQuery, MAX_QUERY_LEN, format, ap);    \
        va_end(ap); \
        if(-1 == res)   \
        {   \
            //TODO, "SQL Query truncated (and not execute) for format: %s",format
            LOG(ERROR)<<"SQL Query truncated (and not execute) for format: "<<format;   \
            return false;   \
        }   \
    }


/// ---- QUERY / MEMBER ----

template<class Class>
bool Database::AsyncQuery(Class* object, void (Class::*method)(QueryResult*), const char* sql)
{
    ASYNC_QUERY_BODY(sql);
    return m_threadBody->Delay(new SqlQuery(sql, new Jovi::QueryCallBack<Class>(object, method), m_pResultQueue))
}

template<class Class, typename ParamType1>
bool Database::AsyncQuery(Class* object, void(Class::*method)(QueryResult*, ParamType1), ParamType1 param1, const char* sql)
{
    ASYNC_QUERY_BODY(sql);
    return m_threadBody->Delay(new SqlQuery(sql, new Jovi::QueryCallBack<Class, ParamType1>(object, method, NULL, param1), m_pResultQueue));
}

template<class Class, typename ParamType1, typename ParamType2>
bool Database::AsyncQuery(Class* object, void(Class::*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* sql)
{
    ASYNC_QUERY_BODY(sql);
    return m_threadBody->Delay(new SqlQuery(sql, new Jovi::QueryCallBack<Class, ParamType1, ParamType2>(object, method, NULL, param1, param2), m_pResultQueue));
}

template<class Class, typename ParamType1, typename ParamType2, typename ParamType3>
bool Database::AsyncQuery(Class* object, void(Class::*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* sql)
{
    ASYNC_QUERY_BODY(sql);
    return m_threadBody->Delay(new SqlQuery(sql, new Jovi::QueryCallBack<Class, ParamType1, ParamType2, ParamType3>(object, method, NULL, param1, param2, param3), m_pResultQueue));
}


/// ---- QUERY / STATIC ----

template<typename ParamType1>
bool Database::AsyncQuery(void (*method)(QueryResult*, ParamType1), ParamType1 param1, const char* sql)
{
    ASYNC_QUERY_BODY(sql);
    return m_threadBody->Delay(new SqlQuery(sql, new Jovi::SQueryCallback<ParamType1>(method, NULL, param1), m_pResultQueue));
}

template<typename ParamType1, typename ParamType2>
bool Database::AsyncQuery(void (*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* sql)
{
    ASYNC_QUERY_BODY(sql);
    return m_threadBody->Delay(new SqlQuery(sql, new Jovi::SQueryCallback<ParamType1, ParamType2>(method, NULL, param1, param2), m_pResultQueue));
}

template<typename ParamType1, typename ParamType2, typename ParamType3>
bool Database::AsyncQuery(void (*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* sql)
{
    ASYNC_QUERY_BODY(sql);
    return m_threadBody->Delay(new SqlQuery(sql, new Jovi::SQueryCallback<ParamType1, ParamType2, ParamType3>(method, NULL, param1, param2, param3), m_pResultQueue));
}


/// --- PQUERY / MEMBER ----

template<class Class>
bool Database::AsyncPQuery(Class* object, void (Class::*method)(QueryResult*), const char* format, ...)
{
    ASYNC_PQUERY_BODY(format, szQuery);
    return AsyncQuery(object, method, szQuery);
}

template<class Class, typename ParamType1>
bool Database::AsyncPQuery(Class* object, void (Class::*method)(QueryResult*, ParamType1), ParamType1 param1, const char* format, ...)
{
    ASYNC_PQUERY_BODY(format, szQuery);
    return AsyncQuery(object, method, param1, szQuery);
}

template<class Class, typename ParamType1, typename ParamType2>
bool Database::AsyncPQuery(Class* object, void (Class::*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* format, ...)
{
    ASYNC_PQUERY_BODY(format, szQuery);
    return AsyncQuery(object, method, param1, param2, szQuery);
}

template<class Class, typename ParamType1, typename ParamType2, typename ParamType3>
bool Database::AsyncPQuery(Class* object, void (Class::*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* format, ...)
{
    ASYNC_PQUERY_BODY(format, szQuery);
    return AsyncQuery(object, method, param1, param2, param3, szQuery);
}


/// ---- PQUERY / STATIC ----

template<typename ParamType1>
bool Database::AsyncPQuery(void (*method)(QueryResult*, ParamType1), ParamType1 param1, const char* format, ...)
{
    ASYNC_PQUERY_BODY(format, szQuery);
    return AsyncQuery(method, param1, szQuery);
}

template<typename ParamType1, typename ParamType2>
bool Database::AsyncPQuery(void (*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char* format, ...)
{
    ASYNC_PQUERY_BODY(format, szQuery);
    return AsyncQuery(method, param1, param2, szQuery);
}

template<typename ParamType1, typename ParamType2, typename ParamType3>
bool Database::AsyncPQuery(void (*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char* format, ...)
{
    ASYNC_PQUERY_BODY(format, szQuery);
    return AsyncQuery(method, param1, param2, param3, szQuery);
}


/// ---- QUERY HOLEER ----

template<class Class>
bool Database::DelayQueryHolder(Class* object, void (Class::*method)(QueryResult*, SqlQueryHolder*), SqlQueryHolder* holder)
{
    ASYNC_DELAYHOLDER_BODY(holder);
    return holder->Execute(new Jovi::QueryCallBack<Class, SqlQueryHolder*>(object, method, NULL, holder), m_threadBody, m_pResultQueue);
}

template<class Class, typename ParamType1>
bool Database::DelayQueryHolder(Class* object, void (Class::*method)(QueryResult*, SqlQueryHolder*, ParamType1), SqlQueryHolder* holder, ParamType1 param1)
{
    ASYNC_DELAYHOLDER_BODY(holder);
    return holder->Execute(new Jovi::QueryCallBack<Class, SqlQueryHolder*>(object, method, NULL, holder, param1), m_threadBody, m_pResultQueue);
}

#undef ASYNC_QUERY_BODY
#undef ASYNC_PQUERY_BODY
#undef ASYNC_DELAYHOLDER_BODY

#endif//_DATABASE_IMPL_H_