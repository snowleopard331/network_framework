/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/09/06
*/

#include "nosql.h"
#include <memory>

RedisManager::RedisManager()
{

}

RedisManager::~RedisManager()
{

}

bool RedisManager::createConnect(const char* ip, ushort port)
{
    redisContext* redisClient = redisConnect(ip, port);
    // two different types of errors
    if (redisClient == nullptr || redisClient->err)
    {
        if (redisClient->err)
        {
            LOG(ERROR) << redisClient->errstr;
        }
        
        redisFree(redisClient);

        LOG(ERROR) << "connect redis server failed" << SEPARATOR_COMMA
            << "ip " << ip << SEPARATOR_SPACE << "port " << port;

        return false;
    }

    LOG(INFO) << "connect redis server success" << SEPARATOR_COMMA
        << "ip " << ip << SEPARATOR_SPACE << "port " << port;

    m_connectsList[static_cast<uint>(redisClient->fd)] = redisClient;

    return true;
}

void RedisManager::closeConnect(uint fd)
{
    if (m_connectsList.empty())
    {
        LOG(WARNING) << "close redis connect fd: " << fd << "failed" << SEPARATOR_COMMA
            << "connect pool is empty";
        return;
    }

    ConnectsList::iterator iter = m_connectsList.find(fd);
    if (iter == m_connectsList.end())
    {
        LOG(WARNING) << "close redis connect fd: " << fd << "failed";
        return;
    }

    redisFree(iter->second);
    m_connectsList.erase(iter);
}

void RedisManager::closeAllConnect()
{
    for (ConnectsList::iterator iter = m_connectsList.begin(); iter != m_connectsList.end();)
    {
        ConnectsList::iterator iterTemp = iter++;
        redisFree(iterTemp->second);
        m_connectsList.erase(iterTemp);
    }
}

/* key中有%的情况需要测试 */
bool RedisManager::set(redisContext* redis, const std::string& key, const std::string& value, 
    RedisOptionTypes opReplace /* = REDIS_COMMAND_OPTION_NULL */, 
    RedisOptionTypes opTime /* = REDIS_COMMAND_OPTION_NULL */, uint timeValue /* = 0 */)
{
    // param check
    if (redis == nullptr)
    {
        return false;
    }

    if (key.empty() || value.empty())
    {
        return false;
    }

    // option
    std::string setOption;
    if (opTime != REDIS_COMMAND_OPTION_NULL)
    {
        if (0 == timeValue)
        {
            return false;
        }

        if (opTime == REDIS_COMMAND_SET_EX)
        {
            setOption.append(" EX ");
        }
        else if (opTime == REDIS_COMMAND_SET_PX)
        {
            setOption.append(" PX ");
        }
        else
        {
            return false;
        }

        std::stringstream ss;
        ss << timeValue;

        std::string strNum;
        ss >> strNum;

        setOption.append(strNum);
    }

    if (opReplace != REDIS_COMMAND_OPTION_NULL)
    {
        if (opReplace == REDIS_COMMAND_SET_NX)
        {
            setOption.append(" NX");
        }
        else if ( opReplace == REDIS_COMMAND_SET_XX)
        {
            setOption.append(" XX");
        }
        else
        {
            return false;
        }
    }

    std::string valueOp = std::string("\"") + value + std::string("\"");
    
    if (!setOption.empty())
    {
        valueOp.append(setOption);
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SET %s %s", key.c_str(), valueOp.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    return replyStateIsOK(reply);
}

bool RedisManager::mset(redisContext* redis, std::map< std::string, std::string >& value, RedisOptionTypes opReplace /* = REDIS_COMMAND_OPTION_NULL */)
{
    if(redis == nullptr || value.empty())
    {
        return false;
    }

    if(opReplace != REDIS_COMMAND_OPTION_NULL &&
        opReplace != REDIS_COMMAND_MSET_NX)
    {
        return false;
    }

    std::string valueList;
    
    for (std::map< std::string, std::string >::iterator iter = value.begin(); iter != value.end(); ++iter)
    {
        if(iter->first.empty() || iter->second.empty())
        {
            return false;
        }

        valueList.append(iter->first);  // key
        valueList.append(" ");
        valueList.append("\"");
        valueList.append(iter->second); // value
        valueList.append("\" ");
    }

    return ((opReplace == REDIS_COMMAND_OPTION_NULL) ? _mset(redis, valueList) : _msetnx(redis, valueList));
}

bool RedisManager::replyStateIsOK(redisReply* reply)
{
    if (!reply)
    {
        return false;
    }

    if (reply->type == REDIS_REPLY_STATUS &&
        strcasecmp(reply->str, "OK") == 0/* 忽略大小写比较字符串 */)
    {
        return true;
    }

    return false;
}

bool RedisManager::replyErrOrNullCheck(redisReply* reply)
{
    if (reply == nullptr)
    {
        return false;
    }

    if (reply->type == REDIS_REPLY_ERROR)
    {
        std::string strErr(reply->str, reply->len);
        LOG(ERROR) << strErr;
        return false;
    }

    return true;
}

bool RedisManager::_mset(redisContext* redis, std::string& value)
{
    if(value.empty())
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "MSETNX %s", value.c_str()));
    if(!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    return replyStateIsOK(reply);
}

bool RedisManager::_msetnx(redisContext* redis, std::string& value)
{
    if (value.empty())
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "MSET %s", value.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    return (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) ? true : false;
}

// 暂时没有对keys中的每个key的合法性做检查(例如某个key中含有空格)
bool RedisManager::mget(redisContext* redis, std::vector< std::string >& keys, std::map< std::string, std::string >& outMapInfo)
{
    // param check
    if (redis == nullptr || keys.empty())
    {
        return false;
    }
    outMapInfo.clear();

    // construct string keys
    std::string valueList;
    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        valueList.append(*iter);
        valueList.append(" ");
    }

    // excute mget
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "MGET %s", valueList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);
    
    // check reply
    if (reply->type != REDIS_REPLY_ARRAY ||
        reply->elements != keys.size())
    {
        return false;
    }

    // output data key-value
    for (int i = 0; i < reply->elements; i++)
    {
        redisReply* eleReply = reply->element[i];

        if (eleReply->type == REDIS_REPLY_STRING)
        {
            outMapInfo[keys[i]] = eleReply->str;
        }
        else if(eleReply->type == REDIS_REPLY_NIL)
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool RedisManager::getValueLen(redisContext* redis, std::string& key, uint& len)
{
    // param check
    if(redis == nullptr || key.empty())
    {
        return false;
    }

    // excute mget
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "STRLEN %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER &&
        0 != reply->integer)
    {
        len = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::get(redisContext* redis, const std::string& key, std::string& value)
{
    // param check
    if(redis == nullptr || key.empty())
    {
        return false;
    }
    
    // excute mget
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "GET %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    // output data
    if (reply->type == REDIS_REPLY_STRING)
    {
        value.clear();
        value = reply->str;
        return true;
    }
    
    return false;
}