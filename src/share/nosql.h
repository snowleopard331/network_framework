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

    // lpush
    REDIS_COMMAND_LPUSHX,               // 若key不存在, 则 LPUSHX 不创建新list

    // rpush
    REDIS_COMMAND_RPUSHX,               // 若key不存在, 则 RPUSHX 不创建新list
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

    bool get(redisContext* redis, const std::string& key, std::string& value);

    /*
        插入多个key-value

        opReplace = REDIS_COMMAND_OPTION_NULL 或 REDIS_COMMAND_MSET_NX
            
            REDIS_COMMAND_OPTION_NULL   表示如果某个给定key已经存在, 那么会用新值覆盖原来的旧值
            REDIS_COMMAND_MSET_NX       表示同时设置多个key-value对, 当且仅当所有给定key都不存在. 即使只有一个给定key已存在, 也会拒绝执行所有给定key的设置操作
    */
    bool mset(redisContext* redis, std::map < std::string, std::string >& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL);

    bool mget(redisContext* redis, std::vector< std::string >& keys, std::map< std::string, std::string >& outMapInfo);

    bool getValueLen(redisContext* redis, std::string& key, uint& len);


    // -- list

    // 获取list长度
    bool llen(redisContext* redis, const std::string& key, uint& len);

    // 插入表头
    bool lpush(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);
    bool lpush(redisContext* redis, const std::string& key, std::vector<std::string>& values, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);

    // 插入表尾
    bool rpush(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);
    bool rpush(redisContext* redis, const std::string& key, std::vector<std::string>& values, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);

    // 移除表头元素
    bool lpop(redisContext* redis, const std::string& key);

    // 移除表尾元素
    bool rpop(redisContext* redis, const std::string& key);

    bool lrem

    // -- hash
    // -- set
    // -- sortset
    // -- transaction

private:

    bool replyStateIsOK(redisReply* reply);

    // if reply is nulptr or state is error, return false
    bool replyErrOrNullCheck(redisReply* reply);

    bool _mset(redisContext* redis, std::string& value);
    bool _msetnx(redisContext* redis, std::string& value);

private:

    ConnectsList    m_connectsList;
};

#endif//_NOSQL_H_
