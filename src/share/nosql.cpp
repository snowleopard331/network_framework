/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/09/06
*/

#include "nosql.h"

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