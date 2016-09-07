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
    REDIS_COMMAND_SET_EX,               // ��ʱʱ��, second 
    REDIS_COMMAND_SET_PX,               // ��ʱʱ��, millisecond

    REDIS_COMMAND_SET_NX,               // ֻ�ڼ�������ʱ, �ŶԼ��������ò���
    REDIS_COMMAND_SET_XX,               // ֻ�ڼ��Ѿ�����ʱ, �ŶԼ��������ò���

    // mset
    REDIS_COMMAND_MSET_NX,              // ���ö��key-value��, ���ҽ�������key��������. ��ʹֻ��һ������key�Ѵ��ڣ�Ҳ��ܾ�ִ�����и���key�����ò���
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
        ���ַ���value������key

        opReplace = REDIS_COMMAND_SET_NX �� REDIS_COMMAND_SET_XX �� REDIS_COMMAND_OPTION_NULL
        opTime = REDIS_COMMAND_SET_EX �� REDIS_COMMAND_SET_PX �� REDIS_COMMAND_OPTION_NULL
        timeValue �� opTime != REDIS_COMMAND_OPTION_NULL ʱ��ʾ���ݳ�ʱʱ��
    */
    bool set(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opReplace = REDIS_COMMAND_OPTION_NULL, 
        RedisOptionTypes opTime = REDIS_COMMAND_OPTION_NULL, uint timeValue = 0);

    /*
        ������key-value

        opReplace = REDIS_COMMAND_OPTION_NULL �� REDIS_COMMAND_MSET_NX
            
            REDIS_COMMAND_OPTION_NULL   ��ʾ���ĳ������key�Ѿ�����, ��ô������ֵ����ԭ���ľ�ֵ
            REDIS_COMMAND_MSET_NX       ��ʾͬʱ���ö��key-value��, ���ҽ������и���key��������. ��ʹֻ��һ������key�Ѵ���, Ҳ��ܾ�ִ�����и���key�����ò���
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
