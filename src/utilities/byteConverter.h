/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/03/29
*/

#ifndef _BYTE_CONVERTER_H_
#define _BYTE_CONVERTER_H_

#include <algorithm>

namespace ByteConverter
{
    template<size_t T>
    inline void convert(char* val)
    {
        std::swap(*val, *(val + T - 1));
        convert < T - 2 >(val + 1);
    }

    template<>
    inline void convert<0>(char*)
    {

    }

    template<>
    inline void convert<1>(char*)
    {

    }

    template<typename T>
    inline void apply(T* val)
    {
        convert<sizeof(T)>((char*)(val));
    }
}

// consider the endian of storage conversion(internal data encryption)
#if EVIL_ENDIAN == EVIL_LITTLE_ENDIAN

    template<typename T>
    inline void EndianConvert(T& val)
    {
        ByteConverter::apply<T>(&val);
    }

    template<typename T>
    inline void EndianConvertReverse(T&)
    {

    }

#else

    template<typename T>
    inline void EndianConvert(T&)
    {
    
    }

    template<typename T>
    inline void EndianConvertReverse(T& val)
    {
        ByteConverter::apply<T>(&val);
    }

#endif


//inline void EndianConvert(uint&)
//{
//
//}
//
//inline void EndianConvert(int&)
//{
//
//}
//
//inline void EndianConvertReverse(uint&)
//{
//
//}
//
//inline void EndianConvertReverse(int&)
//{
//
//}


#endif//_BYTE_CONVERTER_H_