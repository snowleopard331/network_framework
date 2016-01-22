/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/20
*/

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "Common.h"

class Buffer
{
public:
    enum Flag
    {
        BUFFER_FLAG_ALLOCATE        = 0x00000001,
    };
public:
    Buffer(char* buf, uint32 bufSize)
        : m_Buffer(buf)
        , m_BufferSize(bufSize)
        , m_BufferFlag(0)
        , m_rPtr(0)
        , m_wPtr(0)
    {

    }

    Buffer(uint32 bufSize)
        : m_Buffer(NULL)
        , m_BufferSize(bufSize)
        , m_BufferFlag(0)
        , m_rPtr(0)
        , m_wPtr(0)
    {
        if(bufSize > 0)
        {
            m_Buffer = new char[m_BufferSize];
            setFlag(BUFFER_FLAG_ALLOCATE);
        }
    }

    ~Buffer()
    {
        if(m_Buffer && isFlag(BUFFER_FLAG_ALLOCATE))
        {
            delete [] m_Buffer;
        }

        m_Buffer = NULL;
        m_rPtr = m_wPtr = 0;

        m_BufferFlag = 0;
    }

public:

    void wr_ptr(uint pos)
    {
        m_wPtr = pos;
    }

    char* wr_ptr()
    {
        return m_Buffer + m_wPtr;
    }

    // set flag
    void setFlag(uint32 flag)
    {
        m_BufferFlag |= flag;
    }

    void unsetFlag(uint32 flag)
    {
        m_BufferFlag &= ~flag;
    }

    // check flag
    bool isFlag(uint32 flag) const
    {
        return (m_BufferFlag & flag) == flag;
    }

private:
    char*       m_Buffer;
    uint32      m_BufferSize;
    // is allocate self
    uint32      m_BufferFlag;

    uint32      m_rPtr;
    uint32      m_wPtr;
};

#endif//_BUFFER_H_