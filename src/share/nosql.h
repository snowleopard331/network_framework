/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/09/06
*/

#ifndef _NOSQL_H_
#define _NOSQL_H_

#include "Common.h"
#include <hiredis/hiredis.h>

typedef std::map<uint, redisContext*>   ConnectsList;   // key is fd

enum RedisOptionTypes
{
    REDIS_COMMAND_OPTION_NULL       = 0,

    // set
    REDIS_COMMAND_SET_EX,               // 超时时间, second 
    REDIS_COMMAND_SET_PX,               // 超时时间, millisecond

    REDIS_COMMAND_SET_NX,               // 只在键不存在时, 才对键进行设置操作
    REDIS_COMMAND_SET_XX,               // 只在键已经存在时, 才对键进行设置操作

    // mset
    REDIS_COMMAND_MSET_NX,              // 设置多个key-value对, 当且仅当所有key都不存在. 即使只有一个给定key已存在，也会拒绝执行所有给定key的设置操作
};

class RedisManager
{
public:

    RedisManager();
    ~RedisManager();


public:

    bool createConnect(const char* ip, ushort port);
    void closeConnect(uint fd);
    void closeAllConnect();


public:
    /* redis command */
    
    // -- string

    /*
        将字符串value关联到key

        opReplace = REDIS_COMMAND_SET_NX 或 REDIS_COMMAND_SET_XX 或 REDIS_COMMAND_OPTION_NULL
        opTime = REDIS_COMMAND_SET_EX 或 REDIS_COMMAND_SET_PX 或 REDIS_COMMAND_OPTION_NULL
        timeValue 当 opTime != REDIS_COMMAND_OPTION_NULL 时表示数据超时时间
    */
    bool set(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL, 
        RedisOptionTypes opTime = REDIS_COMMAND_OPTION_NULL, uint timeValue = 0);

    /*
        插入多个key-value

        opReplace = REDIS_COMMAND_OPTION_NULL 或 REDIS_COMMAND_MSET_NX
            
            REDIS_COMMAND_OPTION_NULL   表示如果某个给定key已经存在, 那么会用新值覆盖原来的旧值
            REDIS_COMMAND_MSET_NX       表示同时设置多个key-value对, 当且仅当所有给定key都不存在. 即使只有一个给定key已存在, 也会拒绝执行所有给定key的设置操作
    */
    bool mset(redisContext* redis, std::map < std::string, std::string >& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL);

    // -- list
    // -- hash
    // -- set
    // -- sortset
    // -- transaction

private:

    bool replyStateIsOK(redisReply* reply);


private:

    ConnectsList    m_connectsList;
};

#endif//_NOSQL_H_
