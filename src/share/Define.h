/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/08
*/

#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <assert.h>
#include "StackTrace.h"
#include "log/glogHelper.h"

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#define DEBUG_INFO

typedef char                int8;
typedef unsigned char       uint8, uchar;
typedef short               int16;
typedef unsigned short      uint16, ushort;
typedef int                 int32;
typedef unsigned int        uint32, uint;
typedef long long           int64;
typedef unsigned long long  uint64;


typedef boost::asio::io_service                 Proactor;
typedef boost::asio::ip::tcp::socket            BSocket;
typedef boost::asio::ip::tcp::endpoint          EndPoint;


/// ---- RETURN ----

#ifndef IF_NOT_RETURN
#   define IF_NOT_RETURN(CONDITION) \
    if(!(CONDITION))    \
    {   \
        LOG(ERROR)<<#CONDITION; \
        return; \
    }
    // TODO, log
#endif//IF_NOT_RETURN

#ifndef IF_NOT_RETURN_FALSE
#   define IF_NOT_RETURN_FALSE(CONDITION) \
    if(!(CONDITION))    \
    {   \
        LOG(ERROR)<<#CONDITION; \
        return false; \
    }
    // TODO, log
#endif//IF_NOT_RETURN_FALSE

#ifndef IF_NOT_RETURN_VALUE
#   define IF_NOT_RETURN_VALUE(CONDITION, VALUE) \
    if(!(CONDITION))    \
    {   \
        LOG(ERROR)<<#CONDITION; \
        return (VALUE); \
    }
    // TODO, log
#endif//IF_NOT_RETURN_FALSE

/// ---- SAFEDELETE ----

#ifndef SafeDelete
#   define SafeDelete(p)   \
    if((p)) \
    {   \
        delete (p); \
        (p) = 0;    \
    }
#endif//SafeDelete
#ifndef SafeDeleteArray
#   define SafeDeleteArray(p)   \
    if((p)) \
    {   \
        delete [] (p);  \
        (p) = 0;    \
    }
#endif//SafeDeleteArray


/// ---- ERROR ----

#define WPErrorAssert(CONDITION)  \
    if(!(CONDITION))  \
    {   \
        StackTrace  stack;  \
        LOG(FATAL)<<__FILE__<<": "<<__LINE__<<": Error: Assertion in "<<__FUNCTION__<<" failed: "<<#CONDITION;  \
        LOG(FATAL)<<"Stack Trace: ";    \
        LOG(FATAL)<<stack.c_str();  \
        assert(0);   \
    }
    // TODO, ��־, ��ӡ�ļ���, �к�, ������, CONDITION, ��ջ
#define WPErrorExit(CONDITION)  \
    if(!(CONDITION))  \
    {   \
        StackTrace  stack;  \
        LOG(FATAL)<<__FILE__<<": "<<__LINE__<<": Error: Assertion in "<<__FUNCTION__<<" failed: "<<#CONDITION;  \
        LOG(FATAL)<<"Stack Trace: ";    \
        LOG(FATAL)<<stack.c_str();  \
        exit(-1);   \
    }
    // TODO, ��־, ��ӡ�ļ���, �к�, ������, CONDITION, ��ջ
#ifdef DEBUG_INFO
    #define Jovi_ASSERT     WPErrorAssert
#else
    #define Jovi_ASSERT
#endif

#ifndef SOCKET_READ_BUFFER_SIZE
#define SOCKET_READ_BUFFER_SIZE  4096
#endif


#endif//_DEFINE_H_