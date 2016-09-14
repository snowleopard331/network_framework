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
    REDIS_COMMAND_OPTION_SET_EX,               // 超时时间, second 
    REDIS_COMMAND_OPTION_SET_PX,               // 超时时间, millisecond

    REDIS_COMMAND_OPTION_SET_NX,               // 只在键不存在时, 才对键进行设置操作
    REDIS_COMMAND_OPTION_SET_XX,               // 只在键已经存在时, 才对键进行设置操作

    // mset
    REDIS_COMMAND_OPTION_MSET_NX,              // 设置多个key-value对, 当且仅当所有key都不存在. 即使只有一个给定key已存在，也会拒绝执行所有给定key的设置操作

    // lpush
    REDIS_COMMAND_OPTION_LPUSHX,               // 若key不存在, 则 LPUSHX 不创建新list

    // rpush
    REDIS_COMMAND_OPTION_RPUSHX,               // 若key不存在, 则 RPUSHX 不创建新list
};

class RedisManager
{
public:

    RedisManager();
    ~RedisManager();


public:

    // 创建连接
    bool createConnect(const std::string ip, const ushort port, uint timeoutSecs = 0);

    // 验证密码
    bool authPassword(redisContext* redis, const std::string password);

    // 关闭指定连接
    void closeConnect(uint fd);
    
    // 关闭所有连接
    void closeAllConnect();

    // Ping
    bool ping(redisContext* redis);

#ifdef DEBUG_INFO_REDIS

    redisContext* getConnect()
    {
        if (m_connectsList.empty())
        {
            return nullptr;
        }

        return m_connectsList.begin()->second;
    }

#endif

public:
    /* redis command */
    
    // -- key

    // 返回给定key的剩余生存时间
    bool ttl(redisContext* redis, const std::string& key, int& outValue);

    // 删除一个或多个key
    bool del(redisContext* redis, std::vector<std::string>& keys, uint& delSize);
    bool del(redisContext* redis, const std::string& key);

    // 设置生存时间
    bool expire(redisContext* redis, const std::string& key, uint secs);


    // -- string

    /*
        将字符串value关联到key

        opReplace = REDIS_COMMAND_OPTION_SET_NX 或 REDIS_COMMAND_OPTION_SET_XX 或 REDIS_COMMAND_OPTION_NULL
        opTime = REDIS_COMMAND_OPTION_SET_EX 或 REDIS_COMMAND_OPTION_SET_PX 或 REDIS_COMMAND_OPTION_NULL
        timeValue 当 opTime != REDIS_COMMAND_OPTION_NULL 时表示数据超时时间
    */
    bool set(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL, 
        RedisOptionTypes opTime = REDIS_COMMAND_OPTION_NULL, uint timeValue = 0);

    bool get(redisContext* redis, const std::string& key, std::string& value);

    /*
        插入多个key-value

        opReplace = REDIS_COMMAND_OPTION_NULL 或 REDIS_COMMAND_OPTION_MSET_NX
            
            REDIS_COMMAND_OPTION_NULL   表示如果某个给定key已经存在, 那么会用新值覆盖原来的旧值
            REDIS_COMMAND_OPTION_MSET_NX       表示同时设置多个key-value对, 当且仅当所有给定key都不存在. 即使只有一个给定key已存在, 也会拒绝执行所有给定key的设置操作
    */
    bool mset(redisContext* redis, std::map < std::string, std::string >& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL);

    bool mget(redisContext* redis, std::vector< std::string >& keys, std::map< std::string, std::string >& outMapInfo);

    bool getValueLen(redisContext* redis, const std::string& key, uint& len);


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

    // 移除与value相同的值. count为移除个数, 正数表示从前到后移除count个，负数表示从后往前移除count个，0表示移除所有
    bool lrem(redisContext* redis, const std::string& key, const std::string& value, uint& removeSize, int count = 0);


    // -- hash

    /*
        将哈希表 key 中的域 field 的值设为 value, 若 key 不存在, 则创建 key 的哈希表, 若存在且域已经存在, 旧值将被覆盖.
        ret 返回1表示 field 是哈希表的一个新建域并设置成功, 0表示field已经存在且被新值覆盖
    */
    bool hset(redisContext* redis, const std::string& key, const std::string& field, const std::string& value, uint& ret);

    // 哈希表 key 中给定域 field 的值
    bool hget(redisContext* redis, const std::string& key, const std::string& field, std::string& outValue);

    // 同时将多个 field-value 对设置到哈希表 key 中, 此命令会覆盖哈希表中已存在的域
    bool hmset(redisContext* redis, const std::string& key, std::map<std::string, std::string>& values);

    // 返回哈希表 key 中，一个或多个给定域的值
    bool hmget(redisContext* redis, const std::string& key, std::vector<std::string>& fields, std::map<std::string, std::string>& outMapInfo);

    // 返回哈希表 key 中所有的域和值
    bool hgetall(redisContext* redis, const std::string& key, std::map<std::string, std::string>& outMapInfo);

    // 返回哈希表 key 中所有的域
    bool hkeys(redisContext* redis, const std::string& key, std::vector<std::string>& outFields);

    // 返回哈希表 key 中所有域的值
    bool hvals(redisContext* redis, const std::string& key, std::vector<std::string>& outValues);

    // 删除哈希表 key 中的多个域
    bool hdel(redisContext* redis, const std::string& key, std::vector<std::string>& fields, uint& delSize);
    bool hdel(redisContext* redis, const std::string& key, std::string& field, uint& delSize);

    // 判断哈希表 key 中, 给定域 filed 是否存在
    bool hexists(redisContext* redis, const std::string& key, const std::string& field, bool& isExit);

    // 获取哈希表 key 中域的数量
    bool hlen(redisContext* redis, const std::string& key, uint& fieldSize);


    // -- set

    // 判断 member 是否是集合 key 的成员
    bool sismember(redisContext* redis, const std::string& key, const std::string& member, bool& isMember);

    // 将一个或多个member 元素加入到集合key 当中，已经存在于集合的member 元素将被忽略
    bool sadd(redisContext* redis, const std::string& key, const std::string& member);
    bool sadd(redisContext* redis, const std::string& key, std::vector<std::string>& members);

    // 获取集合中元素的数量
    bool scard(redisContext* redis, const std::string& key, uint& memSize);

    // 获取多个 key 的差集(属于第一个key 且不属于之后任何一个key的元素)
    bool sdiff(redisContext* redis, const std::string& key, std::vector<std::string>& otherKeys, std::vector<std::string>& outInfo);

    // 获取多个 key 的交集
    bool sinter(redisContext* redis, std::vector<std::string>& keys, std::vector<std::string>& outInfo);

    // 获取多个 key 的并集
    bool sunion(redisContext* redis, std::vector<std::string>& keys, std::vector<std::string>& outInfo);

    // 获取集合 key 中所有成员
    bool smembers(redisContext* redis, const std::string& key, std::vector<std::string>& outMembers);

    // 移除集合 key 中的元素
    bool srem(redisContext* redis, const std::string& key, std::vector<std::string>& members, uint& removeSize);
    bool srem(redisContext* redis, const std::string& key, const std::string& member);


    // -- sortset

    // 添加一个或多个 member 元素及其 score 值加入到有序集 key 当中, 若 member 已存在则更新 score 值, addSize返回被成功添加的数量(不包括更新的和已经存在的)
    bool zadd(redisContext* redis, const std::string& key, std::map<int, std::string>& members/* score - member */, uint& addSize);
    bool zadd(redisContext* redis, const std::string& key, int score, const std::string& member);

    // 返回有序集合的member数量
    bool zcard(redisContext* redis, const std::string& key, uint& memberSize);

    // 返回有序集合中, score值在min和max间(闭区间)的成员数量
    bool zcount(redisContext* redis, const std::string& key, int scoreMin, int scoreMax, uint& size);

    // 为有序集合 key 的成员 member 的 score 加上增量 inc
    bool zincrby(redisContext* redis, const std::string& key, int inc, const std::string& member, int& newScore);

    /*
        下标0表示第一个member, 1表示第二个member, -1表示最后一个member
        例: start = 0, stop = -1  表示显示整个有序集合成员
    */
    // 返回指定闭区间成员, score从小到大排序, 不包括score值
    bool zrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers);
    // 返回指定闭区间成员, score从小到大排序, 包括score值
    bool zrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers, std::vector<int>& outScores);
    // 返回指定闭区间成员, score从大到小排序, 不包括score值
    bool zrevrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers);
    // 返回指定闭区间成员, score从大到小排序, 包括score值
    bool zrevrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers, std::vector<int>& outScores);

    /* 当区间为正无限或者负无限情况的api如需要, 则另行添加 */
    // 返回有序集合中所有score介于min和max之间的成员(闭区间), 从小到大, 不包括score
    bool zrangebyscore(redisContext* redis, const std::string& key, int min, int max, std::vector<std::string>& outMembers);
    // 返回有序集合中所有score介于min和max之间的成员(闭区间), 从小到大, 包括score
    bool zrangebyscore(redisContext* redis, const std::string& key, int min, int max, std::vector<std::string>& outMembers, std::vector<int>& outScores);
    // 返回有序集合中所有score介于max和min之间的成员(闭区间), 从大到小, 不包括score
    bool zrevrangebyscore(redisContext* redis, const std::string& key, int max, int min, std::vector<std::string>& outMembers);
    // 返回有序集合中所有score介于max和min之间的成员(闭区间), 从大到小, 包括score
    bool zrevrangebyscore(redisContext* redis, const std::string& key, int max, int min, std::vector<std::string>& outMembers, std::vector<int>& outScores);


    // 返回有序集合 key 中 member 的排名, 排名从0开始
    bool zrank(redisContext* redis, const std::string& key, const std::string& member, uint& ranking);      // 从大到小
    bool zrevrank(redisContext* redis, const std::string& key, const std::string& member, uint& ranking);   // 从小到大

    // 移除有序集合 key 中的成员
    bool zrem(redisContext* redis, const std::string& key, std::vector<std::string>& members, uint& removeSize);
    bool zrem(redisContext* redis, const std::string& key, const std::string& member);

    // 移除有序集合 key 中, 指定排名区间内的所有成员(闭区间)
    bool zremrangebyrank(redisContext* redis, const std::string& key, int start, int stop, uint& removeSize);

    // 移除有序集合中, 所有score介于min和max之间的成员(闭区间)
    bool zremrangebyscore(redisContext* redis, const std::string& key, int min, int max, uint& removeSize);

    // 返回有序集合中, 成员 member 的 score 值
    bool zscore(redisContext* redis, const std::string& key, const std::string& member, int& score);


    // -- transaction

private:

    bool replyStateIsOK(redisReply* reply);

    // if reply is nulptr or state is error, return false
    bool replyErrOrNullCheck(redisReply* reply);

    bool _mset(redisContext* redis, const std::string& value);
    bool _msetnx(redisContext* redis, const std::string& value);

    // bool _lpush(redisContext* redis, const std::string& key, const std::string& values);
    // bool _lpushx(redisContext* redis, const std::string& key, const std::string& values);
    // bool _rpush(redisContext* redis, const std::string& key, const std::string& values);
    // bool _rpushx(redisContext* redis, const std::string& key, const std::string& values);

private:

    ConnectsList    m_connectsList;
};

#endif//_NOSQL_H_
