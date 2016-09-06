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
    bool stringSet(redisContext* client, const std::string& key, const std::string& value);

    // -- list
    // -- hash
    // -- set
    // -- sortset
    // -- transaction

private:
    ConnectsList    m_connectsList;
};

#endif//_NOSQL_H_
