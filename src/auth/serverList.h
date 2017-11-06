/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/22
*/

#ifndef _SERVER_LIST_H_
#define _SERVER_LIST_H_

#include "Common.h"
#include "Singleton.h"

// Storage object for the list of realms on the server

struct ServerInfo
{
    std::string     address;
    uint16          port;
    uint            connections;
    uint            registTime;

    ServerInfo() : port(0), connections(0), registTime(0) {}
};

class ServerList
{
public:

    ServerList();

    ~ServerList();

public:

    typedef std::list<ServerInfo*>      ServerListType;

public:

    void add(std::string& addr, uint16 port);

    uint size() const
    {
        return m_serverList.size();
    }

    void erase(std::string& addr, uint16 port);

    void clear();

private:

    ServerListType      m_serverList;
};

#endif//_SERVER_LIST_H_