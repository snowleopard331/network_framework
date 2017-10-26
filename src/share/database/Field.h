/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/07
*/

#ifndef _FIELD_H_
#define _FIELD_H_

#include <stdlib.h>
#include <stdio.h>
#include "Common.h"

class Field
{
public:
    enum DataTypes
    {
        DB_TYPE_UNKNOWN = 0x00,
        DB_TYPE_STRING  = 0x01,
        DB_TYPE_INTEGER = 0x02,
        DB_TYPE_FLOAT   = 0x03,
        DB_TYPE_BOOL    = 0x04,
    };

    Field()
        : m_Value(NULL)
        , m_Type(DB_TYPE_UNKNOWN) 
    {

    }

    Field(const char* value, enum DataTypes type)
        : m_Value(value)
        , m_Type(type)  
    {

    }

    ~Field() 
    {

    }

    enum DataTypes GetType() const 
    {
        return m_Type;
    }

    bool IsNULL() const 
    {
        return m_Value == NULL;
    }

    const char* GetString() const 
    {
        return m_Value;
    }

    std::string GetCppString() const
    {
        return m_Value ? m_Value : "";          // std::string s = 0 have undefine result in C++
    }

    float GetFloat() const
    {
        return m_Value ? static_cast<float>(atof(m_Value)) : 0.0f;
    }

    bool GetBool() const 
    {
        return m_Value ? atoi(m_Value) > 0 : false;
    }

    int32 GetInt32() const
    {
        return m_Value ? static_cast<int32>(atol(m_Value)) : int32(0);
    }

    uint8 GetUInt8() const
    {
        return m_Value ? static_cast<uint8>(atol(m_Value)) : uint8(0);
    }

    uint16 GetUInt16() const
    {
        return m_Value ? static_cast<uint16>(atol(m_Value)) : uint16(0);
    }

    int16 GetInt16() const
    {
        return m_Value ? static_cast<int16>(atol(m_Value)) : int16(0);
    }

    uint32 GetUInt32() const
    {
        return m_Value ? static_cast<uint32>(atol(m_Value)) : uint32(0);
    }

    uint64 GetUInt64() const
    {
        uint64 value = 0;
        if(!m_Value || sscanf(m_Value, "%lld", &value) < 0)
        {
            return 0;
        }

        return value;
    }

    void SetType(enum DataTypes type)   
    {
        m_Type = type;
    }

    /**
    * @brief no need for memory allocations to store resultset field strings
    *
    * all we need is to cache pointers returned by different DBMS APIs
    *
    * @param value
    */
    void SetValue(const char* value) 
    {
        m_Value = value;
    }

private:
    Field(Field const&);

    Field& operator=(Field const&);

    const char* m_Value;
    enum DataTypes m_Type;
};

#endif//_FIELD_H_