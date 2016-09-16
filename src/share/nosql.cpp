/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/09/06
*/

#include "nosql.h"
#include <memory>

#define SPACE   " "

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

    // construct command
    std::string cmd = "AUTH ";
    cmd.append(password);

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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
    if (redis == nullptr || key.empty() || value.empty())
    {
        return false;
    }

    if (opTime != REDIS_COMMAND_OPTION_NULL && 0 == timeValue)
    {
        return false;
    }

    // construct command
    std::string cmd = "SET ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(value);
    cmd.append(SPACE);

    // option
    if (opTime == REDIS_COMMAND_OPTION_SET_EX)
    {
        cmd.append("EX ");
        cmd.append(numToStr(timeValue));
        cmd.append(SPACE);
    }
    else if(opTime == REDIS_COMMAND_OPTION_SET_PX)
    {
        cmd.append("PX ");
        cmd.append(numToStr(timeValue));
        cmd.append(SPACE);
    }

    if (opReplace == REDIS_COMMAND_OPTION_SET_NX)
    {
        cmd.append("NX");
    }
    else if (opReplace == REDIS_COMMAND_OPTION_SET_XX)
    {
        cmd.append("XX");
    }

    // execute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    return replyStateIsOK(reply);
}

bool RedisManager::mset(redisContext* redis, std::map< std::string, std::string >& value, RedisOptionTypes opReplace /* = REDIS_COMMAND_OPTION_NULL */)
{
    // param check
    if(redis == nullptr || value.empty())
    {
        return false;
    }

    if(opReplace != REDIS_COMMAND_OPTION_NULL &&
        opReplace != REDIS_COMMAND_OPTION_MSET_NX)
    {
        return false;
    }

    // construct command option
    std::string valueList;    
    for (std::map< std::string, std::string >::iterator iter = value.begin(); iter != value.end(); ++iter)
    {
        if(iter->first.empty() || iter->second.empty())
        {
            return false;
        }

        valueList.append(iter->first);  // key
        valueList.append(SPACE);
        valueList.append(iter->second); // value
        valueList.append(SPACE);
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

    // construct command
    std::string cmd = "MSET ";
    cmd.append(value);

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "MSETNX ";
    cmd.append(value);

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "MGET ";
    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);  // key
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    outMapInfo.clear();

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

bool RedisManager::getValueLen(redisContext* redis, const std::string& key, uint& len)
{
    // param check
    if(redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command
    std::string cmd = "STRLEN ";
    cmd.append(key);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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
    
    // construct command
    std::string cmd = "GET ";
    cmd.append(key);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "LLEN ";
    cmd.append(key);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd;
    if (opCreateList == REDIS_COMMAND_OPTION_NULL)
    {
        cmd.append("LPUSH ");
    }
    else if(opCreateList == REDIS_COMMAND_OPTION_LPUSHX)
    {
        cmd.append("LPUSHX ");
    }
    else
    {
        return false;
    }

    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector< std::string >::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);  // value
        cmd.append(SPACE);
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd;
    if (opCreateList == REDIS_COMMAND_OPTION_NULL)
    {
        cmd.append("RPUSH ");
    }
    else if (opCreateList == REDIS_COMMAND_OPTION_RPUSHX)
    {
        cmd.append("RPUSHX ");
    }
    else
    {
        return false;
    }

    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector< std::string >::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);  // value
        cmd.append(SPACE);
    }

    // execute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

bool RedisManager::rpush(redisContext* redis, const std::string& key, const std::string& value, RedisOptionTypes opCreateList /* = REDIS_COMMAND_OPTION_NULL */)
{
    std::vector<std::string> values;
    values.push_back(value);

    return rpush(redis, key, values, opCreateList);
}

bool RedisManager::lpop(redisContext* redis, const std::string& key)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command
    std::string cmd = "LPOP ";
    cmd.append(key);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "RPOP ";
    cmd.append(key);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "LREM ";

    std::string strCount;
    if (!numToStr<int>(count, strCount))
    {
        return false;
    }

    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(strCount);
    cmd.append(SPACE);
    cmd.append(value);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HDEL ";
    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector< std::string >::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);  // field
        cmd.append(SPACE);
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HEXISTS ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(field);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HGET ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(field);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HGETALL ";
    cmd.append(key);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HVALS ";
    cmd.append(key);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HKEYS ";
    cmd.append(key);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HLEN ";
    cmd.append(key);

    // excute
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HMGET ";
    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector<std::string>::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);  // field
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HMSET ";
    cmd.append(key);
    cmd.append(SPACE);
    for (std::map<std::string, std::string>::iterator iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->first.empty() || iter->second.empty())
        {
            return false;
        }

        cmd.append(iter->first);  // field
        cmd.append(SPACE);
        cmd.append(iter->second); // value
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "HSET ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(field);
    cmd.append(SPACE);
    cmd.append(value);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SADD ";
    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector<std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);  // member
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SCARD ";
    cmd.append(key);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SDIFF ";
    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector<std::string>::iterator iter = otherKeys.begin(); iter != otherKeys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);  // member
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SINTER ";
    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);          // key
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SISMEMBER ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(member);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SMEMBERS ";
    cmd.append(key);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SREM ";
    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector<std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);          // member
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "SUNION ";
    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);          // key
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZADD ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string score;
    for (std::map<int, std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->second.empty())
        {
            return false;
        }

        if (!numToStr<int>(iter->first, score))
        {
            return false;
        }

        cmd.append(score);          // score
        cmd.append(SPACE);
        cmd.append(iter->second);   // member
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZCARD ";
    cmd.append(key);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZCOUNT ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string min, max;
    if (!numToStr<int>(scoreMin, min) || !numToStr<int>(scoreMax, max))
    {
        return false;
    }

    cmd.append(min);
    cmd.append(SPACE);
    cmd.append(max);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZRANGE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strStart, strStop;
    if (!numToStr(start, strStart) || !numToStr(stop, strStop))
    {
        return false;
    }

    cmd.append(strStart);
    cmd.append(SPACE);
    cmd.append(strStop);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZRANGE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strStart, strStop;
    if (!numToStr(start, strStart) || !numToStr(stop, strStop))
    {
        return false;
    }

    cmd.append(strStart);
    cmd.append(SPACE);
    cmd.append(strStop);
    cmd.append(SPACE);
    cmd.append("WITHSCORES");

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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
                if (!strToNum(strEle, score))
                {
                    return false;
                }

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

    // construct command
    std::string cmd = "ZRANGEBYSCORE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strMin, strMax;
    if(!numToStr(min, strMin) || !numToStr(max, strMax))
    {
        return false;
    }

    cmd.append(strMin);
    cmd.append(SPACE);
    cmd.append(strMax);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZRANGEBYSCORE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strMin, strMax;
    if (!numToStr(min, strMin) || !numToStr(max, strMax))
    {
        return false;
    }

    cmd.append(strMin);
    cmd.append(SPACE);
    cmd.append(strMax);
    cmd.append(SPACE);
    cmd.append("WITHSCORES");

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREVRANGEBYSCORE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strMin, strMax;
    if (!numToStr(min, strMin) || !numToStr(max, strMax))
    {
        return false;
    }

    cmd.append(strMax);
    cmd.append(SPACE);
    cmd.append(strMin);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREVRANGEBYSCORE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strMin, strMax;
    if (!numToStr(min, strMin) || !numToStr(max, strMax))
    {
        return false;
    }

    cmd.append(strMax);
    cmd.append(SPACE);
    cmd.append(strMin);
    cmd.append(SPACE);
    cmd.append("WITHSCORES");

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZRANK ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(member);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREM ";
    cmd.append(key);
    cmd.append(SPACE);
    for (std::vector<std::string>::iterator iter = members.begin(); iter != members.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);      // member
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREMRANGEBYRANK ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strStart, strStop;
    if(!numToStr(start, strStart) || !numToStr(stop, strStop))
    {
        return false;
    }

    cmd.append(strStart);
    cmd.append(SPACE);
    cmd.append(strStop);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREMRANGEBYSCORE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strMin, strMax;
    if (!numToStr(min, strMin) || !numToStr(max, strMax))
    {
        return false;
    }

    cmd.append(strMin);
    cmd.append(SPACE);
    cmd.append(strMax);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREVRANGE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strStart, strStop;
    if (!numToStr(start, strStart) || !numToStr(stop, strStop))
    {
        return false;
    }

    cmd.append(strStart);
    cmd.append(SPACE);
    cmd.append(strStop);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREVRANGE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strStart, strStop;
    if (!numToStr(start, strStart) || !numToStr(stop, strStop))
    {
        return false;
    }

    cmd.append(strStart);
    cmd.append(SPACE);
    cmd.append(strStop);
    cmd.append(SPACE);
    cmd.append("WITHSCORES");

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZREVRANK ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(member);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZSCORE ";
    cmd.append(key);
    cmd.append(SPACE);
    cmd.append(member);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

    // construct command
    std::string cmd = "ZSCORE ";
    cmd.append(key);
    cmd.append(SPACE);

    std::string strInc;
    if(!numToStr(inc, strInc))
    {
        return false;
    }

    cmd.append(strInc);
    cmd.append(SPACE);
    cmd.append(member);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
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

bool RedisManager::ping(redisContext* redis)
{
    // param check
    if(redis == nullptr)
    {
        return false;
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "PING"));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_STATUS)
    {
        return (strcasecmp(reply->str, "pong") == 0);
    }

    return false;
}

bool RedisManager::ttl(redisContext* redis, const std::string& key, int& outValue)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command
    std::string cmd = "TTL ";
    cmd.append(key);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        outValue = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::expire(redisContext* redis, const std::string& key, uint secs)
{
    // param check
    if (redis == nullptr || key.empty())
    {
        return false;
    }

    // construct command
    std::string cmd = "EXPIRE ";
    
    std::string strSecs;
    if(!numToStr<uint>(secs, strSecs))
    {
        return false;
    }

    cmd.append(strSecs);

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER && 
        1 == reply->integer)
    {
        return true;
    }

    return false;
}

bool RedisManager::del(redisContext* redis, std::vector<std::string>& keys, uint& delSize)
{
    // param check
    if (redis == nullptr || keys.empty())
    {
        return false;
    }

    // construct command
    std::string cmd = "DEL ";
    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        if (iter->empty())
        {
            return false;
        }

        cmd.append(*iter);              // key
        cmd.append(SPACE);
    }

    // excute command
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, cmd.c_str()));
    if (!replyErrOrNullCheck(reply))
    {
        return false;
    }

    // auto free reply
    std::unique_ptr<redisReply, decltype(freeReplyObject)*> p(reply, freeReplyObject);

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        delSize = reply->integer;
        return true;
    }

    return false;
}

bool RedisManager::del(redisContext* redis, const std::string& key)
{
    std::vector<std::string> keyList;
    keyList.push_back(key);
    uint delSize = 0;
    return del(redis, keyList, delSize);
}