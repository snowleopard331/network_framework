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
    REDIS_COMMAND_OPTION_SET_EX,               // ��ʱʱ��, second 
    REDIS_COMMAND_OPTION_SET_PX,               // ��ʱʱ��, millisecond

    REDIS_COMMAND_OPTION_SET_NX,               // ֻ�ڼ�������ʱ, �ŶԼ��������ò���
    REDIS_COMMAND_OPTION_SET_XX,               // ֻ�ڼ��Ѿ�����ʱ, �ŶԼ��������ò���

    // mset
    REDIS_COMMAND_OPTION_MSET_NX,              // ���ö��key-value��, ���ҽ�������key��������. ��ʹֻ��һ������key�Ѵ��ڣ�Ҳ��ܾ�ִ�����и���key�����ò���

    // lpush
    REDIS_COMMAND_OPTION_LPUSHX,               // ��key������, �� LPUSHX ��������list

    // rpush
    REDIS_COMMAND_OPTION_RPUSHX,               // ��key������, �� RPUSHX ��������list
};

class RedisManager
{
public:

    RedisManager();
    ~RedisManager();


public:

    // ��������
    bool createConnect(const std::string ip, const ushort port, uint timeoutSecs = 0);

    // ��֤����
    bool authPassword(redisContext* redis, const std::string password);

    // �ر�ָ������
    void closeConnect(uint fd);
    
    // �ر���������
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

    // ���ظ���key��ʣ������ʱ��
    bool ttl(redisContext* redis, const std::string& key, int& outValue);

    // ɾ��һ������key
    bool del(redisContext* redis, std::vector<std::string>& keys, uint& delSize);
    bool del(redisContext* redis, const std::string& key);

    // ��������ʱ��
    bool expire(redisContext* redis, const std::string& key, uint secs);


    // -- string

    /*
        ���ַ���value������key

        opReplace = REDIS_COMMAND_OPTION_SET_NX �� REDIS_COMMAND_OPTION_SET_XX �� REDIS_COMMAND_OPTION_NULL
        opTime = REDIS_COMMAND_OPTION_SET_EX �� REDIS_COMMAND_OPTION_SET_PX �� REDIS_COMMAND_OPTION_NULL
        timeValue �� opTime != REDIS_COMMAND_OPTION_NULL ʱ��ʾ���ݳ�ʱʱ��
    */
    bool set(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL, 
        RedisOptionTypes opTime = REDIS_COMMAND_OPTION_NULL, uint timeValue = 0);

    bool get(redisContext* redis, const std::string& key, std::string& value);

    /*
        ������key-value

        opReplace = REDIS_COMMAND_OPTION_NULL �� REDIS_COMMAND_OPTION_MSET_NX
            
            REDIS_COMMAND_OPTION_NULL   ��ʾ���ĳ������key�Ѿ�����, ��ô������ֵ����ԭ���ľ�ֵ
            REDIS_COMMAND_OPTION_MSET_NX       ��ʾͬʱ���ö��key-value��, ���ҽ������и���key��������. ��ʹֻ��һ������key�Ѵ���, Ҳ��ܾ�ִ�����и���key�����ò���
    */
    bool mset(redisContext* redis, std::map < std::string, std::string >& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL);

    bool mget(redisContext* redis, std::vector< std::string >& keys, std::map< std::string, std::string >& outMapInfo);

    bool getValueLen(redisContext* redis, const std::string& key, uint& len);


    // -- list

    // ��ȡlist����
    bool llen(redisContext* redis, const std::string& key, uint& len);

    // �����ͷ
    bool lpush(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);
    bool lpush(redisContext* redis, const std::string& key, std::vector<std::string>& values, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);

    // �����β
    bool rpush(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);
    bool rpush(redisContext* redis, const std::string& key, std::vector<std::string>& values, RedisOptionTypes opCreateList = REDIS_COMMAND_OPTION_NULL);

    // �Ƴ���ͷԪ��
    bool lpop(redisContext* redis, const std::string& key);

    // �Ƴ���βԪ��
    bool rpop(redisContext* redis, const std::string& key);

    // �Ƴ���value��ͬ��ֵ. countΪ�Ƴ�����, ������ʾ��ǰ�����Ƴ�count����������ʾ�Ӻ���ǰ�Ƴ�count����0��ʾ�Ƴ�����
    bool lrem(redisContext* redis, const std::string& key, const std::string& value, uint& removeSize, int count = 0);


    // -- hash

    /*
        ����ϣ�� key �е��� field ��ֵ��Ϊ value, �� key ������, �򴴽� key �Ĺ�ϣ��, �����������Ѿ�����, ��ֵ��������.
        ret ����1��ʾ field �ǹ�ϣ���һ���½������óɹ�, 0��ʾfield�Ѿ������ұ���ֵ����
    */
    bool hset(redisContext* redis, const std::string& key, const std::string& field, const std::string& value, uint& ret);

    // ��ϣ�� key �и����� field ��ֵ
    bool hget(redisContext* redis, const std::string& key, const std::string& field, std::string& outValue);

    // ͬʱ����� field-value �����õ���ϣ�� key ��, ������Ḳ�ǹ�ϣ�����Ѵ��ڵ���
    bool hmset(redisContext* redis, const std::string& key, std::map<std::string, std::string>& values);

    // ���ع�ϣ�� key �У�һ�������������ֵ
    bool hmget(redisContext* redis, const std::string& key, std::vector<std::string>& fields, std::map<std::string, std::string>& outMapInfo);

    // ���ع�ϣ�� key �����е����ֵ
    bool hgetall(redisContext* redis, const std::string& key, std::map<std::string, std::string>& outMapInfo);

    // ���ع�ϣ�� key �����е���
    bool hkeys(redisContext* redis, const std::string& key, std::vector<std::string>& outFields);

    // ���ع�ϣ�� key ���������ֵ
    bool hvals(redisContext* redis, const std::string& key, std::vector<std::string>& outValues);

    // ɾ����ϣ�� key �еĶ����
    bool hdel(redisContext* redis, const std::string& key, std::vector<std::string>& fields, uint& delSize);
    bool hdel(redisContext* redis, const std::string& key, std::string& field, uint& delSize);

    // �жϹ�ϣ�� key ��, ������ filed �Ƿ����
    bool hexists(redisContext* redis, const std::string& key, const std::string& field, bool& isExit);

    // ��ȡ��ϣ�� key ���������
    bool hlen(redisContext* redis, const std::string& key, uint& fieldSize);


    // -- set

    // �ж� member �Ƿ��Ǽ��� key �ĳ�Ա
    bool sismember(redisContext* redis, const std::string& key, const std::string& member, bool& isMember);

    // ��һ������member Ԫ�ؼ��뵽����key ���У��Ѿ������ڼ��ϵ�member Ԫ�ؽ�������
    bool sadd(redisContext* redis, const std::string& key, const std::string& member);
    bool sadd(redisContext* redis, const std::string& key, std::vector<std::string>& members);

    // ��ȡ������Ԫ�ص�����
    bool scard(redisContext* redis, const std::string& key, uint& memSize);

    // ��ȡ��� key �Ĳ(���ڵ�һ��key �Ҳ�����֮���κ�һ��key��Ԫ��)
    bool sdiff(redisContext* redis, const std::string& key, std::vector<std::string>& otherKeys, std::vector<std::string>& outInfo);

    // ��ȡ��� key �Ľ���
    bool sinter(redisContext* redis, std::vector<std::string>& keys, std::vector<std::string>& outInfo);

    // ��ȡ��� key �Ĳ���
    bool sunion(redisContext* redis, std::vector<std::string>& keys, std::vector<std::string>& outInfo);

    // ��ȡ���� key �����г�Ա
    bool smembers(redisContext* redis, const std::string& key, std::vector<std::string>& outMembers);

    // �Ƴ����� key �е�Ԫ��
    bool srem(redisContext* redis, const std::string& key, std::vector<std::string>& members, uint& removeSize);
    bool srem(redisContext* redis, const std::string& key, const std::string& member);


    // -- sortset

    // ���һ������ member Ԫ�ؼ��� score ֵ���뵽���� key ����, �� member �Ѵ�������� score ֵ, addSize���ر��ɹ���ӵ�����(���������µĺ��Ѿ����ڵ�)
    bool zadd(redisContext* redis, const std::string& key, std::map<int, std::string>& members/* score - member */, uint& addSize);
    bool zadd(redisContext* redis, const std::string& key, int score, const std::string& member);

    // �������򼯺ϵ�member����
    bool zcard(redisContext* redis, const std::string& key, uint& memberSize);

    // �������򼯺���, scoreֵ��min��max��(������)�ĳ�Ա����
    bool zcount(redisContext* redis, const std::string& key, int scoreMin, int scoreMax, uint& size);

    // Ϊ���򼯺� key �ĳ�Ա member �� score �������� inc
    bool zincrby(redisContext* redis, const std::string& key, int inc, const std::string& member, int& newScore);

    /*
        �±�0��ʾ��һ��member, 1��ʾ�ڶ���member, -1��ʾ���һ��member
        ��: start = 0, stop = -1  ��ʾ��ʾ�������򼯺ϳ�Ա
    */
    // ����ָ���������Ա, score��С��������, ������scoreֵ
    bool zrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers);
    // ����ָ���������Ա, score��С��������, ����scoreֵ
    bool zrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers, std::vector<int>& outScores);
    // ����ָ���������Ա, score�Ӵ�С����, ������scoreֵ
    bool zrevrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers);
    // ����ָ���������Ա, score�Ӵ�С����, ����scoreֵ
    bool zrevrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers, std::vector<int>& outScores);

    /* ������Ϊ�����޻��߸����������api����Ҫ, ��������� */
    // �������򼯺�������score����min��max֮��ĳ�Ա(������), ��С����, ������score
    bool zrangebyscore(redisContext* redis, const std::string& key, int min, int max, std::vector<std::string>& outMembers);
    // �������򼯺�������score����min��max֮��ĳ�Ա(������), ��С����, ����score
    bool zrangebyscore(redisContext* redis, const std::string& key, int min, int max, std::vector<std::string>& outMembers, std::vector<int>& outScores);
    // �������򼯺�������score����max��min֮��ĳ�Ա(������), �Ӵ�С, ������score
    bool zrevrangebyscore(redisContext* redis, const std::string& key, int max, int min, std::vector<std::string>& outMembers);
    // �������򼯺�������score����max��min֮��ĳ�Ա(������), �Ӵ�С, ����score
    bool zrevrangebyscore(redisContext* redis, const std::string& key, int max, int min, std::vector<std::string>& outMembers, std::vector<int>& outScores);


    // �������򼯺� key �� member ������, ������0��ʼ
    bool zrank(redisContext* redis, const std::string& key, const std::string& member, uint& ranking);      // �Ӵ�С
    bool zrevrank(redisContext* redis, const std::string& key, const std::string& member, uint& ranking);   // ��С����

    // �Ƴ����򼯺� key �еĳ�Ա
    bool zrem(redisContext* redis, const std::string& key, std::vector<std::string>& members, uint& removeSize);
    bool zrem(redisContext* redis, const std::string& key, const std::string& member);

    // �Ƴ����򼯺� key ��, ָ�����������ڵ����г�Ա(������)
    bool zremrangebyrank(redisContext* redis, const std::string& key, int start, int stop, uint& removeSize);

    // �Ƴ����򼯺���, ����score����min��max֮��ĳ�Ա(������)
    bool zremrangebyscore(redisContext* redis, const std::string& key, int min, int max, uint& removeSize);

    // �������򼯺���, ��Ա member �� score ֵ
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
