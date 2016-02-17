/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#ifndef _BYTE_BUFFER_H_
#define _BYTE_BUFFER_H_

#include "Common.h"

class ByteBufferException
{
    bool        m_isAdd;
    size_t      m_pos;
    size_t      m_esize;
    size_t      m_size;

public:
    ByteBufferException(bool add, size_t pos, size_t esize, size_t size)
        : m_isAdd(add)
        , m_pos(pos)
        , m_esize(esize)
        , m_size(size)
    {
        printPosError();
    }

    void printPosError() const;
};


template<class T>
struct Unused
{
    Unused(){}
};

class ByteBuffer
{
public:
    const static size_t DEFAULT_SIZE = 4096;

    ByteBuffer()
        : _rpos(0)
        , _wpos(0)
    {
        _storage.reserve(DEFAULT_SIZE);
    }

    ByteBuffer(size_t res)
        : _rpos(0)
        , _wpos(0)
    {
        _storage.reserve(res);
    }

    ByteBuffer(const ByteBuffer& buf)
        : _rpos(buf._rpos)
        , _wpos(buf.wpos)
        , _storage(buf._storage)
    {

    }

    void clear()
    {
        _storage.clear();
        _rpos = _wpos = 0;
    }

    void put(size_t pos, const uint8* src, size_t cnt)
    {
        if(pos + cnt > _storage.size())
        {
            throw ByteBufferException(true, pos, cnt, _storage.size());
        }

        memcpy(&_storage[pos], src, cnt);
    }

    template<typename T>
    void put(size_t pos, T value)
    {
        // ? endian
        put(pos, (uint8*)&value, sizeof(value));
    }

    ByteBuffer& operator<<(uint8 value)
    {
        append<uint8>(value);
        return *this;
    }

    ByteBuffer& operator<<(uint16 value)
    {
        append<uint16>(value);
        return *this;
    }

    ByteBuffer& operator<<(uint32 value)
    {
        append<uint32>(value);
        return *this;
    }

    ByteBuffer& operator<<(uint64 value)
    {
        append<uint64>(value);
        return *this;
    }

    ByteBuffer& operator<<(int8 value)
    {
        append<int8>(value);
        return *this;
    }

    ByteBuffer& operator<<(int16 value)
    {
        append<int16>(value);
        return *this;
    }

    ByteBuffer& operator<<(int32 value)
    {
        append<int32>(value);
        return *this;
    }

    ByteBuffer& operator<<(int64 value)
    {
        append<int64>(value);
        return *this;
    }

    ByteBuffer& operator<<(float value)
    {
        append<float>(value);
        return *this;
    }

    ByteBuffer& operator<<(double value)
    {
        append<double>(value);
        return *this;
    }

    ByteBuffer& operator<<(const std::string& value)
    {
        append((uint8 const*)value.c_str(), value.length());
        append((uint8)0);
        return *this;
    }

    ByteBuffer& operator<<(const char* value)
    {
        append((uint8 const*)value, value ? strlen(value) : 0);
        append((uint8)0);
        return *this;
    }

    ByteBuffer& operator>>(bool& value)
    {
        value = read<char>() > 0 ? true : false;
        return *this;
    }

    ByteBuffer& operator>>(uint8& value)
    {
        value = read<uint8>();
        return *this;
    }

    ByteBuffer& operator>>(uint16& value)
    {
        value = read<uint16>();
        return *this;
    }

    ByteBuffer& operator>>(uint32& value)
    {
        value = read<uint32>();
        return *this;
    }

    ByteBuffer& operator>>(uint64& value)
    {
        value = read<uint64>();
        return *this;
    }

    ByteBuffer& operator>>(int8& value)
    {
        value = read<int8>();
        return *this;
    }

    ByteBuffer& operator>>(int16& value)
    {
        value = read<int16>();
        return *this;
    }

    ByteBuffer& operator>>(int32& value)
    {
        value = read<int32>();
        return *this;
    }

    ByteBuffer& operator>>(int64& value)
    {
        value = read<int64>();
        return *this;
    }

    ByteBuffer& operator>>(float& value)
    {
        value = read<float>();
        return *this;
    }

    ByteBuffer& operator>>(double& value)
    {
        value = read<double>();
        return *this;
    }

    ByteBuffer& operator>>(std::string& value)
    {
        value.clear();
        // prevent crash at wrong string format in packet
        while(_rpos < _storage.size())
        {
            char c = read<char>();
            if(0 == c)
            {
                break;
            }

            value += c;
        }

        return *this;
    }

    template<class T>
    ByteBuffer& operator>>(Unused<T> const&)
    {
        read_skip<T>();
        return *this;
    }

    uint8 operator[](size_t pos) const
    {
        return read<uint8>(pos);
    }

    size_t rpos() const
    {
        return _rpos;
    }

    size_t rpos(size_t rpos_)
    {
        _rpos = rpos_;
        return _rpos;
    }

    size_t wpos() const
    {
        return _wpos;
    }

    size_t wpos(size_t wpos_)
    {
        _wpos = wpos_;
        return _wpos;
    }

    size_t size() const
    {
        return _storage.size();
    }

    void append(const uint8* src, size_t cnt)
    {
        if(!cnt)
        {
            return;
        }

        Jovi_ASSERT(_storage.size() < 100000000);

        if(_storage.size() < _wpos + cnt)
        {
            _storage.resize(_wpos + cnt);
        }

        memcpy(&_storage[_wpos], src, cnt);
        _wpos += cnt;
    }

    void append(const std::string& str)
    {
        append((uint8 const*)str.c_str(), str.size() + 1);
    }

    template<class T>
    void append(const T* src, size_t cnt)
    {
        return append((const uint8*)src, cnt * sizeof(T));
    }

    void append(const char* src, size_t cnt)
    {
        return append((const uint8*)src, cnt);
    }

    void append(const ByteBuffer& buffer)
    {
        if(buffer.wpos())
        {
            append(buffer.contents(), buffer.wpos());
        }
    }

    template<typename T>
    T read()
    {
        T r = read<T>(_rpos);
        _rpos += sizeof(T);
        return r;
    }

    template<typename T>
    T read(size_t pos) const
    {
        if(pos + sizeof(T) > _storage.size())
        {
            throw ByteBufferException(false, pos, sizeof(T), _storage.size());
        }
        T val = *((T const*)&_storage[pos]);
        // ? endian
        return val;
    }

    void read(uint8* dest, size_t len)
    {
        if(_rpos + len > _storage.size())
        {
            throw ByteBufferException(false, _rpos, len, _storage.size());
        }
        memcpy(dest, &_storage[_rpos], len);
        _rpos += len;
    }

    template<typename T>
    void read_skip()
    {
        read_skip(sizeof(T));
    }

    void read_skip(size_t skip)
    {
        if(_rpos + skip > _storage.size())
        {
            throw ByteBufferException(false, _rpos, skip, _storage.size());
        }
        _rpos += skip;
    }

    const uint8* contents() const
    {
        return &_storage[0];
    }

    bool empty() const
    {
        return _storage.empty();
    }

    void resize(size_t newsize)
    {
        _storage.resize(newsize);
        _rpos = 0;
        _wpos = _storage.size();
    }

    void reserve(size_t ressize)
    {
        if(ressize > _storage.size())
        {
            _storage.reserve(ressize);
        }
    }



    //void print_storage() const;

    //void textlike() const;

    //void hexlike() const;

private:
    // limited for internal use because can "append" any unexpected type (like pointer and etc) with hard detection problem
    template<typename T>
    void append(T value)
    {
        // ? endian
        append((uint8*)&value, sizeof(value));
    }

protected:
    size_t _rpos, _wpos;
    std::vector<uint8> _storage;
};

#endif//_BYTE_BUFFER_H_