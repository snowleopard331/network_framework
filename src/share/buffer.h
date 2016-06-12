/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/20
*/

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "Common.h"
#include "buffer.h"

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

    Buffer()
        : m_Buffer(NULL)
        , m_BufferSize(0)
        , m_BufferFlag(0)
        , m_rPtr(0)
        , m_wPtr(0)
    {

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

    void wr_ptr(size_t len)
    {
        if(len > this->space())
        {
            return;
        }

        m_wPtr += len;
    }

    char* wr_ptr() const
    {
        return m_Buffer + m_wPtr;
    }

    void wr_ptr(char* new_ptr)
    {
        if(static_cast<size_t>(new_ptr - m_Buffer) >= m_BufferSize)
        {
            return;
        }

        m_wPtr = new_ptr - m_Buffer;
    }

    void rd_ptr(size_t len)
    {
        if(len > m_BufferSize - m_rPtr)
        {
            return;
        }

        m_rPtr += len;
    }

    char* rd_ptr() const
    {
        return m_Buffer + m_rPtr;
    }

    void rd_ptr(char* new_ptr)
    {
        if(static_cast<size_t>(new_ptr - m_Buffer) >= m_BufferSize)
        {
            return;
        }

        m_rPtr = new_ptr - m_Buffer;
    }

    // set flag
    void setFlag(size_t flag)
    {
        m_BufferFlag |= flag;
    }

    void unsetFlag(size_t flag)
    {
        m_BufferFlag &= ~flag;
    }

    // check flag
    bool isFlag(size_t flag) const
    {
        return (m_BufferFlag & flag) == flag;
    }

    // get the number of bytes available after the <m_wPtr> in the top level
    size_t space() const
    {
        return m_BufferSize - m_wPtr;
    }

    /*
        Fun:
            Copies data into this Buffer from buf. 
            Data is copied into the block starting at the current write pointer.
        Param:
            buf  Pointer to the buffer to copy from.  
            n  The number of bytes to copy. 

        Return:
        0  on success; the write pointer is advanced by len

        -1  if the amount of free space following the write pointer in the block is less than
        n. Free space can be checked by calling space().  

    */
    int copy(const char* buf, size_t len) // ???????????????????´ýÐÞ¸Ä
    {
        if(space() < len)
        {
            return -1;
        }

        memcpy(m_Buffer + m_wPtr, buf, len);
        m_wPtr += len;

        return 0;
    }

    // Get the length of the readable message
    size_t length() const
    {
        return m_wPtr > m_rPtr ? m_wPtr - m_rPtr : 0;
    }

    // if isFlag BUFFER_FLAG_ALLOCATE but isDelete = false will lead to memery leak
    void base(char* data, size_t size, bool isDelete = false)
    {
        if(m_Buffer && isDelete && isFlag(BUFFER_FLAG_ALLOCATE))
        {
            delete [] m_Buffer;
            m_Buffer = NULL;
            m_BufferSize = 0;
            m_rPtr = m_wPtr = 0;
            unsetFlag(BUFFER_FLAG_ALLOCATE);
        }

        m_Buffer = data;
        m_BufferSize = size;
        m_rPtr = m_wPtr = 0;
    }

    char* base() const
    {
        return m_Buffer;
    }

    void reset()
    {
        m_wPtr = m_rPtr = 0;
        m_BufferFlag = 0;
    }

    int crunch()
    {
        if(this->rd_ptr() != 0)
        {
            if(this->rd_ptr() > this->wr_ptr())
            {
                return -1;
            }

            const size_t len = length();
            memmove(this->base(), this->rd_ptr(), len);

            this->rd_ptr(this->base());
            this->wr_ptr(this->base() + len);
        }

        return 0;
    }

private:
    char*       m_Buffer;
    size_t      m_BufferSize;
    // is allocate self
    uint32      m_BufferFlag;

    size_t      m_rPtr;
    size_t      m_wPtr;
};

#endif//_BUFFER_H_