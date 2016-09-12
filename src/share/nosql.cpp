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

bool RedisManager::createConnect(const std::string ip, const ushort port, uint timeoutSecs /* = 0 */)
{
    redisContext* redisClient = nullptr;

    if (timeoutSecs)
    {
        timeval tv;
        tv.tv_sec = timeoutSecs;

        redisClient = redisConnectWithTimeout(ip.c_str(), port, tv);
    }
    else
    {
        redisClient = redisConnect(ip.c_str(), port);
    }

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

bool RedisManager::authPassword(redisContext* redis, const std::string password)
{
    if (redis == nullptr || password.empty())
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "AUTH %s", password.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    freeReplyObject(reply);

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

        if (opTime == REDIS_COMMAND_OPTION_SET_EX)
        {
            setOption.append(" EX ");
        }
        else if (opTime == REDIS_COMMAND_OPTION_SET_PX)
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
        if (opReplace == REDIS_COMMAND_OPTION_SET_NX)
        {
            setOption.append(" NX");
        }
        else if ( opReplace == REDIS_COMMAND_OPTION_SET_XX)
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
        opReplace != REDIS_COMMAND_OPTION_MSET_NX)
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

bool RedisManager::_mset(redisContext* redis, const std::string& value)
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

bool RedisManager::_msetnx(redisContext* redis, const std::string& value)
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

    // excute command
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
    for (size_t i = 0; i < reply->elements; i++)
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

    // excute command
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
    
    // excute command
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

bool RedisManager::llen(redisContext* redis, const std::string& key, uint& len)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "LLEN %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    // output data, empty list will return 0;
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        len = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::lpush(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opCreateList /* = REDIS_COMMAND_OPTION_NULL */)
{
    std::vector<std::string> values;
    values.push_back(value);

    return lpush(redis, key, values, opCreateList);
}

bool RedisManager::lpush(redisContext* redis, const std::string& key, std::vector<std::string>& values, RedisOptionTypes opCreateList /* = REDIS_COMMAND_OPTION_NULL */)
{
    // param check
    if (key.empty() || values.empty())
    {
        return false;
    }

    if (opCreateList != REDIS_COMMAND_OPTION_NULL ||
        opCreateList != REDIS_COMMAND_OPTION_LPUSHX)
    {
        return false;
    }

    // construct valueList
    std::string valueList;
    for (std::vector< std::string >::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        valueList.append(*iter);  // value
        valueList.append(" ");
    }

    return ((opCreateList == REDIS_COMMAND_OPTION_NULL) ? _lpush(redis, key, valueList) : _lpushx(redis, key, valueList));
}

bool RedisManager::_lpush(redisContext* redis, const std::string& key, const std::string& values)
{
    // param check
    if(redis == nullptr || values.empty())
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "LPUSH %s %s", key.c_str(), values.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);
    
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        return true;
    }

    return false;
}

bool RedisManager::_lpushx(redisContext* redis, const std::string& key, const std::string& values)
{
    // param check
    if (redis == nullptr || values.empty())
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "LPUSHX %s %s", key.c_str(), values.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        return true;
    }

    return false;
}

bool RedisManager::rpush(redisContext* redis, const std::string& key, std::vector<std::string>& values, RedisOptionTypes opCreateList /* = REDIS_COMMAND_OPTION_NULL */)
{
    // param check
    if (key.empty() || values.empty())
    {
        return false;
    }

    if (opCreateList != REDIS_COMMAND_OPTION_NULL ||
        opCreateList != REDIS_COMMAND_OPTION_RPUSHX)
    {
        return false;
    }

    // construct valueList
    std::string valueList;
    for (std::vector< std::string >::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        valueList.append(*iter);  // value
        valueList.append(" ");
    }

    return ((opCreateList == REDIS_COMMAND_OPTION_NULL) ? _rpush(redis, key, valueList) : _rpush(redis, key, valueList));
}

bool RedisManager::rpush(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opCreateList /* = REDIS_COMMAND_OPTION_NULL */)
{
    std::vector<std::string> values;
    values.push_back(value);

    return rpush(redis, key, values, opCreateList);
}

bool RedisManager::_rpush(redisContext* redis, const std::string& key, const std::string& values)
{
    // param check
    if (redis == nullptr || values.empty())
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "RPUSH %s %s", key.c_str(), values.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        return true;
    }

    return false;
}

bool RedisManager::_rpushx(redisContext* redis, const std::string& key, const std::string& values)
{
    // param check
    if (redis == nullptr || values.empty())
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "RPUSHX %s %s", key.c_str(), values.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        return true;
    }

    return false;
}

bool RedisManager::lpop(redisContext* redis, const std::string& key)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "LPOP %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    // return head ele is successful
    if (reply->type == REDIS_REPLY_STRING)
    {
        return true;
    }

    return false;
}

bool RedisManager::rpop(redisContext* redis, const std::string& key)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "RPOP %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    // return tail ele is successful
    if (reply->type == REDIS_REPLY_STRING)
    {
        return true;
    }

    return false;
}

bool RedisManager::lrem(redisContext* redis, const std::string& key, const std::string& value, uint& removeSize, int count /* = 0 */)
{
    // param check
    if (redis == nullptr || key.empty() || value.empty())
    {
        return false;
    }

    std::string strCount;
    std::stringstream ss;
    ss << count;
    ss >> strCount;

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "LREM %s %s %s", key.c_str(), strCount.c_str(), value.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        removeSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::hdel(redisContext* redis, const std::string& key, std::vector<std::string>& fields, uint& delSize)
{
    // param check
    if (key.empty() || key.empty() || fields.empty())
    {
        return false;
    }

    // construct fieldList
    std::string fieldList;
    for (std::vector< std::string >::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        fieldList.append(*iter);  // field
        fieldList.append(" ");
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HDEL %s %s", key.c_str(), fieldList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        delSize = reply->integer;   // 被成功移除的域的数量
        return true;
    }

    return false;
}

bool RedisManager::hdel(redisContext* redis, const std::string& key, std::string& field, uint& delSize)
{
    std::vector<std::string> fields;
    fields.push_back(field);
    
    return hdel(redis, key, fields, delSize);
}

bool RedisManager::hexists(redisContext* redis, const std::string& key, const std::string& field, bool& isExit)
{
    // param check
    if (redis == nullptr || key.empty() || field.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HEXISTS %s %s", key.c_str(), field.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);
    

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        isExit = ((reply->integer == 0) ? false : true);
        return true;
    }

    return false;
}

bool RedisManager::hget(redisContext* redis, const std::string& key, const std::string& field, std::string& outValue)
{
    // param check
    if (redis == nullptr || key.empty() || field.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HGET %s %s", key.c_str(), field.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);


    if (reply->type == REDIS_REPLY_STRING)
    {
        outValue = reply->str;
        return true;
    }

    return false;
}

bool RedisManager::hgetall(redisContext* redis, const std::string& key, std::map<std::string, std::string>& outMapInfo)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HGETALL %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);


    if (reply->type == REDIS_REPLY_ARRAY && 0 == (reply->elements % 2/*元素个数必须是双数*/))
    {
        outMapInfo.clear();
        
        for (size_t index = 0; index < reply->elements; index++)
        {
            redisReply* eleReply = reply->element[index];

            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            if (index % 2 == 1)
            {
                outMapInfo[reply->element[index - 1]->str] = eleReply->str;
            }
        }

        return true;
    }

    return false;
}

bool RedisManager::hvals(redisContext* redis, const std::string& key, std::vector<std::string>& outValues)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HVALS %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);


    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outValues.clear();

        for (size_t index = 0; index < reply->elements; index++)
        {
            redisReply* eleReply = reply->element[index];

            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            std::string value(eleReply->str, eleReply->len);
            outValues.push_back(value);
        }

        return true;
    }

    return false;
}

bool RedisManager::hkeys(redisContext* redis, const std::string& key, std::vector<std::string>& outFields)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HKEYS %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);


    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outFields.clear();

        for (size_t index = 0; index < reply->elements; index++)
        {
            redisReply* eleReply = reply->element[index];

            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            std::string field(eleReply->str, eleReply->len);
            outFields.push_back(field);
        }

        return true;
    }

    return false;
}

bool RedisManager::hlen(redisContext* redis, const std::string& key, uint& fieldSize)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HLEN %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);


    if (reply->type == REDIS_REPLY_INTEGER)
    {
        fieldSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::hmget(redisContext* redis, const std::string& key, std::vector<std::string>& fields, std::map<std::string, std::string>& outMapInfo)
{
    // param check
    if (redis == nullptr || key.empty() || fields.empty())
    {
        return false;
    }

    // construct fields
    std::string fieldList;
    for (std::vector<std::string>::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        fieldList.append(*iter);
        fieldList.append(" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HMGET %s %s", key.c_str(), fieldList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    // check reply
    if (reply->type != REDIS_REPLY_ARRAY ||
        reply->elements != fields.size())
    {
        return false;
    }

    outMapInfo.clear();

    // output data key-value
    for (size_t i = 0; i < reply->elements; i++)
    {
        redisReply* eleReply = reply->element[i];

        if (eleReply->type == REDIS_REPLY_STRING)
        {
            outMapInfo[fields[i]] = eleReply->str;
        }
        else if (eleReply->type == REDIS_REPLY_NIL)
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

bool RedisManager::hmset(redisContext* redis, const std::string& key, std::map<std::string, std::string>& values)
{
    // param check
    if (redis == nullptr || key.empty() || values.empty())
    {
        return false;
    }

    // construct fields
    std::string fieldValueList;
    for (std::map<std::string, std::string>::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->first.empty() || iter->second.empty())
        {
            return false;
        }

        fieldValueList.append(iter->first);  // field
        fieldValueList.append(" ");
        // fieldValueList.append("\"");
        fieldValueList.append(iter->second); // value
        // fieldValueList.append("\" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HMSET %s %s", key.c_str(), fieldValueList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    return replyStateIsOK(reply);
}

bool RedisManager::hset(redisContext* redis, const std::string& key, const std::string& field, const std::string& value, uint& ret)
{
    // param check
    if (redis == nullptr || key.empty() || field.empty() || value.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        ret = reply->integer;

        return true;
    }

    return false;
}

bool RedisManager::sadd(redisContext* redis, const std::string& key, std::vector<std::string>& members)
{
    // param check
    if (redis == nullptr || key.empty() || members.empty())
    {
        return false;
    }

    // construct command params
    std::string memberList;
    for (std::vector<std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        memberList.append("\"");
        memberList.append(*iter); // member
        memberList.append("\" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SADD %s %s", key.c_str(), memberList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        return true;
    }

    return false;
}

bool RedisManager::scard(redisContext* redis, const std::string& key, uint& memSize)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SCARD %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        memSize = reply->integer;

        return true;
    }

    return false;
}

bool RedisManager::sdiff(redisContext* redis, const std::string& key, std::vector<std::string>& otherKeys, std::vector<std::string>& outInfo)
{
    // param check
    if (redis == nullptr || key.empty() || otherKeys.empty())
    {
        return false;
    }

    // construct command params
    std::string otherKeyList;
    for (std::vector<std::string>::iterator iter = otherKeys.begin(); iter != otherKeys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        //otherKeyList.append("\"");
        otherKeyList.append(*iter); // member
        //otherKeyList.append("\"");
        otherKeyList.append(" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SDIFF %s %s", key.c_str(), otherKeyList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outInfo.clear();

        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply* eleReply = reply->element[i];
            if(eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outInfo.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::sinter(redisContext* redis, std::vector<std::string>& keys, std::vector<std::string>& outInfo)
{
    // param check
    if (redis == nullptr || keys.empty())
    {
        return false;
    }

    // construct command params
    std::string keyList;
    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        //otherKeyList.append("\"");
        keyList.append(*iter);      // member
        //otherKeyList.append("\"");
        keyList.append(" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SINTER %s", keyList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outInfo.clear();

        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply* eleReply = reply->element[i];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outInfo.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::sismember(redisContext* redis, const std::string& key, const std::string& member, bool& isMember)
{
    // param check
    if (redis == nullptr || key.empty() || member.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SISMEMBER %s %s", key.c_str(), member.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        isMember = ((reply->integer == 0) ? false : true);

        return true;
    }

    return false;
}

bool RedisManager::smembers(redisContext* redis, const std::string& key, std::vector<std::string>& outMembers)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SMEMBERS %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outMembers.clear();

        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply* eleReply = reply->element[i];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outMembers.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::srem(redisContext* redis, const std::string& key, const std::string& member)
{
    std::vector<std::string> memberList;
    memberList.push_back(member);

    uint removeSize = 0;
    return srem(redis, key, memberList, removeSize);
}

bool RedisManager::srem(redisContext* redis, const std::string& key, std::vector<std::string>& members, uint& removeSize)
{
    // param check
    if (redis == nullptr || key.empty() || members.empty())
    {
        return false;
    }

    // construct command params
    std::string memberList;
    for (std::vector<std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        // memberList.append("\"");
        memberList.append(*iter);      // member
        // memberList.append("\"");
        memberList.append(" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SREM %s %s", key.c_str(), memberList));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        removeSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::sunion(redisContext* redis, std::vector<std::string>& keys, std::vector<std::string>& outInfo)
{
    // param check
    if (redis == nullptr || keys.empty())
    {
        return false;
    }

    // construct command params
    std::string keyList;
    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        // keyList.append("\"");
        keyList.append(*iter);      // member
        // keyList.append("\"");
        keyList.append(" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SUNION %s", keyList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outInfo.clear();

        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply* eleReply = reply->element[i];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outInfo.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::zadd(redisContext* redis, const std::string& key, std::map<int, std::string>& members, uint& addSize)
{
    // param check
    if (redis == nullptr || key.empty() || members.empty())
    {
        return false;
    }

    // construct command params
    std::string memberList, score;
    std::stringstream   ss;
    for (std::map<int, std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->second.empty())
        {
            return false;
        }

        // int to string
        ss.clear();
        ss << iter->first;
        score.clear();
        ss >> score;

        memberList.append(score);           // score
        memberList.append(" ");
        memberList.append(iter->second);    // member
        memberList.append(" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZADD %s %s", key.c_str(), memberList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        addSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zadd(redisContext* redis, const std::string& key, int score, const std::string& member)
{
    std::map<int, std::string> memberlist;
    memberlist[score] = member;
    uint addSize = 0;
    return zadd(redis, key, memberlist, addSize);
}

bool RedisManager::zcard(redisContext* redis, const std::string& key, uint& memberSize)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZCARD %s", key.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        memberSize = reply->integer;
        return true;
    }

    return false;
}

// 没有判断 min 与 max 的大小关系
bool RedisManager::zcount(redisContext* redis, const std::string& key, int scoreMin, int scoreMax, uint& size)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string min, max;
    std::stringstream ss;
    ss << scoreMin;
    ss >> min;
    ss << scoreMax;
    ss >> max;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZCOUNT %s %s %s", key.c_str(), min.c_str(), max.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        size = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strStart, strStop;
    std::stringstream ss;
    ss << start;
    ss >> strStart;
    ss << stop;
    ss >> strStop;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZRANGE %s %s %s", key.c_str(), strStart.c_str(), strStop.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outMembers.clear();

        for (size_t i = 0; i < reply->elements; ++i)
        {
            redisReply* eleReply = reply->element[i];
            if(eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outMembers.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::zrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers, std::vector<int>& outScores)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strStart, strStop;
    std::stringstream ss;
    ss << start;
    ss >> strStart;
    ss << stop;
    ss >> strStop;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZRANGE %s %s %s WITHSCORES", key.c_str(), strStart.c_str(), strStop.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY &&
        0 == (reply->elements % 2))
    {
        outMembers.clear();
        std::stringstream ss;

        for (size_t index = 0; index < reply->elements; ++index)
        {
            redisReply* eleReply = reply->element[index];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            std::string strEle(eleReply->str, eleReply->len);
            if (0 == (index % 2))   // member
            {
                // 两次拷贝, 效率太低
                outMembers.push_back(strEle);
            }
            else    // score
            {
                int score = 0;
                ss << strEle;
                ss >> score;

                outScores.push_back(score);
            }
        }

        return true;
    }

    return false;
}

bool RedisManager::zrangebyscore(redisContext* redis, const std::string& key, int min, int max, std::vector<std::string>& outMembers)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strMin, strMax;
    std::stringstream ss;
    ss << min;
    ss >> strMin;
    ss << max;
    ss >> strMax;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZRANGEBYSCORE %s %s %s", key.c_str(), strMin.c_str(), strMax.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outMembers.clear();

        for (size_t i = 0; i < reply->elements; ++i)
        {
            redisReply* eleReply = reply->element[i];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outMembers.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::zrangebyscore(redisContext* redis, const std::string& key, int min, int max, std::vector<std::string>& outMembers, std::vector<int>& outScores)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strMin, strMax;
    std::stringstream ss;
    ss << min;
    ss >> strMin;
    ss << max;
    ss >> strMax;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZRANGEBYSCORE %s %s %s WITHSCORES", key.c_str(), strMin.c_str(), strMax.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY &&
        0 == (reply->elements % 2))
    {
        outMembers.clear();
        std::stringstream ss;

        for (size_t index = 0; index < reply->elements; ++index)
        {
            redisReply* eleReply = reply->element[index];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            std::string strEle(eleReply->str, eleReply->len);
            if (0 == (index % 2))   // member
            {
                // 两次拷贝, 效率太低
                outMembers.push_back(strEle);
            }
            else    // score
            {
                int score = 0;
                ss << strEle;
                ss >> score;

                outScores.push_back(score);
            }
        }

        return true;
    }

    return false;
}

bool RedisManager::zrevrangebyscore(redisContext* redis, const std::string& key, int max, int min, std::vector<std::string>& outMembers)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strMin, strMax;
    std::stringstream ss;
    ss << min;
    ss >> strMin;
    ss << max;
    ss >> strMax;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREVRANGEBYSCORE %s %s %s", key.c_str(), strMin.c_str(), strMax.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outMembers.clear();

        for (size_t i = 0; i < reply->elements; ++i)
        {
            redisReply* eleReply = reply->element[i];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outMembers.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::zrevrangebyscore(redisContext* redis, const std::string& key, int max, int min, std::vector<std::string>& outMembers, std::vector<int>& outScores)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strMin, strMax;
    std::stringstream ss;
    ss << min;
    ss >> strMin;
    ss << max;
    ss >> strMax;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREVRANGEBYSCORE %s %s %s WITHSCORES", key.c_str(), strMin.c_str(), strMax.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY &&
        0 == (reply->elements % 2))
    {
        outMembers.clear();
        std::stringstream ss;

        for (size_t index = 0; index < reply->elements; ++index)
        {
            redisReply* eleReply = reply->element[index];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            std::string strEle(eleReply->str, eleReply->len);
            if (0 == (index % 2))   // member
            {
                // 两次拷贝, 效率太低
                outMembers.push_back(strEle);
            }
            else    // score
            {
                int score = 0;
                ss << strEle;
                ss >> score;

                outScores.push_back(score);
            }
        }

        return true;
    }

    return false;
}

bool RedisManager::zrank(redisContext* redis, const std::string& key, const std::string& member, uint& ranking)
{
    // param check
    if (redis == nullptr || key.empty() || member.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZRANK %s %s", key.c_str(), member.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        ranking = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zrem(redisContext* redis, const std::string& key, std::vector<std::string>& members, uint& removeSize)
{
    // param check
    if (redis == nullptr || key.empty() || members.empty())
    {
        return false;
    }

    // construct command params
    std::string memberList;
    for (std::vector<std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        memberList.append(*iter);           // member
        memberList.append(" ");
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREM %s %s", key.c_str(), memberList.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        removeSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zrem(redisContext* redis, const std::string& key, const std::string& member)
{
    std::vector<std::string> memberList;
    memberList.push_back(member);
    uint removeSize = 0;
    return zrem(redis, key, memberList, removeSize);
}

bool RedisManager::zremrangebyrank(redisContext* redis, const std::string& key, int start, int stop, uint& removeSize)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strStart, strStop;
    std::stringstream ss;
    ss << start;
    ss >> strStart;
    ss << stop;
    ss >> strStop;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREMRANGEBYRANK %s %s %s", key.c_str(), strStart.c_str(), strStop.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        removeSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zremrangebyscore(redisContext* redis, const std::string& key, int min, int max, uint& removeSize)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strMin, strMax;
    std::stringstream ss;
    ss << min;
    ss >> strMin;
    ss << max;
    ss >> strMax;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREMRANGEBYSCORE %s %s %s", key.c_str(), strMin.c_str(), strMax.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        removeSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zrevrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strStart, strStop;
    std::stringstream ss;
    ss << start;
    ss >> strStart;
    ss << stop;
    ss >> strStop;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREVRANGE %s %s %s", key.c_str(), strStart.c_str(), strStop.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        outMembers.clear();

        for (size_t i = 0; i < reply->elements; ++i)
        {
            redisReply* eleReply = reply->element[i];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            // 两次拷贝, 效率太低
            std::string member(eleReply->str, eleReply->len);
            outMembers.push_back(member);
        }

        return true;
    }

    return false;
}

bool RedisManager::zrevrange(redisContext* redis, const std::string& key, int start, int stop, std::vector<std::string>& outMembers, std::vector<int>& outScores)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command params
    std::string strStart, strStop;
    std::stringstream ss;
    ss << start;
    ss >> strStart;
    ss << stop;
    ss >> strStop;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREVRANGE %s %s %s WITHSCORES", key.c_str(), strStart.c_str(), strStop.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_ARRAY &&
        0 == (reply->elements % 2))
    {
        outMembers.clear();
        std::stringstream ss;

        for (size_t index = 0; index < reply->elements; ++index)
        {
            redisReply* eleReply = reply->element[index];
            if (eleReply->type != REDIS_REPLY_STRING)
            {
                return false;
            }

            std::string strEle(eleReply->str, eleReply->len);
            if (0 == (index % 2))   // member
            {
                // 两次拷贝, 效率太低
                outMembers.push_back(strEle);
            }
            else    // score
            {
                int score = 0;
                ss << strEle;
                ss >> score;

                outScores.push_back(score);
            }
        }

        return true;
    }

    return false;
}

bool RedisManager::zrevrank(redisContext* redis, const std::string& key, const std::string& member, uint& ranking)
{
    // param check
    if (redis == nullptr || key.empty() || member.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZREVRANK %s %s", key.c_str(), member.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        ranking = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zscore(redisContext* redis, const std::string& key, const std::string& member, int& score)
{
    // param check
    if (redis == nullptr || key.empty() || member.empty())
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZSCORE %s %s", key.c_str(), member.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        score = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::zincrby(redisContext* redis, const std::string& key, int inc, const std::string& member, int& newScore)
{
    // param check
    if (redis == nullptr || key.empty() || member.empty() || 0 == inc)
    {
        return false;
    }

    // construct command params
    std::string strInc;
    std::stringstream ss;
    ss << inc;
    ss >> strInc;

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "ZINCRBY %s %s %s", key.c_str(), strInc.c_str(), member.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        newScore = reply->integer;
        return true;
    }

    return false;
}