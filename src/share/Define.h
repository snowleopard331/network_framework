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
typedef boost::asio::ip::tcp::acceptor          Acceptor;


/*
    LOG with function name, used for business logic layer
    __FUNCTION__ / __func__ have no class name, only function name in linux
*/
#define ELOG(SEVERITY)  LOG(SEVERITY)<<"("<<BOOST_CURRENT_FUNCTION<<") "

/// ---- RETURN ----

#ifndef IF_NOT_RETURN
#   define IF_NOT_RETURN(CONDITION) \
    if(!(CONDITION))    \
    {   \
        ELOG(ERROR)<<#CONDITION; \
        return; \
    }
    // TODO, log
#endif//IF_NOT_RETURN

#ifndef IF_NOT_RETURN_FALSE
#   define IF_NOT_RETURN_FALSE(CONDITION) \
    if(!(CONDITION))    \
    {   \
        ELOG(ERROR)<<#CONDITION; \
        return false; \
    }
    // TODO, log
#endif//IF_NOT_RETURN_FALSE

#ifndef IF_NOT_RETURN_VALUE
#   define IF_NOT_RETURN_VALUE(CONDITION, VALUE) \
    if(!(CONDITION))    \
    {   \
        ELOG(ERROR)<<#CONDITION; \
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
        (p) = nullptr;    \
    }
#endif//SafeDelete
#ifndef SafeDeleteArray
#   define SafeDeleteArray(p)   \
    if((p)) \
    {   \
        delete [] (p);  \
        (p) = nullptr;    \
    }
#endif//SafeDeleteArray


/// ---- ERROR ----

#define WPErrorAssert(CONDITION)  \
    if(!(CONDITION))  \
    {   \
        StackTrace  stack;  \
        LOG(FATAL)<<__FILE__<<": "<<__LINE__<<": Error: Assertion in "<<"("<<BOOST_CURRENT_FUNCTION<<") "<<" failed: "<<#CONDITION;  \
        LOG(FATAL)<<"Stack Trace: ";    \
        LOG(FATAL)<<stack.c_str();  \
        assert(0);   \
    }
    // TODO, 日志, 打印文件名, 行号, 函数名, CONDITION, 堆栈
#define WPErrorExit(CONDITION)  \
    if(!(CONDITION))  \
    {   \
        StackTrace  stack;  \
        LOG(FATAL)<<__FILE__<<": "<<__LINE__<<": Error: Assertion in "<<"("<<BOOST_CURRENT_FUNCTION<<") "<<" failed: "<<#CONDITION;  \
        LOG(FATAL)<<"Stack Trace: ";    \
        LOG(FATAL)<<stack.c_str();  \
        exit(-1);   \
    }
    // TODO, 日志, 打印文件名, 行号, 函数名, CONDITION, 堆栈
#ifdef DEBUG_INFO
    #define Evil_ASSERT     WPErrorAssert
#else
    #define Evil_ASSERT
#endif


#ifndef SOCKET_READ_BUFFER_SIZE
#define SOCKET_READ_BUFFER_SIZE     4096
#endif

#ifndef SOCKET_WRITE_BUFFER_SIZE
#define SOCKET_WRITE_BUFFER_SIZE    4096
#endif


#endif//_DEFINE_H_